//=====================================================================
// MAIN.CPP
//    Workplace Shell for Windows initialization and main functions.
//
// Date Created :  01/25/94
// Last Update  :  10/10/95
// Written By   :  Mario E. De Armas
// Compiler Used:  MS Visual C++ 1.52
//
// Copyright IBM Corporation, 1994-1995. All rights reserved.
//
//
// FILE LOG:
// DATE        | ACTION/CHANGE                           | PROGRAMMER
// 01/25/94    | Creation.                               | MDA
//=====================================================================

//---------------------------------------------------------------------
// HEADER FILES
//---------------------------------------------------------------------
// Workplace Shell header files.
#include "wps.hpp"
// C-runtime library header files.
#include <string.h>
#include <ctype.h>
#ifdef _DEBUG
#include <stdarg.h>
#endif
// Workplace Shell resource ID's header file.
#include "resource.h"
// Windows 3D control library header file (CTL3D.DLL).
#include "ctl3d.h"

//---------------------------------------------------------------------
// LOCAL FUNCTION PROTOTYPES
//---------------------------------------------------------------------
// From MAIN.CPP file:
void ExecuteCommandLine(char *pszCommandLine);
BOOL CheckWindowsVersion(void);
BOOL LoadNLSResourceLibrary(void);
void InitializeEnvironmentStructure(HINSTANCE hInstance);

//---------------------------------------------------------------------
// EXTERNAL FUNCTION PROTOTYPES
//---------------------------------------------------------------------
// From MISCFUNC.CPP file:
UINT        ExecuteApplication(LPCSTR pszCmdLine,UINT nCmdShow);
HINSTANCE   ExecuteApplication(HWND hwndParent,LPCSTR pszOp,LPCSTR pszFile,LPCSTR pszParams,LPCSTR pszDir,int nCmdShow);
BOOL        LoadStringTable(HINSTANCE hInstance,int nTotalStrings);
void        FreeStringTable(int nTotalStrings);

//---------------------------------------------------------------------
// LOCAL CONSTANTS
//---------------------------------------------------------------------
// Shell queue size needs to be larger than default to prevent queue overflow.
#define WPSHELL_QUEUE_SIZE    25    // default = 8, maximum = 120.

//---------------------------------------------------------------------
// GLOBAL VARIABLES
//---------------------------------------------------------------------
// This structure holds information that is needed globally.
WPENVIRONMENT g_wpEnvironment;


