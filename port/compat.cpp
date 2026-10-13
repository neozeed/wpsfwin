#include "compat.hpp"
UINT PortGetAppDirectory(char* out,UINT count) {
 DWORD n=GetModuleFileNameA(NULL,out,count); if (!n || n>=count) return 0;
 char* p=strrchr(out,'\\'); if(p) *p=0; return (UINT)strlen(out);
}
UINT PortGetDriveType(UINT i) { char r[]="A:\\"; r[0]+=(char)i; UINT t=GetDriveTypeA(r); return t==DRIVE_NO_ROOT_DIR?0:t; }
static unsigned findCopy(_find_t* f,WIN32_FIND_DATAA* d) {
 f->attrib=d->dwFileAttributes; f->size=d->nFileSizeLow;
 FILETIME local; FileTimeToLocalFileTime(&d->ftLastWriteTime,&local);
 FileTimeToDosDateTime(&local,&f->wr_date,&f->wr_time);
 strcpy(f->name,d->cFileName); return 0;
}
void PortFindClose(_find_t* f) { if(f->search!=INVALID_HANDLE_VALUE) FindClose(f->search); f->search=INVALID_HANDLE_VALUE; }
static bool findAccept(_find_t* f,WIN32_FIND_DATAA* d) {
 unsigned restricted=FILE_ATTRIBUTE_DIRECTORY|FILE_ATTRIBUTE_HIDDEN|FILE_ATTRIBUTE_SYSTEM;
 return !(d->dwFileAttributes & restricted & ~f->mask);
}
unsigned _dos_findfirst(const char* p,unsigned mask,_find_t* f) {
 WIN32_FIND_DATAA d; f->mask=mask; f->search=FindFirstFileA(p,&d);
 if(f->search==INVALID_HANDLE_VALUE) return 1;
 do { if(findAccept(f,&d)) return findCopy(f,&d); } while(FindNextFileA(f->search,&d));
 PortFindClose(f);return 1;
}
unsigned _dos_findnext(_find_t* f) {
 WIN32_FIND_DATAA d; if(f->search==INVALID_HANDLE_VALUE) return 1;
 while(FindNextFileA(f->search,&d)) if(findAccept(f,&d)) return findCopy(f,&d);
 PortFindClose(f);return 1;
}
