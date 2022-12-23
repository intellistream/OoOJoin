//
// Created by tony on 23/12/22.
//
#include <Utils/IntelliLog.h>
#include <string.h>
void INTELLI::IntelliLog::log(std::string level, std::string message, const std::source_location source) {
  time_t now = time(0);

  // 把 now 转换为字符串形式
  char* dt = ctime(&now);
  std::string str=level+":";
  dt[strlen(dt)-1]=0;
  str+=dt;
  //str+= static_cast<char>(level);
  // str+="],";
  str+=source.file_name();
  str+=+"|";
  str+=+source.function_name();
  str+=+"|";
  str+=to_string(source.line());
  str+="|:";
  str+=message;
  cout<<str+"\r\n";

}