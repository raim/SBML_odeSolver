/*
  Last changed Time-stamp: <2005-10-05 15:04:55 raim>
  $Id: sbml.h,v 1.3 2005/10/12 12:52:09 raimc Exp $
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

SBMLDocument_t*parseModel(
    char *file,
    int printMessage,
    int validate,
    char *schemaPath,
    char *schema11FileName,
    char *schema12FileName,
    char *schema21FileName);

#endif

/* End of file */
