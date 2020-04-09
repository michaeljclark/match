# matcher

_Use the Rabin-Karp algorithm in C++ to find recurring substrings
in a string and emit a near minimal edit list._

## Overview

This is a single header C++ implementation of the Rabin-Karp algorithm that
is used to find recurring substrings in a string. This code finds self-
matching substrings using a rolling hash value called a Rabin-Karp signature,
which is composed incrementally from increasing length combinations of
substrings offset from a marker.

![Rabin-Karp algorithm illustration](rabin-karp.png)

The algorithm records offsets of substring occurances in a hash table
(_'head'_) indexed by the Rabin-Karp signature and uses the hash table
information to compose a chain of occurances in a second table (_'prev'_),
indexed by offset, which leads from the current match to the last match.
Each position in the previous match table contains an offset to the last
match for the same signature recorded at a prior offset.

The search algorithm, after updating the hash table and previous match table,
checks for a hit, and if there is a hit, it follows the chains of past offsets
leading through all prior occurances of a particular Rabin-Karp signature,
all the way back to the earliest match. The algorithm then verifies these
approximate matches, and if valid, creates copy instructions for the best
match, or, alternatively emits literal instructions for new text. The matches
are approximate because there is a collision probability, however, there are
multiple offsets that can be used to match a prior occurance, which aids with
the probabalistic nature.

The output is a list of instructions referencing new data
or copies of previous data.

- ___Vector&lt;Match&gt;___
  - ___Type___ type (* _Literal_ or _Copy_ *)
  - ___Size___ offset
  - ___Size___ length

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