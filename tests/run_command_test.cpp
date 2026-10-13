#include "../port/run_command.hpp"
#include <cassert>
#include <iostream>
static void check(const char* command,bool whole,const char* file,const char* args) {
 std::string f,a; assert(PortSplitCommand(command,whole,f,a)); assert(f==file);assert(a==args);
}
int main() {
 check("notepad.exe",false,"notepad.exe","");
 check("  cmd.exe /c echo hello  ",false,"cmd.exe","/c echo hello");
 check("\"C:\\Program Files\\App\\app.exe\" \"a b.txt\" /x",false,"C:\\Program Files\\App\\app.exe","\"a b.txt\" /x");
 check("\"C:\\My App\\app.exe\"",false,"C:\\My App\\app.exe","");
 check("C:\\My Documents\\file.txt",true,"C:\\My Documents\\file.txt","");
 check("https://example.com/a?q=b",false,"https://example.com/a?q=b","");
 check("cmd.exe\t/c echo \"hello world\"",false,"cmd.exe","/c echo \"hello world\"");
 std::string f,a;
 assert(!PortSplitCommand("",false,f,a));
 assert(!PortSplitCommand(" \t\r\n",false,f,a));
 assert(!PortSplitCommand("\"unfinished",false,f,a));
 assert(!PortSplitCommand("\"\" args",false,f,a));
 assert(!PortSplitCommand("\"app.exe\"oops",false,f,a));
 std::cout << "12 command parsing cases passed\n";
}
