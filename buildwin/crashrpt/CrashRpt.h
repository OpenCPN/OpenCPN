/************************************************************************************* 
This file is a part of CrashRpt library.
Copyright (c) 2003-2013 The CrashRpt project authors. All Rights Reserved.

Use of this source code is governed by a BSD-style license
that can be found in the License.txt file in the root of the source
tree. All contributing project authors may
be found in the Authors.txt file in the root of the source tree.
***************************************************************************************/

/*! \file   CrashRpt.h
*  \brief  Defines the interface for the CrashRpt.DLL.
*  \date   2003
*  \author Michael Carruth 
*  \author Oleg Krivtsov (zeXspectrum) 
*/

#ifndef _CRASHRPT_H_
#define _CRASHRPT_H_

#include <windows.h>
#include <dbghelp.h>

// Define SAL macros to be empty if some old Visual Studio used
#ifndef __reserved
#define __reserved
#endif
#ifndef __in
#define __in
#endif
#ifndef __in_opt
#define __in_opt
#endif
#ifndef __out_ecount_z
#define __out_ecount_z(x)
#endif

#ifdef __cplusplus
#define CRASHRPT_EXTERNC extern "C"
#else
#define CRASHRPT_EXTERNC
#endif

#define CRASHRPTAPI(rettype) CRASHRPT_EXTERNC rettype WINAPI

//! Current CrashRpt version
#define CRASHRPT_VER 1403

/*! \defgroup CrashRptAPI CrashRpt Functions */
/*! \defgroup DeprecatedAPI Obsolete Functions */
/*! \defgroup CrashRptStructs CrashRpt Structures */
/*! \defgroup CrashRptWrappers CrashRpt Wrapper Classes */

/*! \ingroup DeprecatedAPI
*  \brief Client crash callback function prototype
*  \param[in] lpvState Must be set to NULL.
*
*  \remarks
*
*  This function is deprecated, it is recommended to use \ref PFNCRASHCALLBACK()
*  instead.
*
*  The crash callback function is called when crash occurs. This way client application is
*  notified about the crash.
*
*  It is generally unsafe to do complex actions (e.g. memory allocation, heap operations) inside of this callback.
*  The application state may be unstable.
*
*  One reason the application may use this callback for is to close handles to open log files that the 
*  application plans to include into the error report. Files should be accessible for reading, otherwise
*  CrashRpt won't be able to include them into error report.
*
*  It is also possible (but not recommended) to add files, properties, desktop screenshots, 
*  registry keys inside of the crash callback function.
*  
*  The crash callback function should typically return \c TRUE to allow generate error report.  
*  Returning \c FALSE will prevent crash report generation.
*
*  The following example shows how to use the crash callback function.
*
*  \code
*  // define the crash callback
*  BOOL CALLBACK CrashCallback(LPVOID lpvState)
*  {    
*     // Do something...
*
*     return TRUE;
*  }
*  \endcode
*
*  \sa crAddFile2(), PFNCRASHCALLBACK()
*/
typedef BOOL (CALLBACK *LPGETLOGFILE) (__reserved LPVOID lpvState);

// Exception types used in CR_EXCEPTION_INFO::exctype structure member.
#define CR_SEH_EXCEPTION                0    //!< SEH exception.
#define CR_CPP_TERMINATE_CALL           1    //!< C++ terminate() call.
#define CR_CPP_UNEXPECTED_CALL          2    //!< C++ unexpected() call.
#define CR_CPP_PURE_CALL                3    //!< C++ pure virtual function call (VS .NET and later).
#define CR_CPP_NEW_OPERATOR_ERROR       4    //!< C++ new operator fault (VS .NET and later).
#define CR_CPP_SECURITY_ERROR           5    //!< Buffer overrun error (VS .NET only).
#define CR_CPP_INVALID_PARAMETER        6    //!< Invalid parameter exception (VS 2005 and later).
#define CR_CPP_SIGABRT                  7    //!< C++ SIGABRT signal (abort).
#define CR_CPP_SIGFPE                   8    //!< C++ SIGFPE signal (flotating point exception).
#define CR_CPP_SIGILL                   9    //!< C++ SIGILL signal (illegal instruction).
#define CR_CPP_SIGINT                   10   //!< C++ SIGINT signal (CTRL+C).
#define CR_CPP_SIGSEGV                  11   //!< C++ SIGSEGV signal (invalid storage access).
#define CR_CPP_SIGTERM                  12   //!< C++ SIGTERM signal (termination request).


/*! \ingroup CrashRptStructs
*   \brief This structure contains information about the crash.
*  
*  The information provided by this structure includes the exception type, exception code, 
*  exception pointers and so on. These are needed to generate crash minidump file and
*  provide the developer with other information about the error. This structure is used by
*  the crGenerateErrorReport() function. Pointer to this structure is also passed to the
*  crash callback function (see the \ref PFNCRASHCALLBACK() function prototype).
*
*  Structure members details are provided below:
*
*  \b cb [in] 
*
*  This must contain the size of this structure in bytes.
*
*  \b pexcptrs [in, optional]
*
*    Should contain the exception pointers. If this parameter is NULL, 
*    the current CPU state is used to generate exception pointers.
*
*  \b exctype [in]
*  
*    The type of exception. This parameter may be one of the following:
*     - \ref CR_SEH_EXCEPTION             SEH (Structured Exception Handling) exception
*     - \ref CR_CPP_TERMINATE_CALL        C++ terminate() function call
*     - \ref CR_CPP_UNEXPECTED_CALL       C++ unexpected() function call
*     - \ref CR_CPP_PURE_CALL             Pure virtual method call (Visual Studio .NET 2003 and later) 
*     - \ref CR_CPP_NEW_OPERATOR_ERROR    C++ 'new' operator error (Visual Studio .NET 2003 and later)
*     - \ref CR_CPP_SECURITY_ERROR        Buffer overrun (Visual Studio .NET 2003 only) 
*     - \ref CR_CPP_INVALID_PARAMETER     Invalid parameter error (Visual Studio 2005 and later) 
*     - \ref CR_CPP_SIGABRT               C++ SIGABRT signal 
*     - \ref CR_CPP_SIGFPE                C++ floating point exception
*     - \ref CR_CPP_SIGILL                C++ illegal instruction
*     - \ref CR_CPP_SIGINT                C++ SIGINT signal
*     - \ref CR_CPP_SIGSEGV               C++ invalid storage access
*     - \ref CR_CPP_SIGTERM               C++ termination request
* 
*   \b code [in, optional]
*
*      Used if \a exctype is \ref CR_SEH_EXCEPTION and represents the SEH exception code. 
*      If \a pexptrs is NULL, this value is used when generating exception information for initializing
*      \c pexptrs->ExceptionRecord->ExceptionCode member, otherwise it is ignored.
*
*   \b fpe_subcode [in, optional]
*
*      Used if \a exctype is equal to \ref CR_CPP_SIGFPE. It defines the floating point
*      exception subcode (see \c signal() function ducumentation in MSDN).
* 
*   \b expression, \b function, \b file and \b line [in, optional]
*
*     These parameters are used when \a exctype is \ref CR_CPP_INVALID_PARAMETER. 
*     These members are typically non-zero when using debug version of CRT.
*
*  \b bManual [in]
*
*     Since v.1.2.4, \a bManual parameter should be equal to TRUE if the report is generated manually. 
*     The value of \a bManual parameter affects the automatic application restart behavior. If the application
*     restart is requested by the \ref CR_INST_APP_RESTART flag of CR_INSTALL_INFO::dwFlags structure member, 
*     and if \a bManual is FALSE, the application will be
*     restarted after error report generation. If \a bManual is TRUE, the application won't be restarted.
*
*  \b hSenderProcess [out]
*
*     As of v.1.2.8, \a hSenderProcess parameter will contain the handle to the <b>CrashSender.exe</b> process when 
*     \ref crGenerateErrorReport function returns. The caller may use this handle to wait until <b>CrashSender.exe</b> 
*     process exits and check the exit code. When the handle is not needed anymore, release it with the \b CloseHandle() function.
*/

typedef struct tagCR_EXCEPTION_INFO
{
    WORD cb;                   //!< Size of this structure in bytes; should be initialized before using.
    PEXCEPTION_POINTERS pexcptrs; //!< Exception pointers.
    int exctype;               //!< Exception type.
    DWORD code;                //!< Code of SEH exception.
    unsigned int fpe_subcode;  //!< Floating point exception subcode.
    const wchar_t* expression; //!< Assertion expression.
    const wchar_t* function;   //!< Function in which assertion happened.
    const wchar_t* file;       //!< File in which assertion happened.
    unsigned int line;         //!< Line number.
    BOOL bManual;              //!< Flag telling if the error report is generated manually or not.
    HANDLE hSenderProcess;     //!< Handle to the CrashSender.exe process.
}
CR_EXCEPTION_INFO;

typedef CR_EXCEPTION_INFO* PCR_EXCEPTION_INFO;

// Stages of crash report generation (used by the crash callback function).
#define CR_CB_STAGE_PREPARE      10  //!< Stage after exception pointers've been retrieved.
#define CR_CB_STAGE_FINISH       20  //!< Stage after the launch of CrashSender.exe process.

