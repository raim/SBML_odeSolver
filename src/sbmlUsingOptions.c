#include "sbmlsolver/sbmlUsingOptions.h"

#include "sbmlsolver/options.h"
#include "sbmlsolver/sbml.h"

SBMLDocument_t*
parseModel (char *file)
{
    return parseModelPassingOptions(
        file, Opt.PrintMessage, Opt.Validate, Opt.SchemaPath,
	Opt.Schema11, Opt.Schema12, Opt.Schema21);
}
