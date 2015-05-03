#!/usr/bin/perl
# -*- coding: UTF-8 -*-

use SparrowComp;

# Open the log file
open(LOG_FILE, ">& STDOUT") || die("Can't dup STDOUT: $!\n");

our $summary = "";

sub handleTestSuccess
{
    #my ($test) = @_;
    print "\n>>> OK\n";
    $summary .= ".";
}

sub handleTestFailure
{
    my ($test, $msg) = @_;
    my $knownError = false;
    if ( !$showKnownProblemsAsErrors && ($test =~ /.*KNOWN-PROBLEM.*/) )
    {
        print "\n>>> KNOWN-PROBLEM: $msg\n";
        $summary .= "K";
    }
    else
    {
        print "\n>>> FAILURE: $msg\n";
        $summary .= "E";
    }
    return 0;
}



die("Expected test filename as parameter!") if 0 == scalar @ARGV;

our $testFile = $ARGV[0];
compileFileRunTests($testFile, 1);
print "\n";
print "Summary: $summary\n";
print "\n";

close(LOG_FILE);
