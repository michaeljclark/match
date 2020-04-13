#!/bin/bash

# matcher test cases
# todo: diff against exemplar output

bits=15

./build/match -v -b ${bits} -t cowFOOcow
./build/match -v -b ${bits} -t cowFOOcowFOOcow
./build/match -v -b ${bits} -t gooseABCgooseDEFgoose
./build/match -v -b ${bits} -t the_quick_the_round_fox_the_round
./build/match -v -b ${bits} -t foo_ate_foo_bar_baz_bar_ate_foo
./build/match -v -b ${bits} -t foo_ate_foo_bar_baz_bar_ate_bar
./build/match -v -b ${bits} -t TGGGCGTGCGCTTGAAAAGAGCCTAAGAAGAGGGGGCGTCTGGAAGGAACCGCAACGCCAAGGGAGGGTG