#include "interface.h"
/*
  Last changed Time-stamp: <2005-08-02 00:02:02 raim>
  $Id: interface.c,v 1.1 2005/10/18 16:45:51 raimc Exp $
*/


void printvs(VarySettings2 *vs);

static void
printResults(SBMLResults_t *results);

/*Here some testing how to convert an R matrix into an  VarySettings2 struct takes place*/


static void cleanupRS(SEXP r)
{
  SBMLResults_free((SBMLResults_t *)  EXTPTR_PTR(r));
}

static void cleanupRSAr(SEXP r)
{
  SBMLResultsArray_free((SBMLResultsArray_t *)  EXTPTR_PTR(r));
}



/*End Eryk*/

SEXP convertMatrix(timeCourse_t* timecourse)
{
  SEXP result,tmp, species, dimnames,names;
  int nx, ny,i,j;

  nx = timecourse->timepoints;
  ny = timecourse->num_val;
  /*tmp = GET_SLOT(result,install("species"));*/
  /*Copy values to R matrix*/
  PROTECT(species = allocMatrix(REALSXP,nx ,ny));
  for(i = 0; i < nx; i++)
    {
      for(j = 0; j < ny; j++)
	{
	  REAL(species)[i + nx*j] = timecourse->values[i][j];
	}
    }
  /*Setting the names of the variables*/
  PROTECT(dimnames = allocVector(VECSXP, 2));
  PROTECT(names = allocVector(STRSXP, timecourse->num_val));
  for(i =0; i<timecourse->num_val; i++)
    {
      SET_STRING_ELT(names, i, mkChar(timecourse->names[i]));
    }
  SET_VECTOR_ELT(dimnames, 1, names);
  setAttrib(species , R_DimNamesSymbol , dimnames);
  UNPROTECT(3);
  return(species);
}


/*assembles the SBMLResults object*/
SEXP convertResult( SBMLResults_t * sbmlresult)
{
  int j , i ;
  SEXP result , tmp, time ,int_m;
  result = PROTECT(NEW_OBJECT(MAKE_CLASS("SBMLResults"))); 
  SET_SLOT( result, install("species") , convertMatrix(sbmlresult->species));
  if( sbmlresult->compartments->num_val != 0)
    {
      SET_SLOT(result,install("compartments") , convertMatrix(sbmlresult->compartments));
    }
  if(sbmlresult->parameters->num_val != 0)
    {
      SET_SLOT(result,install("parameters") , convertMatrix(sbmlresult->parameters));
    }
  SET_SLOT(result,install("fluxes") , convertMatrix(sbmlresult->fluxes));
  
  
  time = allocVector(REALSXP,sbmlresult->timepoints);
  for(i = 0 ; i < sbmlresult->timepoints ;i++)
    {
/*      printf("%f\n",sbmlresult->time[i]);*/
      REAL(time)[i]=sbmlresult->time[i];
    }

  SET_SLOT(result,install("time"),time);
  
  int_m = NEW_INTEGER(1);
  INTEGER(int_m)[0]=sbmlresult->timepoints;
  SET_SLOT(result,install("timepoints"),int_m);
  UNPROTECT(1);
  return(result);
}

SEXP convertToResultclasses(SBMLResultsArray_t *rs)
{
  SEXP resultlist,result;
  int i,j;
  PROTECT(resultlist = allocVector(VECSXP, rs->length));
  for(i=0; i< rs->length;i++)
    {
      result = convertResult(rs->resultsArray[i]);
      SET_VECTOR_ELT(resultlist, i , result);
    }
  UNPROTECT(1);
  return(resultlist);
}


