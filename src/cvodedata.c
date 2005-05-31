/*
  Last changed Time-stamp: <2005-05-31 12:29:56 raim>
  $Id: cvodedata.c,v 1.2 2005/05/31 13:54:00 raimc Exp $
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* libSBML header files */
#include <sbml/SBMLTypes.h>
#include <sbml/common/common.h>

/* own header files */
#include "sbmlsolver/options.h"
#include "sbmlsolver/cvodedata.h"
#include "sbmlsolver/util.h"

/*
  The functions
  "CvodeData CvodeData_create(int neq, int nconst, int nass, int nevents)"
  and
  "static void CvodeData_free(CvodeData data)"
  allocate and free the memory for CvodeData
  needed by the CVODE integrator functions, respectively. 
*/


CvodeData
CvodeData_create(int neq, int nconst, int nass, int nevents){

  int i;
  CvodeData data;

  if(!(data = (CvodeData) safe_calloc(1, sizeof(*data))))
    fprintf(stderr, "failed!\n");
  
  if(!(data->trigger = (int *) safe_calloc(nevents, sizeof(int))))
    fprintf(stderr, "failed!\n");

  if(!(data->value =  (double *)safe_calloc(neq, sizeof(double))))
    fprintf(stderr, "failed!\n");
  if(!(data->species = (char **)safe_calloc(neq, sizeof(char *))))
    fprintf(stderr, "failed!\n");
  if(!(data->speciesname = (char **)safe_calloc(neq, sizeof(char *))))
    fprintf(stderr, "failed!\n");
  if(!(data->ode = (ASTNode_t **)safe_calloc(neq, sizeof(ASTNode_t *))))    
    fprintf(stderr, "failed!\n");
  
  if(!(data->jacob =
       (ASTNode_t ***)safe_calloc(neq, sizeof(ASTNode_t **))))
    fprintf(stderr, "failed!\n");
  for ( i=0; i<neq; i++ ) {
    if(!(data->jacob[i] = (ASTNode_t **)safe_calloc(neq, sizeof(ASTNode_t *))))
      fprintf(stderr, "failed!\n");     
  }

  if(!(data->pvalue =  (double *)safe_calloc(nconst, sizeof(double))))
    fprintf(stderr, "failed!\n");
  if(!(data->parameter = (char **)safe_calloc(nconst, sizeof(char *))))
    fprintf(stderr, "failed!\n");
  
  if(!(data->avalue =  (double *)safe_calloc(nass, sizeof(double))))
    fprintf(stderr, "failed!\n");
  if(!(data->ass_parameter = (char **)safe_calloc(nass, sizeof(char *))))
    fprintf(stderr, "failed!\n");
  if(!(data->assignment =
       (ASTNode_t **)safe_calloc(nass, sizeof(ASTNode_t *))))    
    fprintf(stderr, "failed!\n");  

  /** Set the filename to write results to.    */
  if ( Opt.Write && !Opt.Xmgrace ) {
    data->filename = (char *) calloc(strlen(Opt.ModelPath)+
				     strlen(Opt.ModelFile)+5, sizeof(char));
    sprintf(data->filename, "%s%s.dat", Opt.ModelPath, Opt.ModelFile);
    data->outfile = fopen(data->filename, "w");
  }
  else {
    data->outfile = stdout;
  }
    
  return data;
}

