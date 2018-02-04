#!/usr/bin/env python

from __future__ import print_function

import sys, os, subprocess, argparse, re, time, datetime
from email.Utils import formatdate

from test import CompilerLookup

# Configuration settings
tests = [
    ('Comp.Ranges', 'StdLib/RangesTest.spr'),
    ('Comp.LoopTesterApp', 'PerfTests/GoogleBench/LoopTesterApp.spr'),

    ('Exec.BenchmarkGame.nbody', 'BenchmarkGame/nbody.spr', '5000000'),
    ('Exec.BenchmarkGame.fasta', 'BenchmarkGame/fasta.spr', '2500000'),
    ('Exec.BenchmarkGame.fastaredux', 'BenchmarkGame/fastaredux.spr', '2500000'),
    ('Exec.BenchmarkGame.meteor', 'BenchmarkGame/meteor.spr', '2100'),
    ('Exec.BenchmarkGame.fannkuchredux', 'BenchmarkGame/fannkuchredux.spr', '11'),
    ('Exec.Examples.LoopTesterApp', 'PerfTests/GoogleBench/LoopTesterApp.spr', '2'),
    ('Exec.Examples.FibRange', 'PerfTests/FibRanges/fib_op.spr', '50000'),
    ('Exec.Examples.Collatz', 'PerfTests/LazyRanges/lazy_ranges.spr', '600'),
]

re_TotalTime = re.compile(r'#TotalTime: (\d+)\n')
re_ImplicitLibTime = re.compile(r'#ImplicitLibTime: (\d+)\n')
re_LlcTime = re.compile(r'#LlcTime: (\d+)\n')
re_OptTime = re.compile(r'#OptTime: (\d+)\n')
re_FinalLinkTime = re.compile(r'#FinalLinkTime: (\d+)\n')
re_NumCtEvals = re.compile(r'#NumCtEvals: (\d+)\n')
re_CtEvalsTime = re.compile(r'#CtEvalsTime: (\d+)\n')

def testCompilePerf(mKey, filename, args, compilerLookup):
    resDict = {
        'TotalTime': [],
        'ImplicitLibTime': [],
        'LlcTime': [],
        'OptTime': [],
        'FinalLinkTime': [],
        'NumCtEvals': [],
        'CtEvalsTime': []
    }

    filename = os.path.abspath(filename)
    path, file = os.path.split(filename)
    outputFile = os.path.splitext(file)[0] + args.outExt

    # Remove the output file if we have it
    outputFileAbs = '%s/%s' % (path, outputFile)
    if not os.path.isfile(outputFileAbs):
        os.remove(outputFileAbs)

    # Compute the command line
    changeDirPrefix = 'cd %s && ' % path
    compilerArgs = ' -fno-colors -dump-compile-stats ' + args.compilerArgs
    cmd = changeDirPrefix + compilerLookup.getCompiler() + ' ' + file + ' -o ' + outputFile + compilerArgs
    cmd += ' 2>&1'

    # Run all the measurements
    for i in range(args.numRepeats):
        print(cmd, file=sys.stderr)
        p = subprocess.Popen(cmd, shell=True, stdout=subprocess.PIPE)
        compilerOutput = p.communicate()[0]
        retCode = p.returncode

        if retCode != 0:
            print('Return code:', retCode, file=sys.stderr)
            sys.exit(retCode)

        print(compilerOutput, file=sys.stderr)
        resDict['TotalTime'].append(int(re_TotalTime.search(compilerOutput).group(1)))
        resDict['ImplicitLibTime'].append(int(re_ImplicitLibTime.search(compilerOutput).group(1)))
        resDict['LlcTime'].append(int(re_LlcTime.search(compilerOutput).group(1)))
        resDict['OptTime'].append(int(re_OptTime.search(compilerOutput).group(1)))
        resDict['FinalLinkTime'].append(int(re_FinalLinkTime.search(compilerOutput).group(1)))
        resDict['NumCtEvals'].append(int(re_NumCtEvals.search(compilerOutput).group(1)))
        resDict['CtEvalsTime'].append(int(re_CtEvalsTime.search(compilerOutput).group(1)))

    # Return the measurements strings
    res = []
    for key, values in resDict.iteritems():
        outLine = '  ' + mKey + '.' + key + ': ' + str(values)
        res.append(outLine)
    return res

