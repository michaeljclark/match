/*
 * Matcher
 *
 * Using the Rabin-Karp algorithm to find recurring substrings.
 *
 * Copyright (c) 2019, Michael Clark <michaeljclark@mac.com>
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <cassert>
#include <vector>
#include <string>
#include <limits>

#define MATCHER_DEBUG

#ifdef MATCHER_DEBUG
#define MATCHER_STATS_VARS(a, b) size_t a, b
#define MATCHER_STATS_INIT(a, b) a = b = 0
#define MATCHER_STATS_INCR(x) x++
#else
#define MATCHER_STATS_VARS(a, b)
#define MATCHER_STATS_INIT(a, b)
#define MATCHER_STATS_INCR(x)
#endif

template <typename T>
using Vector = std::vector<T>;

/** enum used to tag match type. */
enum MatchType { Literal, Copy };

/** structure used to return matches. */
template <typename Size>
struct Match
{
    MatchType type;
    Size offset;
    Size length;
};

/** incremental matcher algorithm to find recurring substrings. */
template <typename Sym = char, typename Size = uint64_t>
struct Matcher
{
    const size_t hash_prime = 1047887;
    const size_t hash_size = 1048576;

    const size_t min_match = 3;
    const size_t max_match = 8;

    Vector<Sym> data;
    Vector<Size> prev;
    Vector<Size> head;
    size_t mark;

    Vector<Match<Size>> matches;

    MATCHER_STATS_VARS(i1, i2);

    Matcher();

    template <typename Iterator>
    void append(Iterator begin, Iterator end);

    Size hash_add(Size hval, Sym symbol);
    size_t hash_slot(Size hval);
    size_t check_match(size_t last, size_t pos);

    void decompose();
};

/** construct matcher instance with default hash table size. */
template <typename Sym, typename Size>
Matcher<Sym,Size>::Matcher() : data(), prev(), head(), mark(0), matches()
{
    MATCHER_STATS_INIT(i1, i2);

    head.resize(hash_size);
}

/** append input data into the internal buffer. */
template <typename Sym, typename Size>
template <typename Iterator>
void Matcher<Sym,Size>::append(Iterator begin, Iterator end)
{
    assert(data.size() + std::distance(begin, end)
        < std::numeric_limits<Size>::max());

    data.insert(data.end(), begin, end);
    prev.resize(data.size());
}

/** check whether a hashtable hit matches and return its total length. */
template <typename Sym, typename Size>
size_t Matcher<Sym,Size>::check_match(size_t last, size_t pos)
{
    /* exclude matches later in the string than us. */
    if (last > mark + pos - min_match) return 0;

    /* check past the end of the match up to the limit of available data. */
    size_t i = 0, limit = data.size() - mark;
    for (i = 0; i < limit; i++) {
        if (data[last-pos+i] != data[mark+i]) break;
    }
    return i;
}

/** incrementally add a symbol to a hash value to form a new hash value. */
template <typename Sym, typename Size>
Size Matcher<Sym,Size>::hash_add(Size hval, Sym symbol)
{
    /* simple feedback shift xor hash function */
    return (hval << 5) ^ symbol;
}

/** translate a hash value to a hash table slot using prime modulus. */
template <typename Sym, typename Size>
size_t Matcher<Sym,Size>::hash_slot(Size hval)
{
    /* prime number has better diffusion than hval & (hash_size-1). */
    return hval % hash_prime; 
}

/** incrementally run the match algorithm on new data past mark. */
template <typename Sym, typename Size>
void Matcher<Sym,Size>::decompose()
{
    /* 
     * Use the Rabin-Karp algorithm to find recurring substrings in a string
     * by hashing multiple length combinations from a mark, checking these
     * hashes for prior matches in a hash chain, inserting copy instructions
     * for the best matches or new literal instructions for new data. Outputs
     * a list of instructions referencing new data or copies of prior data.
     *
     * - Match[type=Literal] - create new literal from source.
     * - Match[type=Copy] - self referential copy from context.
     *
     * Complexity ~ O(n·log₂(n))
     */
    while (mark < data.size())
    {
        /* Use the Rabin-Karp algorithm to match substrings from our mark. */
        Size hval = 0;
        size_t limit = std::min(data.size() - mark, max_match);
        size_t best = 0, len = 0;
        for (size_t pos = 0; pos < limit; pos++)
        {
            /*
             * Add symbol to rolling hash, retrieve prior hash chain offset
             * (if any) and add current hash value to head of hash chain.
             */
            hval = hash_add(hval, data[mark + pos]);
            size_t hpos = hash_slot(hval);
            size_t last = prev[mark + pos] = head[hpos];
            head[hpos] = mark + pos;

            MATCHER_STATS_INCR(i1);

            if (pos < min_match-1) continue;

            /*
             * check and follow hash table hits through chain matches to
             * find the best matches and save longer or earlier matches.
             */
            while (last) {
                size_t match_len = check_match(last, pos);
                if (match_len >= min_match &&
                     (match_len > len ||
                        (match_len == len && last < best && last > pos)))
                {
                    best = last - pos;
                    len = match_len;
                }

                MATCHER_STATS_INCR(i2);

                /* follow the match hash chain if it is earlier */
                last = prev[last] < last ? prev[last] : 0;
            }

            /* if hash table hit finds longer entry, we'll bail early. */
            if (len > pos + 1) break;
        }

        if (len >= min_match) {
            /* add copy instruction to list of matches. */
            mark += len;
            matches.push_back({ MatchType::Copy, Size(best), Size(len) });
        } else {
            /* add new literal instruction if required. */
            if (matches.size() == 0 ||
                matches.back().offset + matches.back().length != mark) {
                matches.push_back({ MatchType::Literal, Size(mark), Size(1) });
            } else {
                matches.back().length++;
            }
            /* advance mark by one. */
            mark++;
        }
    }
}
