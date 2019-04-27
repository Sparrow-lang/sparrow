#!/bin/bash

set -e
set -x

CFG_PARAMS=

if [ $TRAVIS_OS_NAME == osx ]; then
    CFG_PARAMS="-DLLVM_DIR=/usr/local/opt/llvm@7/lib/cmake/llvm"
fi

if [ $TRAVIS_OS_NAME == linux ]; then
    export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/usr/lib/x86_64-linux-gnu:/usr/lib/llvm-5.0/lib/
fi

cd build
cmake .. $CFG_PARAMS
make
sudo make install

echo "---------- Unit testing ----------"

bin/SparrowUnitTests

echo "---------- Testing ----------"

cd ../tests

python test.py StdLib/RangesTest.spr --returnError
python test.py parserTest --returnError
python test.py --returnError
