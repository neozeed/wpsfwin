#include "compat.hpp"
#include <objidl.h>
#include <gdiplus.h>
#include <commdlg.h>
HBITMAP PortLoadBackground(const char* path) {
 if(!path || !path[0]) return NULL;
 WCHAR wide[MAX_PATH];
 if(!MultiByteToWideChar(CP_ACP,0,path,-1,wide,MAX_PATH)) return NULL;
 Gdiplus::GdiplusStartupInput input; ULONG_PTR token=0;
 if(Gdiplus::GdiplusStartup(&token,&input,NULL)!=Gdiplus::Ok) return NULL;
 HBITMAP result=NULL;
 {
  Gdiplus::Bitmap image(wide);
  // Bound decoded image memory. Large photographs should be resized first.
  UINT w=image.GetWidth(),h=image.GetHeight();
  if(image.GetLastStatus()==Gdiplus::Ok && w && h && w<=16384 && h<=16384 && (ULONGLONG)w*h<=64000000)
   image.GetHBITMAP(Gdiplus::Color(255,255,255),&result);
 }
 Gdiplus::GdiplusShutdown(token);return result;
}
BOOL PortBrowseBackground(HWND owner,char* path,UINT size) {
 char selected[MAX_PATH]="";OPENFILENAMEA ofn={};ofn.lStructSize=sizeof(ofn);
 ofn.hwndOwner=owner;ofn.lpstrFile=selected;ofn.nMaxFile=sizeof(selected);
 ofn.lpstrTitle="Choose background image";
 ofn.lpstrFilter="Background images\0*.bmp;*.dib;*.png;*.jpg;*.jpeg\0All files\0*.*\0";
 ofn.Flags=OFN_FILEMUSTEXIST|OFN_PATHMUSTEXIST|OFN_NOCHANGEDIR;
 if(!GetOpenFileNameA(&ofn))return FALSE;
 HBITMAP check=PortLoadBackground(selected);
 if(!check) {MessageBoxA(owner,"This image could not be loaded. Choose a BMP, PNG or JPEG up to 64 million pixels.","Background image",MB_OK|MB_ICONERROR);return FALSE;}
 DeleteObject(check);lstrcpynA(path,selected,size);return TRUE;
}
