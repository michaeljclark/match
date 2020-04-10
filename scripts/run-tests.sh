#!/bin/bash

# matcher test cases
# todo: diff against exemplar output

./build/match -v -t cowFOOcow
./build/match -v -t cowFOOcowFOOcow
./build/match -v -t gooseABCgooseDEFgoose
./build/match -v -t the_quick_the_round_fox_the_round
./build/match -v -t foo_ate_foo_bar_baz_bar_ate_foo
./build/match -v -t foo_ate_foo_bar_baz_bar_ate_bar
./build/match -v -t TGGGCGTGCGCTTGAAAAGAGCCTAAGAAGAGGGGGCGTCTGGAAGGAACCGCAACGCCAAGGGAGGGTG