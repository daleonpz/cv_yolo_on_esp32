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

find . -type f -name "*.cc" -o -name "*.h" -not -path "./build/*" -not -path "./third-party/*" | while read -r file ; do
    echo "Formatting $file"
    clang-format -i --style=google $file
done

