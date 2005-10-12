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
