#!/usr/bin/perl
# -*- coding: UTF-8 -*-

my $osname = $^O;

our $curDir = "";
if ( $osname eq 'MSWin32' )
{
    $curDir = `cd`;

}
else
{
    $curDir = `pwd`;
}
chomp($curDir);
our $executablePath = findExecutablePath();
#print "Executable path: $executablePath\n";
our $sparrowCompiler = "$executablePath/SparrowCompiler.exe";
our $lliProgram = "$executablePath/llvm/lli.exe";
our $outExt = "exe";
our $compilerParams = "-v";
our $useSimpleLinkinigBelowNRunTests = 1;       # Pass here a large number to always use simple linking, -1 to disable simple linking
our $debugOutput = hasArgument('--debug');
our $captureTime = hasArgument('--time');



if ( $osname eq 'darwin' )
{
    $outExt = "out";
    $sparrowCompiler = $executablePath . '/SparrowCompiler';
    $lliProgram = $executablePath . '/llvm/lli';
    local $ENV{PATH} = "$ENV{PATH}:/Users/lucteo/MyWork/Sparrow_SVN/trunk/sc2/dev/.out/Debug/llvm";
}

sub hasArgument
{
    my $argName = $_[0];
    foreach my $arg (@ARGV)
    {
        if ( $arg eq $argName )
        {
            return 1;
        }
    }
    return 0;
}

#system('call "%VS100COMNTOOLS%/vsvars32.bat"');
#system('call set');

# Get the path of the executable (Debug, Release, ...) based on which is newer
sub findExecutablePath
{
    my $basePath = "../.out/";
    my @modes = ('Release', 'MinSizeRel', 'RelWithDebInfo', 'Debug');
    my $bestPath = $basePath."Release";
    my $bestTime = 10000;
    foreach my $mode(@modes)
    {
        my $curPath = $basePath.$mode;
        next unless -d $curPath;
        my $curTime = -M $curPath;
        next unless $curTime < $bestTime;

        $bestPath = $curPath;
        $bestTime = $curTime;
    }
    return "$curDir/$bestPath";
}

# Removes the spaces at the begin and end of the string given as parameter
sub trim($)
{
    my $string = shift;
    $string =~ s/^\s+//;
    $string =~ s/\s+$//;
    return $string;
}

