#pragma once

#include <unordered_map>
#include <algorithm>

namespace chowdsp
{
/**
 * A search database containing a series of tagged entries.
 *
 * The user can add entries to the database at any time using `addEntry()`.
 * Once the database has been filled, the user can enter "search" mode by
 * calling `prepareForSearch()`, and then call `search` as needed to retrieve search results.
 *
 * Note that `prepareForSearch()` must be called before calling `search()`,
 * after new entries are added to the database.
 *
 * \tparam Key The "key" type used by the database.
 * \tparam numFields The number of "tag" fields that each entry has.
 */
template <typename Key = std::string, size_t numFields = 1>
class SearchDatabase
{
public:
    /** The result type returned to the user. */
    struct Result
    {
        Key key;
        float score;
    };

private:
    struct WordFromField
    {
        int wordIndex = -1; // in global list
        int fieldIndex = -1;
        int wordIndexInField = -1; // store some order

        WordFromField() = default;

        WordFromField (int wIndex, int fIndex, int wIndexInField)
            : wordIndex (wIndex), fieldIndex (fIndex), wordIndexInField (wIndexInField)
        {
        }

        bool operator<(const WordFromField& other) const
        {
            if (wordIndex != other.wordIndex)
                return wordIndex < other.wordIndex;

            return fieldIndex < other.fieldIndex;
        }

        bool operator== (const WordFromField& other) const
        {
            return wordIndex == other.wordIndex && fieldIndex == other.fieldIndex;
        }
    };

    // to penalize multi-word queries where the order of hits are reversed
    struct TempResultOrderPenalty
    {
        int bestIndex = 0;
        int misses = 0;
    };

    // temporary struct to make sorting smooth
    struct TempResult
    {
        int entryIndex = 0;
        float score = 0.0f;

        TempResult() = default;

        bool operator<(const TempResult& other) const
        {
            return score > other.score; // reversed to sort high score on top
        }
    };

    // this is the entry, all words are stored in the "WordStorage", only indices here
    struct Entry
    {
        Key key;
        SmallVector<WordFromField, numFields * 5> words {}; // index into words from strings
    };

    // =======================================================
    search_database::WordStorage wordStorage {};
    std::vector<Entry> entries;
    std::array<float, numFields> fieldWeights;
    float threshold = 0.1f;

    mutable ArenaAllocator<> searchArena {};

    static void scoreEveryWord (nonstd::span<float> perWordScores, const search_database::WordStorage& ws, std::string_view queryWord) // NOLINT
    {
        const search_helpers::WordHist qHist { queryWord };
        const search_helpers::WordPairHist qHist2 { queryWord }; // calculate fancy letter-pair histogram

        // visit every word in memory-order
        for (auto [i, wordView] : enumerate (ws.wordViewList))
        {
            if (qHist.canSkip (ws.wordHist[i]))
            {
                perWordScores[i] = 0.0f;
            }
            else
            {
                const auto word = ws.getString (wordView);
                perWordScores[i] = search_helpers::scoreQueryWordToWord (qHist2, queryWord, word);
            }
        }
    }

    float scoreEntry (const Entry& e, int& bestIndex, nonstd::span<const float> perWordScores) const // NOLINT
    {
        float bestScore = 0;
        float sum = 0;

        size_t wordCount = e.words.size();

        for (size_t ei = 0; ei < wordCount; ++ei)
        {
            const WordFromField& wff = e.words[ei];
            if (wff.wordIndex < 0)
                continue;

            const float localScore = perWordScores[(uint32_t) wff.wordIndex];
            if (localScore > 0)
            {
                float weight = fieldWeights[(uint32_t) wff.fieldIndex];
                float weightedScore = localScore * weight;
                sum += weightedScore;
                if (weightedScore > bestScore)
                {
                    bestScore = weightedScore;
                    bestIndex = wff.wordIndexInField;
                }
            }
        }

        // skip if nothing good (to avoid log-spam)
        const float epsilon = 0.001f;
        if (sum < epsilon)
        {
            return 0.0f;
        }

        // soft-clip and scale down to never over-power full match
        sum = sum / (1.0f + sum);
        // scale down a bit to make sure one perfect match is higher than a few decent ones
        // can't scale down too much
        sum *= 0.125f;

        float score = bestScore + sum;

        // if we have many words penalize a little bit
        if (wordCount > 1)
        {
            // very very slight penalty (1/200)
            float multiWordPenalty = 1.0f / (1.0f + (float) wordCount * 0.005f);
            score *= multiWordPenalty;
        }

        return score;
    }