/*! \ingroup CrashRptStructs
*  \struct CR_CRASH_CALLBACK_INFOW()
*  \brief This structure contains information passed to crash callback function PFNCRASHCALLBACK().
*
*  \remarks
*  
*  The information contained in this structure may be used by the crash callback function
*  to determine what type of crash has occurred and perform some action. For example,
*  the client application may prefer to continue its execution on some type of crash, and
*  terminate itself on another type of crash.
*
*  Below, the stucture fields are described:
*
*  \b cb [in] 
*
*    This contains the size of this structure in bytes. 
*
*  \b nStage [in]
*
*    This field specifies the crash report generation stage. The callback function
*    can be called once per each stage (depending on callback function's return value).
*    Currently, there are two stages:
*      - \ref CR_CB_STAGE_PREPARE   Stage after exception pointers've been retrieved.
*      - \ref CR_CB_STAGE_FINISH    Stage after the launch of CrashSender.exe process.
*
*  \b pszErrorReportFolder [in]
*
*    This field contains the absolute path to the directory containing uncompressed 
*    crash report files. 
*
*  \b pExceptionInfo [in]
*
*    This field contains a pointer to \ref CR_EXCEPTION_INFO structure.
*
*  \b pUserParam [in, optional]
*
*    This is a pointer to user-specified data passed to the crSetCrashCallback() function
*    as \b pUserParam argument.
*
*  \b bContinueExecution [in, out]
*
*    This field is set to FALSE by default. The crash callback function may set it
*    to true if it wants to continue its execution after crash report generation 
*    (otherwise the program will be terminated).
*
*  \ref CR_CRASH_CALLBACK_INFOW and \ref CR_CRASH_CALLBACK_INFOA are 
*  wide-character and multi-byte character versions of \ref CR_CRASH_CALLBACK_INFO
*  structure. In your program, use the \ref CR_CRASH_CALLBACK_INFO typedef which 
*  is a character-set-independent version of the structure name.
*
*  \sa PFNCRASHCALLBACK()
*/
typedef struct tagCR_CRASH_CALLBACK_INFOW
{
    WORD cb;                            //!< Size of this structure in bytes.
	int nStage;                         //!< Stage.
	LPCWSTR pszErrorReportFolder;       //!< Directory where crash report files are located.
    CR_EXCEPTION_INFO* pExceptionInfo;  //!< Pointer to information about the crash.
	LPVOID pUserParam;                  //!< Pointer to user-defined data.
	BOOL bContinueExecution;            //!< Whether to terminate the process (the default) or to continue program execution.		
}
CR_CRASH_CALLBACK_INFOW;

/*! \ingroup CrashRptStructs
*  \struct CR_CRASH_CALLBACK_INFOA
*  \copydoc CR_CRASH_CALLBACK_INFOW
*/
typedef struct tagCR_CRASH_CALLBACK_INFOA
{
    WORD cb;                            //!< Size of this structure in bytes.
	int nStage;                         //!< Stage.
	LPCSTR pszErrorReportFolder;        //!< Directory where crash report files are located.
    CR_EXCEPTION_INFO* pExceptionInfo;  //!< Pointer to information about the crash.
	LPVOID pUserParam;                  //!< Pointer to user-defined data.
	BOOL bContinueExecution;            //!< Whether to terminate the process (the default) or to continue program execution.		
}
CR_CRASH_CALLBACK_INFOA;

/*! \brief Character set-independent mapping of CR_CRASH_CALLBACK_INFOW and CR_CRASH_CALLBACK_INFOA structures.
*  \ingroup CrashRptStructs
*/
#ifdef UNICODE
typedef CR_CRASH_CALLBACK_INFOW CR_CRASH_CALLBACK_INFO;
#else
typedef CR_CRASH_CALLBACK_INFOA CR_CRASH_CALLBACK_INFO;
#endif // UNICODE

// Constants that may be returned by the crash callback function.
#define CR_CB_CANCEL             0 //!< Cancel crash report generation on the current stage.
#define CR_CB_DODEFAULT          1 //!< Proceed to the next stages of crash report generation without calling crash callback function.
#define CR_CB_NOTIFY_NEXT_STAGE  2 //!< Proceed and call the crash callback for the next stage.

/*! \ingroup CrashRptAPI
*  \brief Client crash callback function prototype.
*  \param[in] pInfo Points to information about the crash.
*
*  \remarks
*
*  The crash callback function is called when a crash occurs. This way client application is
*  notified about the crash.
*
*  Crash information is passed by CrashRpt to the callback function through the \b pInfo parameter as
*  a pointer to \ref CR_CRASH_CALLBACK_INFO structure. See below for a code example. 
*
*  It is generally unsafe to do complex actions (e.g. memory allocation, heap operations) inside of this callback.
*  The application state may be unstable.
*
*  One reason the application may use this callback for is to close handles to open log files that the 
*  application plans to include into the error report. Files should be accessible for reading, otherwise
*  CrashRpt won't be able to include them into error report.
*
*  It is also possible (but not recommended) to add files (see crAddFile2()), 
*  properties (see crAddProperty()), desktop screenshots (see crAddScreenshot2())
*  and registry keys (see crAddRegKey()) inside of the crash callback function.
*
*  By default, CrashRpt terminates the client application after crash report generation and
*  launching the <i>CrashSender.exe</i> process. However, it is possible to continue program
*  execution after crash report generation by seting \ref CR_CRASH_CALLBACK_INFO::bContinueExecution
*  structure field to \a TRUE.
*
*  The crash report generation consists of several stages. First, exception pointers are retrieved
*  and the callback function is called for the first time. The callback function may check the
*  retrieved exception information and decide wheter to proceed with crash report generation or to
*  continue client program execution. On the next stage, the \a CrashSender.exe
*  process is launched and the crash callback function is (optionally) called for the second time.
*  Further crash report data collection and delivery work is performed in \a CrashSender.exe process. 
*  The crash callback may use the provided handle to \a CrashSender.exe process to wait until it exits.
*
*  The crash callback function should typically return \ref CR_CB_DODEFAULT constant to proceed 
*  with error report generation without being called back on the next stage(s). Returning the
*  \ref CR_CB_NOTIFY_NEXT_STAGE constant causes CrashRpt to call the crash callback function on the next
*  stage, too. Returning \ref CR_CB_CANCEL constant will prevent further stage(s) of crash report generation.
*
*  \ref PFNCRASHCALLBACKW() and \ref PFNCRASHCALLBACKA() are 
*  wide-character and multi-byte character versions of \ref PFNCRASHCALLBACK()
*  function. 
*
*  The following code example shows the simplest way of using the crash callback function:
*
*  \code
*  // Define the crash callback
*  int CALLBACK CrashCallback(CR_CRASH_CALLBACK_INFO* pInfo)
*  {    
*     
*     // Do something...
*
*     // Proceed with crash report generation. 
*     // This return code also makes CrashRpt to not call this callback function for 
*     // the next crash report generation stage.
*     return CR_CB_DODEFAULT;
*  }
*  \endcode
*
*  The following code example shows how to use the crash callback function to be notified
*  on every stage of crash report generation:
*
*  \code
*  // Define the crash callback
*  int CALLBACK CrashCallback(CR_CRASH_CALLBACK_INFO* pInfo)
*  {    
*     
*     // We want to continue program execution after crash report generation
*     pInfo->bContinueExecution = TRUE;
*
*     switch(pInfo->nStage)
*     {
*         case CR_CB_STAGE_PREPARE:
*               // do something
*               break;
*         case CR_CB_STAGE_FINISH:
*               // do something
*               break;
*     }
*
*     // Proceed to the next stage. 
*     return CR_CB_NOTIFY_NEXT_STAGE;
*  }
*  \endcode
*
*  \sa CR_CRASH_CALLBACK_INFO, crSetCrashCallback(), crAddFile2(), crAddProperty(), crAddScreenshot2(), crAddRegKey()
*/
typedef int (CALLBACK *PFNCRASHCALLBACKW) (CR_CRASH_CALLBACK_INFOW* pInfo);

/*! \ingroup CrashRptAPI
*  \copydoc PFNCRASHCALLBACKW()
*/
typedef int (CALLBACK *PFNCRASHCALLBACKA) (CR_CRASH_CALLBACK_INFOA* pInfo);

/*! \brief Character set-independent mapping of \ref PFNCRASHCALLBACKW() and \ref PFNCRASHCALLBACKA() function prototrypes.
*  \ingroup CrashRptStructs
*/
#ifdef UNICODE
typedef PFNCRASHCALLBACKW PFNCRASHCALLBACK;
#else
typedef PFNCRASHCALLBACKA PFNCRASHCALLBACK;
#endif // UNICODE

/*! \ingroup CrashRptAPI  
*  \brief Sets the crash callback function.
* 
*  \return This function returns zero if succeeded. Use crGetLastErrorMsg() to retrieve the error message on fail.
*
*  \param[in] pfnCallbackFunc  Pointer to the crash callback function.
*  \param[in] lpParam          User defined parameter. Optional. 
*  
*  \remarks 
*
*  Use this to set the crash callback function that will be called on crash. This function
*  is available since v.1.4.0.
*
*  For the crash callback function prototype, see documentation for PFNCRASHCALLBACK().
*
*  Optional \b lpParam parameter can be a pointer to user-defined data. It will be passed to the 
*  crash callback function as \ref CR_CRASH_CALLBACK_INFO::pUserParam structure member.
*
*  \sa
*   PFNCRASHCALLBACK()
*/

CRASHRPTAPI(int)
crSetCrashCallbackW(   
             PFNCRASHCALLBACKW pfnCallbackFunc,
			 LPVOID lpParam
             );


/*! \ingroup CrashRptAPI
*  \copydoc crSetCrashCallbackW()
*/
CRASHRPTAPI(int)
crSetCrashCallbackA(   
             PFNCRASHCALLBACKA pfnCallbackFunc,
			 LPVOID lpParam
             );


/*! \brief Character set-independent mapping of crSetCrashCallbackW() and crSetCrashCallbackA() functions. 
*  \ingroup CrashRptAPI
*/
#ifdef UNICODE
#define crSetCrashCallback crSetCrashCallbackW
#else
#define crSetCrashCallback crSetCrashCallbackA
#endif //UNICODE

// Array indices for CR_INSTALL_INFO::uPriorities.
#define CR_HTTP 0  //!< Send error report via HTTP (or HTTPS) connection.
#define CR_SMTP 1  //!< Send error report via SMTP connection.
#define CR_SMAPI 2 //!< Send error report via simple MAPI (using default mail client).

//! Special priority constant that allows to skip certain delivery method.
#define CR_NEGATIVE_PRIORITY ((UINT)-1)

