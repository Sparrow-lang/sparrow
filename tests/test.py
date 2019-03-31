#!/usr/bin/env python

from __future__ import print_function

import sys, os, subprocess, distutils.spawn, argparse, re


class CompilerLookup:
    ''' Helper class that resolves the paths needed to run the compiler '''

    def __init__(self, compilerArg, lliArg):
        self.compiler = self._findCompiler(compilerArg)
        self.lli = self._findLli(lliArg, self.compiler)

    def getCompiler(self):
        return self.compiler

    def getLli(self):
        return self.lli

    def _findCompiler(self, compilerArg):
        # If compiler is manually specified, use that one
        if compilerArg != 'auto':
            return os.path.abspath(compilerArg)

        # Try different alternatives; pick the most recent one
        alt = [
            '../build/bin/SparrowCompiler',         # In the case we are running from the 'tests' directory
            distutils.spawn.find_executable('SparrowCompiler')  # Search globally
        ]

        # filter out files that do not exist
        alt = [f for f in alt if f and os.path.isfile(f)]

        # Make sure we have at least one entry
        if len(alt) == 0:
            return ''

        # Get the one that has the latest modification date
        alt.sort(key=lambda f: os.path.getmtime(f), reverse=True)

        # Return the absolute path of it
        return os.path.abspath(alt[0]);

    def _findLli(self, lliArg, compiler):
        # If lli is manually specified, use that one
        if lliArg != 'auto':
            return os.path.abspath(lliArg)

        # else, use the lli near the compiler
        return os.path.dirname(compiler) + '/spr-lli'


def getTestFiles(toRun, testsFile):
    ''' Gets the list of test files to be run.
        Returns a list of pair (filename, name).
    '''

    # Check if the 'toRun' indicate to one file
    if toRun and os.path.isfile(toRun):
        # If yes, just return this file
        return [(toRun, toRun)]
    else:
        # Otherwise try to read the tests from the given test file
        # Keep only the tests that match our filter
        res = []

        for line in open(testsFile):
            line = line.strip()

            # Skip empty or comment lines
            if line == '' or line.startswith('//') or line.startswith('#'):
                continue

            # Split the line into filename and test-name
            m = re.match(r'^(.*?):(.*)$', line)
            if not m:
                print('WARNING: Invalid test line: "%s"' % line)
                continue
            filename = m.group(1).strip()
            testName = m.group(2).strip()

            # Apply filter
            if toRun and toRun not in filename:
                continue

            p = (filename, testName)
            res.append(p)

        return res

class SourceFileInfo:
    ''' Class that represents all the information from a source file relevant to testing that file '''
    filename = ''
    content = []        # Array of strings, one item for each line
    errorMarkers = []   # List of line numbers
    runTests = []       # List of triples (test-name, args, expected-output)
    extraArgs = ''

    def __init__(self, filename):
        # Read the content of the file
        self.filename = filename
        with open(filename) as f:
            self.content = f.readlines()

        self._getErrorMarkers()
        self._getRunTests()
        self._getExtraArgs()

    def _getErrorMarkers(self):
        self.errorMarkers = []
        lineIdx = 0
        for line in self.content:
            line = line.strip()
            lineIdx +=1

            # Ignore all the lines with 'IGNORE-ERROR'
            if 'IGNORE-ERROR' in line:
                continue

            for m in re.finditer(r'ERROR', line):
                self.errorMarkers.append(lineIdx)

    def _getRunTests(self):
        self.runTests = []
        allFileContent = ''.join(self.content)
        for m in re.finditer(r'\/\*<<<(.*?)(\((.*?)\))?(\r\n?|\n)((.|\r\n?|\n)*?)>>>\*\/', allFileContent):
            testName = m.group(1)
            args = m.group(3)
            expectedOutput = m.group(5)
            if not args:
                args = ''
            testTuple = (testName, args, expectedOutput)
            self.runTests.append(testTuple)
        for m in re.finditer(r'\/\/<<<(.*?)(\((.*?)\))?(\r\n?|\n)', allFileContent):
            testName = m.group(1)
            args = m.group(3)
            if not args:
                args = ''
            expectedOutput = None
            testTuple = (testName, args, expectedOutput)
            self.runTests.append(testTuple)

    def _getExtraArgs(self):
        self.extraArgs = ''
        for line in self.content:
            m = re.match(r'\/\/\!\!(.*)$', line)
            if m:
                self.extraArgs = m.group(1).strip()
                return