    void scoreEveryEntry (size_t pass,
                          nonstd::span<const float> perWordScores,
                          nonstd::span<TempResult> tempResults,
                          nonstd::span<TempResultOrderPenalty> tempResultsOrderPenalty) const // NOLINT
    {
        if (pass < 1)
        {
            for (size_t i = 0; i < tempResults.size(); ++i)
            {
                const Entry& e = entries[i];
                int bestIndex = 0;
                const float currScore = scoreEntry (e, bestIndex, perWordScores);
                tempResults[i].score = currScore;
                tempResults[i].entryIndex = static_cast<int> (i);
                tempResultsOrderPenalty[i].bestIndex = bestIndex;
                tempResultsOrderPenalty[i].misses = 0;
            }
        }
        else
        {
            const float kEpsilon = 0.001f;

            // multiply
            for (size_t i = 0; i < tempResults.size(); ++i)
            {
                const Entry& e = entries[i];
                const float prevScore = tempResults[i].score;
                if (prevScore < kEpsilon)
                {
                    // no need to even score this entry if it is already filtered out
                    continue;
                }

                int bestIndex = 0;
                const float currScore = scoreEntry (e, bestIndex, perWordScores);
                if (currScore < kEpsilon)
                {
                    // no need to multiply, also make sure to skip future passes
                    tempResults[i].score = 0.0f;
                    continue;
                }

                TempResultOrderPenalty& penalty = tempResultsOrderPenalty[i];
                if (bestIndex < penalty.bestIndex)
                {
                    ++penalty.misses;
                    penalty.bestIndex = bestIndex; // score these misses later
                }

                float newScore = prevScore * currScore;
                tempResults[i].score = newScore;
            }
        }
    }

    [[nodiscard]] std::string_view copy_string (std::string_view s) const
    {
        auto* sc_data = searchArena.allocate<char> (s.size());
        std::copy (s.begin(), s.end(), sc_data);
        return { sc_data, s.size() };
    }

public:
    SearchDatabase()
    {
        resetEntries();
    }

    /** Clears any entries currently in the database. */
    void resetEntries (size_t entriesToReserve = 100, size_t wordsToReserve = 1000, size_t arenaSize = 1 << 14)
    {
        wordStorage.clear();
        entries.clear();
        std::fill (fieldWeights.begin(), fieldWeights.end(), 1.0f);
        threshold = 0.1f;

        entries.reserve (entriesToReserve);
        wordStorage.reserve (wordsToReserve);

        searchArena.reset (arenaSize);
    }

    /** Prepares the database to process new search queries. */
    void prepareForSearch()
    {
        const auto numBytesNeededForSearch =
            2048 // string splitting
            + wordStorage.getWordCount() * sizeof (float) // per-word scores
            + entries.size() * (sizeof (TempResult) + sizeof (TempResultOrderPenalty)) // temp results
            + entries.size() * sizeof (Result) // actual results
            + 1024; // padding
        searchArena.reset (numBytesNeededForSearch);
    }

    /** Adds a new entry to the database. */
    void addEntry (Key key, const std::array<std::string_view, numFields>& fields)
    {
        // create entry
        Entry e;
        e.key = key;
        auto& entryWords = e.words;

        // iterate fields
        for (const auto [fieldIndex, field] : enumerate (fields))
        {
            auto arenaFrame = searchArena.create_frame();

            auto fieldStringCopy = copy_string (field);
            auto words = search_helpers::splitString (fieldStringCopy, searchArena);

            // iterate words in this field
            for (const auto [wordIndexInField, word] : enumerate (words))
            {
                // add the word ( de-dup happens here )
                search_helpers::toLower (word);
                int wordIndex = wordStorage.addWord (word);
                entryWords.emplace_back (wordIndex, static_cast<int> (fieldIndex), static_cast<int> (wordIndexInField));
            }
        }

        // sort indices ( in memory-order )
        std::sort (entryWords.begin(), entryWords.end());

        // keep the lowest index-in-field
        for (size_t i = 1; i < entryWords.size(); ++i)
        {
            if (entryWords[i - 1].wordIndex != entryWords[i].wordIndex)
                continue;

            if (entryWords[i - 1].fieldIndex != entryWords[i].fieldIndex)
                continue;

            // store lower
            int v0 = entryWords[i - 1].wordIndexInField;
            int v1 = entryWords[i].wordIndexInField;
            if (v1 < v0)
            {
                v0 = v1;
            }
            entryWords[i - 1].wordIndexInField = v0;
            entryWords[i].wordIndexInField = v0;
        }

        entryWords.erase (std::unique (entryWords.begin(), entryWords.end()), entryWords.end());

        // finally add
        entries.push_back (e);
    }

