#!/bin/sh
# -*-Bash-*-
# Last changed Time-stamp: <2005-12-15 18:00:54 raim>
# $Id: simple.sh,v 1.9 2005/12/15 17:01:47 raimc Exp $

USAGE="Usage: $0 [-e N] [-h] [-j] [-r N] [-t N] [-x] FOO.xml ..."
XMGR=$(which xmgrace)
PERL=$(which perl)
SOLVER="../odeSolver/odeSolver"
TIME=1e4
ABSE=1e-7
RELE=1e-3
JAC=
NOLEG=

# process command-line arguments
while getopts "e:hjr:t:x" opt; do
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
	    echo "       -x    don't display legend in xmgrace"
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
	x)
	    NOLEG=-noLeg
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
XML=${1-../examples/MAPK.xml}
GR=$(basename $XML .xml).gr

set -e

# remove possible old integration output file
if test -x $GR; then \rm $GR; fi

# show command line
echo "$SOLVER $XML $JAC --time=$TIMEE --printstep=100 --error=$ABSE --rerror=$RELE -l > $GR"

# integrate
OK=$($SOLVER $XML $JAC --time=$TIME --printstep=100 --error=$ABSE --rerror=$RELE -l > $GR)
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
    $PERL ../scripts/xmgracefile.pl $NOLEG $GR | $XMGR -nxy -&
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
