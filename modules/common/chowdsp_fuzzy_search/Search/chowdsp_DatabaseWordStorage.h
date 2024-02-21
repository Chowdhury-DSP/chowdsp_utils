#pragma once

namespace chowdsp::search_database
{
/** helper struct to store words in "flat" memory */
struct WordStorage
{
    struct WordView
    {
        uint32_t start = 0;
        uint32_t end = 0;
    };

    std::vector<char> wordData {}; // all characters from a word, in order in memory
    std::vector<search_helpers::WordHist> wordHist {}; // a tiny histogram per word
    std::vector<WordView> wordMap {}; // map to find index from word

    [[nodiscard]] std::string_view getString (WordView wordView) const noexcept
    {
        return { wordData.data() + wordView.start, wordView.end - wordView.start };
    }

    void clear()
    {
        wordData.clear();
        wordHist.clear();
        wordMap.clear();
    }

    void reserve (size_t wordCount)
    {
        wordData.reserve (wordCount * 8);
        wordHist.reserve (wordCount);
        wordMap.reserve (wordCount);
    }

    [[nodiscard]] auto getWordCount() const noexcept { return wordMap.size(); }

    [[nodiscard]] int addWord (std::string_view word)
    {
        // if exist, return index
        for (const auto [idx, wordView] : enumerate (wordMap))
        {
            if (word == getString (wordView))
                return static_cast<int> (idx);
        }

        const auto newWordIndex = static_cast<int> (wordMap.size());

        // add to data
        const auto wordStart = static_cast<uint32_t> (wordData.size());
        wordData.insert (wordData.end(), word.begin(), word.end());

        // add word view
        const auto newWordView = WordView { wordStart, static_cast<uint32_t> (wordData.size()) };
        wordMap.emplace_back (newWordView);
        wordHist.emplace_back (word);

        // return word-index
        return newWordIndex;
    }
};
} // namespace chowdsp