//---------------------------------------------------------------------
// WinMain():
//    Entry point for WPSHELL application, and main loop code.
//---------------------------------------------------------------------
#pragma warning( disable : 4100 ) // Disable warning about "unreferenced formal parameters".
#pragma warning( disable : 4204 ) // Disable warning about "nonstandard extension used".
int PASCAL WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{

   HACCEL      haccelShell;

   MSG         Msg;

   char appdir[MAX_PATH];
   if (!PortGetAppDirectory(appdir,sizeof(appdir))) return 1;
   SetCurrentDirectoryA(appdir);
   PortStartDiagnostics();
   PortLog("Startup; application directory selected");
   // Any other instances of app running?
   if (hPrevInstance!=NULL)
      // Allow only one copy of shell running at any time, therefore exit.
      return (FALSE);

   // Before we do anything, we must make sure we are running on Windows 3.1 or higher.
   if (CheckWindowsVersion()==FALSE)
      // Windows version is 3.0 or below, therefore exit.
      return (FALSE);

   #ifdef _DEBUG
   // Automatically start the Microsoft "Debug Window" utility which receives our debug output.
   WinExec("DBWIN.EXE",SW_SHOWNORMAL);
   #endif

   // Load NLS resource library which contains language-dependent resources.
   if (LoadNLSResourceLibrary()==FALSE)
      // Error loading NLS resource library, therefore exit.
      return (FALSE);

   PortLog("English resource DLL loaded");
   // Load string resource table from NLS resource library.
   if (LoadStringTable(g_wpEnvironment.hResInst,TOTAL_STRINGS)==FALSE)
   {
      // Free NLS resource library before exiting.
      FreeLibrary(g_wpEnvironment.hResInst);
      // Error loading string resources, therefore exit.
      return (FALSE);
   }

   PortLog("String table loaded");
   // Initialize global structure containing WPS environment values.
   InitializeEnvironmentStructure(hInstance);

   // Increment the default queue size for Workplace Shell task.
   if (SetMessageQueue(WPSHELL_QUEUE_SIZE)==FALSE)
   {
      // Notify user that we could not create message queue.
      MessageBox(NULL,GetString(IDSTR_ERR_CREATE_MSG_QUEUE),
                 GetString(IDSTR_ERR_CAPTION),(MB_OK|MB_ICONSTOP));
      // Exit program.
      return (FALSE);
   }

   // Register this application's dialog boxes to use the 3D control library.
   Ctl3dRegister(hInstance);
   Ctl3dAutoSubclass(hInstance);

   // Register the notebook control and spin button control...
   if (!PortRegisterControls(hInstance)) return 1;
   PortLog("Native notebook and spin controls registered");
   WPDESKTOP wpshellDesktop;

   // Load WPSHELL.INI desktop.
   if (wpshellDesktop.InitializeDesktop()==FALSE)
   {
      // Display error message to user.
      MessageBox(NULL,GetString(IDSTR_ERR_WPSHELL_INI_FILE),
                 GetString(IDSTR_ERR_CAPTION),(MB_OK|MB_ICONSTOP));
      // Exit program.
      return (FALSE);
   }

   PortLog("Desktop initialized and displayed");
   // Load workplace shell accelerator table.
   haccelShell = LoadAccelerators(hInstance,MAKEINTRESOURCE(IDA_WPSHELL));
   if (haccelShell==NULL)
      return (FALSE);

   // If we are the current default Windows shell, then...
   if (wpshellDesktop.IsShell()==TRUE)
      // Execute any programs in the command line.
      ExecuteCommandLine(lpCmdLine);

   // Acquire and dispatch messages until a WM_QUIT message is received...
   while (GetMessage(&Msg,NULL,0,0)>0)
   {
      // Check if message should go to a modeless dialog box (notebook pages).
      if ((g_wpEnvironment.hwndCurrentModelessDialog==NULL) ||
          (IsDialogMessage(g_wpEnvironment.hwndCurrentModelessDialog,&Msg)==FALSE))
      {
         // If we are in here, the message is intented for a window.
         // Check if accelerator table translation mode is enabled ?
         if (g_wpEnvironment.fAcceleratorTranslation==TRUE)
         {
            // It is, so we translate keystrokes using accelerator table.
            if (TranslateAccelerator(GetActiveWindow(),haccelShell,&Msg)==0)
            {
               TranslateMessage(&Msg);   // Translates virtual key codes.
               DispatchMessage(&Msg);    // Dispatches message to window.
            }
         }
         else
         {
            // No accelerator table translation wanted, pass message as is.
            TranslateMessage(&Msg);   // Translates virtual key codes.
            DispatchMessage(&Msg);    // Dispatches message to window.
         }
      }
   }

   PortLog("Message loop exited normally");
   // Unregister this application from 3D control library (CTL3D.DLL).
   Ctl3dUnregister(hInstance);

   // Clean-up libraries before exiting.
       // Spin button control library.
         // Notebook control library.
   FreeLibrary(g_wpEnvironment.hResInst); // NLS resource library.

   // Free string table.
   FreeStringTable(TOTAL_STRINGS);

   // Returns the value from PostQuitMessage() call.
   return (Msg.wParam);
}


//---------------------------------------------------------------------
// ExecuteCommandLine():
//---------------------------------------------------------------------
void ExecuteCommandLine(char *pszCommandLine)
{
   UINT  uReturnCode;
   char  *pszExecute,*pszParam;

   // Execute any applications passed as command line arguments to the shell.
   // Start by initializing first application to execute.
   pszExecute = strtok(pszCommandLine,",");
   // Walk the list and execute the applications specified.
   while (pszExecute!=NULL)
   {
      // Skip any leading spaces in command line string.
      while (IsSpace(*pszExecute))
         pszExecute++;
      #ifdef _DEBUG
      dprintf("tr WinMain(): executing program'%s'\n",pszExecute);
      #endif

      // Get pointer to begining of parameter string (if any).
      pszParam = strchr(pszExecute,' ');
      if (pszParam!=NULL)
      {
         pszParam[0] = '\0';
         pszParam++;
      }
      // Execute current application in list.
      uReturnCode = (UINT_PTR)ExecuteApplication(GetDesktopWindow(),"open",pszExecute,pszParam,"",SW_SHOWNORMAL);

      // Restore original character
      if (pszParam!=NULL)
         pszParam[-1] = ' ';

      #ifdef _DEBUG
      // Check that the execution was successful.
      if (uReturnCode<32)
         dprintf("err WinMain(): error executing '%s' '%s' = %u\n",pszExecute,pszParam,uReturnCode);
      #endif
      // Get next application in list.
      pszExecute = strtok(NULL,",");
   }
}


