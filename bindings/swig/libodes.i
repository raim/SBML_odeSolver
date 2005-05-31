/*
  Last changed Time-stamp: <2005-05-31 16:04:04 raim>
  $Id: libodes.i,v 1.2 2005/05/31 14:05:46 raimc Exp $
*/

%module libodes

%{
#include <sbml/SBMLTypes.h>
#include <sbml/common/common.h>
#include "sbmlsolver/cvodedata.h"
#include "sbmlsolver/sbmlResults.h"
#include "sbmlsolver/batchIntegrator.h"
#include "sbmlsolver/drawGraph.h"
#include "sbmlsolver/odeConstruct.h"
#include "sbmlsolver/options.h"
#include "sbmlsolver/sbml.h"
#include "sbmlsolver/interactive.h"
#include "sbmlsolver/odeIntegrate.h"
#include "sbmlsolver/printModel.h"
#include "sbmlsolver/util.h"
#include "sbmlsolver/modelSimplify.h"
#include "sbmlsolver/odeSolver.h"
#include "sbmlsolver/processAST.h"
%}

%constant double VERSION = 1.5;

%include sbmlsolver/cvodedata.h
%include sbmlsolver/sbmlResults.h
%include sbmlsolver/batchIntegrator.h
%include sbmlsolver/drawGraph.h
%include sbmlsolver/odeConstruct.h
%include sbmlsolver/options.h
%include sbmlsolver/sbml.h
%include sbmlsolver/interactive.h
%include sbmlsolver/odeIntegrate.h
%include sbmlsolver/printModel.h
%include sbmlsolver/util.h
%include sbmlsolver/modelSimplify.h
%include sbmlsolver/odeSolver.h
%include sbmlsolver/processAST.h
