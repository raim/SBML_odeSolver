#include "sbmlsolver/solverError.h"

#include <stdio.h>
#include <malloc.h>
#include <string.h>
#include <stdlib.h>

#ifdef WIN32
#define vsnprintf _vsnprintf
#endif

#include <sbml/util/List.h>

typedef struct solverErrorMessage
{
    char *message ;
    int errorCode ;
} solverErrorMessage_t ;

static List_t *solverErrors[NUMBER_OF_ERROR_TYPES] = { NULL, NULL, NULL };

/* get number of stored errors  of given type */
int SolverError_getNum(errorType_t type)
{
    List_t *errors = solverErrors[type];

    if (!errors)
        return 0;

    return List_size(errors);
}

solverErrorMessage_t *SolverError_getError(errorType_t type, int errorNum)
{
    List_t *errors = solverErrors[type];

    if (!errors)
        return NULL;

    return List_get(errors, errorNum);
}

/* get a stored error message */
char *SolverError_getMessage(errorType_t type, int errorNum)
{
    return SolverError_getError(type, errorNum)->message ;
}

/* get error code */
errorCode_t SolverError_getCode(errorType_t type, int errorNum)
{
    return SolverError_getError(type, errorNum)->errorCode ; 
}

/* get error code of last error stored of given type */
errorCode_t SolverError_getLastCode(errorType_t type)
{
    return SolverError_getCode(type, SolverError_getNum(type) - 1);
}

/* empty error store */
void SolverError_clear()
{
    int i ;

    for (i = 0; i != NUMBER_OF_ERROR_TYPES; i++)
    {
        List_t *l = solverErrors[i];

        if (l)
        {
            while (List_size(l))
            {
                solverErrorMessage_t *m = List_get(l, 0);

                free(m->message);
                free(m);
                List_remove(l, 0);
            }
        }
    }
}

void SolverError_dumpAndClearErrors()
{
    SolverError_dump();
    SolverError_clear();
}


/* create an error */
void SolverError_error(errorType_t type, errorCode_t errorCode, char *fmt, ...)
{
    List_t *errors = solverErrors[type];
    char buffer[2000], *variableLengthBuffer;
    va_list args;
    solverErrorMessage_t *message = (solverErrorMessage_t *)malloc(sizeof(solverErrorMessage_t));

    va_start(args, fmt);
    vsnprintf(buffer, 2000, fmt, args);
    va_end(args);

    variableLengthBuffer = (char *)malloc(strlen(buffer) + 1);
    message->errorCode = errorCode;
    message->message = strcpy(variableLengthBuffer, buffer);

    if (!errors)
        errors = solverErrors[type] = List_create();

    List_add(errors, message);
}

/* exit the program if errors or fatals have been created. */
void SolverError_haltOnErrors()
{
    if (SolverError_getNum(ERROR_ERROR_TYPE) || SolverError_getNum(FATAL_ERROR_TYPE))
        exit(EXIT_FAILURE);
}

/* write all errors and warnings to standard error */
void SolverError_dump()
{
    static char *solverErrorTypeString[] =
        { "Fatal Error",
          "      Error",
          "    Warning" } ;

    int i, j;

    for (i=0; i != NUMBER_OF_ERROR_TYPES; i++)
    {
        List_t *errors = solverErrors[i];

        if (errors)
        {
            for (j=0; j != List_size(errors); j++)
            {
                solverErrorMessage_t *error = List_get(errors, j);
                fprintf(stderr, "%s\t%d\t%s\n", solverErrorTypeString[i], error->errorCode, error->message);
            }
        }
    }
}
