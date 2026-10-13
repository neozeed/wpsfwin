// Native equivalents of the original standalone desktop utilities.
#include "compat.hpp"
#include <commdlg.h>
#include <string>
#include <vector>
static std::vector<std::string> entries;
static std::string ini;
static void SavePad() {
 char key[20];for(int i=0;i<32;++i){sprintf(key,"Item%d",i);WritePrivateProfileStringA("LaunchPad",key,i<(int)entries.size()?entries[i].c_str():NULL,ini.c_str());}
}
static void FillPad(HWND dlg) {
 SendDlgItemMessage(dlg,1701,LB_RESETCONTENT,0,0);
 for(size_t i=0;i<entries.size();++i)SendDlgItemMessageA(dlg,1701,LB_ADDSTRING,0,(LPARAM)entries[i].c_str());
 SendDlgItemMessage(dlg,1701,LB_SETCURSEL,0,0);
}
static void AddPath(HWND dlg,const char* path) {
 if(entries.size()>=32){MessageBoxA(dlg,"LaunchPad holds up to 32 items.","LaunchPad",MB_OK);return;}
 for(size_t i=0;i<entries.size();++i)if(!stricmp(entries[i].c_str(),path))return;
 entries.push_back(path);SavePad();FillPad(dlg);
}
static INT_PTR CALLBACK Pad(HWND dlg,UINT msg,WPARAM wp,LPARAM) {
 if(msg==WM_INITDIALOG){FillPad(dlg);DragAcceptFiles(dlg,TRUE);return TRUE;}
 if(msg==WM_DROPFILES){HDROP drop=(HDROP)wp;UINT count=DragQueryFileA(drop,0xffffffff,NULL,0);
  for(UINT i=0;i<count;++i){char path[MAX_PATH];if(DragQueryFileA(drop,i,NULL,0)>=MAX_PATH)continue;DragQueryFileA(drop,i,path,sizeof(path));AddPath(dlg,path);}DragFinish(drop);return TRUE;}
 if(msg==WM_CLOSE){EndDialog(dlg,0);return TRUE;}
 if(msg!=WM_COMMAND)return FALSE;
 switch(LOWORD(wp)) {
 case IDCANCEL:EndDialog(dlg,0);return TRUE;
 case 1702:{char file[MAX_PATH]="";OPENFILENAMEA ofn={};ofn.lStructSize=sizeof(ofn);ofn.hwndOwner=dlg;ofn.lpstrFile=file;ofn.nMaxFile=sizeof(file);ofn.lpstrFilter="Programs and shortcuts\0*.exe;*.com;*.bat;*.cmd;*.lnk\0All files\0*.*\0";ofn.Flags=OFN_FILEMUSTEXIST|OFN_NOCHANGEDIR;if(GetOpenFileNameA(&ofn))AddPath(dlg,file);return TRUE;}
 case 1703:{LRESULT row=SendDlgItemMessage(dlg,1701,LB_GETCURSEL,0,0);if(row>=0 && row<(LRESULT)entries.size()){entries.erase(entries.begin()+row);SavePad();FillPad(dlg);}return TRUE;}
 case IDOK:case 1701:
  if(LOWORD(wp)==1701 && HIWORD(wp)!=LBN_DBLCLK)return FALSE;
  {LRESULT row=SendDlgItemMessage(dlg,1701,LB_GETCURSEL,0,0);if(row>=0 && row<(LRESULT)entries.size())ShellExecuteA(dlg,"open",entries[row].c_str(),NULL,NULL,SW_SHOWNORMAL);}return TRUE;
 }return FALSE;
}
static BOOL ShutdownPrivilege() {
 HANDLE token;if(!OpenProcessToken(GetCurrentProcess(),TOKEN_ADJUST_PRIVILEGES|TOKEN_QUERY,&token))return FALSE;
 TOKEN_PRIVILEGES p={};p.PrivilegeCount=1;
 if(!LookupPrivilegeValueA(NULL,SE_SHUTDOWN_NAME,&p.Privileges[0].Luid)){CloseHandle(token);return FALSE;}
 p.Privileges[0].Attributes=SE_PRIVILEGE_ENABLED;SetLastError(ERROR_SUCCESS);
 BOOL ok=AdjustTokenPrivileges(token,FALSE,&p,0,NULL,NULL);DWORD error=GetLastError();CloseHandle(token);return ok && error==ERROR_SUCCESS;
}
static INT_PTR CALLBACK Shut(HWND dlg,UINT msg,WPARAM wp,LPARAM) {
 if(msg==WM_INITDIALOG){const char* labels[]={"Lock workstation","Sign out","Restart Windows","Shut down Windows"};for(int i=0;i<4;++i)SendDlgItemMessageA(dlg,1706,CB_ADDSTRING,0,(LPARAM)labels[i]);SendDlgItemMessage(dlg,1706,CB_SETCURSEL,0,0);return TRUE;}
 if(msg==WM_CLOSE){EndDialog(dlg,0);return TRUE;}
 if(msg!=WM_COMMAND)return FALSE;
 if(LOWORD(wp)==IDCANCEL){EndDialog(dlg,0);return TRUE;}
 if(LOWORD(wp)==IDOK){int action=(int)SendDlgItemMessage(dlg,1706,CB_GETCURSEL,0,0);if(action<0)return TRUE;
  const char* questions[]={"Lock this workstation?","Sign out of Windows? Save your work first.","Restart Windows? Save your work first.","Shut down Windows? Save your work first."};
  if(MessageBoxA(dlg,questions[action],"Confirm Windows action",MB_YESNO|MB_DEFBUTTON2|MB_ICONQUESTION)!=IDYES)return TRUE;
  BOOL ok=FALSE;if(action==0)ok=LockWorkStation();else if(action==1)ok=ExitWindowsEx(EWX_LOGOFF,0);else if(ShutdownPrivilege())ok=ExitWindowsEx(action==2?EWX_REBOOT:EWX_POWEROFF,0x80040000);
  if(ok)EndDialog(dlg,0);else MessageBoxA(dlg,"Windows did not accept the requested action.","Windows action",MB_OK|MB_ICONERROR);return TRUE;
 }return FALSE;
}
int WINAPI WinMain(HINSTANCE inst,HINSTANCE,LPSTR,int) {
 char path[MAX_PATH];GetModuleFileNameA(NULL,path,sizeof(path));char* base=strrchr(path,'\\');base=base?base+1:path;
 if(!stricmp(base,"WNDLIST.EXE")){PortShowWindows(NULL);return 0;}
 if(!stricmp(base,"WINSHUT.EXE")){DialogBoxA(inst,MAKEINTRESOURCEA(1705),NULL,Shut);return 0;}
 char dir[MAX_PATH];PortGetAppDirectory(dir,sizeof(dir));ini=std::string(dir)+"\\launchpad.ini";
 char key[20],value[MAX_PATH];for(int i=0;i<32;++i){sprintf(key,"Item%d",i);GetPrivateProfileStringA("LaunchPad",key,"",value,sizeof(value),ini.c_str());if(value[0])entries.push_back(value);}
 DialogBoxA(inst,MAKEINTRESOURCEA(1700),NULL,Pad);return 0;
}
