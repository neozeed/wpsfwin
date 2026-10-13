#include "compat.hpp"
#include "wps.hpp"
// Compile-time invariants guarding the native pointer representation.
typedef char pointer_fits_list[(sizeof(((LIST_NODE*)0)->m_dwValue)>=sizeof(void*))?1:-1];
typedef char pointer_fits_message[(sizeof(LPARAM)>=sizeof(void*))?1:-1];
void PortLog(const char* message) {
 FILE* f=fopen("wpshell-port.log","a");
 if(f){SYSTEMTIME t;GetLocalTime(&t);fprintf(f,"%02u:%02u:%02u %s\n",t.wHour,t.wMinute,t.wSecond,message);fclose(f);}
}
static LONG WINAPI crash(EXCEPTION_POINTERS* e) {
 char msg[300];ULONG_PTR addr=(ULONG_PTR)e->ExceptionRecord->ExceptionAddress;
 sprintf(msg,"Unhandled exception %08lx at %p (EXE relative %llx)",
  e->ExceptionRecord->ExceptionCode,(void*)addr,(unsigned long long)(addr-(ULONG_PTR)GetModuleHandle(NULL)));
 PortLog(msg);
 MessageBoxA(NULL,"The experimental port encountered an exception. Please send wpshell-port.log with the steps that led to it.","Workplace Shell native port",MB_OK|MB_ICONERROR);
 return EXCEPTION_EXECUTE_HANDLER;
}
void PortStartDiagnostics() {
 SetUnhandledExceptionFilter(crash);
 FILE* f=fopen("wpshell-port.log","w");if(f){fprintf(f,"WPSFWIN native port R5; pointer size %u\n",(unsigned)sizeof(void*));fclose(f);}
}
