#!/usr/local/bin/perl -w

#use lib "$ENV{HOME}/perl/L";
#use LibSBML;

my $steps = 0;
my $flag6 = 0;
my $error = "";
my $test = "";

($test = $ARGV[1]) =~ s/(-l2)?\.xml/\.test/g;

open(LOGFILE, ">>testlog.txt");
print LOGFILE "\nTEST $test\n";

print(LOGFILE "odeSolver $ARGV[1] --time=$ARGV[2] --printstep=$ARGV[3] --error=$ARGV[0] --rerror=1e-5\n");
open(TESTRUN,
     "odeSolver $ARGV[1] --time=$ARGV[2] --printstep=$ARGV[3] --error=$ARGV[0] --rerror=1e-10 2>> testlog.txt |");
while(<TESTRUN>) {

    @values = split(/ /, $_);
    
    if ( $#values > 1 && $values[0] eq "#t" ) {
	foreach $num (1 .. $#values-1) {
	    $SPECIES[$num] = sprintf $values[$num];
	}
    }
    
    if ($#values > 1 && $values[0]=~/^\d.*/ )  {
	foreach $num (1 .. $#values-1) {
	    $VAL{$values[0]}{$SPECIES[$num]} = $values[$num];
	    $time[$steps] = $values[0];
	}
	$steps++;
    }


    if ($_=~/CVode\ failed/) {
	$flag6 =1;
	$error = $_;
    }

}
close TESTRUN;

unlink($ARGV[4]);
open(CSVFILE, ">$ARGV[4]" );


if ( $flag6 == 1 ) {
    print CSVFILE "TRY AGAIN";    
    print LOGFILE "CVODE FAILURE using --error $ARGV[0]: $error";    
}
else {
    
    print CSVFILE "time";
    foreach $argnum (6 .. $#ARGV) {
	print CSVFILE ",$ARGV[$argnum]";
    }
    print CSVFILE "\n";

    foreach $stepnum (0 .. $#time) {
	print CSVFILE "$time[$stepnum]";
	foreach $argnum (6 .. $#ARGV) {
	    print CSVFILE ",$VAL{$time[$stepnum]}{$ARGV[$argnum]}";
	}
	print CSVFILE "\n";
    }
    print LOGFILE "SUCCESS, using --error $ARGV[0]\n";
}

print LOGFILE "\n";

close CSVFILE;
close LOGFILE;

