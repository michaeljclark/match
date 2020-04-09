#!/bin/bash

# matcher test cases
# todo: diff against exemplar output

./build/test cowFOOcow
./build/test cowFOOcowFOOcow
./build/test gooseABCgooseDEFgoose
./build/test the_quick_the_round_fox_the_round
./build/test foo_ate_foo_bar_baz_bar_ate_foo
./build/test foo_ate_foo_bar_baz_bar_ate_bar
./build/test TGGGCGTGCGCTTGAAAAGAGCCTAAGAAGAGGGGGCGTCTGGAAGGAACCGCAACGCCAAGGGAGGGTG
