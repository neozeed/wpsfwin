#include "compat.hpp"
#include "resource.h"
#include <map>
#include <string>

struct LaunchedProcess { HANDLE process; DWORD pid; LaunchedProcess():process(NULL),pid(0){} };
BOOL g_portLegacyDataProfile=FALSE;
static std::map<void*,LaunchedProcess> launched;
void PortForgetProgram(void* object) {
 std::map<void*,LaunchedProcess>::iterator i=launched.find(object);
 if(i!=launched.end()) {CloseHandle(i->second.process);launched.erase(i);}
}
struct FindWindowData {DWORD pid; HWND window;};
static BOOL CALLBACK FindProcessWindow(HWND window,LPARAM param) {
 FindWindowData* d=(FindWindowData*)param;DWORD pid=0;GetWindowThreadProcessId(window,&pid);
 if(pid==d->pid && IsWindowVisible(window) && !GetWindow(window,GW_OWNER)) {d->window=window;return FALSE;}return TRUE;
}
static void Activate(HWND window) {
 if(IsIconic(window)) ShowWindow(window,SW_RESTORE);
 SetForegroundWindow(window);
}
BOOL PortLaunchProgram(void* object,HWND owner,const char* file,const char* args,const char* dir,int show,BOOL reuse) {
 std::map<void*,LaunchedProcess>::iterator i=launched.find(object);
 if(i!=launched.end()) {
  if(WaitForSingleObject(i->second.process,0)==WAIT_TIMEOUT && reuse) {
   FindWindowData d={i->second.pid,NULL}; EnumWindows(FindProcessWindow,(LPARAM)&d);
   if(d.window) Activate(d.window);return TRUE;
  }
  PortForgetProgram(object);
 }
 char f[32768],a[32768],d[32768];
 DWORD nf=ExpandEnvironmentStringsA(file?file:"",f,sizeof(f));
 DWORD na=ExpandEnvironmentStringsA(args?args:"",a,sizeof(a));
 DWORD nd=ExpandEnvironmentStringsA(dir?dir:"",d,sizeof(d));
 if(!nf||nf>sizeof(f)||!na||na>sizeof(a)||!nd||nd>sizeof(d)) return FALSE;
 // Object settings store executable and parameters separately.
 size_t n=strlen(f); if(n>=2 && f[0]=='"' && f[n-1]=='"') {memmove(f,f+1,n-2);f[n-2]=0;}
 SHELLEXECUTEINFOA se={};se.cbSize=sizeof(se);se.hwnd=owner;
 se.fMask=SEE_MASK_NOCLOSEPROCESS|SEE_MASK_FLAG_NO_UI;se.lpVerb="open";
 se.lpFile=f;se.lpParameters=a[0]?a:NULL;se.lpDirectory=d[0]?d:NULL;se.nShow=show;
 if(!ShellExecuteExA(&se)) {
  DWORD error=GetLastError();char reason[1024]="",message[1400];
  FormatMessageA(FORMAT_MESSAGE_FROM_SYSTEM|FORMAT_MESSAGE_IGNORE_INSERTS,NULL,error,0,reason,sizeof(reason),NULL);
  snprintf(message,sizeof(message),"Unable to launch program (error %lu).\n\n%s",(unsigned long)error,reason);
  MessageBoxA(owner,message,"Workplace Shell",MB_OK|MB_ICONERROR);return FALSE;
 }
 if(se.hProcess) {LaunchedProcess p;p.process=se.hProcess;p.pid=GetProcessId(se.hProcess);launched[object]=p;}
 return TRUE;
}
static BOOL CALLBACK ListWindow(HWND window,LPARAM param) {
 if(!IsWindowVisible(window)||GetWindow(window,GW_OWNER)||GetWindowLongPtr(window,GWL_EXSTYLE)&WS_EX_TOOLWINDOW) return TRUE;
 char title[512],line[560];if(!GetWindowTextA(window,title,sizeof(title)))return TRUE;
 HWND dlg=(HWND)param;if(window==dlg)return TRUE;
 DWORD pid;GetWindowThreadProcessId(window,&pid);
 snprintf(line,sizeof(line),"%s  [PID %lu]",title,(unsigned long)pid);
 LRESULT row=SendDlgItemMessageA(dlg,1602,LB_ADDSTRING,0,(LPARAM)line);
 if(row!=LB_ERR && row!=LB_ERRSPACE) SendDlgItemMessage(dlg,1602,LB_SETITEMDATA,row,(LPARAM)window);
 return TRUE;
}
static void RefreshWindows(HWND dlg) {
 SendDlgItemMessage(dlg,1602,LB_RESETCONTENT,0,0);EnumWindows(ListWindow,(LPARAM)dlg);
 SendDlgItemMessage(dlg,1602,LB_SETCURSEL,0,0);
}
static INT_PTR CALLBACK WindowsDialog(HWND dlg,UINT msg,WPARAM wp,LPARAM) {
 if(msg==WM_INITDIALOG) {RefreshWindows(dlg);return TRUE;}
 if(msg==WM_CLOSE) {EndDialog(dlg,0);return TRUE;}
 if(msg==WM_COMMAND) {
  if(LOWORD(wp)==IDCANCEL){EndDialog(dlg,0);return TRUE;}
  if(LOWORD(wp)==1603){RefreshWindows(dlg);return TRUE;}
  if(LOWORD(wp)==IDOK || (LOWORD(wp)==1602 && HIWORD(wp)==LBN_DBLCLK)) {
   LRESULT row=SendDlgItemMessage(dlg,1602,LB_GETCURSEL,0,0);
   if(row!=LB_ERR) {
    HWND window=(HWND)SendDlgItemMessage(dlg,1602,LB_GETITEMDATA,row,0);
    if(IsWindow(window)){EndDialog(dlg,0);Activate(window);}else RefreshWindows(dlg);
   }return TRUE;
  }
 }return FALSE;
}
void PortShowWindows(HWND owner) {DialogBoxA(GetModuleHandle(NULL),MAKEINTRESOURCEA(1600),owner,WindowsDialog);}
static const char helpText[]=
"WORKPLACE SHELL FOR WINDOWS - NATIVE EDITION\r\n\r\n"
"Objects and folders\r\nDouble-click an object to open it. Right-click for its menu; Settings opens the notebook. Virtual folders organize objects. Drives and directory objects browse real files. Icon and Details views, Arrange, sorting and Find are available where supported.\r\n\r\n"
"Drag and drop\r\nUse the RIGHT mouse button for the original Workplace Shell drag operations. Drop files or directories from Explorer on a virtual folder or the desktop to create objects referring to those paths. This does not copy the files. Directory windows do not accept this import operation.\r\n\r\n"
"Files and deletion\r\nData and directory objects refer to actual filesystem entries. Their copy, move and delete operations affect those files. The Shredder is not the Windows Recycle Bin. Use disposable files when trying these inherited operations.\r\n\r\n"
"Programs\r\nUse the Program template to create launchers, or drop an executable from Explorer. In Settings, specify the executable, arguments and working directory separately. Environment variables are expanded. Display existing instance reuses a tracked live process; applications that hand off to another process may manage their own windows.\r\n\r\n"
"Run and windows\r\nDesktop menu > Run supports quoted paths, arguments, environment variables, documents, folders and URLs. The last ten successful commands are remembered. Window list shows visible application windows; double-click to switch, or use Refresh. System setup opens Windows Control Panel.\r\n\r\n"
"Templates and shadows\r\nDrag a template to create a new object. Create shadow makes a Workplace Shell reference to an existing object; it is not a Windows .lnk file. Use the original object's menu to inspect its target.\r\n\r\n"
"Keyboard\r\nEnter opens the selected object. F1 opens this guide. Shift+F10 opens the desktop popup. Ctrl+S selects all; Ctrl+D deselects all. In settings use Tab to move between controls; notebook tabs and Previous/Next change pages.\r\n\r\n"
"Background and persistence\r\nSettings includes color-only, normal, tiled and scaled BMP/PNG/JPEG backgrounds. Use Browse on the Background page. Save desktop stores objects and window placement. Keep wpshell32.dat and wpsettings.ini beside the executable. Run history and desktop placement are stored in the INI.\r\n\r\n"
"Compatibility\r\nThis is a standalone native application. Win16 shell replacement, Program Manager DDE migration, global minimized-window management, additional languages, and ancillary Win16 utilities are not implemented. ANSI/MAX_PATH filename limits remain. This guide replaces WinHelp calls; it is not a conversion of every original help topic.\r\n";
static INT_PTR CALLBACK HelpDialog(HWND dlg,UINT msg,WPARAM wp,LPARAM) {
 if(msg==WM_INITDIALOG){SetDlgItemTextA(dlg,1605,helpText);return TRUE;}
 if(msg==WM_CLOSE || (msg==WM_COMMAND && (LOWORD(wp)==IDOK||LOWORD(wp)==IDCANCEL))){EndDialog(dlg,0);return TRUE;}
 return FALSE;
}
BOOL PortWinHelp(HWND owner,LPCSTR,UINT command,ULONG_PTR) {
 if(command==HELP_QUIT)return TRUE;
 return DialogBoxA(GetModuleHandle(NULL),MAKEINTRESOURCEA(1604),owner,HelpDialog)!=-1;
}
