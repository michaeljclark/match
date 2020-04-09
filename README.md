# matcher

_Using the Rabin-Karp algorithm in C++ to find recurring substrings._

## Overview

Single header C++ implementation of the Rabin-Karp algorithm to find
recurring substrings in a string. The code finds substrings using a
rolling hash of multiple length combinations from a mark, and checks
them for prior matches in a hash chain, inserting copy instructions
for the best matches or new literal instructions for new data. Outputs
a list of instructions referencing new data or copies of prior data.

- ___Match[type=Literal]___ - create new literal from source.
- ___Match[type=Copy]___ - self referential copy from context.

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