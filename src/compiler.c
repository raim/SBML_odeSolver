/*
  Last changed Time-stamp: <2005-10-26 17:24:50 raim>
  $Id: compiler.c,v 1.1 2006/03/09 17:23:49 afinney Exp $
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

#ifdef WIN32

#include <windows.h>
#include <tchar.h>
#include <stdio.h>

struct compiled_code
{
    HMODULE dllHandle ;
    char *dllFileName ;
};

#endif

compiled_code_t *Compiler_compile(const char *sourceCode)
{
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
    HMODULE dllHandle ;    
    compiled_code_t *code;

    /*printf("Source code:\n%s\n", sourceCode);*/

    /* avoid creating files in current directory if environment variables not set */
    if (!GetTempPath(MAX_PATH+1, tempDir))
    {
        SolverError_storeLastWin32Error("Trying to find out location of system temp directory");
        return NULL;
    }

    /* compute tcc path from the path to this dll */
    if( !GetModuleFileName( NULL, tccFileName, MAX_PATH ) )
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

    sprintf(command, "%s -o %s -shared %s > %s", tccFileName, dllFileName, cFileName, outFileName);

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
            ERROR_ERROR_TYPE, SOLVER_ERROR_COMPILATION_FAILED, "Compile command failed - returned %d", result);
        remove(cFileName);
        free(cFileName);
        return NULL ;
    }

    remove(cFileName);
    free(cFileName);
    remove(outFileName);
    free(outFileName);

    ASSIGN_NEW_MEMORY_BLOCK(dllFileNameDot, (strlen(dllFileName) + 2), char, NULL);

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

	return code;
#endif
}

void *CompiledCode_getFunction(compiled_code_t *code, const char *symbol)
{
#ifdef WIN32
    void *result = GetProcAddress(code->dllHandle, symbol);

    if (result)
        return result ;

    SolverError_storeLastWin32Error("");

    return NULL ;
#endif
}

void CompiledCode_free(compiled_code_t *code)
{
#ifdef WIN32
    FreeLibrary(code->dllHandle);
    remove(code->dllFileName);
    free(code->dllFileName);
    free(code);
#endif
}
