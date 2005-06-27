/*
  Last changed Time-stamp: <2004-09-30 16:50:25 raim>
  $Id: sbml.h,v 1.2 2005/06/27 15:17:57 afinney Exp $
*/
#ifndef _SBML_H_
#define _SBML_H_

#include <sbml/SBMLTypes.h>

SBMLDocument_t *convertModel (SBMLDocument_t *d1);

SBMLReader_t*newSBMLReader(
    char *schemaPath,
    char *schema11FileName,
    char *schema12FileName,
    char *schema21FileName);

SBMLDocument_t*parseModelPassingOptions(
    char *file,
    int printMessage,
    int validate,
    char *schemaPath,
    char *schema11FileName,
    char *schema12FileName,
    char *schema21FileName);

#endif

/* End of file */
