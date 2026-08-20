// ============================================================
// 图书管理系统
#include <iostream>
#include <windows.h>
#include "util.h"
#include "bookmanager.h"
using namespace std;

int main() {
    // 控制台的输入输出代码页切成 UTF-8
    SetConsoleOutputCP(65001);
    SetConsoleCP(65001);

    cout << "===== 欢迎使用图书管理系统 =====" << endl;
    cout << "（输入菜单对应的数字，按回车确认；选 0 保存并退出）" << endl;

    bookManageMenu();   // 所有功能

    return 0;
}
