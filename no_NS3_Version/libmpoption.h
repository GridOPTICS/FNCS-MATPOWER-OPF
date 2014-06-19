//
// MATLAB Compiler: 4.18.1 (R2013a)
// Date: Mon Jan 13 11:15:08 2014
// Arguments: "-B" "macro_default" "-W" "cpplib:libmpoption" "-T" "link:lib"
// "-d" "/home/laurentiu/work/arch_design/matpower4.1/CPP_wrapper_MATLAB/" "-I"
// "/home/laurentiu/work/arch_design/matpower4.1/" "-I"
// "/usr/local/matlab_2013a/bin/glnxa64/" "-v" "mpoption.m" "-N" "-p"
// "/usr/local/matlab_2013a/toolbox/optim/" 
//

#ifndef __libmpoption_h
#define __libmpoption_h 1

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

#ifdef EXPORTING_libmpoption
#define PUBLIC_libmpoption_C_API __global
#else
#define PUBLIC_libmpoption_C_API /* No import statement needed. */
#endif

#define LIB_libmpoption_C_API PUBLIC_libmpoption_C_API

#elif defined(_HPUX_SOURCE)

#ifdef EXPORTING_libmpoption
#define PUBLIC_libmpoption_C_API __declspec(dllexport)
#else
#define PUBLIC_libmpoption_C_API __declspec(dllimport)
#endif

#define LIB_libmpoption_C_API PUBLIC_libmpoption_C_API


#else

#define LIB_libmpoption_C_API

#endif

/* This symbol is defined in shared libraries. Define it here
 * (to nothing) in case this isn't a shared library. 
 */
#ifndef LIB_libmpoption_C_API 
#define LIB_libmpoption_C_API /* No special import/export declaration */
#endif

extern LIB_libmpoption_C_API 
bool MW_CALL_CONV libmpoptionInitializeWithHandlers(
       mclOutputHandlerFcn error_handler, 
       mclOutputHandlerFcn print_handler);

extern LIB_libmpoption_C_API 
bool MW_CALL_CONV libmpoptionInitialize(void);

extern LIB_libmpoption_C_API 
void MW_CALL_CONV libmpoptionTerminate(void);



extern LIB_libmpoption_C_API 
void MW_CALL_CONV libmpoptionPrintStackTrace(void);

extern LIB_libmpoption_C_API 
bool MW_CALL_CONV mlxMpoption(int nlhs, mxArray *plhs[], int nrhs, mxArray *prhs[]);


#ifdef __cplusplus
}
#endif

#ifdef __cplusplus

/* On Windows, use __declspec to control the exported API */
#if defined(_MSC_VER) || defined(__BORLANDC__)

#ifdef EXPORTING_libmpoption
#define PUBLIC_libmpoption_CPP_API __declspec(dllexport)
#else
#define PUBLIC_libmpoption_CPP_API __declspec(dllimport)
#endif

#define LIB_libmpoption_CPP_API PUBLIC_libmpoption_CPP_API

#else

#if !defined(LIB_libmpoption_CPP_API)
#if defined(LIB_libmpoption_C_API)
#define LIB_libmpoption_CPP_API LIB_libmpoption_C_API
#else
#define LIB_libmpoption_CPP_API /* empty! */ 
#endif
#endif

#endif

extern LIB_libmpoption_CPP_API void MW_CALL_CONV mpoption(int nargout, mwArray& options, mwArray& names, const mwArray& varargin);

extern LIB_libmpoption_CPP_API void MW_CALL_CONV mpoption(int nargout, mwArray& options, mwArray& names);

#endif
#endif
