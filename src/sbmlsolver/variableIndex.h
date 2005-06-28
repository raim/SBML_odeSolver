#ifndef _VARIABLEINDEX_H_
#define _VARIABLEINDEX_H_

typedef enum variableType
{
    SPECIES,
    ASSIGNMENT_PARAMETER,
    PARAMETER
} variableType_t;

struct variableIndex
{
    variableType_t type ;
    int index ;
} ;

#endif
/* _VARIABLEINDEX_H_ */
