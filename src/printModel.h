/*
  Last changed Time-stamp: <2004-11-15 15:22:24 raim>
  $Id: printModel.h,v 1.1 2005/05/30 19:49:13 raimc Exp $
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
printODEs(CvodeData data);
void
printODEsToSBML(CvodeData data);
void
printJacobian(CvodeData data);
/* print results of simulation to stdout */
void
printConcentrationTimeCourse(CvodeData data);
void
printOdeTimeCourse(CvodeData data);
void
printReactionTimeCourse(CvodeData data);
void
printJacobianTimeCourse(CvodeData data);
void
printDeterminantTimeCourse(CvodeData data);

void
printPhase(CvodeData data);

#endif
/* End of file */
