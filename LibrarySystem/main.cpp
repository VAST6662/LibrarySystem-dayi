// ============================================================
// 图书管理系统
// 作者：XXX       学号：XXXXXXXX
// 课程：数据结构课程设计
// 说明：控制台程序，Visual Studio 2022 直接打开 .sln 编译运行
// ============================================================
#include <iostream>
#include <windows.h>
#include "util.h"
#include "bookmanager.h"
using namespace std;

int main() {
    // 把控制台的输入/输出代码页都切成 UTF-8，避免中文乱码
    SetConsoleOutputCP(65001);
    SetConsoleCP(65001);

    cout << "===== 欢迎使用图书管理系统 =====" << endl;
    cout << "（输入菜单对应的数字，按回车确认；选 0 保存并退出）" << endl;

    bookManageMenu();   // 所有功能都在这个菜单里

    return 0;
}
