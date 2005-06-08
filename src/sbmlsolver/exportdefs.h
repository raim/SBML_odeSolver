// The following ifdef block is the standard way of creating macros which make exporting 
// from a DLL simpler. All files within this DLL are compiled with the SBML_ODESOLVER_EXPORTS
// symbol defined on the command line. this symbol should not be defined on any project
// that uses this DLL. This way any other project whose source files include this file see 
// SBML_ODESOLVER_API functions as being imported from a DLL, whereas this DLL sees symbols
// defined with this macro as being exported.
#ifdef WIN32
#ifdef SBML_ODESOLVER_EXPORTS
#define SBML_ODESOLVER_API __declspec(dllexport)
#else
#define SBML_ODESOLVER_API __declspec(dllimport)
#endif
#else
#define SBML_ODESOLVER_API
#endif

/* examples of use...

// This class is exported from the SBML_odeSolver.dll
class SBML_ODESOLVER_API CSBML_odeSolver {
public:
	CSBML_odeSolver(void);
	// TODO: add your methods here.
};

extern SBML_ODESOLVER_API int nSBML_odeSolver;

SBML_ODESOLVER_API int fnSBML_odeSolver(void);

*/