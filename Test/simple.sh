#!/bin/sh
# -*-Bash-*-
# Last changed Time-stamp: <2005-02-14 21:46:44 xtof>
# $Id: simple.sh,v 1.2 2005/05/31 15:41:26 raimc Exp $

XMGR=$(which xmgrace)
PERL=$(which perl)
XML=${1-MAPK.xml}
GR=$(basename $XML .xml).gr

set -e

if test -x $GR; then \rm $GR; fi

# integrate the MAPK.xml model
OK=$(../src/odeSolver $XML --time 1e4 --error 1e-7 --rerror 1e-3 -l> $GR)
if [ $OK ]; then
  echo ""
  echo "!!! Error: something went wrong !!!"
  echo ""
  exit 1
fi


if test -x $PERL; then
  if test -x $XMGR; then
    # add a legend an display results with xmgrace
    $PERL ../Perlen/AddLegend.pl $GR | $XMGR -nxy -&
  else
    echo "Xmgrace not found"
    exit 1
  fi
else
  echo "Perl not found"
  exit 1
fi

# remove tmp file
if test -x ./err; then \rm ./err; fi

echo ""
echo "Every thing works fine, have fun with your odeSolver =;)"
echo ""

# End of file
