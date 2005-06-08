// SBML_odeSolver.cpp : Defines the entry point for the DLL application.
//
#include <windows.h>

BOOL APIENTRY DllMain( HANDLE hModule, 
                       DWORD  ul_reason_for_call, 
                       LPVOID lpReserved
					 )
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
	case DLL_THREAD_ATTACH:
	case DLL_THREAD_DETACH:
	case DLL_PROCESS_DETACH:
		break;
	}
    return TRUE;
}
/* examples of use of export macros
// This is an example of an exported variable
SBML_ODESOLVER_API int nSBML_odeSolver=0;

// This is an example of an exported function.
SBML_ODESOLVER_API int fnSBML_odeSolver(void)
{
	return 42;
}

// This is the constructor of a class that has been exported.
// see SBML_odeSolver.h for the class definition
CSBML_odeSolver::CSBML_odeSolver()
{ 
	return; 
}
*/