//---------------------------------------------------------------------
// CheckWindowsVersion():
//    This function makes sure that the Windows version we are running
// on is at least 3.1 or above.  Workplace Shell does NOT support
// earlier versions of Windows (less than 3.1).
//    If the version is valid, this function returns TRUE (success).
// Otherwise, it informs the user of the error, and returns FALSE.
//---------------------------------------------------------------------
BOOL CheckWindowsVersion(void)
{
   BYTE  bMajor,bMinor;
   char  szMsgText[128];

   // Get major and minor version numbers for Windows operating system.
   bMajor = LOBYTE(LOWORD(GetVersion()));
   bMinor = HIBYTE(LOWORD(GetVersion()));

   // Check if Windows version is 3.1 or above (CHICAGO is Windows 4.0).
   if ((bMajor>0x03)||((bMajor==0x03)&&(bMinor>=0x0A)))
      return (TRUE);    // Return success.
   else
   {
      // Display message informing user of error.
      sprintf(szMsgText,GetString(IDSTR_WIN_VERSION_MESSAGE),(UINT)bMajor,(UINT)bMinor);
      MessageBox(NULL,szMsgText,GetString(IDSTR_ERR_CAPTION),MB_OK|MB_ICONSTOP);
      // Return failure.
      return (FALSE);
   }
}

//---------------------------------------------------------------------
// LoadNLSResourceLibrary():
//---------------------------------------------------------------------
BOOL LoadNLSResourceLibrary(void)
{
   g_wpEnvironment.hResInst=LoadLibraryExA("WPS_ENUS.DLL",NULL,LOAD_LIBRARY_AS_DATAFILE);
   if (!g_wpEnvironment.hResInst) {
      PortLog("ERROR: cannot load WPS_ENUS.DLL");
      MessageBoxA(NULL,"Keep WPS_ENUS.DLL beside wpshell.exe.","Workplace Shell native port",MB_OK|MB_ICONERROR);
      return FALSE;
   }
   return TRUE;
}


