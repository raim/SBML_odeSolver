#include "sbmlsolver/odeConstructUsingOptions.h"

#include <malloc.h>
#include <string.h>

#include "sbmlsolver/odeConstruct.h"
#include "sbmlsolver/options.h"

CvodeData 
constructODEs(Model_t *m)
{
    char *resultsFilename;
    char **parametersNotToBeReplaced[2];
        
    if (Opt.Write || Opt.Xmgrace)
    {
        resultsFilename =
	  (char *) calloc(strlen(Opt.ModelPath)+ strlen(Opt.ModelFile)+5,
			  sizeof(char));
        sprintf(resultsFilename, "%s%s.dat", Opt.ModelPath, Opt.ModelFile);
    }
    else
        resultsFilename = 0;

    if (!Opt.Simplify)
    {
        Opt.PrintJacobian = 0;
        Opt.DrawJacobian = 0;
    }
    
    parametersNotToBeReplaced[0] = Opt.Parameter;
    parametersNotToBeReplaced[1] = NULL ;

    return constructODEsPassingOptions(m, resultsFilename,
				       Opt.Simplify,
				       Opt.Determinant,
                       parametersNotToBeReplaced);
}
