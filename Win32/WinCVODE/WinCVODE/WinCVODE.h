// The following ifdef block is the standard way of creating macros which make exporting 
// from a DLL simpler. All files within this DLL are compiled with the WINCVODE_EXPORTS
// symbol defined on the command line. this symbol should not be defined on any project
// that uses this DLL. This way any other project whose source files include this file see 
// WINCVODE_API functions as being imported from a DLL, whereas this DLL sees symbols
// defined with this macro as being exported.
#ifdef WINCVODE_EXPORTS
#define WINCVODE_API __declspec(dllexport)
#else
#define WINCVODE_API __declspec(dllimport)
#endif

/* examples ...
// This class is exported from the WinCVODE.dll
class WINCVODE_API CWinCVODE {
public:
	CWinCVODE(void);
	// TODO: add your methods here.
};

extern WINCVODE_API int nWinCVODE;

WINCVODE_API int fnWinCVODE(void);
*/