def getCompilationErrors(compilerOutput):
    ''' Get the list of errors from compiler output (list of line numbers) '''
    errorLines = []
    for line in compilerOutput.splitlines():
        m = re.match(r'.*\((\d*):(.*)\) : ERROR : (.*)', line)
        if m:
            errorLines.append(int(m.group(1)))
    return errorLines

def getCharCodeForTestRun(testName, runOk):
    knownProblem = 'KNOWN-PROBLEM' in testName
    if runOk:
        if knownProblem:
            return 'F'
        else:
            return '.'
    elif knownProblem:
        return 'K'
    else:
        return 'E'


class SummaryReporter:
    ''' Reporter that reports just the summary '''

    _curTest = None
    _curTestOk = True
    _numFailedTests = 0
    _numTests = 0
    _failedTests = []
    _fixedTests = []

    def requireCaptureCompilerOutput(self):
        return True     # Make sure it's not printed

    def onStartFile(self, filename, testName):
        self._curTest = testName
        print('%-60s' % testName, end='')

    def onStartCompiling(self, cmd):
        self._curTestOk = True

    def onCompilerOutput(self, compilerOutput):
        pass
    def onCompilationStatus(self, expectedErrors, actualErrors, compilationOk):
        if compilationOk:
            print('.', end='')
        else:
            print('E', end='')
            self._recordFailure('Compilation')

    def beforeTestRun(self, testName, args, expectedOutput, cmd):
        pass
    def afterTestRun(self, testName, actualOutput, expectedOutput, runOk):
        ch = getCharCodeForTestRun(testName, runOk)
        print(ch, end='')
        if ch == 'E':
            self._recordFailure(testName)
        elif ch == 'F':
            self._recordFixedTest(testName)


    def onEndFile(self, filename, testName):
        print()
        self._numTests += 1

    def onFinish(self):
        print()
        if self._numTests > 0:
            if self._numFailedTests == 0:
                print('All tests passed successfully.')
                print()

            if len(self._fixedTests) > 0:
                print('Fixed tests:')
                for f in self._fixedTests:
                    print('    - ' + f)
                print()

            if len(self._failedTests) > 0:
                print('Failed tests:')
                for f in self._failedTests:
                    print('    - ' + f)
                print()
                percent = 100.0 * (self._numTests - self._numFailedTests) / self._numTests
                print('Result: %d failures of %d => success rate = %.2f%%' % ( self._numFailedTests, self._numTests, percent))
                print()

        return self._numFailedTests

    def _recordFailure(self, runTest):
        if self._curTestOk:
            self._numFailedTests += 1
        self._curTestOk = False
        self._failedTests.append(self._curTest + ' / ' + runTest)
    def _recordFixedTest(self, runTest):
        self._fixedTests.append(self._curTest + ' / ' + runTest)


