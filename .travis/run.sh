#!/bin/bash

set -e
set -x

if [[ "$(uname -s)" == 'Darwin' ]]; then
    if which pyenv > /dev/null; then
        eval "$(pyenv init -)"
    fi
    pyenv activate conan
fi

echo "---------- Building ----------"

cd build
conan install .. --build=missing
cmake ..
cmake --build .
sudo cmake --build . -- install

echo "---------- Testing ----------"

cd tests
python test.py