SEXP SEXPbeCalled(
		  SEXP vs /*matrix with design points*/
		  ,SEXP model
		  ,SEXP time
		  ,SEXP printstep
		  )
{
  SEXP id, rid, mat, mat_dim;
  SBMLResultsArray_t * results;
  int countP = 0, i,j;
  char *mod;
  VarySettings2 * vs_c;
  SEXP resultlist;
/* 
  vs_c.charsize    = 250;
  vs_c.nrdesingpoints = 100;
  vs_c.nrparams = 20;
 */
 
 
  mod = CHAR(STRING_ELT(model,0));
  PROTECT(time  = AS_NUMERIC(time)) ; countP++;
  PROTECT(printstep  = AS_NUMERIC(printstep)) ; countP++;
  id = GET_SLOT(vs,install("paramid")) ;
  rid = GET_SLOT(vs,install("reactionid")) ;
  mat = GET_SLOT(vs,install("parameters")) ;
  PROTECT(mat_dim  = GET_DIM(mat)); countP++ ;

  vs_c = varySettings2_create(INTEGER_POINTER(mat_dim)[0] , INTEGER_POINTER(mat_dim)[1] , 250 );

  for( i = 0 ; i< LENGTH(id) ; i++ )
    {
      strcpy(vs_c->id[i],CHAR(STRING_ELT(id,i)));
    }
  for( i = 0 ; i < LENGTH(rid) ; i++ )
    {
      strcpy(vs_c->rid[i], CHAR(STRING_ELT(rid,i)));
    }
/*  printf("nr rows %i nr columns %i \n",INTEGER_POINTER(mat_dim)[0], INTEGER_POINTER(mat_dim)[1]);*/
  for( i = 0 ; i < INTEGER_POINTER(mat_dim)[0]; i++ )
    {
      for(j = 0; j < INTEGER_POINTER(mat_dim)[1]; j++ )
	{
	  vs_c->params[i][j] = NUMERIC_POINTER(mat)[ INTEGER_POINTER(mat_dim)[0] * j + i ];
	  /*printf(" %f ",vs_c->params[i][j]);*/
	  
	}
      /*printf("\n");*/
    }
  
  results = beCalled(
			  mod
			  ,NUMERIC_POINTER(time)[0] 
			  ,NUMERIC_POINTER(printstep)[0] 
			  ,vs_c
			  );
  /*code to compy the SEXP data into C structures goes here*/
  if(results==NULL)
    {
      printf("failure in beCalled ");
      UNPROTECT( countP );
      return(model);
    };
  varySettings2_free(vs_c);
  /*code to free the C structures goes here.*/
  resultlist = convertToResultclasses(results);
  SBMLResultsArray_free(results);
  UNPROTECT( countP );
  return resultlist;
}

SBMLResultsArray_t * beCalled(char *model
			     ,double time /* time to which model is integrated */
			     ,double printstep /* Number of output steps from 0 to 'Time' ignored if 'Indefinitely'*/
			     ,VarySettings2 *vs /* structure with desing points */
			     )
{
  int i,errors;
  SBMLDocument_t *dd;
  
  SBMLReader_t *sr;
  cvodeSettings_t *set;
  Model_t *m ;
  SBMLResultsArray_t * results;

  /** initializing options, that are set via commandline
      arguments in the stand-alone version of the odeSolver.
      Please see file options.h for possible options.
      Option handling will be reorganized soon, so that this
      step will not be needed anymore!! **/

  initializeOptions();
  Opt.PrintMessage = 1;
  /* parsing the SBML model with libSBML */
  sr = SBMLReader_create();
  dd = SBMLReader_readSBML(sr, model);
  errors = SBMLDocument_getNumFatals(dd)+SBMLDocument_getNumErrors(dd)+SBMLDocument_getNumWarnings(dd);
  if (errors > 0)
    { 
      SBMLDocument_printWarnings(dd, stdout);
      SBMLDocument_printErrors  (dd, stdout);
      SBMLDocument_printFatals  (dd, stdout);
      SBMLDocument_getFatal(dd, 0);
      return(0);
    }
  SBMLReader_free(sr);  
  /* Setting SBML ODE Solver integration parameters */
   /* Setting SBML ODE Solver integration parameters with default values */
  set = CvodeSettings_createDefaults();
  /* resetting the values we need */
  set->Time = time;
  set->PrintStep = printstep;  
  set->Error = 1e-18;
  set->RError = 1e-14;
  set->Mxstep = 10000;
  set->SteadyState = 1;
  /* calling the SBML ODE Solver, and retrieving SBMLResults */
  results = Model_odeSolverDesing(dd
				  , set
				  , vs
				  );
  CvodeSettings_free(set);
  SBMLDocument_free(dd);

  if (results == NULL)
    {
      printf("### Parameter variation not succesfull!\n");
      return(NULL);
    }
  return(results);
}

