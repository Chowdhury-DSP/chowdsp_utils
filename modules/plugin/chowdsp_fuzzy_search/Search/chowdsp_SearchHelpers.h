#pragma once

// adapted from: https://bitbucket.org/j_norberg/fuzzysearchdatabase

#include <string_view>
#include <cctype>
#include <cstring>

namespace chowdsp::search_helpers
{
namespace detail
{
    inline int min3 (int a, int b, int c)
    {
        return (a < b) ? (a < c ? a : c) : (b < c ? b : c);
    }

    inline int substringFind (const char* queryWord, size_t queryLength, const char* w, size_t limit)
    {
        for (size_t i = 0; i <= limit; ++i)
        {
            if (0 == memcmp (w + i, queryWord, queryLength))
            {
                return (int) i;
            }
        }

        return -1;
    }

    inline float substringScore (std::string_view query, std::string_view word)
    {
        size_t limit = word.size() - query.size();

        int found = substringFind (query.data(), query.size(), word.data(), limit);
        if (found < 0)
        {
            return 0.0f;
        }

        // score higher if length of substring is closer to the whole word
        float divisor = 1.0f + (float) limit * 0.02f;

        if (found == 0)
        {
            // first
        }
        else if (found == (int) limit)
        {
            // last
            divisor += 0.02f;
        }
        else
        {
            // middle
            divisor += 0.06f;
        }

        float score = 1.0f / divisor;
        return score;
    }

    inline int levDistance (const char* a, size_t aL, const char* b, size_t bL, int& prefixBonus, int& suffixBonus)
    {
        // ------------------------------------------------------
        // https://en.wikipedia.org/wiki/Levenshtein_distance

        prefixBonus = 0;
        suffixBonus = 0;

        // common prefix
        while (aL > 0 && bL > 0 && a[0] == b[0])
        {
            ++a;
            ++b;
            --aL;
            --bL;
            ++prefixBonus;
        }

        // common suffix
        while (aL > 0 && bL > 0 && a[aL - 1] == b[bL - 1])
        {
            --aL;
            --bL;
            ++suffixBonus;
        }

        // simple case
        if (aL < 1)
            return (int) bL;
        if (bL < 1)
            return (int) aL;

        enum
        {
            kBuf = 16,
            kLen = kBuf - 1
        };

        // clamp length
        if (aL > kLen)
            aL = kLen;
        if (bL > kLen)
            bL = kLen;

        // used during distance-calculations
        // currently have a length-limit on words, should be ok
        // this could in theory be just two rows ( or even one )
        int bufDist[kBuf * kBuf];

        size_t stride = aL + 1;

        // init top
        for (size_t x = 0; x <= aL; x++)
            bufDist[x] = (int) x;

        // fill rest of matrix
        for (size_t y = 1; y <= bL; y++)
        {
            size_t index = y * stride + 1;

            // init-left side
            bufDist[index - 1] = (int) y;

            int bChar = static_cast<unsigned char> (b[y - 1]);

            // in theory, could unroll this a little-bit
            for (size_t x = 1; x <= aL; ++x, ++index)
            {
                int substitutionCost = (a[x - 1] == bChar) ? 0 : 1;

                int t = min3 (
                    bufDist[index - 1] + 1,
                    bufDist[index - stride] + 1,
                    bufDist[index - stride - 1] + substitutionCost);

                bufDist[index] = t; // write
            }
        }

        // return
        int dist = bufDist[bL * stride + aL];
        return dist;
    }

    inline bool isDivider (int c)
    {
        // possible improvement, support utf8
        // maybe this is a bit agressive
        // this will treat every character not mentioned below as a delimiter
        if ('a' <= c && c <= 'z')
            return false;
        if ('A' <= c && c <= 'Z')
            return false;
        if ('0' <= c && c <= '9')
            return false;
        if (c == '+')
            return false;
        if (c == '-')
            return false;
        if (c == '&')
            return false;

        return true;
    }
} // namespace detail

inline void toLower (const nonstd::span<char>& s)
{
    for (char& i : s)
        i = static_cast<char> (std::tolower (i));
}

inline void toLower (std::string_view s)
{
    // Careful... this won't work if the string points to immutable data!
    toLower (nonstd::span { const_cast<char*> (s.data()), s.size() });
}

inline nonstd::span<std::string_view> splitString (std::string_view s, ArenaAllocator<>& arena)
{
    const auto stringSplitData = arena.allocate<std::string_view> (s.size());
    if (stringSplitData == nullptr)
        return {};
    auto ss = nonstd::span { stringSplitData, s.size() };
    std::fill (ss.begin(), ss.end(), std::string_view {});
    size_t ss_index = 0;

    size_t left = 0;
    size_t right = 0;

    for (; left < s.size(); ++left)
    {
        // 1. skip all dividers
        if (detail::isDivider (s[left]))
            continue;

        right = left + 1;

        // 2. skip all non-dividers
        for (; right < s.size(); ++right)
            if (detail::isDivider (s[right]))
                break;

        ss[ss_index++] = s.substr (left, right - left);
        left = right;
    }

    return ss.subspan (0, ss_index);
}

// tiny single-bit-histogram
// early out many words with this
struct WordHist
{
    explicit WordHist (std::string_view s)
    {
        _mask = 0;
        for (size_t i = 0; i < s.size(); ++i) // NOLINT
        {
            int c = static_cast<unsigned char> (s[i]);
            _mask |= 1 << charToBit (c);
        }
    }

