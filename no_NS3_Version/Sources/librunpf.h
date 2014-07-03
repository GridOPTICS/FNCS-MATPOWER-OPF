//
// MATLAB Compiler: 4.18.1 (R2013a)
// Date: Mon Apr  7 17:29:10 2014
// Arguments: "-B" "macro_default" "-W" "cpplib:librunpf" "-T" "link:lib" "-d"
// "/home/laurentiu/work/arch_design/matpower4.1/CPP_wrapper_MATLAB/" "-I"
// "/home/laurentiu/work/arch_design/matpower4.1/" "-v" "runpf.m" 
//

#ifndef __librunpf_h
#define __librunpf_h 1

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

#ifdef EXPORTING_librunpf
#define PUBLIC_librunpf_C_API __global
#else
#define PUBLIC_librunpf_C_API /* No import statement needed. */
#endif

#define LIB_librunpf_C_API PUBLIC_librunpf_C_API

#elif defined(_HPUX_SOURCE)

#ifdef EXPORTING_librunpf
#define PUBLIC_librunpf_C_API __declspec(dllexport)
#else
#define PUBLIC_librunpf_C_API __declspec(dllimport)
#endif

#define LIB_librunpf_C_API PUBLIC_librunpf_C_API


#else

#define LIB_librunpf_C_API

#endif

/* This symbol is defined in shared libraries. Define it here
 * (to nothing) in case this isn't a shared library. 
 */
#ifndef LIB_librunpf_C_API 
#define LIB_librunpf_C_API /* No special import/export declaration */
#endif

extern LIB_librunpf_C_API 
bool MW_CALL_CONV librunpfInitializeWithHandlers(
       mclOutputHandlerFcn error_handler, 
       mclOutputHandlerFcn print_handler);

extern LIB_librunpf_C_API 
bool MW_CALL_CONV librunpfInitialize(void);

extern LIB_librunpf_C_API 
void MW_CALL_CONV librunpfTerminate(void);



extern LIB_librunpf_C_API 
void MW_CALL_CONV librunpfPrintStackTrace(void);

extern LIB_librunpf_C_API 
bool MW_CALL_CONV mlxRunpf(int nlhs, mxArray *plhs[], int nrhs, mxArray *prhs[]);


#ifdef __cplusplus
}
#endif

#ifdef __cplusplus

/* On Windows, use __declspec to control the exported API */
#if defined(_MSC_VER) || defined(__BORLANDC__)

#ifdef EXPORTING_librunpf
#define PUBLIC_librunpf_CPP_API __declspec(dllexport)
#else
#define PUBLIC_librunpf_CPP_API __declspec(dllimport)
#endif

#define LIB_librunpf_CPP_API PUBLIC_librunpf_CPP_API

#else

#if !defined(LIB_librunpf_CPP_API)
#if defined(LIB_librunpf_C_API)
#define LIB_librunpf_CPP_API LIB_librunpf_C_API
#else
#define LIB_librunpf_CPP_API /* empty! */ 
#endif
#endif

#endif

extern LIB_librunpf_CPP_API void MW_CALL_CONV runpf(int nargout, mwArray& MVAbase, mwArray& bus, mwArray& gen, mwArray& branch, mwArray& success, mwArray& et, const mwArray& casedata, const mwArray& mpopt, const mwArray& fname, const mwArray& solvedcase);

#endif
#endif