// Flags for CR_INSTALL_INFO::dwFlags
#define CR_INST_STRUCTURED_EXCEPTION_HANDLER      0x1 //!< Install SEH handler (deprecated name, use \ref CR_INST_SEH_EXCEPTION_HANDLER instead).
#define CR_INST_SEH_EXCEPTION_HANDLER             0x1 //!< Install SEH handler.
#define CR_INST_TERMINATE_HANDLER                 0x2 //!< Install terminate handler.
#define CR_INST_UNEXPECTED_HANDLER                0x4 //!< Install unexpected handler.
#define CR_INST_PURE_CALL_HANDLER                 0x8 //!< Install pure call handler (VS .NET and later).
#define CR_INST_NEW_OPERATOR_ERROR_HANDLER       0x10 //!< Install new operator error handler (VS .NET and later).
#define CR_INST_SECURITY_ERROR_HANDLER           0x20 //!< Install security error handler (VS .NET and later).
#define CR_INST_INVALID_PARAMETER_HANDLER        0x40 //!< Install invalid parameter handler (VS 2005 and later).
#define CR_INST_SIGABRT_HANDLER                  0x80 //!< Install SIGABRT signal handler.
#define CR_INST_SIGFPE_HANDLER                  0x100 //!< Install SIGFPE signal handler.   
#define CR_INST_SIGILL_HANDLER                  0x200 //!< Install SIGILL signal handler.  
#define CR_INST_SIGINT_HANDLER                  0x400 //!< Install SIGINT signal handler.  
#define CR_INST_SIGSEGV_HANDLER                 0x800 //!< Install SIGSEGV signal handler.
#define CR_INST_SIGTERM_HANDLER                0x1000 //!< Install SIGTERM signal handler.  

#define CR_INST_ALL_POSSIBLE_HANDLERS          0x1FFF //!< Install all possible exception handlers.
#define CR_INST_CRT_EXCEPTION_HANDLERS         0x1FFE //!< Install exception handlers for the linked CRT module.

#define CR_INST_NO_GUI                         0x2000 //!< Do not show GUI, send report silently (use for non-GUI apps only).
#define CR_INST_HTTP_BINARY_ENCODING           0x4000 //!< Deprecated, do not use.
#define CR_INST_DONT_SEND_REPORT               0x8000 //!< Don't send error report immediately, just save it locally.
#define CR_INST_APP_RESTART                   0x10000 //!< Restart the application on crash.
#define CR_INST_NO_MINIDUMP                   0x20000 //!< Do not include minidump file to crash report.
#define CR_INST_SEND_QUEUED_REPORTS           0x40000 //!< CrashRpt should send error reports that are waiting to be delivered.
#define CR_INST_STORE_ZIP_ARCHIVES            0x80000 //!< CrashRpt should store both uncompressed error report files and ZIP archives.
#define CR_INST_SEND_MANDATORY				 0x100000 //!< This flag removes the "Close" and "Other actions" buttons from Error Report dialog, thus making the sending procedure mandatory for user.
#define CR_INST_SHOW_ADDITIONAL_INFO_FIELDS	 0x200000 //!< Makes "Your E-mail" and "Describe what you were doing when the problem occurred" fields of Error Report dialog always visible.
#define CR_INST_ALLOW_ATTACH_MORE_FILES		 0x400000 //!< Adds an ability for user to attach more files to crash report by clicking "Attach More File(s)" item from context menu of Error Report Details dialog.
#define CR_INST_AUTO_THREAD_HANDLERS         0x800000 //!< If this flag is set, installs exception handlers for newly created threads automatically.

/*! \ingroup CrashRptStructs
*  \struct CR_INSTALL_INFOW()
*  \brief This structure defines the general information used by crInstallW() function.
*
*  \remarks
*
*    \ref CR_INSTALL_INFOW and \ref CR_INSTALL_INFOA structures are wide-character and multi-byte character 
*    versions of \ref CR_INSTALL_INFO. \ref CR_INSTALL_INFO typedef defines character set independent mapping.
*
*    Below, structure members are described in details. Required parameters must always be specified, while optional
*    ones may be set with 0 (zero) or NULL. Most of parameters are optional.
*
*    \b cb [in, required] 
*
*    This must contain the size of this structure in bytes. 
*
*    \b pszAppName [in, optional] 
*
*       This is the friendly name of the client application. The application name is
*       displayed in the Error Report dialog. If this parameter is NULL, the name of EXE file 
*       that was used to start caller process becomes the application name.
*
*    \b pszAppVersion [in, optional] 
*
*       Should be the application version. Example: "1.0.1". 
*
*       If this equals to NULL, product version is extracted from the executable file which started 
*       the caller process, and this product version is used as application version. If the executable file
*       doesn's have a version info resource, the \ref crInstall() function will fail.
* 
*    \b pszEmailTo [in, optional] 
*
*       This is the email address of the recipient of error reports (or several E-mail adresses separated with semicolon), 
*		for example "name@example.com" or "person1@example.com;person2@someserver.com". If several E-mail addresses are
*       specified, error report will be delivered to each of them. If this parameter equals to NULL, 
*       the crash report won't be sent using E-mail client.
*
*       Keep this NULL if you plan to use large error reports (more than several MB in size), because
*       large emails may be rejected by the mail server. 
*
*    \b pszEmailSubject [in, optional] 
*
*       This is the subject of the email message. If this parameter is NULL,
*       the default subject of form '[app_name] [app_version] Error Report' is generated.
*
*    \a pszUrl is the URL of a server-side script that would receive crash report data via HTTP or HTTPS 
*       connection. If this parmeter is NULL, HTTP(S) connection won't be used to send crash reports. For
*       example of a server-side script that can receive crash reports, see \ref sending_error_reports.
*
*       HTTP(S) transport is the recommended way of sending large error reports (more than several MB in size).
*       To define a custom port for HTTP(S) connection, use the following URL format: "http://example.com[:port]/crashrpt.php" or
*       "https://example.com[:port]/crashrpt.php", where optional \a port is the placeholder for the port number.
*
*    \b pszCrashSenderPath [in, optional] 
*
*       This is the absolute path to the directory where CrashSender.exe is located. 
*       The crash sender process is responsible for letting end user know about the crash and 
*       sending the error report. If this is NULL, it is assumed that CrashSender.exe is located in
*       the same directory as CrashRpt.dll.
*
*
*    \b uPriorities [in, optional]
*
*       This is an array that defines the preferred methods of sending error reports. 
*       The available methods are: HTTP (or HTTPS) connection, SMTP connection or simple MAPI (default mail client).
*
*       A priority is a non-negative integer number or special constant \ref CR_NEGATIVE_PRIORITY. 
*       The greater positive number defines the greater priority. 
*       Specify the \ref CR_NEGATIVE_PRIORITY to skip the given delivery method.
*
*       The element having index \ref CR_HTTP defines priority for using HTML connection.
*       The element having index \ref CR_SMTP defines priority for using SMTP connection.
*       The element having index \ref CR_SMAPI defines priority for using the default mail client.
*
*       The methods having greater priority will be tried first. If priorities are equal to each other, HTTP (or HTTPS)
*       connection will be tried the first, SMTP connection will be tried the second and simple MAPI will be tried
*       the last. 
*
*    \b dwFlags [in, optional]
*
*    Since v1.1.2, \a dwFlags can be used to define behavior parameters. This can be a combination of the following values:
*
*    <table>
*    <tr><td colspan="2"> <i>Use the combination of the following constants to specify what exception handlers to install:</i>
*    <tr><td> \ref CR_INST_ALL_POSSIBLE_HANDLERS      <td> Install all available exception handlers.
*    <tr><td> \ref CR_INST_SEH_EXCEPTION_HANDLER      <td> Install SEH exception handler.
*    <tr><td> \ref CR_INST_PURE_CALL_HANDLER          <td> Install pure call handler (VS .NET and later).
*    <tr><td> \ref CR_INST_NEW_OPERATOR_ERROR_HANDLER <td> Install new operator error handler (VS .NET and later).
*    <tr><td> \ref CR_INST_SECURITY_ERROR_HANDLER     <td> Install security errror handler (VS .NET and later).
*    <tr><td> \ref CR_INST_INVALID_PARAMETER_HANDLER  <td> Install invalid parameter handler (VS 2005 and later).
*    <tr><td> \ref CR_INST_SIGABRT_HANDLER            <td> Install SIGABRT signal handler.
*    <tr><td> \ref CR_INST_SIGINT_HANDLER             <td> Install SIGINT signal handler.  
*    <tr><td> \ref CR_INST_SIGTERM_HANDLER            <td> Install SIGTERM signal handler.  
*    <tr><td colspan="2"> <i>Use the combination of the following constants to define behavior parameters:</i>
*    <tr><td> \ref CR_INST_NO_GUI                
*        <td> <b>Available since v.1.2.2</b> Do not show GUI.
* 
*             It is not recommended to use this flag for regular GUI-based applications. 
*             Use this only for services that have no GUI.
*    <tr><td> \ref CR_INST_DONT_SEND_REPORT     
*        <td> <b>Available since v.1.2.2</b> This parameter means 'do not send error report immediately on crash, just save it locally'. 
*             Use this if you have direct access to the machine where crash happens and do not need 
*             to send report over the Internet. You can use this in couple with \ref CR_INST_STORE_ZIP_ARCHIVES flag to store zipped error reports
*             along with uncompressed error report files.
*    <tr><td> \ref CR_INST_APP_RESTART     
*        <td> <b>Available since v.1.2.4</b> This parameter allows to automatically restart the application on crash. The command line
*             for the application is taken from \a pszRestartCmdLine parameter. To avoid cyclic restarts of an application which crashes on startup, 
*             the application is restarted only if at least 60 seconds elapsed since its start.
*    <tr><td> \ref CR_INST_NO_MINIDUMP     
*        <td> <b>Available since v.1.2.4</b> Specify this parameter if you want minidump file not to be included into crash report. The default
*             behavior is to include the minidump file.
*
*    <tr><td> \ref CR_INST_SEND_QUEUED_REPORTS     
*        <td> <b>Available since v.1.2.5</b> Specify this parameter to send all queued reports. Those
*             report files are by default stored in <i>%LOCAL_APPDATA%\\CrashRpt\\UnsentCrashReports\\%AppName%_%AppVersion%</i> folder.
*             If this is specified, CrashRpt checks if it's time to remind user about recent errors in the application and offers to send
*             all queued error reports.
*
*    <tr><td> \ref CR_INST_STORE_ZIP_ARCHIVES     
*        <td> <b>Available since v.1.2.7</b> This parameter can be used in couple with \ref CR_INST_DONT_SEND_REPORT flag to store not only uncompressed
*             error report files, but also ZIP archives. By default (if this flag omitted) CrashRpt stores all error report files
*             in uncompressed state.
*
*    <tr><td> \ref CR_INST_SEND_MANDATORY     
*        <td> <b>Available since v.1.3.1</b> This parameter makes sending procedure mandatory by removing the "Close" button
*			  and "Other actions..." button from the Error Report dialog. Typically, it is not recommended to use this flag,
*             unless you intentionally want users to always send error reports for your application.
*    <tr><td> \ref CR_INST_SHOW_ADDITIONAL_INFO_FIELDS     
*        <td> <b>Available since v.1.3.1</b> This parameter makes "Your E-mail" and "Describe what you were doing when the 
*             problem occurred" fields of Error Report dialog always visible. By default (when this parameter not specified),
*             these fields are hidden and user needs to click the "Provide additional info (recommended)" link to show them.
*
*    <tr><td> \ref CR_INST_ALLOW_ATTACH_MORE_FILES     
*        <td> <b>Available since v.1.3.1</b> Adds an ability for user to attach more files to crash report by choosing 
*             "Attach More File(s)" item from context menu of Error Report Details dialog. By default this feature is disabled.
*
*    <tr><td> \ref CR_INST_AUTO_THREAD_HANDLERS     
*        <td> <b>Available since v.1.4.2</b> Specifying this flag results in automatic installation of all available exception handlers to
*             all threads that will be created in the future. This flag only works if CrashRpt is compiled as a DLL, it does 
*             not work if you compile CrashRpt as static library.
*   </table>
*
*   \b pszPrivacyPolicyURL [in, optional] 
*
*     This parameter defines the URL for the Privacy Policy hyperlink of the 
*     Error Report dialog. If this parameter is NULL, the link is not displayed. For information on 
*     the Privacy Policy, see \ref error_report. This parameter is available since v1.1.2.
*
*   \b pszDebugHelpDLL [in, optional] 
*  
*     This parameter defines the location of the dbghelp.dll to load. 
*     If this parameter is NULL, the dbghelp.dll is searched using the default search sequence.
*     This parameter is available since v1.2.1.
*
*   \b uMiniDumpType [in, optional] 
*
*     This parameter defines the minidump type. For the list of available minidump
*     types, see the documentation for the MiniDumpWriteDump() function in MSDN. 
*     Parameter is available since v.1.2.1.
*
*     It is recommended to set this 
*     parameter with zero (equivalent of MiniDumpNormal constant). Other values may increase the minidump 
*     size significantly. 
*
*   \b pszErrorReportSaveDir [in, optional] 
*
*     This parameter defines the directory where to save the error reports. 
*     If this is NULL, the default directory is used (%%LOCAL_APP_DATA%\\CrashRpt\\UnsentCrashReports\\%%AppName%%_%%AppVersion%).
*     This parameter is available since v.1.2.2.
*
*   \b pszRestartCmdLine [in, optional] 
*
*     This parameter defines the string that specifies the 
*     command-line arguments for the application when it is restarted (when using \ref CR_INST_APP_RESTART flag). 
*     Do not include the name of the executable in the command line; it is added automatically. This parameter 
*     can be NULL. Available since v.1.2.4.
*
*   \b pszLangFilePath [in, optional]
*
*     This parameter defines the absolute path (including file name) for language file.
*     If this is NULL, the lang file is assumed to be located in the same dir as CrashSender.exe file and have 
*     the name crashrpt_lang.ini.
*     This parameter is available since v.1.2.4.
* 
*   \b pszEmailText [in, optional]
*
*     This parameter defines the custom E-mail text that is used when deliverying error report
*     as E-mail. If this is NULL, the default E-mail text is used. It is recommended to set this parameter with NULL.
*     This parameter is available since v.1.2.4.
*  
*   \b pszSmtpProxy [in, optional] 
*
*     This parameter defines the network address (IP or domain) and, optionally, port formatted as "address[:port]" 
*     of SMTP server. Examples: "192.168.1.1:2525", "mail.example.com:25". 
*     If this parameter is NULL, the SMTP server address is resolved using the MX record of recipient's mailbox. 
*     You should typically set this parameter with NULL, except in the
*     case when your software is a server and custom SMTP configuration is required. This parameter is available since v.1.2.4.
*  
*   \b pszCustomSenderIcon [in, optional] 
*
*     This parameter can be used to define a custom icon for <i>Error Report</i> dialog. This parameter is 
*     available since v.1.2.8.
*
*     The value of this parameter should be absolute path to the module containing the icon resource, followed 
*     by resource identifier separated by comma. You can set this parameter with NULL to use the default icon.  
*
*     The resource identifier is a zero-based index of the icon to retrieve. For example, if this value is 0, 
*     the first icon in the specified file is used. If the identifier is a negative number not equal to -1, 
*     the icon in the specified file whose resource identifier is equal to the absolute value of the resource 
*     identifier is used.
*     Example: "D:\MyApp\Resources.dll, -128". 
*  
*   \b pszSmtpLogin [in, optional] 
*      
*     This parameter defines the login name for SMTP authentication. It is typically used together with 
*     \ref pszSmtpProxy and \ref pszSmtpPassword parameter. 
*     If this parameter is ommitted (NULL), no SMTP autentication is used. This parameter is available since v.1.3.1.
*
*   \b pszSmtpPassword [in, optional] 
*      
*     This parameter defines the password for SMTP authentication. It is used in pair with \ref pszSmtpLogin parameter. 
*     This parameter is available since v.1.3.1.
*
*   \b nRestartTimeout [in, optional] 
*
*     This parameter defines the timeout (in seconds) for the application restart (when using \ref CR_INST_APP_RESTART flag). 
*     It is recommended to set this with zero (in such a case, the default restart timeout of 60 seconds is applied). 
*     Available since v.1.4.3.
*/

