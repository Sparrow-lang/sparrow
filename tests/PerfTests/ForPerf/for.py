#!/usr/bin/python

import sys;

if len(sys.argv) < 2:
    sys.exit(-1)
n = int(sys.argv[1])

res = 0
for i in range(0, n):
    for j in range(0, n):
        res += i*j
print res
