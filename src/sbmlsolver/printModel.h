/*
  Last changed Time-stamp: <2005-08-01 16:50:42 raim>
  $Id: printModel.h,v 1.2 2005/08/02 13:20:32 raimc Exp $
*/
#ifndef _PRINTMODEL_H_
#define _PRINTMODEL_H_


void
printModel(Model_t *m);
void
printSpecies(Model_t *m);
void
printReactions(Model_t *m);
void
printODEs(cvodeData_t *data);
void
printODEsToSBML(cvodeData_t *data);
void
printJacobian(cvodeData_t *data);
/* print results of simulation to stdout */
void
printConcentrationTimeCourse(cvodeData_t *data, FILE *f);
void
printOdeTimeCourse(cvodeData_t *data, FILE *f);
void
printReactionTimeCourse(cvodeData_t *data, FILE *f);
void
printJacobianTimeCourse(cvodeData_t *data, FILE *f);
void
printDeterminantTimeCourse(cvodeData_t *data, ASTNode_t* det, FILE *f);

void
printPhase(cvodeData_t *data);

#endif
/* End of file */
