#!/usr/bin/env python

from __future__ import print_function

import sys, os, subprocess, distutils.spawn, argparse, re

def main():
    # Parse the arguments
    parser = argparse.ArgumentParser(description='Formats all the Sparrow files found in a directory')
    parser.add_argument('--dir', action='store', default='.',
        help='The directory to search for Sparrow source files')
    parser.add_argument('--formatArgs', '-a', action='store', default='-format -i',
        help='Formatting arguments')
    parser.add_argument('--formatTool', action='store',
        help='The path to formatTool')
    args = parser.parse_args()

    formatTool = args.formatTool
    if not formatTool:
        baseDir = os.path.os.path.dirname(os.path.realpath(__file__))
        formatTool = os.path.join(baseDir, 'formatTool.out')
        print('formatTool: {}'.format(formatTool))

    # Walk all the files/directories in the given
    for dirName, subdirList, fileList in os.walk(args.dir):
        for fname in fileList:
            if fname.endswith('.spr'):
                fullFilename = os.path.join(dirName, fname)
                cmd = '{} {} {}'.format(formatTool, args.formatArgs, fullFilename)
                print('> {}'.format(cmd))
                os.system(cmd)

if __name__ == "__main__":
    main()

