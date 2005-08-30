#!/bin/sh
# -*-Bash-*-
# Last changed Time-stamp: <2005-08-30 17:58:37 xtof>
# $Id: simple.sh,v 1.3 2005/08/30 16:02:22 chfl Exp $

USAGE="Usage: $0 [-e N] [-h] [-j] [-r N] [-t N] FOO.xml ..."
XMGR=$(which xmgrace)
PERL=$(which perl)
SOLVER="../src/odeSolver"
TIME=1e4
ABSE=1e-7
RELE=1e-3
JAC=

# process command-line arguments
while getopts "e:hjr:t:" opt; do
    case $opt in
	e)
	    ABSE=$OPTARG
	    ;;
	h)
	    echo $USAGE
	    echo "       -e N  set absolute error to N"
	    echo "       -h    display this help message"
	    echo "       -j    use jacobian matrix"
	    echo "       -r N  set relative error to N"
	    echo "       -t N  set integration time to N"
	    exit 1
	    ;;
	j)
	    JAC=-j
	    ;;
	r)
	    RELE=$OPTARG
	    ;;
	t)
	    TIME=$OPTARG
	    ;;
	\?)
	echo $USAGE
	exit 1
	;;
    esac
done

# shift options from arg-array
shift $(($OPTIND - 1))

# get model file or set to default model
XML=${1-MAPK.xml}
GR=$(basename $XML .xml).gr

set -e

# remove possible old integration output file
if test -x $GR; then \rm $GR; fi

# show command line
echo "$SOLVER $XML $JAC --time=$TIME --error=$ABSE --rerror=$RELE -l > $GR"

# integrate
OK=$($SOLVER $XML $JAC --time=$TIME --error=$ABSE --rerror=$RELE -l > $GR)
if [ $OK ]; then
  echo ""
  echo "!!! Error: something went wrong during integration !!!"
  echo ""
  exit 1
fi

# display results
if test -x $PERL; then
  if test -x $XMGR; then
    # add a legend an display results with xmgrace
    $PERL ../Perlen/AddLegend.pl $GR | $XMGR -nxy -&
  else
    echo "!!! ERROR: Xmgrace not found !!!"
    exit 1
  fi
else
  echo "!!! ERROR: Perl not found !!!"
  exit 1
fi

# if we came that far everything workt fine
echo ""
echo "Every thing works fine, have fun with your odeSolver =;)"
echo ""

# End of file
