#!/usr/bin/perl

my $start = 0;
my $end = 34;
my $endSwiUnoptimized = 34;
my $endSwiOptimized = 34;
my $endSparrowUnoptimized = 34;

my $timeCmd = "/usr/bin/time -l";

foreach my $file (glob "*.out")
{
   unlink $file or print "unable to delete $file\n";
}

# Generate the executables
print "Compiling SWI executables...\n";
foreach my $i ($start..$end)
{
    if ( $i <= $endSwiUnoptimized )
    {
        system("swipl --stand_alone=true --goal=\"fib($i, X), write(X), halt.\" -L2g -o fib-swi-$i.out -c ../Fibonacci.pl");
    }
    if ( $i <= $endSwiOptimized )
    {
        system("swipl --stand_alone=true --goal=\"fib($i, X), write(X), halt.\" -L10g -O -o fib-swi-opt-$i.out -c ../Fibonacci.pl");
    }
}
print "\n\n";
foreach my $i ($start..$endSparrowUnoptimized)
{
    print "Sparrow, $i, ";
    system("$timeCmd ../FibProlog_dyn.out $i");
}
print "\n\n";
foreach my $i ($start..$end)
{
    print "Sparrow_native, $i, ";
    system("$timeCmd ../FibProlog_native.out $i");
}
print "\n\n";
foreach my $i ($start..$end)
{
    print "C++, $i, ";
    system("$timeCmd cpp/a.out $i");
}
print "\n\n";
foreach my $i ($start..$endSwiUnoptimized)
{
    print "SWI_prolog, $i, ";
    system("$timeCmd ./fib-swi-$i.out");
}
print "\n\n";
foreach my $i ($start..$endSwiOptimized)
{
    print "SWI_prolog_opt, $i, ";
    system("$timeCmd ./fib-swi-opt-$i.out");
}