typedef struct tagCR_INSTALL_INFOW
{
    WORD cb;                        //!< Size of this structure in bytes; must be initialized before using!
    LPCWSTR pszAppName;             //!< Name of application.
    LPCWSTR pszAppVersion;          //!< Application version.
    LPCWSTR pszEmailTo;             //!< E-mail address of crash reports recipient.
    LPCWSTR pszEmailSubject;        //!< Subject of crash report e-mail. 
    LPCWSTR pszUrl;                 //!< URL of server-side script (used in HTTP connection).
    LPCWSTR pszCrashSenderPath;     //!< Directory name where CrashSender.exe is located.
    LPGETLOGFILE pfnCrashCallback;  //!< Deprecated, do not use.
    UINT uPriorities[5];            //!< Array of error sending transport priorities.
    DWORD dwFlags;                  //!< Flags.
    LPCWSTR pszPrivacyPolicyURL;    //!< URL of privacy policy agreement.
    LPCWSTR pszDebugHelpDLL;        //!< File name or folder of Debug help DLL.
    MINIDUMP_TYPE uMiniDumpType;    //!< Minidump type.
    LPCWSTR pszErrorReportSaveDir;  //!< Directory where to save error reports.
    LPCWSTR pszRestartCmdLine;      //!< Command line for application restart (without executable name).
    LPCWSTR pszLangFilePath;        //!< Path to the language file (including file name).
    LPCWSTR pszEmailText;           //!< Custom E-mail text (used when deliverying report as E-mail).
    LPCWSTR pszSmtpProxy;           //!< Network address and port to be used as SMTP proxy.
    LPCWSTR pszCustomSenderIcon;    //!< Custom icon used for Error Report dialog.
	LPCWSTR pszSmtpLogin;           //!< Login name used for SMTP authentication when sending error report as E-mail.
	LPCWSTR pszSmtpPassword;        //!< Password used for SMTP authentication when sending error report as E-mail.	
	int nRestartTimeout;            //!< Timeout for application restart.
}
CR_INSTALL_INFOW;

typedef CR_INSTALL_INFOW* PCR_INSTALL_INFOW;

/*! \ingroup CrashRptStructs
*  \struct CR_INSTALL_INFOA
*  \copydoc CR_INSTALL_INFOW
*/

typedef struct tagCR_INSTALL_INFOA
{
    WORD cb;                       //!< Size of this structure in bytes; must be initialized before using!
    LPCSTR pszAppName;             //!< Name of application.
    LPCSTR pszAppVersion;          //!< Application version.
    LPCSTR pszEmailTo;             //!< E-mail address of crash reports recipient.
    LPCSTR pszEmailSubject;        //!< Subject of crash report e-mail. 
    LPCSTR pszUrl;                 //!< URL of server-side script (used in HTTP connection).
    LPCSTR pszCrashSenderPath;     //!< Directory name where CrashSender.exe is located.
    LPGETLOGFILE pfnCrashCallback; //!< Deprecated, do not use.
    UINT uPriorities[5];           //!< Array of error sending transport priorities.
    DWORD dwFlags;                 //!< Flags.
    LPCSTR pszPrivacyPolicyURL;    //!< URL of privacy policy agreement.
    LPCSTR pszDebugHelpDLL;        //!< File name or folder of Debug help DLL.
    MINIDUMP_TYPE uMiniDumpType;   //!< Mini dump type.
    LPCSTR pszErrorReportSaveDir;  //!< Directory where to save error reports.
    LPCSTR pszRestartCmdLine;      //!< Command line for application restart (without executable name).
    LPCSTR pszLangFilePath;        //!< Path to the language file (including file name).
    LPCSTR pszEmailText;           //!< Custom E-mail text (used when deliverying report as E-mail).
    LPCSTR pszSmtpProxy;           //!< Network address and port to be used as SMTP proxy.
    LPCSTR pszCustomSenderIcon;    //!< Custom icon used for Error Report dialog.
	LPCSTR pszSmtpLogin;           //!< Login name used for SMTP authentication when sending error report as E-mail.
	LPCSTR pszSmtpPassword;        //!< Password used for SMTP authentication when sending error report as E-mail.	
	int nRestartTimeout;           //!< Timeout for application restart.
}
CR_INSTALL_INFOA;

typedef CR_INSTALL_INFOA* PCR_INSTALL_INFOA;

/*! \brief Character set-independent mapping of CR_INSTALL_INFOW and CR_INSTALL_INFOA structures.
*  \ingroup CrashRptStructs
*/
#ifdef UNICODE
typedef CR_INSTALL_INFOW CR_INSTALL_INFO;
typedef PCR_INSTALL_INFOW PCR_INSTALL_INFO;
#else
typedef CR_INSTALL_INFOA CR_INSTALL_INFO;
typedef PCR_INSTALL_INFOA PCR_INSTALL_INFO; 
#endif // UNICODE