class DetailedReporter:
    ''' Detailed reporter of all the actions performed '''

    _curTestSummary = ''
    _curTestHasErrors = False
    _numErrors = 0

    def requireCaptureCompilerOutput(self):
        return False

    def onStartFile(self, filename, testName):
        self._curTestSummary = ''
        self._curTestHasErrors = False
        pass

    def onStartCompiling(self, cmd):
        print()
        print('Command to run: %s' % cmd)
        print()
        print('------ Compilation')

    def onCompilerOutput(self, compilerOutput):
        print(compilerOutput)

    def onCompilationStatus(self, expectedErrors, actualErrors, compilationOk):
        print()
        print('Expected errors: ', expectedErrors)
        print('Actual errors:   ', actualErrors)
        if expectedErrors != actualErrors:
            print('MISMATCH\n')
        elif not compilationOk:
            print('ERROR: output not found\n')
            self._curTestSummary += 'E'
            self._curTestHasErrors = True
        else:
            print()
            print('>>> OK')
            self._curTestSummary += '.'

    def beforeTestRun(self, testName, args, expectedOutput, cmd):
        print()
        print('------ Running test: %s(%s)' % (testName, args))
        if expectedOutput != None:
            print('Expected result:')
            print(expectedOutput + '.')
        print('Running: \'%s\':' % cmd)

    def afterTestRun(self, testName, actualOutput, expectedOutput, runOk):
        print(actualOutput + '.')
        if runOk:
            print()
            print('>>> OK')
        else:
            if expectedOutput != None:
                print('ERROR: output does not match!')
            else:
                print('ERROR: output contains errors!')
            self._curTestHasErrors = True
        self._curTestSummary += getCharCodeForTestRun(testName, runOk)

    def onEndFile(self, filename, testName):
        print()
        print('Summary: ' + self._curTestSummary)
        print()
        if self._curTestHasErrors:
            self._numErrors += 1

    def onFinish(self):
        return self._numErrors


def doTestFile(testFilePair, reporter, args, compilerLookup):
    ''' Called to execute the tests corresponding to a file '''

    (filename, testName) = testFilePair
    reporter.onStartFile(filename, testName)

    # Get the info from the source file
    fileInfo = SourceFileInfo(filename)

    # Use simple linking?
    useSimpleLinking = not args.debug and len(fileInfo.runTests) <= args.simpleLinkLimit

    # Split filename into directory + file
    path, file = os.path.split(filename)

    # Get the output file that should be generated by the compiler
    outputFile = os.path.splitext(file)[0] + args.outExt
    if useSimpleLinking:
        outputFile += '.bc'

    # Remove the existing output & temporary files
    def _removeFileIfExists(filename):
        if os.path.isfile(filename):
            os.remove(filename)
    _removeFileIfExists('%s/%s' % (path, outputFile))
    _removeFileIfExists('%s/%s.bc' % (path, outputFile))
    _removeFileIfExists('%s/%s.o' % (path, outputFile))
    _removeFileIfExists('%s/%s.ll' % (path, outputFile))
    _removeFileIfExists('%s/%s.one.bc' % (path, outputFile))
    _removeFileIfExists('%s/%s.one.llvm' % (path, outputFile))
    _removeFileIfExists('%s/%s.one.ll' % (path, outputFile))
    _removeFileIfExists('%s/%s.ct.llvm' % (path, outputFile))
    _removeFileIfExists('%s/%s.ct.ll' % (path, outputFile))

    # Should we capture the output?
    captureOutput = (not args.debug and fileInfo.errorMarkers) or reporter.requireCaptureCompilerOutput()

    # Compute the command line
    changeDirPrefix = 'cd %s && ' % path
    # changeDirPrefix = ''
    compilerArgs = ' ' + args.compilerArgs + ' ' + fileInfo.extraArgs
    if useSimpleLinking:
        compilerArgs += ' -S'
    if captureOutput:
        compilerArgs += ' -fno-colors'
    cmd = changeDirPrefix + compilerLookup.getCompiler() + ' ' + file + ' -o ' + outputFile + compilerArgs

    # Actually run the compiler
    reporter.onStartCompiling(cmd)
    retCode = 0
    actualErrors = []
    if captureOutput:
        cmd += ' 2>&1'
        p = subprocess.Popen(cmd, shell=True, stdout=subprocess.PIPE)
        compilerOutput = p.communicate()[0]
        retCode = p.returncode
        reporter.onCompilerOutput(compilerOutput)
    else:
        retCode = os.system(cmd)
        compilerOutput = ''


    # Check the errors & execution status
    actualErrors = getCompilationErrors(compilerOutput)
    actualErrors.sort()
    compilationOk = actualErrors == fileInfo.errorMarkers
    # Check if output file is produced (according to our error makers)
    outputIsPresent = os.path.isfile('%s/%s' % (path, outputFile))
    compilationOk = compilationOk and outputIsPresent == (not fileInfo.errorMarkers)

    reporter.onCompilationStatus(fileInfo.errorMarkers, actualErrors, compilationOk)

    # Run the tests
    if outputIsPresent:
        programToRun = './' + outputFile
        if useSimpleLinking:
            programToRun = compilerLookup.getLli() + ' ' + outputFile

        for t in fileInfo.runTests:
            (testName, testArgs, expectedOutput) = t
            cmd = changeDirPrefix + programToRun + ' ' + testArgs

            reporter.beforeTestRun(testName, testArgs, expectedOutput, cmd)

            # Run the test
            if not args.debug:
                cmd += ' 2>&1'
                p = subprocess.Popen(cmd, shell=True, stdout=subprocess.PIPE)
                actualOutput = p.communicate()[0]

                if expectedOutput != None:
                    runOk = actualOutput == expectedOutput
                else:
                    runOk = not('FAILURE' in actualOutput or 'ERROR' in actualOutput)
                reporter.afterTestRun(testName, actualOutput, expectedOutput, runOk)
            else:
                os.system(cmd)

    reporter.onEndFile(filename, testName)

