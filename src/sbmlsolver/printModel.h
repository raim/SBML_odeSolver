/*
  Last changed Time-stamp: <2005-10-07 16:47:18 raim>
  $Id: printModel.h,v 1.3 2005/10/12 12:52:09 raimc Exp $
*/
#ifndef _PRINTMODEL_H_
#define _PRINTMODEL_H_


void
printModel(Model_t *m, FILE *f);
void
printSpecies(Model_t *m, FILE *f);
void
printReactions(Model_t *m, FILE *f);
void
printODEs(cvodeData_t *data, FILE *f);
void
printODEsToSBML(Model_t *ode, FILE *f);
void
printJacobian(odeModel_t *om, FILE *f);
/* print results of simulation to stdout */
void
printConcentrationTimeCourse(cvodeData_t *data, FILE *f);
void
printOdeTimeCourse(cvodeData_t *data, FILE *f);
void
printReactionTimeCourse(cvodeData_t *data, Model_t *m, FILE *f);
void
printJacobianTimeCourse(cvodeData_t *data, FILE *f);
void
printDeterminantTimeCourse(cvodeData_t *data, ASTNode_t* det, FILE *f);

void
printPhase(cvodeData_t *data);

#endif
/* End of file */
