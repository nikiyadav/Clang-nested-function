#!/bin/bash

for input in test_files/* ; do
    echo $input
    output=$(basename $input)
    echo $output
    touch output_files/$output
    clang-nested-func $input -- > output_files/$output
done