void
CvodeData_free(CvodeData data){

  int i,j;

  if(data == NULL){
    return;
  }

  /* free CVODE results if filled */
  if(data->results != NULL){
    for(i=0;i<data->nass;i++){
      safe_free(data->results->avalue[i]);
    }
    safe_free(data->results->avalue);
    for(i=0;i<data->nconst;i++){
      safe_free(data->results->pvalue[i]);
    }
    safe_free(data->results->pvalue);
    for(i=0;i<data->neq;i++){
      safe_free(data->results->value[i]);
    }
    safe_free(data->results->time);
    safe_free(data->results->value);
    safe_free(data->results);	      
  }

  /* free model name and id */
  safe_free(data->modelName);
  safe_free(data->modelId);

  /* free ODEs */
  for ( i=0; i<data->neq; i++ ) {
    safe_free(data->species[i]);
    safe_free(data->speciesname[i]);
    ASTNode_free(data->ode[i]);
  }
  safe_free(data->species);
  safe_free(data->speciesname);
  safe_free(data->value);
  safe_free(data->ode);

  /* free Jacobian matrix */
  if ( data->jacob != NULL ) {
    for ( i=0; i<data->neq; i++ ) {
      for ( j=0; j<data->neq; j++ ) {
	ASTNode_free(data->jacob[i][j]);
      }
      safe_free(data->jacob[i]);
    }
    safe_free(data->jacob);
  }

  /* free determinant of Jacobian matrix */
  if ( data->det != NULL ) {
    ASTNode_free(data->det);
  }

  /* free assignments */
  for ( i=0; i<data->nass; i++ ) {
    safe_free(data->ass_parameter[i]);
    ASTNode_free(data->assignment[i]);
  }  
  safe_free(data->ass_parameter);
  safe_free(data->avalue);   
  safe_free(data->assignment);

  /* free constants */
  for ( i=0; i<data->nconst; i++ ) {
    safe_free(data->parameter[i]);
  }  
  safe_free(data->parameter);
  safe_free(data->pvalue);

  /* free event trigger flags */
  safe_free(data->trigger);
  
  /* free simplified ODE model */
  if ( data->simple != NULL ) {
    Model_free(data->simple);
  }
  /* save and close results file */
  if ( Opt.Write && !Opt.Xmgrace ) {
    fclose(data->outfile);
    fprintf(stderr, "Saved results to file %s.\n\n", data->filename);
    safe_free(data->filename);
  }

  /* free CvodeData structure */
  safe_free(data);
  
}

/* The function
   "CvodeResults CvodeResults_create(CvodeData) creates
   the structure that contains the integration results produced
   by CVODE.
*/

CvodeResults
CvodeResults_create(CvodeData data){

  int i;
  CvodeResults results;

  if(!(results = (CvodeResults) safe_calloc(1, sizeof(*results)))){
    fprintf(stderr, "failed!\n");
  }
  
  /* The `time' array of contains all timepoints  */
  if(!(results->time = (double *)safe_calloc(data->nout+1, sizeof(double)))){
    fprintf(stderr, "failed!\n");
  }
  
  /* The 2-D array `value' contains the time courses, that are
     calculated by ODEs (SBML species, or compartments and parameters
     defined by rate rules.
  */
  if(!(results->value = (double **)safe_calloc(data->neq, sizeof(double*)))){
    fprintf(stderr, "failed!\n");
  }
  for(i=0;i<data->neq;++i){
    if(!(results->value[i] =
	 (double *)safe_calloc(data->nout+1, sizeof(double)))){
      fprintf(stderr, "failed!\n");
    }
  }
  
  /* The 2-D array `pvalue' contains time courses for all
     constant SBML species. These data are not needed for
     integration, but is convenient to have for output of
     results.
  */
  if(!(results->pvalue =
       (double **)safe_calloc(data->nconst, sizeof(double*)))){
    fprintf(stderr, "failed!\n");
  }
  for(i=0;i<data->nconst;++i){
    if(!(results->pvalue[i] =
	 (double *)safe_calloc(data->nout+1, sizeof(double)))){
      fprintf(stderr, "failed!\n");
    }
  }
  
  /* The 2-D array `avalue' contains time courses for all
     SBML species, compartments and parameters that are defined
     by SBML assignment rules. Again, this is not needed for
     integration, but convenient to have for output of results.
   */
  if(!(results->avalue =
       (double **)safe_calloc(data->nass, sizeof(double*)))){
    fprintf(stderr, "failed!\n");
  }
  for(i=0;i<data->nass;++i){
    if(!(results->avalue[i] =
	 (double *)safe_calloc(data->nout+1, sizeof(double)))){
      fprintf(stderr, "failed!\n");
    }
  }
  
  return results;  
}


/* End of file */
