// Native replacements for the two binary-only IBM Win16 controls.
// Implements the subset of BKM/SPBM messages used by WPSFWIN.
#include "compat.hpp"
#include "fclnbw.h"
#include "fclspbw.h"
#include <vector>
#include <string>
struct Page { HWND hwnd; WORD style; std::string tab,status; Page():hwnd(0),style(0){} };
struct Book { std::vector<Page> pages; int top; Book():top(-1){} };
static void bookLayout(HWND h,Book* b) {
 RECT r; GetClientRect(h,&r);
 for(size_t i=0;i<b->pages.size();++i) if(b->pages[i].hwnd) {
  MoveWindow(b->pages[i].hwnd,14,14,max(1L,r.right-154),max(1L,r.bottom-52),TRUE);
  ShowWindow(b->pages[i].hwnd,(int)i==b->top?SW_SHOW:SW_HIDE);
 }
 InvalidateRect(h,NULL,TRUE);
}
static void bookSelect(HWND h,Book* b,int i) {
 if(i<0 || i>=(int)b->pages.size()) return;
 b->top=i; bookLayout(h,b);
 SendMessage(GetParent(h),WM_COMMAND,MAKEWPARAM(GetDlgCtrlID(h),BKN_PAGESELECTED),(LPARAM)h);
}
static LRESULT CALLBACK BookProc(HWND h,UINT m,WPARAM w,LPARAM l) {
 Book* b=(Book*)GetWindowLongPtr(h,GWLP_USERDATA);
 if(m==WM_CREATE) { b=new Book;SetWindowLongPtr(h,GWLP_USERDATA,(LONG_PTR)b);return 0; }
 if(!b) return DefWindowProc(h,m,w,l);
 int i=(int)w-1;
 switch(m) {
 case BKM_INSERTPAGE: { Page p;p.style=LOWORD(l);b->pages.push_back(p);return b->pages.size(); }
 case BKM_SETPAGEWINDOWHWND: if(i>=0 && i<(int)b->pages.size()){b->pages[i].hwnd=(HWND)l;bookLayout(h,b);return TRUE;} return FALSE;
 case BKM_QUERYPAGEWINDOWHWND: return i>=0 && i<(int)b->pages.size()?(LRESULT)b->pages[i].hwnd:0;
 case BKM_SETTABTEXT: if(i>=0 && i<(int)b->pages.size()){b->pages[i].tab=l?(char*)l:"";return TRUE;}return FALSE;
 case BKM_SETSTATUSLINETEXT: if(i>=0 && i<(int)b->pages.size()){b->pages[i].status=l?(char*)l:"";return TRUE;}return FALSE;
 case BKM_TURNTOPAGE: bookSelect(h,b,i);return TRUE;
 case BKM_QUERYPAGEID: return b->top+1;
 case BKM_QUERYPAGECOUNT:return b->pages.size();
 case BKM_DELETEPAGE: b->pages.clear();b->top=-1;return TRUE;
 case BKM_SETDIMENSIONS:case BKM_SETNOTEBOOKCOLORS:return TRUE;
 case WM_SIZE:bookLayout(h,b);return 0;
 case WM_KEYDOWN:if(w==VK_NEXT || w==VK_RIGHT)bookSelect(h,b,min(b->top+1,(int)b->pages.size()-1));else if(w==VK_PRIOR||w==VK_LEFT)bookSelect(h,b,max(b->top-1,0));return 0;
 case WM_LBUTTONDOWN: {
  RECT r;GetClientRect(h,&r);int x=GET_X_LPARAM(l),y=GET_Y_LPARAM(l),tab=0;
  if(x>=r.right-130 && y<r.bottom-32) for(size_t j=0;j<b->pages.size();++j) if(!b->pages[j].tab.empty()) {if(y>=14+tab*34 && y<48+tab*34){bookSelect(h,b,j);break;}++tab;}
  if(y>=r.bottom-32)bookSelect(h,b,x<r.right/2?max(b->top-1,0):min(b->top+1,(int)b->pages.size()-1));
  return 0;
 }
 case WM_PAINT: {
  PAINTSTRUCT ps;HDC dc=BeginPaint(h,&ps);RECT r;GetClientRect(h,&r);
  FillRect(dc,&r,(HBRUSH)GetStockObject(LTGRAY_BRUSH));
  RECT page={6,6,r.right-136,r.bottom-36};DrawEdge(dc,&page,EDGE_RAISED,BF_RECT);
  HFONT old=(HFONT)SelectObject(dc,GetStockObject(DEFAULT_GUI_FONT));SetBkMode(dc,TRANSPARENT);
  int tab=0;for(size_t j=0;j<b->pages.size();++j) if(!b->pages[j].tab.empty()) {
   RECT t={r.right-133,14+tab*34,r.right-6,46+tab*34};
   FillRect(dc,&t,(HBRUSH)GetStockObject((int)j==b->top?WHITE_BRUSH:LTGRAY_BRUSH));DrawEdge(dc,&t,EDGE_RAISED,BF_RECT);InflateRect(&t,-6,0);
   DrawTextA(dc,b->pages[j].tab.c_str(),-1,&t,DT_SINGLELINE|DT_VCENTER);++tab;
  }
  if(b->top>=0) {RECT s={14,r.bottom-30,r.right-14,r.bottom-6};std::string label="< Previous     "+b->pages[b->top].status+"     Next >";DrawTextA(dc,label.c_str(),-1,&s,DT_CENTER|DT_SINGLELINE|DT_VCENTER);}
  SelectObject(dc,old);EndPaint(h,&ps);return 0;
 }
 case WM_NCDESTROY:delete b;SetWindowLongPtr(h,GWLP_USERDATA,0);break;
 }
 return DefWindowProc(h,m,w,l);
}
struct Spin { HWND edit,up; LONG lo,hi; Spin():edit(0),up(0),lo(0),hi(100){} };
static LRESULT CALLBACK SpinProc(HWND h,UINT m,WPARAM w,LPARAM l) {
 Spin* s=(Spin*)GetWindowLongPtr(h,GWLP_USERDATA);
 if(m==WM_CREATE){s=new Spin;SetWindowLongPtr(h,GWLP_USERDATA,(LONG_PTR)s);
 s->edit=CreateWindowExA(WS_EX_CLIENTEDGE,"EDIT","0",WS_CHILD|WS_VISIBLE|WS_TABSTOP|ES_NUMBER,0,0,1,1,h,(HMENU)1,GetModuleHandle(NULL),0);
 s->up=CreateWindowExA(0,UPDOWN_CLASSA,"",WS_CHILD|WS_VISIBLE|UDS_SETBUDDYINT|UDS_ARROWKEYS,0,0,1,1,h,(HMENU)2,GetModuleHandle(NULL),0);
 SendMessage(s->up,UDM_SETBUDDY,(WPARAM)s->edit,0);SendMessage(s->edit,WM_SETFONT,(WPARAM)GetStockObject(DEFAULT_GUI_FONT),TRUE);return 0;}
 if(!s)return DefWindowProc(h,m,w,l);
 switch(m){
 case WM_SIZE:MoveWindow(s->edit,0,0,max(1,(int)LOWORD(l)-18),HIWORD(l),TRUE);MoveWindow(s->up,max(1,(int)LOWORD(l)-18),0,18,HIWORD(l),TRUE);return 0;
 case SPBM_SETLIMITS:{SPBLIMITS* p=(SPBLIMITS*)l;s->lo=p->lLowerLimit;s->hi=p->lUpperLimit;SendMessage(s->up,UDM_SETRANGE32,s->lo,s->hi);return TRUE;}
 case SPBM_SETCURRENTVALUE:SendMessage(s->up,UDM_SETPOS32,0,l);return TRUE;
 case SPBM_QUERYVALUE:{SPBQUERYVALUE* p=(SPBQUERYVALUE*)l;LONG n=(LONG)SendMessage(s->up,UDM_GETPOS32,0,0);n=max(s->lo,min(s->hi,n));if(p && p->lpValueBuf)*(LONG*)p->lpValueBuf=n;return TRUE;}
 case WM_SETFOCUS:SetFocus(s->edit);return 0;
 case WM_NCDESTROY:delete s;SetWindowLongPtr(h,GWLP_USERDATA,0);break;
 }
 return DefWindowProc(h,m,w,l);
}
BOOL PortRegisterControls(HINSTANCE inst) {
 INITCOMMONCONTROLSEX cc={sizeof(cc),ICC_UPDOWN_CLASS};InitCommonControlsEx(&cc);
 WNDCLASSA wc={0};wc.hInstance=inst;wc.hCursor=LoadCursor(NULL,IDC_ARROW);wc.hbrBackground=(HBRUSH)GetStockObject(LTGRAY_BRUSH);wc.lpfnWndProc=BookProc;wc.lpszClassName=CCL_NOTEBOOK;
 if(!RegisterClassA(&wc))return FALSE;wc.lpfnWndProc=SpinProc;wc.lpszClassName=CCL_SPINBTN;return RegisterClassA(&wc)!=0;
}