/*! \ingroup CrashRptAPI 
*  \brief  Installs exception handlers for the caller process.
*
*  \return
*    This function returns zero if succeeded.
*
*  \param[in] pInfo General congiration information.
*
*  \remarks
*
*    This function installs unhandled exception filter for the caller process.
*    It also installs various CRT exception/error handlers that function for all threads of the caller process.
*    For more information, see \ref exception_handling
*
*    Below is the list of installed handlers:
*     - Top-level SEH exception filter [ \c SetUnhandledExceptionFilter() ]
*     - C++ pure virtual call handler (Visual Studio .NET 2003 and later) [ \c _set_purecall_handler() ]
*     - C++ invalid parameter handler (Visual Studio .NET 2005 and later) [ \c _set_invalid_parameter_handler() ]
*     - C++ new operator error handler (Visual Studio .NET 2003 and later) [ \c _set_new_handler() ]
*     - C++ buffer overrun handler (Visual Studio .NET 2003 only) [ \c _set_security_error_handler() ]
*     - C++ abort handler [ \c signal(SIGABRT) ]
*     - C++ illegal instruction handler [ \c signal(SIGINT) ]
*     - C++ termination request [ \c signal(SIGTERM) ]
*
*    In a multithreaded program, additionally use crInstallToCurrentThread2() function for each execution
*    thread, except the main one.
* 
*    The \a pInfo parameter contains all required information needed to install CrashRpt.
*
*    This function fails when \a pInfo->pszCrashSenderPath doesn't contain valid path to CrashSender.exe
*    or when \a pInfo->pszCrashSenderPath is equal to NULL, but \b CrashSender.exe is not located in the
*    directory where \b CrashRpt.dll located.
*
*    On crash, the crash minidump file is created, which contains CPU information and 
*    stack trace information. Also XML file is created that contains additional 
*    information that may be helpful for crash analysis. These files along with several additional
*    files added with crAddFile2() are packed to a single ZIP file.
*
*    When crash information is collected, another process, <b>CrashSender.exe</b>, is launched 
*    and the process where crash had occured is terminated. The CrashSender process is 
*    responsible for letting the user know about the crash and send the error report.
* 
*    The error report can be sent over E-mail using address and subject passed to the
*    function as \ref CR_INSTALL_INFO structure members. Another way of sending error report is an HTTP 
*    request using \a pszUrl member of \ref CR_INSTALL_INFO. 
*
*    This function may fail if an appropriate language file (\b crashrpt_lang.ini) is not found 
*    in the directory where the \b CrashSender.exe file is located.
*
*    If this function fails, use crGetLastErrorMsg() to retrieve the error message.
*
*    crInstallW() and crInstallA() are wide-character and multi-byte character versions of crInstall()
*    function. The \ref crInstall macro defines character set independent mapping for these functions.
*
*    For code example, see \ref simple_example.
*
*  \sa crInstallW(), crInstallA(), crInstall(), CR_INSTALL_INFOW, 
*      CR_INSTALL_INFOA, CR_INSTALL_INFO, crUninstall(), 
*      CrAutoInstallHelper
*/

CRASHRPTAPI(int)
crInstallW(
           __in PCR_INSTALL_INFOW pInfo
           );

/*! \ingroup CrashRptAPI
*  \copydoc crInstallW()
*/

CRASHRPTAPI(int)
crInstallA(
           __in PCR_INSTALL_INFOA pInfo
           );

/*! \brief Character set-independent mapping of crInstallW() and crInstallA() functions. 
* \ingroup CrashRptAPI
*/
#ifdef UNICODE
#define crInstall crInstallW
#else
#define crInstall crInstallA
#endif //UNICODE

/*! \ingroup CrashRptAPI 
*  \brief Uninitializes the CrashRpt library and unsinstalls exception handlers previously installed with crInstall().
*
*  \return
*    This function returns zero if succeeded.
*
*  \remarks
*
*    Call this function on application exit to uninitialize the library and uninstall exception
*    handlers previously installed with crInstall(). After function call, the exception handlers
*    are restored to states they had before calling crInstall().
*
*    This function fails if crInstall() wasn't previously called in context of the
*    caller process.
*
*    When this function fails, use the crGetLastErrorMsg() function to retrieve the error message.
*
*  \sa crInstallW(), crInstallA(), crInstall(),
*      CrAutoInstallHelper
*/

CRASHRPTAPI(int)
crUninstall();

/*! \ingroup CrashRptAPI
*  \brief Installs exception handlers to the caller thread.
*  \return This function returns zero if succeeded.
*  \param[in] dwFlags Flags.
*
*  \remarks
*
*  This function is available <b>since v.1.1.2</b>.
*
*  The function sets exception handlers for the caller thread. If you have
*  several execution threads, you ought to call the function for each thread,
*  except the main one.
*   
*  \a dwFlags defines what exception handlers to install. Use zero value
*  to install all possible exception handlers. Or use a combination of the following constants:
*
*      - \ref CR_INST_TERMINATE_HANDLER              Install terminate handler
*      - \ref CR_INST_UNEXPECTED_HANDLER             Install unexpected handler
*      - \ref CR_INST_SIGFPE_HANDLER                 Install SIGFPE signal handler   
*      - \ref CR_INST_SIGILL_HANDLER                 Install SIGILL signal handler  
*      - \ref CR_INST_SIGSEGV_HANDLER                Install SIGSEGV signal handler 
* 
*  Example:
*
*   \code
*   DWORD WINAPI ThreadProc(LPVOID lpParam)
*   {
*     // Install exception handlers
*     crInstallToCurrentThread2(0);
*
*     // Your code...
*
*     // Uninstall exception handlers
*     crUninstallFromCurrentThread();
*    
*     return 0;
*   }
*   \endcode
* 
*  \sa 
*    crInstall()
*/

CRASHRPTAPI(int)
crInstallToCurrentThread2(DWORD dwFlags);

/*! \ingroup CrashRptAPI  
*  \brief Uninstalls C++ exception handlers from the current thread.
*  \return This function returns zero if succeeded.
*  
*  \remarks
*
*    This function unsets exception handlers from the caller thread. If you have
*    several execution threads, you ought to call the function for each thread.
*    After calling this function, the exception handlers for current thread are
*    replaced with the handlers that were before call of crInstallToCurrentThread2().
*
*    This function fails if crInstallToCurrentThread2() wasn't called for current thread.
*
*    When this function fails, use crGetLastErrorMsg() to retrieve the error message.
*
*    No need to call this function for the main execution thread. The crUninstall()
*    will automatically uninstall C++ exception handlers for the main thread.
*
*   \sa crInstallToCurrentThread2(),
*       crUninstallFromCurrentThread(), CrThreadAutoInstallHelper
*/

CRASHRPTAPI(int)
crUninstallFromCurrentThread();

// Flags for crAddFile2() function.

#define CR_AF_TAKE_ORIGINAL_FILE  0 //!< Take the original file (do not copy it to the error report folder).
#define CR_AF_MAKE_FILE_COPY      1 //!< Copy the file to the error report folder.
#define CR_AF_FILE_MUST_EXIST     0 //!< Function will fail if file doesn't exist at the moment of function call.
#define CR_AF_MISSING_FILE_OK     2 //!< Do not fail if file is missing (assume it will be created later).
#define CR_AF_ALLOW_DELETE        4 //!< If this flag is specified, the file will be deletable from context menu of Error Report Details dialog.

/*! \ingroup CrashRptAPI  
*  \brief Adds a file to crash report.
* 
*  \return This function returns zero if succeeded.
*
*  \param[in] pszFile     Absolute path to the file (or file search pattern) to add to crash report, required.
*  \param[in] pszDestFile Destination file name, optional.
*  \param[in] pszDesc     File description (used in Error Report Details dialog), optional.
*  \param[in] dwFlags     Flags, optional.
*
*    This function can be called anytime after crInstall() to add one or more
*    files to the generated crash report. 
*  
*    When this function is called, the file is marked to be added to the error report, 
*    then the function returns control to the caller.
*    When a crash occurs, all marked files are added to the report by the \b CrashSender.exe process. 
*    If a file is locked by someone for exclusive access, the file won't be included. 
*    Inside of \ref PFNCRASHCALLBACK() crash callback, 
*    close open file handles and ensure files to be included are acessible for reading.
*
*    \a pszFile should be either a valid absolute path to the file or a file search 
*    pattern (e.g. "*.log") to be added to crash report. 
*
*    \a pszDestFile should be the name of destination file. This parameter can be used
*    to specify different file name for the file in ZIP archive. If this parameter is NULL, the pszFile
*    file name is used as destination file name. If \a pszFile is a search pattern, this argument
*    is ignored.
*
*    \a pszDesc is a short description of the file. It can be NULL.
*
*    \a dwFlags parameter defines the behavior of the function. This can be a combination of the following flags:
*       - \ref CR_AF_TAKE_ORIGINAL_FILE  On crash, the \b CrashSender.exe process will try to locate the file from its original location. This behavior is the default one.
*       - \ref CR_AF_MAKE_FILE_COPY      On crash, the \b CrashSender.exe process will make a copy of the file and save it to the error report folder.  
*
*       - \ref CR_AF_FILE_MUST_EXIST     The function will fail if file doesn't exist at the moment of function call (the default behavior). 
*       - \ref CR_AF_MISSING_FILE_OK     The function will not fail if file is missing (assume it will be created later).
*
*       - \ref CR_AF_ALLOW_DELETE        If this flag is specified, the user will be able to delete the file from error report using context menu of Error Report Details dialog.
*
*    If you do not use error report delivery (\ref CR_INST_DONT_SEND_REPORT flag) or if you use postponed error report delivery 
*    (if you specify \ref CR_INST_SEND_QUEUED_REPORTS flag) 
*    you must also specify the \ref CR_AF_MAKE_FILE_COPY as \a dwFlags parameter value. This will
*    guarantee that a snapshot of your file at the moment of crash is taken and saved to the error report folder.
*    The error report folder is a folder where files included into the crash report are stored
*    until they are sent to recipient.
*
*    This function fails if \a pszFile doesn't exist at the moment of function call, 
*    unless you specify \ref CR_AF_MISSING_FILE_OK flag. 
* 
*    The crAddFile2W() and crAddFile2A() are wide-character and multibyte-character
*    versions of crAddFile2() function. The crAddFile2() macro defines character set
*    independent mapping.
*
*  Usage example:
*
*  \code
*
*  // Add the error.log file to crash report. At the moment of crash, 
*  // the file will be copied to crash report folder. The end user 
*  // will be able to delete the file using CrashRpt GUI.
*  int nResult = crAddFile2(
*             _T("C:\\Program Files (x86)\MyApp\\error.log"), 
*             _T("error.log"), 
*             _T("Log file"), 
*             CR_AF_MAKE_FILE_COPY|CR_AF_ALLOW_DELETE);
*  if(nResult!=0)
*  {
*    // Get the status message
*    TCHAR szErrorMsg[256];
*    crGetLastErrorMsg(szErrorMsg, 256);
*  }
*
*  // Add all *.txt files found in the folder. At the moment of crash, 
*  // the file(s) will be copied to crash report folder. The end user 
*  // won't be able to delete the file(s).
*  crAddFile2(_T("C:\\Program Files (x86)\MyApp\\*.txt"), 
*      NULL, _T("TXT file"), CR_AF_MAKE_FILE_COPY);
*
*  \endcode
*
*  \sa crAddFile2W(), crAddFile2A(), crAddFile2()
*/

