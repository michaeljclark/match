#!/bin/bash

# matcher test cases
# todo: diff against exemplar output

./build/match cowFOOcow
./build/match cowFOOcowFOOcow
./build/match gooseABCgooseDEFgoose
./build/match the_quick_the_round_fox_the_round
./build/match foo_ate_foo_bar_baz_bar_ate_foo
./build/match foo_ate_foo_bar_baz_bar_ate_bar
./build/match TGGGCGTGCGCTTGAAAAGAGCCTAAGAAGAGGGGGCGTCTGGAAGGAACCGCAACGCCAAGGGAGGGTG
