#ifndef WPS_PORT_COMPAT_HPP
#define WPS_PORT_COMPAT_HPP
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <windowsx.h>
#include <commctrl.h>
#include <algorithm>
template<class A,class B> inline A min(A a,B b) { return a<b?a:(A)b; }
template<class A,class B> inline A max(A a,B b) { return a>b?a:(A)b; }
#undef VIEW_DETAILS
#define FIELDOFFSET FIELD_OFFSET
#define OF_SEARCH 0
#include <shellapi.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <io.h>
#include <direct.h>
#include <fcntl.h>
#include <sys/stat.h>
#define _loadds
#define _export
#define __export
#define _far
#define _near
#define huge
#define _fmemcpy memcpy
#define _fmemset memset
#define _fstrcpy strcpy
#define _fstrlen strlen
#define _fstrcmp strcmp
#undef MakeProcInstance
#define MakeProcInstance(proc,inst) (proc)
#undef FreeProcInstance
#define FreeProcInstance(proc) ((void)0)
#define SetMessageQueue(n) TRUE
#define GetCurrentTask() ((HTASK)(ULONG_PTR)GetCurrentProcessId())
#define SetWindowOrg(h,x,y) SetWindowOrgEx(h,x,y,NULL)
inline POINT PortMakePoint(LPARAM l) { POINT p={(short)LOWORD(l),(short)HIWORD(l)}; return p; }
#define MAKEPOINT(l) PortMakePoint(l)
// Match the square custom system-menu icon, even when Windows caption
// buttons are wider than they are tall. Screen coordinates must be signed.
inline LRESULT PortCaptionHitTest(HWND window, WPARAM wp, LPARAM lp) {
 LRESULT hit = DefWindowProc(window, WM_NCHITTEST, wp, lp);
 RECT r; GetWindowRect(window, &r);
 POINT p = PortMakePoint(lp);
 int x = p.x-r.left, y = p.y-r.top;
 int left = GetSystemMetrics(SM_CXFRAME), top = GetSystemMetrics(SM_CYFRAME);
 if ((hit == HTCAPTION || hit == HTSYSMENU) &&
     x >= left && x < left+GetSystemMetrics(SM_CYSIZE)+1 &&
     y >= top && y < top+GetSystemMetrics(SM_CYSIZE)+1)
    return HTSYSMENU;
 return hit;
}
INT_PTR CALLBACK PortRunDialog(HWND,UINT,WPARAM,LPARAM);
void PortSaveDesktopPlacement(HWND);
void PortRestoreDesktopPlacement(HWND);
BOOL PortLaunchProgram(void*,HWND,const char*,const char*,const char*,int,BOOL);
void PortForgetProgram(void*);
void PortShowWindows(HWND);
BOOL PortWinHelp(HWND,LPCSTR,UINT,ULONG_PTR);
#undef WinHelp
#define WinHelp PortWinHelp
inline BOOL PortIsProgramPath(const char* path) {
 const char* ext=strrchr(path,'.');
 return ext && (!stricmp(ext,".exe") || !stricmp(ext,".com") || !stricmp(ext,".bat") || !stricmp(ext,".cmd") || !stricmp(ext,".pif"));
}
extern BOOL g_portLegacyDataProfile;
HBITMAP PortLoadBackground(const char*);
BOOL PortBrowseBackground(HWND,char*,UINT);
void PortStartDiagnostics();
void PortLog(const char*);
BOOL PortRegisterControls(HINSTANCE);
UINT PortGetDriveType(UINT);
UINT PortGetAppDirectory(char*,UINT);
struct _find_t {
 unsigned attrib; unsigned short wr_time,wr_date; unsigned long size; char name[MAX_PATH];
 HANDLE search; unsigned mask;
};
unsigned _dos_findfirst(const char*,unsigned,_find_t*);
unsigned _dos_findnext(_find_t*);
void PortFindClose(_find_t*);
inline unsigned _dos_setfileattr(const char* p,unsigned a) { return SetFileAttributesA(p,a?a:FILE_ATTRIBUTE_NORMAL)?0:1; }
#endif
