/*
  Last changed Time-stamp: <2005-05-09 07:29:18 raim>
  $Id: sbml.c,v 1.1 2005/05/30 19:49:13 raimc Exp $
*/
#include <stdio.h>
#include <stdlib.h>
#include <sbml/SBMLTypes.h>
#include <sbml/common/common.h>

#include "sbml.h"
#include "util.h"
#include "options.h"

/** C.1 Load, validate and parse SBML file,
    also converts SBML level 1 to level 2 files
*/
SBMLDocument_t*
parseModel (char *file) {

  unsigned int errors = 0;
  SBMLDocument_t *d;
  SBMLDocument_t *d2;
  SBMLReader_t *sr;

  if ( Opt.Validate ) {
    if (  Opt.PrintMessage ) {
      fprintf(stderr, "Validating SBML.\n");
      fprintf(stderr, "This can take a while for SBML level 2.\n");
    }
    sr = newSBMLReader();
  }
  else {
    sr = SBMLReader_create();
  }

  d = SBMLReader_readSBML(sr, file);
  SBMLReader_free(sr);

  /* check for warnings and errors */
  if ( (errors 
	= SBMLDocument_getNumWarnings(d)
	+ SBMLDocument_getNumErrors(d)
	+ SBMLDocument_getNumFatals(d)) > 0) {
    SBMLDocument_printWarnings(d, stderr);
    SBMLDocument_printErrors  (d, stderr);
    SBMLDocument_printFatals  (d, stderr);
    fflush(stderr);
    return (0);
  }
  /* convert level 1 models to level 2 */
  if ( SBMLDocument_getLevel(d) == 1 ) {
    d2 = convertModel(d);
    SBMLDocument_free(d);
    if (  Opt.PrintMessage )
      fprintf(stderr, "SBML converted from level 1 to level 2.\n"); 
    return (d2);
  }
  return (d);
}

SBMLReader_t*
newSBMLReader (void)
{
  SBMLReader_t *sr;
  char *schema[3];

  schema[0] = concat(Opt.SchemaPath, Opt.Schema11);
  schema[1] = concat(Opt.SchemaPath, Opt.Schema12);
  schema[2] = concat(Opt.SchemaPath, Opt.Schema21);

  sr = SBMLReader_create();

  SBMLReader_setSchemaValidationLevel(sr, XML_SCHEMA_VALIDATION_BASIC);
  SBMLReader_setSchemaFilenameL1v1(sr, schema[0]);
  SBMLReader_setSchemaFilenameL1v2(sr, schema[1]);
  SBMLReader_setSchemaFilenameL2v1(sr, schema[2]);

  safe_free(schema[0]);
  safe_free(schema[1]);
  safe_free(schema[2]);
  
  return (sr);  
}

SBMLDocument_t *
convertModel (SBMLDocument_t *d1)
{
  char *model;
  SBMLDocument_t *d2;

  SBMLDocument_setLevel(d1, 2);
  SBMLDocument_setVersion(d1, 1);
  model = writeSBMLToString(d1);
  d2 = readSBMLFromString(model); 
  safe_free(model);
  return d2;
}

/* End of file */
