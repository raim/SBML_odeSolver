/*
  Last changed Time-stamp: <2005-10-26 17:02:28 raim>
  $Id: variableIndex.h,v 1.5 2005/10/26 15:02:54 raimc Exp $ 
*/

#ifndef _VARIABLEINDEX_H_
#define _VARIABLEINDEX_H_

typedef enum variableType
  {
    ODE_VARIABLE,
    ASSIGNMENT_VARIABLE,
    CONSTANT
  } variableType_t; 	 
 
struct variableIndex
{
  variableType_t type;
  int index ;
} ;

#endif
/* _VARIABLEINDEX_H_ */