CRASHRPTAPI(int)
crAddFile2W(
            LPCWSTR pszFile,
            LPCWSTR pszDestFile,
            LPCWSTR pszDesc,
            DWORD dwFlags
            );

/*! \ingroup CrashRptAPI
*  \copydoc crAddFile2W()
*/

CRASHRPTAPI(int)
crAddFile2A(
            LPCSTR pszFile,
            LPCSTR pszDestFile,
            LPCSTR pszDesc,
            DWORD dwFlags
            );

/*! \brief Character set-independent mapping of crAddFile2W() and crAddFile2A() functions. 
*  \ingroup CrashRptAPI
*/
#ifdef UNICODE
#define crAddFile2 crAddFile2W
#else
#define crAddFile2 crAddFile2A
#endif //UNICODE


// Flags for crAddScreenshot function.
#define CR_AS_VIRTUAL_SCREEN  0  //!< Take a screenshot of the virtual screen.
#define CR_AS_MAIN_WINDOW     1  //!< Take a screenshot of application's main window.
#define CR_AS_PROCESS_WINDOWS 2  //!< Take a screenshot of all visible process windows.
#define CR_AS_GRAYSCALE_IMAGE 4  //!< Make a grayscale image instead of a full-color one.
#define CR_AS_USE_JPEG_FORMAT 8  //!< Store screenshots as JPG files.
#define CR_AS_ALLOW_DELETE   16  //!< If this flag is specified, the file will be deletable from context menu of Error Report Details dialog.

/*! \ingroup DeprecatedAPI  
*  \brief Adds a screenshot to the crash report.
* 
*  \return This function returns zero if succeeded. Use crGetLastErrorMsg() to retrieve the error message on fail.
*
*  \param[in] dwFlags Flags, optional.
*  
*  \remarks 
*
*  As of v.1.3.1, this function is deprecated and may be removed in one of the next releases. Use 
*  \ref crAddScreenshot2() function instead.
*
*  This function can be used to take a screenshot at the moment of crash and add it to the error report. 
*  Screenshot information may help the developer to better understand the state of the application
*  at the moment of crash and reproduce the error.
*
*  When this function is called, screenshot flags are saved, 
*  then the function returns control to the caller.
*  When crash occurs, screenshot is made by the \b CrashSender.exe process and added to the report. 
* 
*  \b dwFlags 
*
*    - \ref CR_AS_ALLOW_DELETE        If this flag is specified, the user will be able to delete the file from error report using context menu of Error Report Details dialog.
*
*    Use one of the following constants to specify what part of virtual screen to capture:
*    - \ref CR_AS_VIRTUAL_SCREEN  Use this to take a screenshot of the whole desktop (virtual screen).
*    - \ref CR_AS_MAIN_WINDOW     Use this to take a screenshot of the application's main window.
*    - \ref CR_AS_PROCESS_WINDOWS Use this to take a screenshot of all visible windows that belong to the process.
*
*  The main application window is a window that has a caption (\b WS_CAPTION), system menu (\b WS_SYSMENU) and
*  the \b WS_EX_APPWINDOW extended style. If CrashRpt doesn't find such window, it considers the first found process window as
*  the main window.
*
*  Screenshots are added in form of PNG files by default. You can specify the \ref CR_AS_USE_JPEG_FORMAT flag to save
*  screenshots as JPEG files instead. 
*
*  In addition, you can specify the \ref CR_AS_GRAYSCALE_IMAGE flag to make a grayscale screenshot 
*  (by default color image is made). Grayscale image gives smaller file size.
*
*  If you use JPEG image format, you may better use the \ref crAddScreenshot2() function, that allows to
*  define JPEG image quality. 
*
*  When capturing entire desktop consisting of several monitors, 
*  one screenshot file is added per each monitor. 
*
*  You should be careful when using this feature, because screenshots may contain user-identifying 
*  or private information. Always specify purposes you will use collected 
*  information for in your Privacy Policy. 
*
*  \sa
*   crAddFile2()
*/

CRASHRPTAPI(int)
crAddScreenshot(
                DWORD dwFlags
                );

/*! \ingroup CrashRptAPI  
*  \brief Adds a screenshot to the crash report.
* 
*  \return This function returns zero if succeeded. Use crGetLastErrorMsg() to retrieve the error message on fail.
*
*  \param[in] dwFlags Flags, optional.
*  \param[in] nJpegQuality Defines the JPEG image quality, optional.
*  
*  \remarks 
* 
*  This function can be used to take a screenshot at the moment of crash and add it to the error report. 
*  Screenshot information may help the developer to better understand state of the application
*  at the moment of crash and reproduce the error.
*
*  When this function is called, screenshot flags are saved, then the function returns control to the caller.
*  When crash occurs, screenshot is made by the \b CrashSender.exe process and added to the report. 
* 
*  \b dwFlags 
*
*    Use one of the following constants to specify what part of virtual screen to capture:
*    - \ref CR_AS_VIRTUAL_SCREEN  Use this to take a screenshot of the whole desktop (virtual screen).
*    - \ref CR_AS_MAIN_WINDOW     Use this to take a screenshot of the main application main window.
*    - \ref CR_AS_PROCESS_WINDOWS Use this to take a screenshot of all visible windows that belong to the process.
* 
*  The main application window is a window that has a caption (\b WS_CAPTION), system menu (\b WS_SYSMENU) and
*  the \b WS_EX_APPWINDOW extended style. If CrashRpt doesn't find such window, it considers the first found process window as
*  the main window.
*
*  Screenshots are added in form of PNG files by default. You can specify the \ref CR_AS_USE_JPEG_FORMAT flag to save
*  screenshots as JPEG files instead. 
*
*  If you use JPEG format, you can use the \a nJpegQuality parameter to define the JPEG image quality. 
*  This should be the number between 0 and 100, inclusively. The bigger the number, the better the quality and the bigger the JPEG file size. 
*  If you use PNG file format, this parameter is ignored.
*
*  In addition, you can specify the \ref CR_AS_GRAYSCALE_IMAGE flag to make a grayscale screenshot 
*  (by default color image is made). Grayscale image gives smaller file size.
*
*  When capturing entire desktop consisting of several monitors, 
*  one screenshot file is added per each monitor. 
*
*  You should be careful when using this feature, because screenshots may contain user-identifying 
*  or private information. Always specify purposes you will use collected 
*  information for in your Privacy Policy. 
*
*  \sa
*   crAddFile2()
*/

CRASHRPTAPI(int)
crAddScreenshot2(
                 DWORD dwFlags,
                 int nJpegQuality
                 );

// Flags for crAddVideo function.
#define CR_AV_VIRTUAL_SCREEN  0  //!< Capture the whole virtual screen.
#define CR_AV_MAIN_WINDOW     1  //!< Capture the area of application's main window.
#define CR_AV_PROCESS_WINDOWS 2  //!< Capture all visible process windows.
#define CR_AV_QUALITY_LOW     0  //!< Low quality video encoding, smaller file size.
#define CR_AV_QUALITY_GOOD    4  //!< Good encoding quality, larger file size.
#define CR_AV_QUALITY_BEST    8  //!< The best encoding quality, the largest file size.
#define CR_AV_NO_GUI         16  //!< Do not display the notification dialog.
#define CR_AV_ALLOW_DELETE   32  //!< If this flag is specified, the file will be deletable from context menu of Error Report Details dialog.

