#include "util.h"
#include <iostream>
#include <cstdlib>
using namespace std;

// 去掉字符串首尾空白，中间不动
string trim(const string& s) {
    size_t begin = s.find_first_not_of(" \t\r\n");
    if (begin == string::npos) return "";   // 全是空白
    size_t end = s.find_last_not_of(" \t\r\n");
    return s.substr(begin, end - begin + 1);
}

// 读整数：输入不是数字就一直重新问，防止程序卡死
int readInt(const string& tip) {
    int n;
    while (true) {
        cout << tip;
        if (cin >> n) {
            cin.ignore(1024, '\n');   // 吃掉数字后面的回车
            return n;
        }
        if (cin.eof()) {              // 输入流结束（比如按了 Ctrl+Z）
            cout << "\n输入已结束，程序退出。" << endl;
            exit(0);
        }
        cin.clear();                  // 清除错误状态
        cin.ignore(1024, '\n');      // 丢掉错误的那一行
        cout << "输入无效，请输入一个整数：" << endl;
    }
}

// 读小数，逻辑和 readInt 一样
double readDouble(const string& tip) {
    double d;
    while (true) {
        cout << tip;
        if (cin >> d) {
            cin.ignore(1024, '\n');
            return d;
        }
        if (cin.eof()) {
            cout << "\n输入已结束，程序退出。" << endl;
            exit(0);
        }
        cin.clear();
        cin.ignore(1024, '\n');
        cout << "输入无效，请输入一个数字：" << endl;
    }
}

// 读一行字符串，自动 trim 掉首尾空格
string readLine(const string& tip) {
    cout << tip;
    string s;
    getline(cin, s);
    return trim(s);
}

// 完整转换 double：要求整段都能被解析，防止 "12abc" 这种被当成 12
bool strToDouble(const string& s, double& val) {
    string t = trim(s);
    if (t.empty()) return false;
    char* end = nullptr;
    val = strtod(t.c_str(), &end);
    return end != t.c_str() && *end == '\0';
}

bool strToInt(const string& s, int& val) {
    string t = trim(s);
    if (t.empty()) return false;
    char* end = nullptr;
    val = (int)strtol(t.c_str(), &end, 10);
    return end != t.c_str() && *end == '\0';
}


//统一字符