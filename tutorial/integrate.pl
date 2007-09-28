#!/usr/bin/perl
# -*-Perl-*-
# Last changed Time-stamp: <2007-09-28 14:49:23 xtof>
# $Id: integrate.pl,v 1.1 2007/09/28 13:14:16 raimc Exp $

use Getopt::Long;
use Pod::Usage;

use ODES;
use LibSBML;
use warnings;
use vars qw/$TIME $STEPS $SBMLFILE/;
use strict;

# defaults for global var(s)
$TIME  = 10000;
$STEPS = 100;
$SBMLFILE = undef;

# process command-line options
pod2usage(-verbose => 0)
    unless GetOptions("steps=i" => \$STEPS,
		      "time=f"  => \$TIME,
		      "man"     => sub{pod2usage(-verbose => 2)},
		      "help"    => sub{pod2usage(-verbose => 1)});

# read SBML-filename
$SBMLFILE = shift();
do {
  print STDERR "Error: No SBML filename specified\n";
  pod2usage(-verbose => 0)
} unless defined $SBMLFILE;

# use LibSBML functions to parse the SBML file
my $rd = new LibSBML::SBMLReader;
my $d  = $rd->readSBML($SBMLFILE);

# set global integration behaviour
my $settings = ODES::CvodeSettings_create();
$settings->CvodeSettings_setTime($TIME, $STEPS);

# integrate the model
my $results = ODES::SBML_odeSolver($d, $settings);

# print timecourse to STDOUT
$results->SBMLResults_dumpSpecies();

##################################################

=head1 NAME

integrate.pl - integrate a SBML model

=head1 SYNOPSIS

integrate.pl [[-time I<FLT>] [-steps I<INT>]] SBML-filename

=head1 DESCRIPTION

The program takes a SBML file as input and dumps the timecourse of the
species to I<STDOUT>

=head1 OPTIONS

=over 4

=item B<-time> I<FLT>

Set integration endtime to I<FLT> (default: 10000)

=item B<-steps> I<INT>

Set output printsteps to I<INT> (default: 100)

=head1 AUTHORS

Christoph Flamm, Rainer Machne

=head1 BUGS

Please send comments and bug reports to {xtof,raim}@tbi.univie.ac.at

=cut

__END__
