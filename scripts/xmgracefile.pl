#!/usr/bin/perl
# -*-Perl-*-
# Last changed Time-stamp: <2005-08-30 18:31:41 xtof>
# $Id: xmgracefile.pl,v 1.1 2005/12/08 12:13:16 raimc Exp $

use Getopt::Long;
use File::Basename;
use strict;

# some globals
my @LOC = (0.20, 0.90);
my $FSIZE = .5;
my $LEG = 'on'; 

usage() unless GetOptions("loc=s" => sub{local $_ = $_[1]; @LOC = split},
			  "lfs=f" => \$FSIZE,
			  "noLeg" => sub{$LEG = 'off'},
			  "h"     => \&usage);

my $FILENAME = $ARGV[0] || "stdin";

make_legend(search_legend());
while (<>) {print}

#---
sub search_legend { local $_; while ((<>)) { last if m/^\#t/ } return $_ }

#---
sub make_legend {
  local $_ = shift;
  my @F = split;
  shift @F;
  print << "EOL";
@    title "$FILENAME"
@    legend $LEG
@    legend loctype view
@    legend $LOC[0], $LOC[1]
@    legend box color 1
@    legend box pattern 1
@    legend box linewidth 1.0
@    legend box linestyle 1
@    legend box fill color 0
@    legend box fill pattern 1
@    legend font 8
@    legend char size $FSIZE
@    legend color 1
@    legend length 4
@    legend vgap 1
@    legend hgap 2
@    legend invert false
EOL

  my $i = 0;
  foreach my $species (@F) {
    print "\@    s$i legend  \"$species\"\n";
    $i++;
  }
}

#---
sub usage {
  print STDERR
      "\n  Usage: @{[basename($0)]} [options]\n";
  print STDERR
      "     -loc  <Str>  Set location of legend box (upper left corner)\n",
      "                  (default: @LOC)\n",
      "     -lfs  <Flt>  Sets size of legend label characters\n",
      "                  (default: $FSIZE)\n",      
      "     -h           display help message\n\n";
    exit(0);
}



__END__