sub removeColors
{
    my $string = shift;
    $string =~ s/\x{1b}\[(\d+\;)*\d+m//g;
    return $string;
}

sub getOutputFilename
{
    my $filename = $_[0];

    my $outputFile = $filename;
    $outputFile =~ s/\.spr$/\.$outExt/;
    $outputFile =~ s/\.llvm$/\.$outExt/;
    $outputFile =~ s/\.test$/\.$outExt/;
    $outputFile =~ s/\.fsimple$/\.$outExt/;
    
    return $outputFile;
}

# Get the compilation parameters from the source (parse the source file)
sub getCompilationParametersFromSource
{
    my $filename = $_[0];
    
    open(F, $filename) || die("Can't open '$filename': $!\n");
    foreach my $line(<F>)
    {
        if ( $line =~ /\/\/\!\!(.*)$/g )
        {
            my $res = $1;
            $res =~ s/\r//g;
            $res =~ s/\n//g;
            close(F);
            return $res;
        }
    }
    close(F);
    
    return "";
}

# Get the error markers from the source filename (parse the source file)
sub getSourceFileErrorMarkers
{
    my $filename = $_[0];
    my @errorMarkers = ();
    
    open(F, $filename) || die("Can't open '$filename': $!\n");
    my $lineNo=1;
    foreach my $line(<F>)
    {
        while ( $line =~ /ERROR(\((.*)\))?/g )
        {
            #print "Error marker: $lineNo - '$2'\n";
            my @arr = ($lineNo, $2);
            push(@errorMarkers, \@arr);
        }
        
        $lineNo += 1;
    }
    close(F);
    
    return @errorMarkers;
}

# Given the text returned by compilation, get the actual compilation errors
sub getCompilationErrors
{
    my $result = $_[0];
    my @errors = ();
   
    while ( $result =~ /.*\((\d*):(.*)\) : ERROR : (.*)/g )
    {
        # print "Compilation error; line=$1, text: $3\n";
        my @arr = ($1, $3);
        push(@errors, \@arr);
    }
    
    return @errors;
}

# Get from the source filenames the tests that need to be executed after compiling the source file
# Returns an array of (test_name, params, expected_output)
sub getSourceFileRunTests
{
    my $filename = $_[0];
    my @runTests = ();
    
    open(F, $filename) || die("Can't open '$filename': $!\n");
    my $content = join('', <F>);
    while ( $content =~ /\/\*<<<(.*?)\((.*?)\)(\r\n?|\n)((.|\r\n?|\n)*?)>>>\*\//g )
    {
        #print "Test name: '$1'; params: '$2'; content: '$4'\n";
        my @arr = ($1, $2, $4);
        push(@runTests, \@arr);
    }
    close(F);
    
    return @runTests;
}

# Compare the two strings, ignoring the end-of-line differences
sub compareStringsNormalized
{
    my ($str1, $str2) = @_;
    
    $str1 =~ s/\r\n/\n/g;
    $str1 =~ s/\r/\n/g;
    $str2 =~ s/\r\n/\n/g;
    $str2 =~ s/\r/\n/g;
    
    return $str1 eq $str2;
}

# Try to compile the given file and run the corresponding tests
# Returns false if something fails
# Requires handleTestSuccess(test) and handleTestFailure(test, msg) functions to be defined
sub compileFileRunTests
{
    my $filename = $_[0];
    my $consolePrint = (defined $_[1] ? $_[1] : 0);
    
    # Get the expected error markers
    my @expectedErrors = getSourceFileErrorMarkers($filename);
    
    my @runTests = getSourceFileRunTests($filename);

    # Get the compilation flags
    my $compFlags = getCompilationParametersFromSource($filename);
    if ( !($compFlags =~ /-t/ ) )
    {
        $compFlags .= " ".$compilerParams;
    }
    
    # Perform simple linking if there aren't many tests to run
    $compFlags .= ' --simple-linking' if $debugOutput==0 and $#runTests < $useSimpleLinkinigBelowNRunTests;

    print LOG_FILE "\nFlags: $compFlags\n";

    
    #Split filename into directory + file
    my $dir = ".";
    if ( $filename =~ /(.*)[\/\\](.*)/ )
    {
        $dir = $1;
        $filename = $2;
    }

    my $outputFile = getOutputFilename($filename);

    # Remove the output files
    #print LOG_FILE "Removing output file: $outputFile (+others)... ";
    unlink("$dir/$outputFile");
    unlink("$dir/$outputFile.bc");
    unlink("$dir/$outputFile.o");
    unlink("$dir/$outputFile.one.bc");
    unlink("$dir/$outputFile.one.llvm");
    unlink("$dir/$outputFile.ct.llvm");
    
    # Perform the compilation
    print LOG_FILE "\n\n";
    print LOG_FILE "\n------ Compilation\n";
    my $compOutput = "";
    if ( $consolePrint and not (@expectedErrors) )
    {
        system("cd $dir && \"$sparrowCompiler\" $filename -o $dir/$outputFile $compFlags");
        # Warning: We are not obtaining the compiler output - we do this to preserve the coloring of the output
    }
    else
    {
        $compOutput = `cd $dir && "$sparrowCompiler" $filename -o $dir/$outputFile $compFlags 2>&1`;
        print LOG_FILE $compOutput;
        $compOutput = removeColors($compOutput);
    }

    # Check the errors
    my @actualErrors = getCompilationErrors($compOutput);
    @actualErrors = sort {$a->[0] <=> $b->[0]} @actualErrors;
    print LOG_FILE "\n\n";
    print LOG_FILE "Expected errors: ", join(',', map {$_->[0]} @expectedErrors), "\n";
    print LOG_FILE "Actual errors:   ", join(',', map {$_->[0]} @actualErrors), "\n";
    if ( $#actualErrors != $#expectedErrors )
    {
        return handleTestFailure("compilation", "Different number of errors");
    }
    foreach my $i(0..$#expectedErrors)
    {
        if ( $expectedErrors[$i]->[0] != $actualErrors[$i]->[0] )
        {
            return handleTestFailure("compilation", "Different error lines: ".$expectedErrors[$i]->[0]." != ".$actualErrors[$i]->[0]);
        }
        if ( $expectedErrors[$i]->[1] ne "" && $expectedErrors[$i]->[1] ne $actualErrors[$i]->[1] )
        {
            return handleTestFailure("compilation", "Different error messages: ".$expectedErrors[$i]->[1]." != ".$actualErrors[$i]->[1]);
        }
    }
    if ( $#expectedErrors < 0 )
    {
        # Make sure the output file exists
        if ( !-e "$dir/$outputFile" and !-e "$dir/$outputFile.bc" )
        {
            return handleTestFailure("compilation", "Output file not found");
        }
    }
    handleTestSuccess("compilation");
    
    # Check the run tests
    my $hasRunErrors = 0;
    my $progToRun = "./$outputFile";
    if ( !(-e "$dir/$outputFile") and -e "$dir/$outputFile.bc" )
    {
        # If the output is a ".bc" file, interpret it with lli
        $progToRun = $lliProgram." ".$outputFile.".bc";
    }
    foreach my $tst(@runTests)
    {
        my $name = $tst->[0];
        my $params = $tst->[1];
        my $expectedResult = $tst->[2];
        
        print LOG_FILE "\n------ Running test: $name($params)\n";
        print LOG_FILE "Expected result:\n$expectedResult.\n";
        
        # Run the test
        my $toRun = $progToRun;
        $toRun = "cd $dir && $progToRun $params"      if !$captureTime;
        $toRun = "cd $dir && time $progToRun $params" if $captureTime;
        print LOG_FILE "Running: '$toRun':\n";
        my $res = "";
        if ( $debugOutput )
        {
            system($toRun);
        }
        else
        {
            $res = `$toRun`;
        }
        print LOG_FILE "$res.\n";
        
        if ( !compareStringsNormalized($res, $expectedResult) )
        {
            $skipError = handleTestFailure($name, "Different running output");
            $hasRunErrors = 1 unless $skipError;
            next;
        }
        
        handleTestSuccess($name);
    }

    return 0 == $hasRunErrors;
}




1;