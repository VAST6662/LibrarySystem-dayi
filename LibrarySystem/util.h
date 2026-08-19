#ifndef UTIL_H
#define UTIL_H

#include <string>
using namespace std;

// 读取一个整数，输入非法会重新提示
int readInt(const string& tip);

// 读取一个小数
double readDouble(const string& tip);

// 读取一行字符串（自动去掉首尾空格）
string readLine(const string& tip);

// 去掉字符串首尾的空白字符
string trim(const string& s);

// 把字符串完整转成 double / int，成功返回 true
bool strToDouble(const string& s, double& val);
bool strToInt(const string& s, int& val);

#endif
