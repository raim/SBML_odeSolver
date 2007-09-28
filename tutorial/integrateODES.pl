#!/usr/bin/perl
# -*-Perl-*-
# Last changed Time-stamp: <2007-09-26 16:23:22 xtof>
# $Id: integrateODES.pl,v 1.1 2007/09/28 13:14:16 raimc Exp $

use ODES;
use LibSBML;
use strict;

#my $ode0 = 'a-(b+1)*u+v*u^2'; # u'
#my $ode1 = 'b*u-v*u^2';       # v'
#my @x  = qw/u v a b/;
#my @x0 = (1.0, 1.5, 1.0, 1.5);

my $ode0 = '((150.0 * (3.8 - (p * D_cy) - (p * C_cy)) * (1.0 - (p * C_cy))) - (9.0 * C_cy))';
my $ode1 = '(3.8 - (3.0 * D_cy) - (p * D_cy) - (p * C_cy))';

# use LibSBML function to convert the formula into
# an abstract syntax tree (AST) representation
my $astA = LibSBML::parseFormula($ode0);
my $astB = LibSBML::parseFormula($ode1);
my @AST = ($astA, $astB);
my $neq = scalar @AST;
my @x = qw/C_cy D_cy p/;
my @x0 = (0.0, 9.0, 0.2);

my $om = ODES::ODEModel_createFromODEs(\@AST, 2, 0, 1, \@x, \@x0, undef);

my $settings = ODES::CvodeSettings_create();
$settings->CvodeSettings_setTime(0.5, 10);
$settings->CvodeSettings_setSensitivity(1);

my $ii = ODES::IntegratorInstance_create($om, $settings);

while( ! $ii->IntegratorInstance_timeCourseCompleted() ) {
  if ( ! $ii->IntegratorInstance_integrateOneStep() ) {
    ODES::SolverError_dump();
    last;
  }
  else {
    $ii->IntegratorInstance_dumpData();
  }
}

my $vi = $om->ODEModel_getVariableIndex('C_cy');

printf "\nVariable %s has final value of %g at time %g\n\n",
         $om->ODEModel_getVariableName($vi),
         $ii->IntegratorInstance_getVariableValue($vi),
         $ii->IntegratorInstance_getTime();

$vi->VariableIndex_free();

$vi = $om->ODEModel_getVariableIndex('p');

printf "Sensitivies to %s:\n",
    $om->ODEModel_getVariableName($vi);

$ii->IntegratorInstance_dumpPSensitivities($vi);
print "\n";

$om->ODEModel_free();
$settings->CvodeSettings_free();
$vi->VariableIndex_free();

#---
sub substitute_params {
  my ($eq, $params) = @_;
  for my $key (keys %$params) {
    $eq =~ s/$key/$params->{$key}/g;
  }

  return $eq;
}

__END__
my $sbmlfile = '../../examples/MAPK.xml';

my $rd = new LibSBML::SBMLReader;
my $d  = $rd->readSBML($sbmlfile);

my $settings = ODES::CvodeSettings_create();
$settings->CvodeSettings_setTime(10000, 100);

my $results = ODES::SBML_odeSolver($d, $settings);

$results->SBMLResults_dump();

