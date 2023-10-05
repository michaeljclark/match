/*
 * Match
 *
 * Using the Robin-Karp algorithm to find recurring substrings.
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

#include <cstdio>
#include <sys/stat.h>
#include <algorithm>

#include "matcher.h"

static const char* filename = nullptr;
static const char* separator = nullptr;
static const char* text = nullptr;
static bool debug = false;
static bool verbose = false;
static bool help = false;
static int bits = 15;

/* trim leading whitesspace */
static std::string ltrim(std::string s)
{
    s.erase(s.begin(), std::find_if(s.begin(), s.end(),
            std::not1(std::ptr_fun<int, int>(std::isspace))));
    return s;
}

/* trim trailing whitesspace */
static std::string rtrim(std::string s)
{
    s.erase(std::find_if(s.rbegin(), s.rend(),
            std::not1(std::ptr_fun<int, int>(std::isspace))).base(), s.end());
    return s;
}

/* split string into vector */
static std::vector<std::string> split(std::string str,
    std::string sep, bool inc_sep = false, bool inc_empty = false)
{
    size_t i, j = 0;
    std::vector<std::string> comps;
    while ((i = str.find_first_of(sep, j)) != std::string::npos) {
        if (inc_empty || i - j > 0) comps.push_back(str.substr(j, i - j));
        if (inc_sep) comps.push_back(str.substr(i, 1));
        j = i + 1; /* assumes separator is 1-byte */
    }
    if (inc_empty || str.size() - j > 0) {
        comps.push_back(str.substr(j, str.size() - j));
    }
    return comps;
}

/** read file into std::vector using buffered file IO */
static size_t read_file(std::vector<uint8_t> &buf, const char* filename)
{
    FILE *f;
    struct stat statbuf;
    if ((f = fopen(filename, "r")) == nullptr) {
        fprintf(stderr, "fopen: %s\n", strerror(errno));
        exit(1);
    }
    if (fstat(fileno(f), &statbuf) < 0) {
        fprintf(stderr, "fstat: %s\n", strerror(errno));
        exit(1);
    }
    buf.resize(statbuf.st_size);
    size_t len = fread(buf.data(), 1, buf.size(), f);
    assert(buf.size() == len);
    fclose(f);
    return buf.size();
}

/** string constant for match type. */
static const char* match_type_name(MatchType type)
{
    switch (type) {
    case MatchType::Literal: return "Literal";
    case MatchType::Copy: return "Copy";
    }
    return nullptr;
}

struct matcher_stats
{
    size_t literals;
    size_t copies;
};

template <typename M>
matcher_stats calc_stats(M &m)
{
    size_t literals = 0, copies = 0;
    for (auto &n : m.matches) {
        switch (n.type) {
        case MatchType::Literal: literals += n.length; break;
        case MatchType::Copy: copies += n.length; break;
        }
    }
    return { literals, copies };
}

template <typename M>
void dump_matches(M &m)
{
    /* output matches in this format: New/Copy [inclusive,exclusive) */
    ssize_t offset = 0;
    for (auto &n : m.matches) {
        printf("[%3zu] : %7s [ %3zd,%3zu )   # \"%s\"\n",
            std::distance(&m.matches[0], &n), match_type_name(n.type),
            offset - n.offset, size_t(n.length),
            std::string(&m.data[n.offset], n.length).c_str());
        offset += n.length;
    }
}

/** test that runs the matcher and prints out the edit instructions. */
void match_text(const char *syms, size_t length)
{
    Matcher<> m(bits);

    if (separator) {
        std::vector<std::string> symbols =
            split(rtrim(ltrim(std::string(syms, length))), separator);
        if (verbose) {
            for (auto sym : symbols) {
                printf("Symbol: %s\n", sym.c_str());
            }
        }
        for (auto sym : symbols) {
            m.append(sym.begin(), sym.end());
            m.decompose();
        }
    } else {
        if (verbose) {
            printf("OriginalText: %s\n", std::string(syms, length).c_str());
        }
        m.append(syms, syms + length);
        m.decompose();
    }

    if (verbose) {
        dump_matches(m);
    }

    matcher_stats s = calc_stats(m);

    printf("DataSize/Literals/Copies: %zu/%zu/%zu\n", m.data.size(), s.literals, s.copies);
    MATCHER_DEBUG_PRINT("OuterIterations/InnerIterations: %zu/%zu\n", m.i1, m.i2);
}

/*
 * command line options
 */

void print_help(int argc, char **argv)
{
    fprintf(stderr,
        "Usage: %s [options]\n"
        "\n"
        "Options:\n"
        "  -t, --text <text>            symbols from argument\n"
        "  -f, --file <filename>        symbols from file\n"
        "  -s, --split <separator>      split input symbols\n"
        "  -b, --bits <width>           specity hash table size\n"
        "  -v, --verbose                enable verbose output\n"
        "  -d, --debug                  enable debug output\n"
        "  -h, --help                   command line help\n",
        argv[0]);
}

bool check_param(bool cond, const char *param)
{
    if (cond) {
        printf("error: %s requires parameter\n", param);
    }
    return (help = cond);
}

bool match_opt(const char *arg, const char *opt, const char *longopt)
{
    return strcmp(arg, opt) == 0 || strcmp(arg, longopt) == 0;
}

void parse_options(int argc, char **argv)
{
    int i = 1;
    while (i < argc) {
        if (match_opt(argv[i], "-t", "--text")) {
            if (check_param(++i == argc, "--text")) break;
            text = argv[i++];
        } else if (match_opt(argv[i], "-f", "--file")) {
            if (check_param(++i == argc, "--file")) break;
            filename = argv[i++];
        } else if (match_opt(argv[i], "-s", "--separator")) {
            if (check_param(++i == argc, "--separator")) break;
            separator = argv[i++];
        } else if (match_opt(argv[i], "-b", "--bits")) {
            if (check_param(++i == argc, "--bits")) break;
            bits = atoi(argv[i++]);
        } else if (match_opt(argv[i], "-d", "--debug")) {
            debug = true;
            i++;
        } else if (match_opt(argv[i], "-v", "--verbose")) {
            verbose = true;
            i++;
        } else if (match_opt(argv[i], "-h", "--help")) {
            help = true;
            i++;
        } else {
            fprintf(stderr, "error: unknown option: %s\n", argv[i]);
            help = true;
            break;
        }
    }

    if (help) {
        print_help(argc, argv);
        exit(1);
    }
}

/*
 * $ c++ -O2 match.cc -o match
 * $ ./match -t TGGGCGTGCGCTTGAAAAGAGCCTAAGAAGAGGGGGCGTCTGGAAGGAACCGCAACGCCAAGGGAGGGTG
 * $ ./match -f sample.txt
 */
/*
 * main program
 */

int main(int argc, char **argv)
{
    parse_options(argc, argv);

    if (filename) {
        std::vector<uint8_t> buf;
        size_t len = read_file(buf, filename);
        match_text((const char*)&buf[0], len);
    } else if (text) {
        match_text(text, strlen(text));
    } else {
        fprintf(stderr, "error: must specify --text or --file\n");
        exit(9);
    }
}
