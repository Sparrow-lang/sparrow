#!/bin/bash

set -e
set -x

echo "Travis OS name: $TRAVIS_OS_NAME"
echo "Travis branch: $TRAVIS_BRANCH"
echo "Travis tag: $TRAVIS_TAG"
echo "LANG=$LANG"
echo "LC_ALL=$LC_ALL"

if [ $TRAVIS_OS_NAME == osx ]; then
    # brew update
    brew upgrade boost@1.60
    brew install llvm@5
fi
