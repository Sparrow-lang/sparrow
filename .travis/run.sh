#!/bin/bash

set -e
set -x

if [[ "$(uname -s)" == 'Darwin' ]]; then
    if which pyenv > /dev/null; then
        eval "$(pyenv init -)"
    fi
    pyenv activate conan
fi

cd build
conan install .. --build=missing
cmake ..
cmake --build .
sudo cmake --build . -- install

echo "---------- Testing ----------"

cd ../tests
id
ls -lsa /usr/local/bin/llvm/
ls -lsa StdLib/
/usr/local/bin/llvm/llc --filetype=obj --help
python test.py StdLib/RangesTest.spr
/usr/local/bin/llvm/llc --filetype=obj StdLib/RangesTest.out.bc -o StdLib/RangesTest.out.o
python test.py
