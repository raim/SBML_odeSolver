/*
  Last changed Time-stamp: <2007-05-15 20:11:58 raim>
  $Id: compiler.c,v 1.14 2007/05/15 18:51:59 raimc Exp $
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
 *     Andrew Finney
 *
 * Contributor(s):
 */

#include "compiler.h"
#include "solverError.h"

/* System specific definitions,
   created by configure script */
#ifndef WIN32
#include "config.h"
#endif

#ifdef WIN32

#include <windows.h>
#include <tchar.h>
#include <stdio.h>

/**
   A structure that stores compiled code
*/
struct compiled_code
{
  HMODULE dllHandle ;
  char *dllFileName ;
};

#elif USE_TCC == 1

#include <stdio.h>
#include <stdlib.h>
#include <libtcc.h>
struct compiled_code
{
  TCCState *s;
};

#endif

/**
   Returns a pointer to code that is compiled from the given source code
*/
compiled_code_t *Compiler_compile(const char *sourceCode)
{
  compiled_code_t *code = NULL;
  
#ifdef WIN32

  char tempDir[MAX_PATH+1];
  TCHAR tccFileName[MAX_PATH]; 
  int i;
  int result;
  char *cFileName;
  char *dllFileName;
  char *outFileName;
  FILE *cFile;
  char command[4*MAX_PATH];
  char *dllFileNameDot ;
  HMODULE dllHandle, solverHandle ;
#ifdef _DEBUG
  char *solverFileName = "SBML_odeSolverD.dll";
#else
  char *solverFileName = "SBML_odeSolver.dll";
#endif

  /*printf("Source code:\n%s\n", sourceCode);*/

  /* avoid creating files in current directory if environment
     variables not set */
  if (!GetTempPath(MAX_PATH+1, tempDir))
  {
    SolverError_storeLastWin32Error("Trying to find out location of system temp directory");
    return NULL;
  }

  solverHandle = GetModuleHandle(solverFileName);
    
    if (!solverHandle)
    {
      SolverError_storeLastWin32Error("Trying to get handle of solver dll");
      return NULL;
    }

    /* compute tcc path from the path to this dll */
    if( !GetModuleFileName( solverHandle, tccFileName, MAX_PATH ) )
    {
      SolverError_storeLastWin32Error("Trying find location of the soslib dll");
      return NULL ;
    }

    for (i = strlen(tccFileName); i != -1 && tccFileName[i] != '\\'; i--);

    tccFileName[i + 1] = '\0';
    strcat(tccFileName, "tcc\\tcc.exe");

    cFileName = tempnam(tempDir, "temp_soslib_c_file");
    dllFileName = tempnam(tempDir, "temp_soslib_dll");
    outFileName = tempnam(tempDir, "temp_soslib_compilation_output");
    cFile = fopen(cFileName, "w");

    if (!cFile)
    {
      SolverError_storeLastWin32Error("Unable to open C source file for write");
      return NULL;
    }

    fprintf(cFile, sourceCode);
    fclose(cFile);

    sprintf(command, "%s -o %s -shared %s > %s",
	    tccFileName, dllFileName, cFileName, outFileName);

    /*printf("Command:\n%s\n", command);*/

    result = system(command);

    if (result == -1)
    {
      SolverError_storeLastWin32Error("Whilst running compile command");
      remove(cFileName);
      free(cFileName);
      return NULL ;
    }
    else if (result != 0)
    {
      SolverError_error(
			ERROR_ERROR_TYPE, SOLVER_ERROR_COMPILATION_FAILED,
			"Compile command failed - returned %d", result);
      remove(cFileName);
      free(cFileName);
      return NULL ;
    }

    remove(cFileName);
    free(cFileName);
    remove(outFileName);
    free(outFileName);

    ASSIGN_NEW_MEMORY_BLOCK(dllFileNameDot, (strlen(dllFileName) + 2),
			    char, NULL);

    strcpy(dllFileNameDot, dllFileName);
    strcat(dllFileNameDot, ".");

    dllHandle = LoadLibrary(dllFileNameDot);
    free(dllFileNameDot);

    if (!dllHandle)
    {
      SolverError_storeLastWin32Error("While loading compiled dll");
      return NULL;
    }

    ASSIGN_NEW_MEMORY(code, compiled_code_t, NULL);

    code->dllHandle = dllHandle ;
    code->dllFileName = dllFileName;
    

#elif USE_TCC == 1
    
    int failed;

    ASSIGN_NEW_MEMORY(code, compiled_code_t, NULL);

    code->s = tcc_new();
    /* tcc_enable_debug(code->s);  */

    if ( !code->s )
    {
      SolverError_error(FATAL_ERROR_TYPE, SOLVER_ERROR_COMPILATION_FAILED,
			"TCC compilation failed: tcc_new() was empty.");
      return NULL;
    }
 
    /* MUST BE CALLED before any compilation or file loading */
    tcc_set_output_type(code->s, TCC_OUTPUT_MEMORY);

    /* add include path */
    tcc_add_include_path(code->s, SUNDIALS_CFLAGS);
    tcc_add_library_path(code->s, SUNDIALS_LDFLAGS);
    tcc_add_library(code->s, SUNDIALS_LIB3);
    tcc_add_library(code->s, SUNDIALS_LIB5);

    /* compile with TCC :) */
    failed = tcc_compile_string(code->s, sourceCode);
    if ( failed != 0 )
      SolverError_error(FATAL_ERROR_TYPE, SOLVER_ERROR_COMPILATION_FAILED,
			"Online compilation failed - returned %d", failed);

    failed = tcc_relocate(code->s);
    if ( failed != 0 )
      SolverError_error(FATAL_ERROR_TYPE, SOLVER_ERROR_COMPILATION_FAILED,
			"TCC failed: couldn't relocate TCCState");

    /* printf("TCC pointer %d\n", code->s); */

#ifdef _DEBUG /* write out source file for debugging*/
    FILE *src;
    char *srcname =  "functions.c";
    src = fopen(srcname, "w");
    fprintf(src, sourceCode);
    fclose(src);
#endif
    
#endif

   return (code);
}

/**
   returns a pointer to the function named 'symbol' in the given 'code'
*/
void *CompiledCode_getFunction(compiled_code_t *code, const char *symbol)
{
  void *result = NULL;

#ifdef WIN32
  result = GetProcAddress(code->dllHandle, symbol);

  if (result)
    return result ;

  SolverError_storeLastWin32Error("");
  result = NULL;
#elif USE_TCC == 1

  int failed;
  unsigned long val;
    
  failed = tcc_get_symbol(code->s, &val, symbol);
  result = (void *)val;
  if ( failed != 0 )
    SolverError_error(FATAL_ERROR_TYPE, SOLVER_ERROR_COMPILATION_FAILED,
		      "TCC failed: couldn't get function: %s", symbol);
  
#endif
  return (result);
}

/**
   frees the given code
*/
void CompiledCode_free(compiled_code_t *code)
{
#ifdef WIN32
  FreeLibrary(code->dllHandle);
  remove(code->dllFileName);
  free(code->dllFileName);
  free(code);
#elif USE_TCC == 1
  if ( code->s != NULL ) tcc_delete(code->s);
  free(code);
  code = NULL;
#endif
}