    static uint32_t charToBit (int c)
    {
        // all numbers collapse to same symbol (31)
        uint32_t b = 31;

        // support 'a' .. 'z'
        if (c >= 'a' && c <= 'z')
            b = (uint32_t) (c - 'a');

        // fixme deal with +/&? (maybe not needed for early-outs)
        return b;
    }

    [[nodiscard]] bool canSkip (WordHist o) const
    {
        uint32_t v = _mask & o._mask;
        return v == 0;
    }

    uint32_t _mask = 0; // one bit per letter
};

// tiny word-pair histogram
struct WordPairHist
{
    explicit WordPairHist (std::string_view s)
    {
        size_t len = s.size();
        uint32_t a = WordHist::charToBit (s[0]);
        for (size_t i = 1; i < len; ++i)
        {
            uint32_t b = WordHist::charToBit (s[i]);
            followingMask[a] |= 1 << b;
            a = b;
        }
    }

    [[nodiscard]] int countMatchingPairs (std::string_view s) const
    {
        int sum = 0;

        uint32_t a = WordHist::charToBit (s[0]);
        for (size_t i = 1; i < s.size(); ++i)
        {
            uint32_t b = WordHist::charToBit (s[i]);
            auto mask = (uint32_t) (1 << b);
            uint32_t overlap = followingMask[a] & mask;
            if (overlap != 0)
                ++sum;
            a = b;
        }
        return sum;
    }

    uint32_t followingMask[32] {};
};

// query is expected to be shorter than word
inline float scoreLev (std::string_view query, std::string_view word)
{
    int prefixBonus = 0;
    int suffixBonus = 0;
    auto fuzzyDist = (float) detail::levDistance (query.data(), query.size(), word.data(), word.size(), prefixBonus, suffixBonus);

    // if dist is very very bad
    if (fuzzyDist >= (float) query.size())
    {
        // late-out
        return 0;
    }

    if (prefixBonus > 0)
    {
        float prefixBonusMul = 4.0f / (4.0f + (float) prefixBonus * 0.25f);
        fuzzyDist *= prefixBonusMul;
    }

    if (suffixBonus > 0)
    {
        float suffixBonusMul = 4.0f / (4.0f + (float) suffixBonus * 0.125f);
        fuzzyDist *= suffixBonusMul;
    }

    // if the distance is very high ( more than half of the word-length )
    // it's not good enough
    if ((fuzzyDist * 2) > (float) word.size())
    {
        // not good enough
        return 0;
    }

    float fuzzyDistFrac = fuzzyDist / (float) word.size();
    float fuzzyScore = 1.0f - fuzzyDistFrac;

    return fuzzyScore;
}

inline float scoreShortQueryWordToWord (const WordPairHist& hist, std::string_view query, std::string_view word)
{
    // query is shorter than word

    int matchingPairs = hist.countMatchingPairs (word);
    if ((static_cast<size_t> (matchingPairs) + 1) < query.size())
    {
        // early-out since substring is impossible
        return 0.0f;
    }

    // do we only support actual substring?
    float rawScore = detail::substringScore (query, word);
    return rawScore;
}

inline float scoreLongQueryWordToWord (const WordPairHist& hist, std::string_view query, std::string_view word)
{
    if (word.size() < 3)
    {
        // word is short (1 or 2 letters)

        if (word.size() < 2)
        {
            // it's too easy to get bad results for single-character words
            return 0.0f;
        }

        if (query.size() > 5)
        {
            // if the query is much longer... not a good fit
            return 0.0f;
        }
    }

    // query is soo much longer than word
    if (query.size() > word.size() + 8)
    {
        return 0.0f; // don't even worry about it
    }

    int matchingPairs = hist.countMatchingPairs (word);
    if (matchingPairs + 1 < (int) word.size())
    {
        // early-out since substring is impossible
        return 0.0f;
    }

    // so test the other way around (and weigh less)
    // do we only support actual substring?
    float rawScore = detail::substringScore (word, query);
    float otherWayPenalty = 0.7f;

    // adjust even more for very short words (like "and" being a substring of "band")
    if (word.size() < 4)
    {
        otherWayPenalty = 0.6f;
    }

    float adjustedScore = otherWayPenalty * rawScore;
    return adjustedScore;
}

// qw - query-Word
// this gives a score on how well a query-word fits a word
// 1.0 is a full match
static inline float scoreQueryWordToWord (const WordPairHist& hist, std::string_view query, std::string_view word)
{
    // for very short queries
    if (query.size() <= 3)
    {
        // only accept words of same length or longer
        if (word.size() < query.size())
        {
            // can not accept backword matching for these
            return 0.0f;
        }

        // for very short queries, only do substrings
        return scoreShortQueryWordToWord (hist, query, word);
    }

    if (query.size() + 3 < word.size())
    {
        // query is substring (score early substring better)
        return scoreShortQueryWordToWord (hist, query, word);
    }

    if (query.size() > word.size() + 1)
    {
        // word is substring of query (how much longer can a query be?)
        return scoreLongQueryWordToWord (hist, query, word);
    }

    // query is close in length (lev-dist)
    int matchingPairs = hist.countMatchingPairs (word);

    if (matchingPairs + 4 < (int) query.size())
    {
        // early-out
        return 0.0f;
    }

    if (matchingPairs + 1 == (int) query.size())
    {
        if (query.size() == word.size())
        {
            // test for 100% match
            if (0 == memcmp (query.data(), word.data(), query.size()))
            {
                return 1.0f;
            }
        }
    }

    float score = scoreLev (query, word);
    return score;
}
} // namespace chowdsp::search_helpers
