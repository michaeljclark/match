# matcher

_Use the Rabin-Karp algorithm in C++ to find recurring substrings
in a string and emit a near minimal edit list._

## Overview

This is a single header C++ implementation of the Rabin-Karp algorithm that
is used to find recurring substrings in a string. This code finds self-
matching substrings using a rolling hash value called a Rabin-Karp signature,
which is composed incrementally from increasing length combinations of
substrings offset from a marker.

The algorithm records offsets in a hash table indexed by the Rabin-Karp
signature and saves a chain of the previous matching offsets for each
signature in another table indexed by offset. Before updating the hash
table with the current offset, the previous offset for the current signature
is saved into the previous table and a chain indexed from the current offset
to the previous offset has now been created.

The search algorithm proceeds to check for hits and follows the chains of
offsets leading through all prior occurances of a given Rabin-Karp signature,
all the way back to the earliest match. The algorithm checks these approximate
matches, and if valid, inserts copy instructions for the best match, into
the match list, otherwise, emitting instructions to add new literals. The
matches are approximate because there is a collision probability, however there
are multiple entries that can be used to predict previous offsets.

The output is a list of instructions referencing new data
or copies of previous data.

- ___Vector&lt;Match&gt;___
  - ___Type___ type (* _Literal_ or _Copy_ *)
  - ___Size___ offset
  - ___Size___ length

The matcher is designed to be use after a symbol input stage in a text
indexer or compression engine. The matches in the match list can be
converted from absolute offsets to relative distances and then fed into
an entropy coder to implement a simple compression algorithm. See the
[Rabin-Karp algorithm](https://en.wikipedia.org/wiki/Rabin%E2%80%93Karp_algorithm)
article for a more complete description of the rolling hash approximate
matching algorithm.

## Example

The following is an example invocation of the included test program:

___Test command___:
```$ ./build/test TGGGCGTGCGCTTGAAAAGAGCCTAAGAAGAGGGGGCGTCTGGAAGGAACCGCAACGCCAAGGGAGGGTG
```

___Expected output___:
```Original: TGGGCGTGCGCTTGAAAAGAGCCTAAGAAGAGGGGGCGTCTGGAAGGAACCGCAACGCCAAGGGAGGGTG
[  0] : Literal [   0,  7 )   # "TGGGCGT"
[  1] :    Copy [   3,  6 )   # "GCG"
[  2] : Literal [  10, 24 )   # "CTTGAAAAGAGCCT"
[  3] :    Copy [  16, 20 )   # "AAGA"
[  4] :    Copy [  17, 21 )   # "AGAG"
[  5] :    Copy [   1,  4 )   # "GGG"
[  6] :    Copy [   3,  7 )   # "GCGT"
[  7] : Literal [  39, 40 )   # "C"
[  8] :    Copy [   0,  3 )   # "TGG"
[  9] :    Copy [  16, 19 )   # "AAG"
[ 10] :    Copy [  13, 16 )   # "GAA"
[ 11] : Literal [  49, 55 )   # "CCGCAA"
[ 12] :    Copy [   8, 11 )   # "CGC"
[ 13] :    Copy [  52, 55 )   # "CAA"
[ 14] :    Copy [   1,  4 )   # "GGG"
[ 15] : Literal [  64, 65 )   # "A"
[ 16] :    Copy [   1,  4 )   # "GGG"
[ 17] : Literal [  68, 70 )   # "TG"
DataSize/Literals/Copies/Iterations: 70/31/39/1580
```