/*! \ingroup CrashRptAPI  
*  \brief Allows to record what happened before crash to a video file and include the file to crash report.
* 
*  \return This function returns zero if succeeded. Use \ref crGetLastErrorMsg() to retrieve the error message on failure.
*
*  \param[in] dwFlags Flags, optional.
*  \param[in] nDuration Video duration (in milliseconds). Optional.
*  \param[in] nFrameInterval Interval between subsequent frames (in milliseconds). Optional.
*  \param[in] pDesiredFrameSize Defines the desired video frame size, optional.
*  \param[in] hWndParent Window that becomes the parent for GUI displayed by this function. Optional.
*  
*  \remarks 
*
*  This function is available as of v.1.4.0.
*
*  \b dwFlags can be a combination of the following constants:
*
*   - use one of the following constants to specify what part of virtual screen to capture:
*    - \ref CR_AV_VIRTUAL_SCREEN  Use this to capture the whole desktop (virtual screen). This is the default.
*    - \ref CR_AV_MAIN_WINDOW     Use this to capture the application's main window.
*    - \ref CR_AV_PROCESS_WINDOWS Use this to capture all visible windows that belong to the process.
* 
*   - use one of the following constants to define the desired video encoding quality:
*    - \ref CR_AV_QUALITY_LOW     Low-quality video encoding. This is the default.
*    - \ref CR_AV_QUALITY_GOOD    Good encoding quality, larger file.
*    - \ref CR_AV_QUALITY_BEST    The best encoding quality, the largest file.
*
*   - use the \ref CR_AV_ALLOW_DELETE to allow the user to delete the recorded video file from error report using context menu of Error Report Details dialog.
*
*  The main application window is a window that has a caption (\b WS_CAPTION), system menu (\b WS_SYSMENU) and
*  the \b WS_EX_APPWINDOW extended style. If CrashRpt doesn't find such a window, it considers the first found process window as
*  the main window.
*
*  When the function is called, it displays a dialog notifying the user about video recording.
*  The displayed dialog's parent window can be specified with the \b hWndParent argument.
*  If the \b hWndParent is \a NULL, the currently active process window becomes the parent.
*  If you do not want to display the dialog, specify the \ref CR_AV_NO_GUI flag for \b dwFlags argument.
*
*  The recorded video will be maximum \b nDuration milliseconds long with \b nFrameInterval
*  milliseconds interval between subsequent video frames. If \b nDuration and\or \b nFrameInterval
*  are set to zero (0), the default implementation-defined duration and frame interval are used.
*
*  The \b pDesiredFrameSize parameter allows to define the desired video frame size.
*  Frame width and height must be a multiple of 16 (OGG Theora video codec's requirement). 
*  If they are not, they are modified automatically to be a multiple of 16.
*
*  To preserve correct aspect ratio of the captured area, set \b pDesiredFrameSize->cx or \b pDesiredFrameSize->cy
*  to zero. For example, setting \b pDesiredFrameSize->cx=640 and \b pDesiredFrameSize->cy=0
*  results in video frames whose width is 640 pixels and height is calculated to preserve the
*  correct aspect ratio of the captured area. If both \b cx and \b cy are specified, the aspect ratio
*  of the captured area is not preserved.
*
*  Setting the \b pDesiredFrameSize
*  parameter to \a NULL makes the function to determine the best video frame size automatically.
*
*  This function can be used to record the state of end user's desktop just before the moment 
*  of crash and add the video file to the error report. The recorded information may help the 
*  software vendor to better understand the state of the client application at the moment of 
*  crash and reproduce the error.
*
*  When this function is called, CrashRpt launches another process named \b CrashSender.exe. 
*  The \b CrashSender.exe process then continuously captures the desktop screenshots in background 
*  mode and stores them to disk as image files. To avoid high CPU load, image files are stored 
*  in uncompressed state as raw bitmap (BMP) files. When the count of screenshot files exceeds 
*  the predefined maximum number, the old screenshot files are reused cyclically. 
*
*  If the client application does not crash and its main code or main window loop exits successfully, 
*  the captured desktop screenshot files are removed by the \ref crUninstall() function call and
*  \b CrashSender.exe process is terminated.
*
*  If the client application crashes at some moment of time, the recorded screenshot files are compressed by 
*  <a href="http://www.theora.org/">OGG Theora video codec</a> and written into an .OGG file. The 
*  uncompressed temporary screenshots are then removed, and the resulting
*  OGG file is included into crash report archive.
*
*  The <a href="http://en.wikipedia.org/wiki/Ogg">OGG video format</a> is a widely used 
*  video container provided by the open-source OGG Project.
*  OGG files can be opened in a browser like Google Chrome or Mozilla Firefox or in 
*  another video player understanding this format, like ffmpeg.
*
*  Use this function only when necessary, because it may cause end user's computer performance
*  loss. It also requires some amount of free disk space.
*
*  The recorded video may contain user-identifying or private information. Always 
*  specify the purposes you will use collected information for in your Privacy Policy. 
*
*  Usage example:
*
*  \code
*
*  // First install CrashRpt with crInstall() function
*
*  ...
*
*  SIZE FrameSize = {0, 600}; // Frames 600 px in height
*                      // Frame width is calculated automatically
*
*  // Start capturing desktop. Desktop capture video will
*  // be added to crash report on crash
*  int nResult = crAddVideo(
*         CR_AV_VIRTUAL_SCREEN|CR_AV_QUALITY_GOOD, // Capture entire desktop
*                                               // Good encoding quality
*         10000,   // 10 seconds long video
*         300,     // 300 msec between frames (3.33 FPS)
*         &FrameSize,
*         NULL
*    );
*
*  \endcode
*
*  \sa
*   crAddFile2(), crAddScreenshot2(), crAddRegKey(), crUninstall().
*/

CRASHRPTAPI(int)
crAddVideo(
            DWORD dwFlags,
			int nDuration,
			int nFrameInterval,
            PSIZE pDesiredFrameSize,
			HWND hWndParent
            );

/*! \ingroup CrashRptAPI  
*  \brief Adds a string property to the crash report. 
* 
*  \return This function returns zero if succeeded. Use crGetLastErrorMsg() to retrieve the error message on fail.
*
*  \param[in] pszPropName   Name of the property, required.
*  \param[in] pszPropValue  Value of the property, required.
*  
*  \remarks 
*
*  Use this function to add a string property to the crash description XML file.
*  User-added properties are listed under \<CustomProps\> tag of the XML file.
*  In the XML file properties are ordered by names in alphabetic order.
*
*  The following example shows how to add information about the amount of free disk space 
*  to the crash description XML file:
*
*  \code
*  // It is assumed that you already calculated the amount of free disk space, 
*  // converted it to text and stored it as szFreeSpace string.
*  LPCTSTR szFreeSpace = _T("0 Kb");
*  crAddProperty(_T("FreeDiskSpace"), szFreeSpace);
*
*  \endcode
*
*  \sa
*   crAddFile2(), crAddScreenshot()
*/

CRASHRPTAPI(int)
crAddPropertyW(
               LPCWSTR pszPropName,
               LPCWSTR pszPropValue
               );

/*! \ingroup CrashRptAPI
*  \copydoc crAddPropertyW()
*/

CRASHRPTAPI(int)
crAddPropertyA(
               LPCSTR pszPropName,
               LPCSTR pszPropValue
               );

/*! \brief Character set-independent mapping of crAddPropertyW() and crAddPropertyA() functions. 
*  \ingroup CrashRptAPI
*/
#ifdef UNICODE
#define crAddProperty crAddPropertyW
#else
#define crAddProperty crAddPropertyA
#endif //UNICODE

// Flags that can be passed to crAddRegKey() function
#define CR_AR_ALLOW_DELETE   0x1  //!< If this flag is specified, the file will be deletable from context menu of Error Report Details dialog.

/*! \ingroup CrashRptAPI  
*  \brief Adds a registry key dump to the crash report.
* 
*  \return This function returns zero if succeeded. Use crGetLastErrorMsg() to retrieve the error message on fail.
*
*  \param[in] pszRegKey        Registry key to dump, required.
*  \param[in] pszDstFileName   Name of the destination file, required. 
*  \param[in] dwFlags          Flags, reserved.
*  
*  \remarks 
*
*  Use this function to add a dump of a Windows registry key into the crash report. This function
*  is available since v.1.2.6.
*
*  The \a pszRegKey parameter must be the name of the registry key. The key name should begin with "HKEY_CURRENT_USER"
*  or "HKEY_LOCAL_MACHINE". Other root keys are not supported.
*
*  The content of the key specified by the \a pszRegKey parameter will be stored in a human-readable XML
*  format and included into the error report as \a pszDstFileName destination file. You can dump multiple registry keys
*  to the same destination file.
*
*  The \a dwFlags parameter can be either set to zero (no flags) or with the following constant:
* 
*  - \ref CR_AR_ALLOW_DELETE allows the user to delete the file from error report using context menu of Error Report Details dialog.
*
*  The following example shows how to dump two registry keys to a single \a regkey.xml file:
*
*  \code
*  
*  crAddRegKey(_T("HKEY_CURRENT_USER\\Software\\MyApp"), _T("regkey.xml"), 0);
*  crAddRegKey(_T("HKEY_LOCAL_MACHINE\\Software\\MyApp"), _T("regkey.xml"), 0);
*
*  \endcode
*
*  \sa
*   crAddFile2(), crAddScreenshot(), crAddProperty()
*/

CRASHRPTAPI(int)
crAddRegKeyW(   
             LPCWSTR pszRegKey,
             LPCWSTR pszDstFileName,
             DWORD dwFlags
             );

/*! \ingroup CrashRptAPI
*  \copydoc crAddRegKeyW()
*/

CRASHRPTAPI(int)
crAddRegKeyA(   
             LPCSTR pszRegKey,
             LPCSTR pszDstFileName,
             DWORD dwFlags
             );

/*! \brief Character set-independent mapping of crAddRegKeyW() and crAddRegKeyA() functions. 
*  \ingroup CrashRptAPI
*/
#ifdef UNICODE
#define crAddRegKey crAddRegKeyW
#else
#define crAddRegKey crAddRegKeyA
#endif //UNICODE

/*! \ingroup CrashRptAPI  
*  \brief Manually generates an error report.
*
*  \return This function returns zero if succeeded. When failed, it returns a non-zero value.
*     Use crGetLastErrorMsg() function to retrieve the error message.
*  
*  \param[in] pExceptionInfo Exception information. 
*
*  \remarks
*
*    Call this function to manually generate a crash report. When crash information is collected,
*    control is returned to the caller. The crGenerateErrorReport() doesn't terminate the caller process.
*
*    The crash report may contain the crash minidump file, crash description file in XML format and
*    additional custom files added with a function like crAddFile2().
*
*    The exception information should be passed using \ref CR_EXCEPTION_INFO structure. 
*
*    The following example shows how to use crGenerateErrorReport() function.
*
*    \code
*    CR_EXCEPTION_INFO ei;
*    memset(&ei, 0, sizeof(CR_EXCEPTION_INFO));
*    ei.cb = sizeof(CR_EXCEPTION_INFO);
*    ei.exctype = CR_SEH_EXCEPTION;
*    ei.code = 1234;
*    ei.pexcptrs = NULL;
*
*    int result = crGenerateErrorReport(&ei);
*
*    if(result!=0)
*    {
*      // If goes here, crGenerateErrorReport() has failed
*      // Get the last error message
*      TCHAR szErrorMsg[256];
*      crGetLastErrorMsg(szErrorMsg, 256);
*    }
*   
*    // Manually terminate program
*    ExitProcess(0);
*
*    \endcode
*/

CRASHRPTAPI(int)
crGenerateErrorReport(   
                      __in_opt CR_EXCEPTION_INFO* pExceptionInfo
                      );