    /** Returns the number of entries currently stored in the database. */
    auto countEntries() const
    {
        return entries.size();
    }

    /** Sets the "weights" given to each field. */
    void setWeights (const std::array<float, numFields>& newFieldWeights)
    {
        fieldWeights = newFieldWeights;
    }

    /** Any search-result scoring below the threshold will not be returned from the search. */
    void setThreshold (float newThreshold)
    {
        threshold = newThreshold;
    }

    /**
     * Returns a list of search results (sorted and with a score).
     * Note that the list is only valid until the next call to `search()`.
     */
    [[nodiscard]] nonstd::span<const Result> search (std::string_view queryString) const
    {
        if (queryString.size() > 100)
            return {}; // upper bound!

        searchArena.clear(); // this will invalidate the previuosly returned results!

        // 0. prepare query (query-string -> query-words)
        auto queryStringCopy = copy_string (queryString);
        auto queryWords = search_helpers::splitString (queryStringCopy, searchArena);
        for (auto queryWord : queryWords)
            search_helpers::toLower (queryWord);

        // 1. loop over each word in query
        auto perWordScores = nonstd::span { searchArena.allocate<float> (wordStorage.getWordCount()), wordStorage.getWordCount() };
        std::fill (perWordScores.begin(), perWordScores.end(), 0.0f);

        auto tempResults = nonstd::span { searchArena.allocate<TempResult> (entries.size()), entries.size() };
        std::fill (tempResults.begin(), tempResults.end(), TempResult {});

        auto tempResultsOrderPenalty = nonstd::span { searchArena.allocate<TempResultOrderPenalty> (entries.size()), entries.size() };
        std::fill (tempResultsOrderPenalty.begin(), tempResultsOrderPenalty.end(), TempResultOrderPenalty {});

        for (const auto [qi, queryWord] : enumerate (queryWords))
        {
            // 2. score every word against this query-word
            scoreEveryWord (perWordScores, wordStorage, queryWord);

            // 3. score each entry
            scoreEveryEntry (qi, perWordScores, tempResults, tempResultsOrderPenalty);
        }

        if (queryWords.size() > 1)
        {
            // we need to apply the order-penalty
            for (size_t i = 0; i < tempResultsOrderPenalty.size(); ++i)
            {
                TempResultOrderPenalty& penalty = tempResultsOrderPenalty[i];
                const int misses = penalty.misses;
                if (misses > 0)
                {
                    float p = 1.0f / (1.0f + misses * 0.1f);
                    tempResults[i].score *= p;
                }
            }
        }

        // at this point all scores are in tempResults vector
        // only sorting left

        // create a lower threshold for multi-word-queries
        float thresholdCorrected = threshold;
        for (size_t i = 1; i < queryWords.size(); ++i)
            thresholdCorrected *= threshold;

        // sort all that remain
        std::sort (tempResults.begin(), tempResults.end());

        // finally copy to the result vector
        auto resultsWithKey = nonstd::span { searchArena.allocate<Result> (tempResults.size()), tempResults.size() };
        size_t resultsCount = 0;

        for (const auto& tempResult : tempResults)
        {
            if (tempResult.score < thresholdCorrected)
                continue;

            auto& resultWithKey = resultsWithKey[resultsCount++];
            resultWithKey.score = tempResult.score;
            resultWithKey.key = entries[(size_t) tempResult.entryIndex].key;
        }

        auto trimmedResults = resultsWithKey.subspan (0, resultsCount);
        return trimmedResults;
    }
};
} // namespace chowdsp
