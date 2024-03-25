#!/bin/bash
set -e

ARGUMENTS=("$@")

git submodule update --init --recursive

mkdir -p build
cd build
cmake ../ "${ARGUMENTS[@]}"
cmake --open .
cd ../