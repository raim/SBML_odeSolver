#ifndef _ARITHMETICCOMPILER_H_
#define _ARITHMETICCOMPILER_H_

typedef struct directCode directCode_t;

#include "sbmlsolver/ASTIndexNameNode.h"
#include "sbmlsolver/cvodeData.h"

struct directCode
{
    int codeSize, FPUstackSize, storageSize;
    int codePosition, FPUstackPosition, storagePosition;
    unsigned char *prog;
    double *FPUstack, *storage;
    double (*evaluate)(void);
} ;

void generateFunction(directCode_t *, ASTNode_t *, cvodeData_t *);
void destructFunction(directCode_t *);

#endif /* _ARITHMETICCOMPILER_H_ */
