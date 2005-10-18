#include <R.h>
#include <Rdefines.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <string.h>
#include <time.h>


#include <sbmlsolver/odeSolver.h>

#include <sbmlsolver/sbml.h>
#include <sbmlsolver/solverError.h>
#include <sbml/SBMLTypes.h>

/*extensiosn to odeSolver.h*/

/*
  This structure keeps the desing points for which the model is evaluated

  the parameters are the columns of the matrix 
  the design points are the rows!!! hence 
  vs->params[desingpointindex][parameterindex]
*/


typedef struct
{
  int nrdesingpoints; /*defines how many design points are set*/
  int nrparams;
  char **id; /* array of SBML ID of the species, compartment or parameter to be varied */
  char **rid; /* SBML Reaction ID, if a local parameter is to be varied */
  double **params; /* two dimensional array with the parmaters */
  int charsize; /* maximal length of character string*/
}VarySettings2;

void VarySettings2_free(VarySettings2 *vs);
VarySettings2 * varySettings2_create(
				     int nrdesingpoints
				     ,int nrparams
				     ,int charsize
				     );


/*extensions to sbmlResults.h*/
/*This structure contains the results of the model evaluation*/
typedef struct{
  SBMLResults_t ** resultsArray;
  int length;
}SBMLResultsArray_t; 

extern SBMLResultsArray_t * Model_odeSolverDesing(SBMLDocument_t *dd
				    , cvodeSettings_t *settings
				    , VarySettings2 *vary
				    );


extern SBMLResultsArray_t * SBMLResultsArray_create(int size);
extern void SBMLResultsArray_free(SBMLResultsArray_t *results);


extern SBMLResults_t * Model_odeSolverM(
					Model_t *m
					, cvodeSettings_t *set
					);


/*stuff needed for R*/

extern SEXP SEXPbeCalled(SEXP vs
			 ,SEXP model
			 ,SEXP time
			 ,SEXP printstep
			 );

extern SBMLResultsArray_t * beCalled(char *model
				     ,double time /* time to which model is integrated */
				     ,double printstep /* Number of output steps from 0 to 'Time' ignored if 'Indefinitely'*/
				     ,VarySettings2 *vs /* structure with desing points */
				     );



