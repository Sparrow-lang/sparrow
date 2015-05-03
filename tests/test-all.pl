#!/usr/bin/perl
# -*- coding: UTF-8 -*-

use SparrowComp;

my $filter = $ARGV[0];
my $showKnownProblemsAsErrors = 0;

# Perform initial cleanup
unlink("test-all.log");

# Open the log file
open(LOG_FILE, "> test-all.log") || die("Can't open 'test-all.log': $!\n");

my @failedTests;
my @fixedTests;
my $cntFailedTests = 0;
my $curDescription;

sub handleTestSuccess
{
    my ($test) = @_;
    if ( $test =~ /.*KNOWN-PROBLEM.*/ )
    {
        print "F";
        push(@fixedTests, "$curDescription / $test");
    }
    else
    {
        print ".";
    }
    print LOG_FILE "[ OK ]\n";
}

sub handleTestFailure
{
    my ($test, $msg) = @_;
    #print "\n *** FAILURE *** $test ($msg)";
    if ( !$showKnownProblemsAsErrors && ($test =~ /.*KNOWN-PROBLEM.*/) )
    {
        print LOG_FILE "[ KNOWN-PROBLEM ] - $msg\n";
        print 'K';
        return 1;
    }
    else
    {
        print LOG_FILE "[ FAIL ] - $msg\n";
        print 'E';
        push(@failedTests, "$curDescription / $test");
        return 0;
    }
}

sub handleFile
{
    my $filename = $_[0];
    my $description = $_[1];

    $curDescription = $description;
    
    printf "\n%-60s", "$description ";
    
    print LOG_FILE "\n\n\n";
    print LOG_FILE "======================================== $filename ========================================\n";
    print LOG_FILE "Description: $description\n";

    my $success = compileFileRunTests($filename);
    $cntFailedTests += 1 unless $success;
}



# Open the tests.in file, read the test files and handle the not-commented-out tests
my $testsNo = 0;
open(TESTS_FILE, "tests.in") || die("Can't open 'tests.in': $!\n");
my $testsInContent = join('', <TESTS_FILE>);
$testsInContent =~ s/\/\*.*?\*\///gs;
foreach my $line(split('\n', $testsInContent))
{
    next if $line eq "";
    next if $line =~ /^\/\//;
    
    #print $line, "\n";
    next if !($line =~ /^(.*?):(.*)$/);
    my $testFile = trim($1);
    my $testDescription = trim($2);

    # Run only the tests that contain the name passed as argument
    next if index($testFile, $filter) == -1;    
    
    handleFile($testFile, $testDescription);
    $testsNo += 1;
}
close(TESTS_FILE);

# Print the results
print "\n\n";
print LOG_FILE "\n\n";
if ( $cntFailedTests > 0 )
{
    print "Failed tests:\n";
    print LOG_FILE "Failed tests:\n";
    foreach my $name(@failedTests)
    {
        print "    - $name\n";
        print LOG_FILE "    - $name\n";
    }
}
else
{
    print "All tests passed sucessfully!\n";
    print LOG_FILE "All tests passed sucessfully!\n";
}

printf "\nResult: %d failures of %d => success rate = %.2f%%\n", $cntFailedTests, $testsNo, 100*($testsNo-$cntFailedTests)/$testsNo;
printf LOG_FILE "\nResult: %d failures of %d => success rate = %.2f%%\n", $cntFailedTests, $testsNo, 100*($testsNo-$cntFailedTests)/$testsNo;

# IF we have some fixed texts, print them
if ( scalar @fixedTests > 0 )
{
    print "\n\nFixed tests:\n";
    print LOG_FILE "\n\nFixed tests:\n";
    foreach my $name(@fixedTests)
    {
        print "    - $name\n";
        print LOG_FILE "    - $name\n";
    }
    print "\n";
}



close(LOG_FILE);
