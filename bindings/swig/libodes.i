/*
  Last changed Time-stamp: <2005-03-09 18:12:22 xtof>
  $Id: libodes.i,v 1.1 2005/05/30 19:49:13 raimc Exp $
*/

%module libodes

%{
#include <sbml/SBMLTypes.h>
#include <sbml/common/common.h>
#include "cvodedata.h"
#include "batchIntegrator.h"
#include "drawGraph.h"
#include "odeConstruct.h"
#include "options.h"
#include "sbml.h"
#include "interactive.h"
#include "odeIntegrate.h"
#include "printModel.h"
#include "util.h"
#include "modelSimplify.h"
#include "odeSolver.h"
#include "processAST.h"
%}

%constant double VERSION = 1.5;

%include cvodedata.h
%include batchIntegrator.h
%include drawGraph.h
%include odeConstruct.h
%include options.h
%include sbml.h
%include interactive.h
%include odeIntegrate.h
%include printModel.h
%include util.h
%include modelSimplify.h
%include odeSolver.h
%include processAST.h