def testRuntimePerf(mKey, filename, runArgs, args, compilerLookup):
    filename = os.path.abspath(filename)
    path, file = os.path.split(filename)
    outputFile = os.path.splitext(file)[0] + args.outExt

    # Compute the command line for compiling
    changeDirPrefix = 'cd %s && ' % path
    compilerArgs = ' -fno-colors ' + args.compilerArgs
    cmd = changeDirPrefix + compilerLookup.getCompiler() + ' ' + file + ' -o ' + outputFile + compilerArgs

    # Do the compiling
    print(cmd, file=sys.stderr)
    p = subprocess.Popen(cmd, shell=True, stdout=subprocess.PIPE)
    compilerOutput = p.communicate()[0]
    print(compilerOutput, file=sys.stderr)
    retCode = p.returncode

    if retCode != 0:
        print('Return code:', retCode, file=sys.stderr)
        sys.exit(retCode)

    # Ensure output is present
    programToCall = '%s/%s' % (path, outputFile)
    if not os.path.isfile(programToCall):
        print('ERROR: Output file cannot be found!', file=sys.stderr)
        sys.exit(1)

    # The command to be called
    cmd = changeDirPrefix + './' + outputFile + ' ' + runArgs + " > /dev/null"

    res = []

    # Run all the measurements with the generated output
    for i in range(args.numRepeats):
        print(cmd, file=sys.stderr)
        startTime = time.time()
        retCode = subprocess.call(cmd, shell=True)
        endTime = time.time()
        timeDiffMs = int((endTime-startTime)*1000)
        print('Execution time: %s ms' % timeDiffMs, file=sys.stderr)
        res.append(timeDiffMs);

        if retCode != 0:
            print('Return code:', retCode, file=sys.stderr)
            sys.exit(retCode)

    # Return the measurement
    return [ '  ' + mKey + ': ' + str(res) ]

def getGitInfo(buildName):
    lines = []

    # If no buildName is given, take the short SHA
    if not buildName:
        buildName = subprocess.check_output(['git', 'show', '--format=%h']).rstrip()

    # Build name and current date
    lines.append('info:')
    lines.append('  name: "' + buildName + '"')
    lines.append('  date: ' + formatdate(time.time(), True))

    # GIT branch
    gitBranch = subprocess.check_output(['git', 'rev-parse', '--abbrev-ref', 'HEAD'])
    lines.append('  branch: ' + gitBranch.rstrip())

    # Get the parent commits
    parents = subprocess.check_output(['git', 'show', '--format=%p']).rstrip().split(' ')
    lines.append('  parents: ' + str(parents))

    # Get most GIT params
    fmtParam = '--format=  sha: %h%n  sha-long: %H%n  commit-author: %an%n  commit-date: %cD%n  commit-message: "%s"'
    gitInfo = subprocess.check_output(['git', 'show', fmtParam])
    lines.append(gitInfo.rstrip())
    return lines;

def main():
    # Make sure to provide a default out extension
    defaultOutExt = '.exe' if sys.platform == 'win32' else '.out'

    # Parse the arguments
    parser = argparse.ArgumentParser(description='Runs performance tests')
    parser.add_argument('buildName', action='store', nargs='?',
        help='The name of the build')
    parser.add_argument('--numRepeats', '-r', action='store', default='7', metavar='N', type=int,
        help='Number of repetitions we shall have')
    parser.add_argument('--compilerArgs', '-c', action='store', default='-O2',
        help='Arguments to be passed directly to the compiler')
    parser.add_argument('--outExt', action='store', default=defaultOutExt,
        help='The extension used for the compiled file')
    parser.add_argument('--compilerProg', action='store', default='auto', metavar='P',
        help='Path to the SparrowCompiler program, or auto')
    parser.add_argument('--lliProg', action='store', default='auto', metavar='P',
        help='Path to the lli program, or auto')
    args = parser.parse_args()

    compilerLookup = CompilerLookup(args.compilerProg, args.lliProg)

    # First, get the git-info data
    resLines = getGitInfo(args.buildName)

    # Run each test
    resLines.append('')
    resLines.append('measurements:')
    for test in tests:
        mKey = test[0]
        filename = test[1]
        if mKey.startswith('Comp.'):
            resLines.extend(testCompilePerf(mKey, filename, args, compilerLookup))
        else:
            runArgs = test[2]
            resLines.extend(testRuntimePerf(mKey, filename, runArgs, args, compilerLookup))

    print('\n\n', file=sys.stderr)
    # Print the results
    for line in resLines:
        print(line)

if __name__ == "__main__":
    main()
