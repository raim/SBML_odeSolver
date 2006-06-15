#!/bin/sh
# -*-Bash-*-
# Last changed Time-stamp: <2006-06-15 14:01:40 xtof>
# $Id: simple.sh,v 1.12 2006/06/15 12:57:24 chfl Exp $

USAGE="Usage: $0 [-b S] [-e N] [-h] [-j] [-m N] [-r N] [-t N] [-x] FOO.xml ..."
XMGR=$(which xmgrace)
PERL=$(which perl)
SOLVER=odeSolver/odeSolver
SOLVERBASE=".."
TIME=1e5
ABSE=1e-9
RELE=1e-4
MAXS=1e4
PRINTS=1e2
JAC=
NOLEG=

# process command-line arguments
while getopts "b:e:hjm:p:r:t:x" opt; do
    case $opt in
	b)  SOLVERBASE=$OPTARG
	    ;;
	e)
	    ABSE=$OPTARG
	    ;;
	h)
	    echo $USAGE
	    echo "       -b S  set solver base directory to S"
	    echo "       -e N  set absolute error to N"
	    echo "       -h    display this help message"
	    echo "       -j    use jacobian matrix"
	    echo "       -m N  set maxstep to N"
	    echo "       -p N  set printstep to N"
	    echo "       -r N  set relative error to N"
	    echo "       -t N  set integration time to N"
	    echo "       -x    don't display legend in xmgrace"
	    exit 1
	    ;;
	j)
	    JAC=-j
	    ;;
	m)
	    MAXS=$OPTARG
	    ;;
	p)
	    PRINTS=$OPTARG
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
echo "${SOLVERBASE}/$SOLVER $XML $JAC --time=$TIME --mxstep=$MAXS --printstep=100 --error=$ABSE --rerror=$RELE -l > $GR"

# integrate
OK=$($SOLVERBASE/$SOLVER $XML $JAC --time=$TIME --mxstep=$MAXS --printstep=100 --error=$ABSE --rerror=$RELE -l > $GR)
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
    $PERL $SOLVERBASE/scripts/xmgracefile.pl $NOLEG $GR | $XMGR -nxy -&
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