//---------------------------------------------------------------------
// InitializeEnvironmentStructure():
//    This function initializes most of the fields in the WPENVIRONMENT
// structure.  It also loads the resource string table and reads the
// SYSTEM.INI entry keys under the "[Workplace_Shell_for_Windows]".
//---------------------------------------------------------------------
void InitializeEnvironmentStructure(HINSTANCE hInstance)
{
   char szValue[128];

   // Store task handle of application in global variable.
   g_wpEnvironment.hTask = GetCurrentTask();
   // Store handle of application instance in global variable.
   g_wpEnvironment.hInst = hInstance;
   // Accelerator table translation is enabled by default.
   g_wpEnvironment.fAcceleratorTranslation = TRUE;
   // Window handle to current modeless dialog (used for notebook control pages).
   g_wpEnvironment.hwndCurrentModelessDialog = NULL;


   // Load WPS system defaults from SYSTEM.INI.

   // Initialize icon indentation values (only applicable for "Icon View").
   // Horizontal icon indentation value.
   g_wpEnvironment.sizeIconIndent.cx = (int)GetPrivateProfileInt(GetString(IDSTR_WPSFWIN_SECTION_NAME),GetString(IDSTR_DESKTOP_HORZICONINDENT),23,GetString(IDSTR_WIN_SYSTEM_PROFILE));
   // Vertical icon indentation value.
   g_wpEnvironment.sizeIconIndent.cy = (int)GetPrivateProfileInt(GetString(IDSTR_WPSFWIN_SECTION_NAME),GetString(IDSTR_DESKTOP_VERTICONINDENT),5,GetString(IDSTR_WIN_SYSTEM_PROFILE));

   // "Confirmations" settings.
   // Confirm on folder delete?
   GetPrivateProfileString(GetString(IDSTR_WPSFWIN_SECTION_NAME),GetString(IDSTR_SYSTEM_CONFIRMFOLDERDELETE),GetString(IDSTR_YES),szValue,sizeof(szValue),GetString(IDSTR_WIN_SYSTEM_PROFILE));
   if (stricmp(szValue,GetString(IDSTR_YES))==0)
      g_wpEnvironment.stSystem.fConfirmOnFolderDelete = TRUE;
   else
      g_wpEnvironment.stSystem.fConfirmOnFolderDelete = FALSE;

   // Confirm on delete?
   GetPrivateProfileString(GetString(IDSTR_WPSFWIN_SECTION_NAME),GetString(IDSTR_SYSTEM_CONFIRMDELETE),GetString(IDSTR_YES),szValue,sizeof(szValue),GetString(IDSTR_WIN_SYSTEM_PROFILE));
   if (stricmp(szValue,GetString(IDSTR_YES))==0)
      g_wpEnvironment.stSystem.fConfirmOnDelete = TRUE;
   else
      g_wpEnvironment.stSystem.fConfirmOnDelete = FALSE;

   // Confirm on file system delete?
   GetPrivateProfileString(GetString(IDSTR_WPSFWIN_SECTION_NAME),GetString(IDSTR_SYSTEM_CONFIRMFILEDELETE),GetString(IDSTR_YES),szValue,sizeof(szValue),GetString(IDSTR_WIN_SYSTEM_PROFILE));
   if (stricmp(szValue,GetString(IDSTR_YES))==0)
      g_wpEnvironment.stSystem.fConfirmOnFileSystemDelete = TRUE;
   else
      g_wpEnvironment.stSystem.fConfirmOnFileSystemDelete = FALSE;


   // "Window" settings.
   // Load default button appearance (hide or minimize).
   GetPrivateProfileString(GetString(IDSTR_WPSFWIN_SECTION_NAME),GetString(IDSTR_SYSTEM_BUTTONAPPEARANCE),GetString(IDSTR_HIDE),szValue,sizeof(szValue),GetString(IDSTR_WIN_SYSTEM_PROFILE));
   if (stricmp(szValue,GetString(IDSTR_HIDE))==0)
      g_wpEnvironment.stSystem.fButtonAppearanceHide = TRUE;
   else
      g_wpEnvironment.stSystem.fButtonAppearanceHide = FALSE;

   // Load default window animation status (enabled or disabled).
   GetPrivateProfileString(GetString(IDSTR_WPSFWIN_SECTION_NAME),GetString(IDSTR_SYSTEM_ANIMATION),GetString(IDSTR_ENABLED),szValue,sizeof(szValue),GetString(IDSTR_WIN_SYSTEM_PROFILE));
   if (stricmp(szValue,GetString(IDSTR_ENABLED))==0)
      g_wpEnvironment.stSystem.fWindowAnimationEnabled = TRUE;
   else
      g_wpEnvironment.stSystem.fWindowAnimationEnabled = FALSE;

   // Load default minimize window button behavior.
   GetPrivateProfileString(GetString(IDSTR_WPSFWIN_SECTION_NAME),GetString(IDSTR_SYSTEM_MINBEHAVIOR),GetString(IDSTR_HIDE),szValue,sizeof(szValue),GetString(IDSTR_WIN_SYSTEM_PROFILE));
   if (stricmp(szValue,GetString(IDSTR_HIDE))==0)
      g_wpEnvironment.stSystem.wMinButtonBehavior = MINBEHAVIOR_HIDE;
   else
   if (stricmp(szValue,GetString(IDSTR_MINTOVIEWER))==0)
      g_wpEnvironment.stSystem.wMinButtonBehavior = MINBEHAVIOR_MIN_TO_VIEWER;
   else
      g_wpEnvironment.stSystem.wMinButtonBehavior = MINBEHAVIOR_MIN_TO_DESKTOP;

   // Load default object open behavior.
   GetPrivateProfileString(GetString(IDSTR_WPSFWIN_SECTION_NAME),GetString(IDSTR_SYSTEM_OPENBEHAVIOR),GetString(IDSTR_CREATENEW),szValue,sizeof(szValue),GetString(IDSTR_WIN_SYSTEM_PROFILE));
   if (stricmp(szValue,GetString(IDSTR_CREATENEW))==0)
      g_wpEnvironment.stSystem.wOpenBehavior = OPENBEHAVIOR_CREATE;
   else
      g_wpEnvironment.stSystem.wOpenBehavior = OPENBEHAVIOR_DISPLAY;

   // "Other" system values needed by shell.
   GetPrivateProfileString(GetString(IDSTR_WPSFWIN_SECTION_NAME),GetString(IDSTR_SYSTEM_SYSTEMSETUPFOLDERNAME),GetString(IDSTR_SYSTEMSETUP),szValue,sizeof(szValue),GetString(IDSTR_WIN_SYSTEM_PROFILE));
   // Make sure the folder name string is null-terminated.
   szValue[MAX_TITLE_NAME] = '\0';
   // Copy string into system global structure field.
   strcpy(g_wpEnvironment.stSystem.szSystemSetupFolderName,szValue);

   // Check if "No Save" feature is enabled.
   GetPrivateProfileString(GetString(IDSTR_WPSFWIN_SECTION_NAME),GetString(IDSTR_SYSTEM_NOSAVE),GetString(IDSTR_NO),szValue,sizeof(szValue),GetString(IDSTR_WIN_SYSTEM_PROFILE));
   if (stricmp(szValue,GetString(IDSTR_ENABLED))==0)
      g_wpEnvironment.stSystem.fNoSaveEnabled = TRUE;
   else
      g_wpEnvironment.stSystem.fNoSaveEnabled = FALSE;

   // Load default "Folder Automatic Close" system property value (Never, Subfolders Only, or All Objects).
   GetPrivateProfileString(GetString(IDSTR_WPSFWIN_SECTION_NAME),GetString(IDSTR_SYSTEM_FOLDERAUTOMATICCLOSE),GetString(IDSTR_NEVER),szValue,sizeof(szValue),GetString(IDSTR_WIN_SYSTEM_PROFILE));
   if (stricmp(szValue,GetString(IDSTR_SUBFOLDERS_ONLY))==0)
      g_wpEnvironment.stSystem.wFolderAutomaticClose = SUBFOLDERS_ONLY;
   else
   if (stricmp(szValue,GetString(IDSTR_ALL_OBJECTS))==0)
      g_wpEnvironment.stSystem.wFolderAutomaticClose = ALL_OBJECTS;
   else
      g_wpEnvironment.stSystem.wFolderAutomaticClose = NEVER;


   // Query shadow text color (default to light blue color).
   GetPrivateProfileString(GetString(IDSTR_WPSFWIN_SECTION_NAME),GetString(IDSTR_COLOR_SHADOWTEXT),"FF0000",szValue,sizeof(szValue),GetString(IDSTR_WIN_SYSTEM_PROFILE));
   // Scan out the color reference value from string read.
   sscanf(szValue,"%08lX",&(g_wpEnvironment.clrrefShadowTextColor));

   // Query compatibility flags from profile
   // (they are used to fix interaction problems with other applications).
   g_wpEnvironment.wCompFlags = (WORD)GetPrivateProfileInt(GetString(IDSTR_WPSFWIN_SECTION_NAME),
      "COMPATIBLE_Flags",0x0000,GetString(IDSTR_WIN_SYSTEM_PROFILE));
}


//---------------------------------------------------------------------
// dprintf():
//    This function is used to help debug this program.  It acts as a
// regular 'printf()' function but directs its output to the "Debug
// Window" application.  This "Debug Window" application is supplied
// with MS Visual C++ 1.x as DBWIN.EXE and DBWIN.DLL.
//    This function can be permanently removed or left here after
// all development is done.
//---------------------------------------------------------------------
void dprintf( char *pszFormat, ... )
{
#ifdef _DEBUG
    char    szStr[256];    // Maximum string allowed is 255 chars + NULL.
    va_list va_ptr;

    // Create string from variable argument list...
    va_start( va_ptr, pszFormat );
    vsprintf( szStr, pszFormat, va_ptr );
    va_end( va_ptr );
    // Display output string in "Debug Window" application.
    OutputDebugString( szStr );
    // optionally, you could display the output on a message box and block.
    //MessageBox(NULL,szStr,"Debug Output",(MB_OK | MB_ICONINFORMATION));
#endif
}


// EOF MAIN.CPP

