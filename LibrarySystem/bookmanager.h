#ifndef BOOKMANAGER_H
#define BOOKMANAGER_H

#include <string>
#include <vector>
using namespace std;

// ---------- 图书信息 ----------
struct Book {
    string id;         // 图书编号
    string name;       // 书名
    string author;     // 作者
    string publisher;  // 出版社
    string isbn;       // ISBN
    double price;      // 单价
    int total;         // 馆藏总量
    int available;     // 可借数量（借出去会减少）
};

// ---------- 借阅记录 ----------
struct BorrowRecord {
    string bookId;     // 图书编号
    string bookName;   // 书名（方便查看）
    string borrower;   // 借书人
    string borrowDate; // 借书日期
    string dueDate;    // 应还日期（借书日 + 30 天，续借会往后推）
    string returnDate; // 还书日期，"未归还"表示还没还
    int renewCount;    // 续借次数
};

// ---------- 借阅规则常量（可以自己改） ----------
const int BORROW_DAYS = 30;      // 借期 30 天
const int MAX_RENEW = 2;         // 最多续借 2 次
const int LOW_STOCK = 2;         // 馆藏数量 <= 2 本时预警
const double FINE_PER_DAY = 0.10; // 逾期每天罚款 0.1 元（示意）

// ---------- 文件读写 ----------
vector<Book> loadBooks(const string& filename);
void saveBooks(const string& filename, const vector<Book>& books);
vector<BorrowRecord> loadBorrowRecords(const string& filename);
void saveBorrowRecords(const string& filename, const vector<BorrowRecord>& records);

// ---------- 日期工具 ----------
string getToday();                          // 今天日期 yyyy-MM-dd
string addDays(const string& date, int days); // 日期加若干天
bool isOverdue(const BorrowRecord& r, const string& today); // 是否逾期
int daysBetween(const string& later, const string& earlier); // 两个日期相差几天

// ---------- 其他工具 ----------
int findBookById(const vector<Book>& books, const string& id);
int countBorrowTimes(const vector<BorrowRecord>& records, const string& bookId);

// ---------- 图书管理主菜单 ----------
void bookManageMenu();

#endif
