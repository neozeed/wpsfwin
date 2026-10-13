#include "compat.hpp"
#include "resource.h"
#include "run_command.hpp"
#include <commdlg.h>
#include <vector>

static std::string SettingsPath() {
 char dir[MAX_PATH];
 if(!PortGetAppDirectory(dir,sizeof(dir))) return "";
 return std::string(dir)+"\\wpsettings.ini";
}
static void ReadHistory(std::vector<std::string>& items) {
 char key[20],value[4096]; std::string path=SettingsPath();
 for(int i=0;i<10;++i) {
  sprintf(key,"Command%d",i);
  memset(value,0,sizeof(value));
  if(GetPrivateProfileStructA("RunHistory",key,value,sizeof(value),path.c_str())) {
   value[sizeof(value)-1]=0;
   if(value[0]) items.push_back(value);
  }
 }
}
static void SaveHistory(const char* command) {
 std::vector<std::string> old; ReadHistory(old);
 std::vector<std::string> items; items.push_back(command);
 for(size_t i=0;i<old.size() && items.size()<10;++i)
  if(old[i]!=command) items.push_back(old[i]);
 std::string path=SettingsPath(); char key[20];
 for(int i=0;i<10;++i) {
  sprintf(key,"Command%d",i);
  if(i<(int)items.size()) {
   char value[4096]={0}; strncpy(value,items[i].c_str(),sizeof(value)-1);
   WritePrivateProfileStructA("RunHistory",key,value,sizeof(value),path.c_str());
  } else WritePrivateProfileStringA("RunHistory",key,NULL,path.c_str());
 }
}
INT_PTR CALLBACK PortRunDialog(HWND dlg,UINT msg,WPARAM wp,LPARAM) {
 if(msg==WM_INITDIALOG) {
  SendDlgItemMessage(dlg,IDE_COMMANDLINE,CB_LIMITTEXT,4095,0);
  std::vector<std::string> items; ReadHistory(items);
  for(size_t i=0;i<items.size();++i)
   SendDlgItemMessageA(dlg,IDE_COMMANDLINE,CB_ADDSTRING,0,(LPARAM)items[i].c_str());
  CheckRadioButton(dlg,IDB_NORMAL,IDB_MAXIMIZED,IDB_NORMAL);
  RECT r,owner; GetWindowRect(dlg,&r); GetWindowRect(GetParent(dlg),&owner);
  SetWindowPos(dlg,NULL,owner.left+(owner.right-owner.left-(r.right-r.left))/2,
   owner.top+(owner.bottom-owner.top-(r.bottom-r.top))/2,0,0,SWP_NOSIZE|SWP_NOZORDER);
  return TRUE;
 }
 if(msg==WM_CLOSE) {EndDialog(dlg,FALSE);return TRUE;}
 if(msg!=WM_COMMAND) return FALSE;
 switch(LOWORD(wp)) {
 case IDCANCEL: EndDialog(dlg,FALSE);return TRUE;
 case IDB_HELP:
  MessageBoxA(dlg,"Enter a program, document, folder or URL.\n\nQuote program paths that contain spaces when adding arguments:\n\"C:\\Program Files\\App\\app.exe\" /option\n\nEnvironment variables such as %SystemRoot% are expanded.\nThe last ten successfully launched commands appear in the list.","Run Program",MB_OK|MB_ICONINFORMATION);
  return TRUE;
 case IDB_RUNBROWSE: {
  char file[MAX_PATH]=""; OPENFILENAMEA ofn={}; ofn.lStructSize=sizeof(ofn);
  ofn.hwndOwner=dlg; ofn.lpstrFile=file; ofn.nMaxFile=sizeof(file);
  ofn.lpstrFilter="Programs\0*.exe;*.com;*.bat;*.cmd\0All files\0*.*\0";
  ofn.Flags=OFN_FILEMUSTEXIST|OFN_PATHMUSTEXIST|OFN_NOCHANGEDIR;
  if(GetOpenFileNameA(&ofn)) {
   std::string quoted=std::string("\"")+file+"\"";
   SetDlgItemTextA(dlg,IDE_COMMANDLINE,quoted.c_str());
  }
  return TRUE;
 }
 case IDOK: {
  char input[4096],expanded[32768]; GetDlgItemTextA(dlg,IDE_COMMANDLINE,input,sizeof(input));
  DWORD n=ExpandEnvironmentStringsA(input,expanded,sizeof(expanded));
  if(!n || n>sizeof(expanded)) {MessageBoxA(dlg,"Command is too long.","Run Program",MB_OK|MB_ICONERROR);return TRUE;}
  std::string line=expanded,file,args;
  size_t first=line.find_first_not_of(" \t\r\n"),last=line.find_last_not_of(" \t\r\n");
  if(first!=std::string::npos) line=line.substr(first,last-first+1);
  bool wholePath=GetFileAttributesA(line.c_str())!=INVALID_FILE_ATTRIBUTES;
  if(!PortSplitCommand(line,wholePath,file,args)) {
   MessageBoxA(dlg,"Enter a command with balanced quotes around the program path.","Run Program",MB_OK|MB_ICONINFORMATION);return TRUE;
  }
  SHELLEXECUTEINFOA se={}; se.cbSize=sizeof(se); se.hwnd=dlg;
  se.fMask=SEE_MASK_FLAG_NO_UI; se.lpVerb="open"; se.lpFile=file.c_str();
  se.lpParameters=args.empty()?NULL:args.c_str();
  se.nShow=IsDlgButtonChecked(dlg,IDB_MINIMIZED)?SW_SHOWMINIMIZED:
   IsDlgButtonChecked(dlg,IDB_MAXIMIZED)?SW_SHOWMAXIMIZED:SW_SHOWNORMAL;
  if(!ShellExecuteExA(&se)) {
   DWORD error=GetLastError(); char reason[1024]="",message[1400];
   FormatMessageA(FORMAT_MESSAGE_FROM_SYSTEM|FORMAT_MESSAGE_IGNORE_INSERTS,NULL,error,0,reason,sizeof(reason),NULL);
   snprintf(message,sizeof(message),"Unable to open the command (error %lu).\n\n%s",(unsigned long)error,reason);
   MessageBoxA(dlg,message,"Run Program",MB_OK|MB_ICONERROR);return TRUE;
  }
  SaveHistory(input); EndDialog(dlg,TRUE);return TRUE;
 }
 }
 return FALSE;
}
void PortSaveDesktopPlacement(HWND window) {
 WINDOWPLACEMENT p={}; p.length=sizeof(p); if(!GetWindowPlacement(window,&p)) return;
 char value[160]; RECT r=p.rcNormalPosition;
 bool maximized=p.showCmd==SW_SHOWMAXIMIZED || (p.flags&WPF_RESTORETOMAXIMIZED);
 snprintf(value,sizeof(value),"%ld,%ld,%ld,%ld,%d",r.left,r.top,r.right,r.bottom,maximized?1:0);
 WritePrivateProfileStringA("NativeDesktop","Placement",value,SettingsPath().c_str());
}
void PortRestoreDesktopPlacement(HWND window) {
 char value[160]; GetPrivateProfileStringA("NativeDesktop","Placement","",value,sizeof(value),SettingsPath().c_str());
 WINDOWPLACEMENT p={}; p.length=sizeof(p); RECT& r=p.rcNormalPosition; int maximized=0;
 if(sscanf(value,"%ld,%ld,%ld,%ld,%d",&r.left,&r.top,&r.right,&r.bottom,&maximized)!=5) return;
 // Reject corrupt coordinates, then bring disconnected-monitor placements back.
 if(r.left < -100000 || r.left > 100000 || r.top < -100000 || r.top > 100000 ||
    r.right < -100000 || r.right > 100000 || r.bottom < -100000 || r.bottom > 100000 ||
    r.right-r.left<320 || r.bottom-r.top<240 || r.right-r.left>20000 || r.bottom-r.top>20000) return;
 MONITORINFO mi={}; mi.cbSize=sizeof(mi);
 if(GetMonitorInfo(MonitorFromRect(&r,MONITOR_DEFAULTTONEAREST),&mi)) {
  // WINDOWPLACEMENT uses work-area coordinates; account for taskbars on top/left.
  RECT work=mi.rcWork; OffsetRect(&work,mi.rcMonitor.left-mi.rcWork.left,mi.rcMonitor.top-mi.rcWork.top);
  int width=min(r.right-r.left,work.right-work.left),height=min(r.bottom-r.top,work.bottom-work.top);
  r.left=max(work.left,min(r.left,work.right-width)); r.top=max(work.top,min(r.top,work.bottom-height));
  r.right=r.left+width;r.bottom=r.top+height;
 }
 p.showCmd=maximized?SW_SHOWMAXIMIZED:SW_SHOWNORMAL; SetWindowPlacement(window,&p);
}
