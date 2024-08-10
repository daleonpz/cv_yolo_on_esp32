#!/bin/bash

# This script formats the code in the repository using clang-format.

# Check if clang-format is installed
if ! command -v clang-format &> /dev/null
then
    echo "clang-format could not be found"
    exit
fi

# Find all the C++ files in the repository
# Exclude the build directory
# Exclude the third-party directory
# Exclude the managed_components directory

find . \( -type d \( -name 'build' -o -name 'managed_components' -o -name 'third_party' \) -prune \) -o -type f -name '*.cc' -o -name '*.h' | while read -r file; do
    echo "Formatting $file"
    clang-format -i --style=google $file
done

