/*
  Last changed Time-stamp: <2005-10-26 15:28:11 raim>
  $Id: printModel.h,v 1.1 2005/10/26 14:27:42 raimc Exp $
*/
#ifndef _PRINTMODEL_H_
#define _PRINTMODEL_H_

/* print model structures to file */
void printModel(Model_t *m, FILE *f);
void printSpecies(Model_t *m, FILE *f);
void printReactions(Model_t *m, FILE *f);
void printODEs(odeModel_t *om, FILE *f);
void printODEsToSBML(Model_t *ode, FILE *f);
void printJacobian(odeModel_t *om, FILE *f);
/* print results of simulation to file */
void printConcentrationTimeCourse(cvodeData_t *data, FILE *f);
void printOdeTimeCourse(cvodeData_t *data, FILE *f);
void printReactionTimeCourse(cvodeData_t *data, Model_t *m, FILE *f);
void printJacobianTimeCourse(cvodeData_t *data, FILE *f);
void printDeterminantTimeCourse(cvodeData_t *data, ASTNode_t* det, FILE *f);

void
printPhase(cvodeData_t *data);

#endif
/* End of file */
