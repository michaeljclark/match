/*
 * Matcher
 *
 * Using the Rabin-Karp algorithm to find recurring substrings.
 *
 * Copyright (c) 2020, Michael Clark <michaeljclark@mac.com>
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
 *
 * algorithm is from studying the 'zlib' compression library.
 *
 * Copyright (C) 1995-2017 Jean-loup Gailly and Mark Adler
 *
 * This software is provided 'as-is', without any express or implied
 * warranty.  In no event will the authors be held liable for any damages
 * arising from the use of this software.
 *
 * Permission is granted to anyone to use this software for any purpose,
 * including commercial applications, and to alter it and redistribute it
 * freely, subject to the following restrictions:
 *
 * 1. The origin of this software must not be misrepresented; you must not
 *    claim that you wrote the original software. If you use this software
 *    in a product, an acknowledgment in the product documentation would be
 *    appreciated but is not required.
 * 2. Altered source versions must be plainly marked as such, and must not be
 *    misrepresented as being the original software.
 * 3. This notice may not be removed or altered from any source distribution.
 *
 * Jean-loup Gailly        Mark Adler
 * jloup@gzip.org          madler@alumni.caltech.edu
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
#define MATCHER_STATS_INCR(x) x++
#define MATCHER_DEBUG_PRINT(...) printf(__VA_ARGS__)
#else
#define MATCHER_STATS_INCR(x)
#define MATCHER_DEBUG_PRINT(...)
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

/** primes less than power of 2 */
static inline uint64_t prime_lt_pow2(int n)
{
    /* create smallest prime less than 2^n */
    static const uint8_t k[64] = {
        /*  0 */   0,   0,   1,   1,   3,   1,   3,   1,
        /*  8 */   5,   3,   3,   9,   3,   1,   3,  19,
        /* 16 */  15,   1,   5,   1,   3,   9,   3,  15,
        /* 24 */   3,  39,   5,  39,  57,   3,  35,   1,
        /* 32 */   5,   9,  41,  31,   5,  25,  45,   7,
        /* 40 */  87,  21,  11,  57,  17,  55,  21, 115,
        /* 48 */  59,  81,  27, 129,  47, 111,  33,  55,
        /* 56 */   5,  13,  27,  55,  93,   1,  57,  25,
    };
    return (1ull << n) - k[n];
}

/** incremental matcher algorithm to find recurring substrings. */
template <typename Symbol = char, typename Size = uint32_t>
struct Matcher
{
    static const size_t kInitialHashBits = 15;

    size_t hash_bits;
    size_t hash_prime;
    size_t hash_size;

    size_t min_match = 3;
    size_t max_match = 32;

    Vector<Symbol> data;
    Vector<Size> prev;
    Vector<Size> head;
    size_t mark;

    Vector<Match<Size>> matches;

#ifdef MATCHER_DEBUG
    size_t i1 = 0, i2 = 0;
#endif

    Matcher();
    Matcher(size_t hash_size);

    void resize(size_t hash_size);

    template <typename Iterator>
    void append(Iterator begin, Iterator end);

    Size hash_add(Size hval, Symbol symbol);
    size_t hash_slot(Size hval);
    size_t check_match(size_t last, size_t pos);

    void decompose(bool partition = true);
};

/** construct matcher instance with default hash table size. */
template <typename Symbol, typename Size>
Matcher<Symbol,Size>::Matcher(size_t hash_bits) : hash_bits(hash_bits),
    data(), prev(), head(), mark(0), matches()
{
    resize(hash_bits);
}

template <typename Symbol, typename Size>
Matcher<Symbol,Size>::Matcher() : Matcher(kInitialHashBits) {}

template <typename Symbol, typename Size>
void Matcher<Symbol,Size>::resize(size_t hash_bits)
{
    hash_size = 1 << hash_bits;
    hash_prime = prime_lt_pow2(hash_bits);
    head.resize(hash_size);
}

/** append input data into the internal buffer. */
template <typename Symbol, typename Size>
template <typename Iterator>
void Matcher<Symbol,Size>::append(Iterator begin, Iterator end)
{
    assert(data.size() + std::distance(begin, end)
        < std::numeric_limits<Size>::max());

    data.insert(data.end(), begin, end);
    prev.resize(data.size());
}

/** check whether a hashtable hit matches and return its total length. */
template <typename Symbol, typename Size>
size_t Matcher<Symbol,Size>::check_match(size_t last, size_t pos)
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
template <typename Symbol, typename Size>
Size Matcher<Symbol,Size>::hash_add(Size hval, Symbol symbol)
{
    /* simple feedback shift xor hash function */
    return (hval << 5) ^ symbol;
}

/** translate a hash value to a hash table slot using prime modulus. */
template <typename Symbol, typename Size>
size_t Matcher<Symbol,Size>::hash_slot(Size hval)
{
    /* prime number has better diffusion than hval & (hash_size-1). */
    return hval % hash_prime;
}

/** incrementally run the match algorithm on new data past mark. */
template <typename Symbol, typename Size>
void Matcher<Symbol,Size>::decompose(bool partition)
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
     * Complexity ~ O(n)
     */

    if (partition && mark < data.size()) {
        matches.push_back({ MatchType::Literal, Size(mark), Size(0) });
    }

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
                last = match_len > pos && prev[last] < last ? prev[last] : 0;
            }

            /* if hash table hit finds longer entry, we'll bail early. */
            if (len > pos + 1) break;
        }

        if (len >= min_match) {
            mark += len;
            /* add copy instruction to list of matches. */
            if (matches.size() == 0 || matches.back().length > 0) {
                matches.push_back({ MatchType::Copy, Size(best), Size(len) });
            } else {
                matches.back() = { MatchType::Copy, Size(best), Size(len) };
            }
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