def main():
    # Make sure to provide a default out extension
    defaultOutExt = '.exe' if sys.platform == 'win32' else '.out'

    # Parse the arguments
    parser = argparse.ArgumentParser(description='Runs the tests found at the given path (one or multiple tests)')
    parser.add_argument('toRun', action='store', nargs='?',
        help='What to run; a file or a directory filter')
    parser.add_argument('--testsFile', action='store', default='tests.in',
        help='The file that describes all the tests that we have')
    parser.add_argument('--compilerArgs', '-c', action='store', default='-v',
        help='Arguments to be passed directly to the compiler')
    parser.add_argument('--outExt', action='store', default=defaultOutExt,
        help='The extension used for the compiled file')
    parser.add_argument('--summary', choices=['never', 'multiple', 'always'], default='multiple',
        help='When to show only summary; \'multiple\' = show summary when more then one test is selected (default)')
    parser.add_argument('--time', action='store_true',
        help='Measures the execution time of the test')
    parser.add_argument('--debug', action='store_true',
        help='Runs the test in debug mode, without capturing its output')
    parser.add_argument('--simpleLinkLimit', action='store', type=int, default=0, metavar='N',
        help='Max num of tests per file needed for which we use simple linking')
    parser.add_argument('--compilerProg', action='store', default='auto', metavar='P',
        help='Path to the SparrowCompiler program, or auto')
    parser.add_argument('--lliProg', action='store', default='auto', metavar='P',
        help='Path to the lli program, or auto')
    parser.add_argument('--returnError', action='store_true',
        help='If errors, return a non-zero error code')
    args = parser.parse_args()

    compilerLookup = CompilerLookup(args.compilerProg, args.lliProg)

    # Get all the tests in the tests file
    tests = getTestFiles(args.toRun, args.testsFile)
    if not tests:
        print('WARNING: No test selected to run')
        return

    # Select the right reporter to be used
    reporter = None
    if args.summary == 'always' or (args.summary == 'multiple' and len(tests) > 1):
        reporter = SummaryReporter()
    else:
        reporter = DetailedReporter()

    # Now run all the tests
    for t in tests:
        doTestFile(t, reporter, args, compilerLookup)

    numErrors = reporter.onFinish()
    if args.returnError:
        sys.exit(numErrors)

if __name__ == "__main__":
    main()


