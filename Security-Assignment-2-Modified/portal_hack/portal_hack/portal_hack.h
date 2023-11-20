// The following ifdef block is the standard way of creating macros which make exporting
// from a DLL simpler. All files within this DLL are compiled with the PORTALHACK_EXPORTS
// symbol defined on the command line. This symbol should not be defined on any project
// that uses this DLL. This way any other project whose source files include this file see
// PORTALHACK_API functions as being imported from a DLL, whereas this DLL sees symbols
// defined with this macro as being exported.
#ifdef PORTALHACK_EXPORTS
#define PORTALHACK_API __declspec(dllexport)
#else
#define PORTALHACK_API __declspec(dllimport)
#endif

// This class is exported from the dll
class PORTALHACK_API Cportalhack {
public:
	Cportalhack(void);
	// TODO: add your methods here.
};

extern PORTALHACK_API int nportalhack;

PORTALHACK_API int fnportalhack(void);
