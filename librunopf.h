//
// MATLAB Compiler: 4.18.1 (R2013a)
// Date: Tue Apr  8 11:01:22 2014
// Arguments: "-B" "macro_default" "-W" "cpplib:librunopf" "-T" "link:lib" "-d"
// "/home/laurentiu/work/arch_design/matpower4.1/CPP_wrapper_MATLAB/" "-v"
// "runopf.m" 
//

#ifndef __librunopf_h
#define __librunopf_h 1

#if defined(__cplusplus) && !defined(mclmcrrt_h) && defined(__linux__)
#  pragma implementation "mclmcrrt.h"
#endif
#include "mclmcrrt.h"
#include "mclcppclass.h"
#ifdef __cplusplus
extern "C" {
#endif

#if defined(__SUNPRO_CC)
/* Solaris shared libraries use __global, rather than mapfiles
 * to define the API exported from a shared library. __global is
 * only necessary when building the library -- files including
 * this header file to use the library do not need the __global
 * declaration; hence the EXPORTING_<library> logic.
 */

#ifdef EXPORTING_librunopf
#define PUBLIC_librunopf_C_API __global
#else
#define PUBLIC_librunopf_C_API /* No import statement needed. */
#endif

#define LIB_librunopf_C_API PUBLIC_librunopf_C_API

#elif defined(_HPUX_SOURCE)

#ifdef EXPORTING_librunopf
#define PUBLIC_librunopf_C_API __declspec(dllexport)
#else
#define PUBLIC_librunopf_C_API __declspec(dllimport)
#endif

#define LIB_librunopf_C_API PUBLIC_librunopf_C_API


#else

#define LIB_librunopf_C_API

#endif

/* This symbol is defined in shared libraries. Define it here
 * (to nothing) in case this isn't a shared library. 
 */
#ifndef LIB_librunopf_C_API 
#define LIB_librunopf_C_API /* No special import/export declaration */
#endif

extern LIB_librunopf_C_API 
bool MW_CALL_CONV librunopfInitializeWithHandlers(
       mclOutputHandlerFcn error_handler, 
       mclOutputHandlerFcn print_handler);

extern LIB_librunopf_C_API 
bool MW_CALL_CONV librunopfInitialize(void);

extern LIB_librunopf_C_API 
void MW_CALL_CONV librunopfTerminate(void);



extern LIB_librunopf_C_API 
void MW_CALL_CONV librunopfPrintStackTrace(void);

extern LIB_librunopf_C_API 
bool MW_CALL_CONV mlxRunopf(int nlhs, mxArray *plhs[], int nrhs, mxArray *prhs[]);


#ifdef __cplusplus
}
#endif

#ifdef __cplusplus

/* On Windows, use __declspec to control the exported API */
#if defined(_MSC_VER) || defined(__BORLANDC__)

#ifdef EXPORTING_librunopf
#define PUBLIC_librunopf_CPP_API __declspec(dllexport)
#else
#define PUBLIC_librunopf_CPP_API __declspec(dllimport)
#endif

#define LIB_librunopf_CPP_API PUBLIC_librunopf_CPP_API

#else

#if !defined(LIB_librunopf_CPP_API)
#if defined(LIB_librunopf_C_API)
#define LIB_librunopf_CPP_API LIB_librunopf_C_API
#else
#define LIB_librunopf_CPP_API /* empty! */ 
#endif
#endif

#endif

extern LIB_librunopf_CPP_API void MW_CALL_CONV runopf(int nargout, mwArray& MVAbase, mwArray& bus, mwArray& gen, mwArray& gencost, mwArray& branch, mwArray& f, mwArray& success, mwArray& et, const mwArray& casedata, const mwArray& mpopt, const mwArray& fname, const mwArray& solvedcase);

#endif
#endif
