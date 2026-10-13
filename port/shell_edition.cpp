#include "compat.hpp"
#include "resource.h"
static BOOL fullSession=FALSE;
static HWND barButtons[5]={};
BOOL PortShellEdition() {
#ifdef WPS_SHELL_EDITION
 return TRUE;
#else
 return FALSE;
#endif
}
BOOL PortInitializeEdition(const char* command) {
 if(!PortShellEdition())return TRUE;
 fullSession=command && (!stricmp(command,"--shell") || !stricmp(command,"/shell"));
 HANDLE mutex=CreateMutexA(NULL,FALSE,"Local\\WPSFWIN.Native.ShellEdition");
 if(mutex && GetLastError()==ERROR_ALREADY_EXISTS) {
  CloseHandle(mutex);MessageBoxA(NULL,"The shell edition is already running in this session.","Workplace Shell",MB_OK);return FALSE;
 }
 // Kept for the lifetime of this process. No shell registration is modified.
 return TRUE;
}
const char* PortEditionTitle() {
 if(!PortShellEdition())return "Workplace Shell for Windows - Native Port R6a";
 return fullSession?"Workplace Shell - Shell Edition R6a":"Workplace Shell - Shell Preview R6a";
}
void PortLayoutShell(HWND window) {
 if(!PortShellEdition())return;
 RECT r;GetClientRect(window,&r);
 int height=32,width=max(1,(r.right-12)/5);
 for(int i=0;i<5;++i)if(barButtons[i])MoveWindow(barButtons[i],6+i*width,max(0,r.bottom-height-6),width-4,height,TRUE);
}
struct DesktopOrder { HWND self,previous;BOOL found; };
static BOOL CALLBACK FindDesktopOrder(HWND window,LPARAM param) {
 DesktopOrder* order=(DesktopOrder*)param;
 if(window==order->self || !IsWindowVisible(window))return TRUE;
 char name[64];GetClassNameA(window,name,sizeof(name));
 if(!strcmp(name,"Progman") || !strcmp(name,"WorkerW")) {order->found=TRUE;return FALSE;}
 if(!(GetWindowLongPtr(window,GWL_EXSTYLE)&WS_EX_TOPMOST))order->previous=window;return TRUE;
}
static void LowerShell(HWND window) {
 DesktopOrder order={window,HWND_TOP,FALSE};EnumWindows(FindDesktopOrder,(LPARAM)&order);
 // Keep above Explorer's desktop host, while below ordinary application windows.
 HWND after=order.found?order.previous:HWND_BOTTOM;
 SetWindowPos(window,after,0,0,0,0,SWP_NOMOVE|SWP_NOSIZE|SWP_NOACTIVATE);
}
void PortSizeShell(HWND window) {
 if(!PortShellEdition())return;
 MONITORINFO mi={};mi.cbSize=sizeof(mi);
 if(!GetMonitorInfo(MonitorFromWindow(window,MONITOR_DEFAULTTOPRIMARY),&mi))return;
 RECT r=fullSession?mi.rcMonitor:mi.rcWork;
 SetWindowPos(window,NULL,r.left,r.top,r.right-r.left,r.bottom-r.top,SWP_NOACTIVATE|SWP_NOZORDER);
 LowerShell(window);
 PortLayoutShell(window);
}
void PortStartShell(HWND window) {
 if(!PortShellEdition())return;
 const char* labels[]={"Run...","Window list","LaunchPad","Explorer",fullSession?"Exit / Explorer":"Exit preview"};
 for(int i=0;i<5;++i) {
  barButtons[i]=CreateWindowExA(0,"BUTTON",labels[i],WS_CHILD|WS_VISIBLE|WS_TABSTOP,
    0,0,1,1,window,(HMENU)(INT_PTR)(1800+i),GetModuleHandle(NULL),NULL);
  SendMessage(barButtons[i],WM_SETFONT,(WPARAM)GetStockObject(DEFAULT_GUI_FONT),TRUE);
 }
 RegisterHotKey(window,1805,MOD_CONTROL|MOD_ALT,VK_F12);
 RegisterHotKey(window,1806,MOD_CONTROL|MOD_ALT,'R');
 RegisterHotKey(window,1807,MOD_CONTROL|MOD_ALT,'W');
 PortSizeShell(window);
}
BOOL PortShellMessage(HWND window,UINT msg,WPARAM wp,LPARAM) {
 if(!PortShellEdition())return FALSE;
 if(msg==WM_SIZE){PortLayoutShell(window);return FALSE;}
 if(msg==WM_DISPLAYCHANGE || msg==WM_SETTINGCHANGE){PortSizeShell(window);return FALSE;}
 if(msg==WM_DESTROY){UnregisterHotKey(window,1805);UnregisterHotKey(window,1806);UnregisterHotKey(window,1807);return FALSE;}
 if(msg==WM_ACTIVATE && LOWORD(wp)!=WA_INACTIVE){LowerShell(window);return FALSE;}
 if(msg==WM_CLOSE && fullSession) {
  if((INT_PTR)ShellExecuteA(window,"open","explorer.exe",NULL,NULL,SW_SHOWNORMAL)<=32) {
   MessageBoxA(window,"Explorer could not be started. The shell will remain open.","Workplace Shell",MB_OK|MB_ICONERROR);return TRUE;
  }
 }
 int action=-1;
 if(msg==WM_COMMAND && LOWORD(wp)>=1800 && LOWORD(wp)<=1804)action=LOWORD(wp)-1800;
 if(msg==WM_HOTKEY && wp==1805)action=4;
 if(msg==WM_HOTKEY && wp==1806)action=0;
 if(msg==WM_HOTKEY && wp==1807)action=1;
 if(action<0)return FALSE;
 if(action<3){int commands[]={IDM_RUN,IDM_WINDOWLIST,IDM_LAUNCHPAD};PostMessage(window,WM_APP+900,commands[action],0);}
 if(action==3) {
  HINSTANCE result=ShellExecuteA(window,"open","explorer.exe",NULL,NULL,SW_SHOWNORMAL);
  if((INT_PTR)result<=32) {MessageBoxA(window,"Explorer could not be started. The shell will remain open.","Workplace Shell",MB_OK|MB_ICONERROR);return TRUE;}
 }
 if(action==4)PostMessage(window,WM_CLOSE,0,0);
 return TRUE;
}
