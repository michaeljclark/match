/*
 * Matcher
 *
 * Using the Robin-Karp algorithm to find recurring substrings.
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

#include <cstdio>
#include <sys/stat.h>
#include <algorithm>

#include "matcher.h"

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

/** test that runs the matcher and prints out the edit instructions. */
void test_matcher(const char *syms, size_t length)
{
    Matcher<> m;

    /* append input data and run the match algorithm */
    printf("Original: %s%s\n", std::string(syms,
        std::min(length,size_t(80))).c_str(), length > 80 ? "..." : "");
    m.append(syms, syms + length);
    m.decompose();

    /* output matches in this format: New/Copy [inclusive,exclusive) */
    size_t literals = 0, copies = 0;
    for (auto &n : m.matches) {
        printf("[%3zu] : %7s [ %3zu,%3zu )   # \"%s\"\n",
            std::distance(&m.matches[0], &n), match_type_name(n.type),
            size_t(n.offset), size_t(n.offset + n.length),
            std::string(&m.data[n.offset], n.length).c_str());
        switch (n.type) {
        case MatchType::Literal: literals += n.length; break;
        case MatchType::Copy: copies += n.length; break;
        }
    }

    /* output matcher stats */
    printf("DataSize/Literals/Copies/Iterations: %zu/%zu/%zu/%zu\n",
        m.data.size(), literals, copies, m.iterations);
}

/*
 * $ c++ -O2 match.cc -o match
 * $ ./match TGGGCGTGCGCTTGAAAAGAGCCTAAGAAGAGGGGGCGTCTGGAAGGAACCGCAACGCCAAGGGAGGGTG
 * $ ./match -f sample.txt
 */
int main(int argc, char **argv)
{
    if (argc == 2) {
        test_matcher(argv[1], strlen(argv[1]));
    } else if (argc == 3 && (strcmp(argv[1], "-f") == 0)) {
        std::vector<uint8_t> buf;
        size_t len = read_file(buf, argv[2]);
        test_matcher((const char*)&buf[0], len);
    } else {
        fprintf(stderr, "usage: %s \"string\"\n", argv[0]);
        exit(9);
    }
}