/*
  Last changed Time-stamp: <2005-12-10 16:20:48 raim>
  $Id: sbml.c,v 1.13 2005/12/16 18:32:15 afinney Exp $
*/
/* 
 *
 * This library is free software; you can redistribute it and/or modify it
 * under the terms of the GNU Lesser General Public License as published
 * by the Free Software Foundation; either version 2.1 of the License, or
 * any later version.
 *
 * This library is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY, WITHOUT EVEN THE IMPLIED WARRANTY OF
 * MERCHANTABILITY OR FITNESS FOR A PARTICULAR PURPOSE. The software and
 * documentation provided hereunder is on an "as is" basis, and the
 * authors have no obligations to provide maintenance, support,
 * updates, enhancements or modifications.  In no event shall the
 * authors be liable to any party for direct, indirect, special,
 * incidental or consequential damages, including lost profits, arising
 * out of the use of this software and its documentation, even if the
 * authors have been advised of the possibility of such damage.  See
 * the GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this library; if not, write to the Free Software Foundation,
 * Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA.
 *
 * The original code contained here was initially developed by:
 *
 *     Rainer Machne
 *
 * Contributor(s):
 *     Christoph Flamm
 *     Andrew M. Finney
 */

#include <stdio.h>
#include <stdlib.h>

/* libSBML header files */
#include <sbml/SBMLTypes.h>
#include <sbml/xml/ParseMessage.h> 

/* own header files */
#include "sbmlsolver/sbml.h"
#include "sbmlsolver/util.h"
#include "sbmlsolver/solverError.h"

void storeSBMLError(errorType_t type, const ParseMessage_t *pm)
{
    SolverError_error(type, ParseMessage_getId(pm),
		      (char*) ParseMessage_getMessage(pm)); 
}

/** C.0: Loads, validates and parses an SBML file,
    also converts SBML level 1 to level 2 files,
    return NULL if errors were encountered during libSBML
    validation and consistency check, stores libSBML warngings
*/
SBMLDocument_t *
parseModel(char *file, int printMessage, int validate, char *schemaPath,
	   char *schema11FileName,
	   char *schema12FileName,
	   char *schema21FileName)
{
    unsigned int i, errors ;
    SBMLDocument_t *d;
    SBMLDocument_t *d2;
    SBMLReader_t *sr;

    if ( validate ) {
        if (  printMessage ) {
            fprintf(stderr, "Validating SBML.\n");
            fprintf(stderr, "This can take a while for SBML level 2.\n");
        }
        sr = newSBMLReader(schemaPath,
			   schema11FileName,
			   schema12FileName,
			   schema21FileName);
    }
    else {
        sr = SBMLReader_create();
    }

    d = SBMLReader_readSBML(sr, file);
    SBMLReader_free(sr);
    
    errors = 0;
    if ( validate ) {
      errors = SBMLDocument_getNumFatals(d) + SBMLDocument_getNumErrors(d);
    }
    
    if (SBMLDocument_getNumFatals(d) + SBMLDocument_getNumErrors(d) == 0)
        SBMLDocument_checkConsistency(d);
        /* AMF 9th Dec 2005 added back because inconsistent models can cause the
           solver to crash despite the consistancy check causing memory leaks - talk to Ben B! */

    /* check for warnings and errors */
    for (i =0 ; i != SBMLDocument_getNumWarnings(d); i++)
        storeSBMLError(WARNING_ERROR_TYPE, SBMLDocument_getWarning(d, i)); 

    for (i =0 ; i != SBMLDocument_getNumErrors(d); i++)
        storeSBMLError(ERROR_ERROR_TYPE, SBMLDocument_getError(d, i)); 

    for (i =0 ; i != SBMLDocument_getNumFatals(d); i++)
        storeSBMLError(FATAL_ERROR_TYPE, SBMLDocument_getFatal(d, i)); 

    RETURN_ON_ERRORS_WITH(NULL);
    
    /* convert level 1 models to level 2 */
    if ( SBMLDocument_getLevel(d) == 1 ) {      
        d2 = convertModel(d);
        SBMLDocument_free(d);
        if ( printMessage )
            fprintf(stderr, "SBML converted from level 1 to level 2.\n"); 
        return (d2);
    }
    return (d);
}

SBMLReader_t *
newSBMLReader (char *schemaPath,
	       char *schema11,
	       char *schema12,
	       char *schema21)
{
  SBMLReader_t *sr;
  char *schema[3];

  schema[0] = concat(schemaPath, schema11);
  schema[1] = concat(schemaPath, schema12);
  schema[2] = concat(schemaPath, schema21);

  sr = SBMLReader_create();

  SBMLReader_setSchemaValidationLevel(sr, XML_SCHEMA_VALIDATION_BASIC);
  SBMLReader_setSchemaFilenameL1v1(sr, schema[0]);
  SBMLReader_setSchemaFilenameL1v2(sr, schema[1]);
  SBMLReader_setSchemaFilenameL2v1(sr, schema[2]);

  free(schema[0]);
  free(schema[1]);
  free(schema[2]);
  
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
  free(model);
  return d2;
}

/* End of file */
