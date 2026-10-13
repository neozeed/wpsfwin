#ifndef WPS_RUN_COMMAND_HPP
#define WPS_RUN_COMMAND_HPP
#include <string>
// Split only the executable token; preserve argument quoting for the child.
inline bool PortSplitCommand(const std::string& line, bool wholePath,
                             std::string& file, std::string& args) {
 file.clear(); args.clear();
 size_t start=line.find_first_not_of(" \t\r\n");
 if(start==std::string::npos) return false;
 size_t end=line.find_last_not_of(" \t\r\n");
 std::string s=line.substr(start,end-start+1);
 if(wholePath) {file=s;return true;}
 size_t split;
 if(s[0]=='"') {
  split=s.find('"',1);
  if(split==std::string::npos || split==1) return false;
  file=s.substr(1,split-1); ++split;
  if(split<s.size() && s[split]!=' ' && s[split]!='\t') return false;
 } else {
  split=s.find_first_of(" \t");
  if(split==std::string::npos) {file=s;return true;}
  file=s.substr(0,split);
 }
 split=s.find_first_not_of(" \t",split);
 if(split!=std::string::npos) args=s.substr(split);
 return !file.empty();
}
#endif