/*! \ingroup CrashRptAPI
*  \brief Can be used as a SEH exception filter.
*
*  \return This function returns \c EXCEPTION_EXECUTE_HANDLER if succeeds; otherwise \c EXCEPTION_CONTINUE_SEARCH.
*
*  \param[in] code Exception code.
*  \param[in] ep   Exception pointers.
*
*  \remarks
*
*     This function can be called instead of a SEH exception filter
*     inside of __try{}__except(Expression){} construction. The function generates an error report
*     and returns control to the exception handler block.
*
*     The exception code is usually retrieved with \b GetExceptionCode() intrinsic function
*     and the exception pointers are retrieved with \b GetExceptionInformation() intrinsic 
*     function.
*
*     If an error occurs, this function returns \c EXCEPTION_CONTINUE_SEARCH.
*     Use crGetLastErrorMsg() to retrieve the error message on fail.
*
*     The following example shows how to use crExceptionFilter().
*    
*     \code
*     int* p = NULL;   // pointer to NULL
*     __try
*     {
*        *p = 13; // causes an access violation exception;
*     }
*     __except(crExceptionFilter(GetExceptionCode(), GetExceptionInformation()))
*     {   
*       // Terminate program
*       ExitProcess(1);
*     }
*
*     \endcode 
*/

CRASHRPTAPI(int)
crExceptionFilter(
                  unsigned int code, 
                  __in_opt struct _EXCEPTION_POINTERS* ep);


// Flags used by crEmulateCrash() function
#define CR_NONCONTINUABLE_EXCEPTION  32  //!< Non continuable sofware exception. 
#define CR_THROW                     33  //!< Throw C++ typed exception.
#define CR_STACK_OVERFLOW			 34  //!< Stack overflow.

/*! \ingroup CrashRptAPI  
*  \brief Emulates a predefined crash situation.
*
*  \return This function doesn't return if succeded. If failed, returns non-zero value. Call crGetLastErrorMsg()
*   to get the last error message.
*
*  \param[in] ExceptionType Type of crash.
*
*  \remarks
*
*    This function uses some a priori incorrect or vulnerable code or raises a C++ signal or raises an uncontinuable
*    software exception to cause crash.
*
*    This function can be used to test if CrashRpt handles a crash situation correctly.
*    
*    CrashRpt will intercept an error or exception if crInstall() and/or crInstallToCurrentThread2() 
*    were previously called. crInstall() installs exception handlers that function on per-process basis.
*    crInstallToCurrentThread2() installs exception handlers that function on per-thread basis.
*    
*  \a ExceptionType can be one of the following constants:
*    - \ref CR_SEH_EXCEPTION  This will generate a null pointer exception.
*    - \ref CR_CPP_TERMINATE_CALL This results in call of terminate() C++ function.
*    - \ref CR_CPP_UNEXPECTED_CALL This results in call of unexpected() C++ function.
*    - \ref CR_CPP_PURE_CALL This emulates a call of pure virtual method call of a C++ class instance (Visual Studio .NET 2003 and later).
*    - \ref CR_CPP_NEW_OPERATOR_ERROR This emulates C++ new operator failure (Visual Studio .NET 2003 and later).
*    - \ref CR_CPP_SECURITY_ERROR This emulates copy of large amount of data to a small buffer (Visual Studio .NET 2003 only).
*    - \ref CR_CPP_INVALID_PARAMETER This emulates an invalid parameter C++ exception (Visual Studio 2005 and later). 
*    - \ref CR_CPP_SIGABRT This raises SIGABRT signal (abnormal program termination).
*    - \ref CR_CPP_SIGFPE This causes floating point exception.
*    - \ref CR_CPP_SIGILL This raises SIGILL signal (illegal instruction signal).
*    - \ref CR_CPP_SIGINT This raises SIGINT signal.
*    - \ref CR_CPP_SIGSEGV This raises SIGSEGV signal.
*    - \ref CR_CPP_SIGTERM This raises SIGTERM signal (program termination request).
*    - \ref CR_NONCONTINUABLE_EXCEPTION This raises a noncontinuable software exception (expected result 
*         is the same as in \ref CR_SEH_EXCEPTION).
*    - \ref CR_THROW This throws a C++ typed exception (expected result is the same as in \ref CR_CPP_TERMINATE_CALL).
*    - \ref CR_STACK_OVERFLOW This causes stack overflow.
*
*  The \ref CR_SEH_EXCEPTION uses null pointer write operation to cause the access violation.
*
*  The \ref CR_NONCONTINUABLE_EXCEPTION has the same effect as \ref CR_SEH_EXCEPTION, but it uses
*  \b RaiseException() WinAPI function to raise noncontinuable software exception.
*
*  The following example shows how to use crEmulateCrash() function.
*
*  \code
*  // emulate null pointer exception (access violation)
*  crEmulateCrash(CR_SEH_EXCEPTION);
*  \endcode
*
*/

CRASHRPTAPI(int)
crEmulateCrash(
               unsigned ExceptionType) throw (...);



/*! \ingroup CrashRptAPI 
*  \brief Gets the last CrashRpt error message.
*
*  \return This function returns length of error message in characters. If output buffer is invalid, returns a negative number.
*
*  \param[out] pszBuffer Pointer to the buffer.
*  \param[in]  uBuffSize Size of buffer in characters.
*
*  \remarks
*
*    This function gets the last CrashRpt error message. You can use this function
*    to retrieve the text status of the last called CrashRpt function.
*
*    If buffer is too small for the error message, the message is truncated.
*
*  crGetLastErrorMsgW() and crGetLastErrorMsgA() are wide-character and multi-byte character versions
*  of crGetLastErrorMsg(). The crGetLastErrorMsg() macro defines character set independent mapping.
*
*  The following example shows how to use crGetLastErrorMsg() function.
*
*  \code
*  
*  // .. call some CrashRpt function
*
*  // Get the status message
*  TCHAR szErrorMsg[256];
*  crGetLastErrorMsg(szErrorMsg, 256);
*  \endcode
*
*  \sa crGetLastErrorMsgA(), crGetLastErrorMsgW(), crGetLastErrorMsg()
*/

CRASHRPTAPI(int)
crGetLastErrorMsgW(
                   __out_ecount_z(uBuffSize) LPWSTR pszBuffer, 
                   UINT uBuffSize);

/*! \ingroup CrashRptAPI
*  \copydoc crGetLastErrorMsgW()
*
*/

CRASHRPTAPI(int)
crGetLastErrorMsgA(
                   __out_ecount_z(uBuffSize) LPSTR pszBuffer, 
                   UINT uBuffSize);

/*! \brief Defines character set-independent mapping for crGetLastErrorMsgW() and crGetLastErrorMsgA().
*  \ingroup CrashRptAPI
*/

#ifdef UNICODE
#define crGetLastErrorMsg crGetLastErrorMsgW
#else
#define crGetLastErrorMsg crGetLastErrorMsgA
#endif //UNICODE


//// Helper wrapper classes

#ifndef _CRASHRPT_NO_WRAPPERS

/*! \class CrAutoInstallHelper
*  \ingroup CrashRptWrappers
*  \brief Installs exception handlers in constructor and uninstalls in destructor.
*  \remarks
*    Use this class to easily install/uninstall exception handlers in you \b main()
*    or \b WinMain() function.
*
*    This wrapper class calls crInstall() in its constructor and calls crUninstall() in
*    its destructor.
*
*    Use CrAutoInstallHelper::m_nInstallStatus member to check the return status of crInstall().
*   
*    Example:
*
*    \code
*    #include <CrashRpt.h>
*
*    void main()
*    {      
*      CR_INSTALL_INFO info;
*      memset(&info, 0, sizeof(CR_INSTALL_INFO));
*      info.cb = sizeof(CR_INSTALL_INFO);  
*      info.pszAppName = _T("My App Name");
*      info.pszAppVersion = _T("1.2.3");
*      info.pszEmailSubject = "Error Report from My App v.1.2.3";
*      // The address to send reports by E-mail
*      info.pszEmailTo = _T("myname@hotmail.com");  
*      // The URL to send reports via HTTP connection
*      info.pszUrl = _T("http://myappname.com/utils/crashrpt.php"); 
*      info.pfnCrashCallback = CrashCallback; 
*      info.uPriorities[CR_HTTP] = 3; // Try HTTP first
*      info.uPriorities[CR_SMTP] = 2; // Try SMTP second
*      info.uPriorities[CR_SMAPI] = 1; // Try system email program last
*
*      // Install crash reporting
*      CrAutoInstallHelper cr_install_helper(&info);
*      // Check that installed OK
*      assert(cr_install_helper.m_nInstallStatus==0);
*
*      // Your code follows here ...
*
*    }
*    \endcode
*/

class CrAutoInstallHelper
{
public:

    //! Installs exception handlers to the caller process
    CrAutoInstallHelper(__in PCR_INSTALL_INFOA pInfo)
    {
        m_nInstallStatus = crInstallA(pInfo);
    }

    //! Installs exception handlers to the caller process
    CrAutoInstallHelper(__in PCR_INSTALL_INFOW pInfo)
    {
        m_nInstallStatus = crInstallW(pInfo);
    }

    //! Uninstalls exception handlers from the caller process
    ~CrAutoInstallHelper()
    {
		if(m_nInstallStatus==0)
			crUninstall();
    }

    //! Install status
    int m_nInstallStatus;
};

/*! \class CrThreadAutoInstallHelper
*  \ingroup CrashRptWrappers
*  \brief Installs (uninstalls) exception handlers for the caller thread in class' constructor (destructor).
*  
*  \remarks
*
*   This wrapper class calls crInstallToCurrentThread2() in its constructor and 
*   calls crUninstallFromCurrentThread() in its destructor.
*
*   Use CrThreadAutoInstallHelper::m_nInstallStatus member to check 
*   the return status of crInstallToCurrentThread2().
*
*   Example:
*
*   \code
*   DWORD WINAPI ThreadProc(LPVOID lpParam)
*   {
*     CrThreadAutoInstallHelper cr_thread_install_helper();
*     assert(cr_thread_install_helper.m_nInstallStatus==0);
*    
*     // Your code follows here ...
*   }
*   \endcode
*/

class CrThreadAutoInstallHelper
{
public:

    //! Installs exception handlers to the caller thread
    CrThreadAutoInstallHelper(DWORD dwFlags=0)
    {
        m_nInstallStatus = crInstallToCurrentThread2(dwFlags);    
    }

    //! Uninstalls exception handlers from the caller thread
    ~CrThreadAutoInstallHelper()
	{
		if (m_nInstallStatus == 0)
			 crUninstallFromCurrentThread();
	}

    //! Install status
    int m_nInstallStatus;
};

#endif //!_CRASHRPT_NO_WRAPPERS

#endif //_CRASHRPT_H_


