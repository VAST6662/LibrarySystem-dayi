#include "bookmanager.h"
#include "util.h"
#include <iostream>
#include <fstream>
#include <algorithm>
#include <iomanip>
#include <ctime>
#include <cstdio>
#include <cstdlib>
#include <cmath>
using namespace std;

const string BOOK_FILE = "books.txt";            // 图书数据文件
const string BORROW_FILE = "borrow_records.txt"; // 借阅记录文件

// ===================== 小工具 =====================

// 按分隔符把一行拆成几段
vector<string> splitLine(const string& line, char sep) {
    vector<string> parts;
    string cur;
    for (char c : line) {
        if (c == sep) {
            parts.push_back(cur);
            cur.clear();
        } else {
            cur += c;
        }
    }
    parts.push_back(cur);   // 最后一段
    return parts;
}

// 取今天日期，格式 yyyy-MM-dd
string getToday() {
    time_t now = time(nullptr);
    tm t;
    localtime_s(&t, &now);
    char buf[20];
    strftime(buf, sizeof(buf), "%Y-%m-%d", &t);
    return string(buf);
}

// 把 yyyy-MM-dd 转成自 1970-01-01 以来的天数（方便做日期加减）
// 用 mktime 转换，中午 12 点是为了避开夏令时导致差一天的问题
long long dateToDays(const string& date) {
    int y = 0, m = 0, d = 0;
    sscanf_s(date.c_str(), "%d-%d-%d", &y, &m, &d);
    tm t = {};
    t.tm_year = y - 1900;
    t.tm_mon = m - 1;
    t.tm_mday = d;
    t.tm_hour = 12;
    t.tm_isdst = -1;
    return (long long)(mktime(&t) / 86400);
}

// 日期加若干天
string addDays(const string& date, int days) {
    long long total = dateToDays(date) + days;
    time_t sec = (time_t)(total * 86400);
    tm t;
    localtime_s(&t, &sec);
    char buf[20];
    strftime(buf, sizeof(buf), "%Y-%m-%d", &t);
    return string(buf);
}

// 两个日期相差几天（later - earlier）
int daysBetween(const string& later, const string& earlier) {
    return (int)(dateToDays(later) - dateToDays(earlier));
}

// 判断一条借阅记录是否逾期：没还 + 今天 > 应还日期
bool isOverdue(const BorrowRecord& r, const string& today) {
    return r.returnDate == "未归还" && today > r.dueDate;
}

int findBookById(const vector<Book>& books, const string& id) {
    for (size_t i = 0; i < books.size(); ++i) {
        if (books[i].id == id) return (int)i;
    }
    return -1;
}

// 统计某本书一共被借过多少次（含已经还了的）
int countBorrowTimes(const vector<BorrowRecord>& records, const string& bookId) {
    int n = 0;
    for (const auto& r : records) {
        if (r.bookId == bookId) n++;
    }
    return n;
}

// ===================== 文件读写 =====================

vector<Book> loadBooks(const string& filename) {
    vector<Book> books;
    ifstream in(filename);
    if (!in.is_open()) {
        return books;   // 第一次运行没有文件，返回空表
    }
    string line;
    while (getline(in, line)) {
        if (line.empty()) continue;
        vector<string> p = splitLine(line, '|');
        if (p.size() < 8) {
            cout << "警告：跳过格式有问题的行：" << line << endl;
            continue;
        }
        Book b;
        b.id = trim(p[0]);
        b.name = trim(p[1]);
        b.author = trim(p[2]);
        b.publisher = trim(p[3]);
        b.isbn = trim(p[4]);
        if (!strToDouble(trim(p[5]), b.price)) b.price = 0.0;
        if (!strToInt(trim(p[6]), b.total)) b.total = 0;
        if (!strToInt(trim(p[7]), b.available)) b.available = 0;
        if (b.available < 0) b.available = 0;
        if (b.available > b.total) b.available = b.total;
        books.push_back(b);
    }
    in.close();
    return books;
}

void saveBooks(const string& filename, const vector<Book>& books) {
    ofstream out(filename);
    if (!out.is_open()) {
        cout << "错误：无法打开 " << filename << " 保存数据！" << endl;
        return;
    }
    for (const auto& b : books) {
        out << b.id << "|" << b.name << "|" << b.author << "|" << b.publisher << "|"
            << b.isbn << "|" << fixed << setprecision(2) << b.price << "|"
            << b.total << "|" << b.available << endl;
    }
    out.close();
}

// 借阅记录格式：编号|书名|借书人|借书日期|应还日期|还书日期|续借次数
vector<BorrowRecord> loadBorrowRecords(const string& filename) {
    vector<BorrowRecord> records;
    ifstream in(filename);
    if (!in.is_open()) return records;
    string line;
    while (getline(in, line)) {
        if (line.empty()) continue;
        vector<string> p = splitLine(line, '|');
        if (p.size() < 7) {
            cout << "警告：跳过格式有问题的借阅记录行：" << line << endl;
            continue;
        }
        BorrowRecord r;
        r.bookId = trim(p[0]);
        r.bookName = trim(p[1]);
        r.borrower = trim(p[2]);
        r.borrowDate = trim(p[3]);
        r.dueDate = trim(p[4]);
        r.returnDate = trim(p[5]);
        if (!strToInt(trim(p[6]), r.renewCount)) r.renewCount = 0;
        records.push_back(r);
    }
    in.close();
    return records;
}

void saveBorrowRecords(const string& filename, const vector<BorrowRecord>& records) {
    ofstream out(filename);
    if (!out.is_open()) {
        cout << "错误：无法打开 " << filename << " 保存借阅记录！" << endl;
        return;
    }
    for (const auto& r : records) {
        out << r.bookId << "|" << r.bookName << "|" << r.borrower << "|"
            << r.borrowDate << "|" << r.dueDate << "|" << r.returnDate << "|"
            << r.renewCount << endl;
    }
    out.close();
}

// ===================== 显示 =====================

void showBook(const Book& b) {
    cout << "编号：" << b.id << "  书名：《" << b.name << "》  作者：" << b.author
         << "  出版社：" << b.publisher << "  ISBN：" << b.isbn
         << "  价格：" << b.price << " 元  馆藏：" << b.total
         << " 本  可借：" << b.available << " 本" << endl;
}

void listAllBooks(const vector<Book>& books, const vector<BorrowRecord>& records) {
    if (books.empty()) {
        cout << "（目前还没有图书，请先添加）" << endl;
        return;
    }
    cout << "序号\t编号\t书名\t作者\t出版社\tISBN\t价格\t馆藏\t可借\t借出" << endl;
    for (size_t i = 0; i < books.size(); ++i) {
        const Book& b = books[i];
        cout << (i + 1) << "\t" << b.id << "\t" << b.name << "\t" << b.author
             << "\t" << b.publisher << "\t" << b.isbn << "\t" << b.price
             << "\t" << b.total << "\t" << b.available << "\t"
             << countBorrowTimes(records, b.id) << endl;
    }
}

// 显示一条借阅记录（带状态：已归还/借阅中/已逾期）
void showBorrowRecord(const BorrowRecord& r, const string& today) {
    string status;
    if (r.returnDate != "未归还") status = "已归还";
    else if (today > r.dueDate) status = "已逾期";
    else status = "借阅中";

    cout << r.bookId << "\t" << r.bookName << "\t" << r.borrower << "\t"
         << r.borrowDate << "\t" << r.dueDate << "\t" << r.returnDate << "\t"
         << r.renewCount << "\t" << status << endl;
}

void showAllBorrowRecords(const vector<BorrowRecord>& records) {
    cout << "\n----- 全部借阅记录 -----" << endl;
    if (records.empty()) {
        cout << "（暂无借阅记录）" << endl;
        return;
    }
    string today = getToday();
    cout << "编号\t书名\t借书人\t借书日期\t应还日期\t还书日期\t续借\t状态" << endl;
    for (const auto& r : records) {
        showBorrowRecord(r, today);
    }
}

// 按借书人查借阅记录
void showMyBorrowRecords(const vector<BorrowRecord>& records) {
    cout << "\n----- 按借书人查询 -----" << endl;
    if (records.empty()) {
        cout << "（暂无借阅记录）" << endl;
        return;
    }
    string borrower = readLine("请输入借书人姓名：");
    if (borrower.empty()) {
        cout << "姓名不能为空！" << endl;
        return;
    }
    string today = getToday();
    int count = 0;
    cout << "编号\t书名\t借书日期\t应还日期\t还书日期\t续借\t状态" << endl;
    for (const auto& r : records) {
        if (r.borrower == borrower) {
            showBorrowRecord(r, today);
            count++;
        }
    }
    if (count == 0) {
        cout << "没有找到 " << borrower << " 的借阅记录。" << endl;
    } else {
        cout << "共 " << count << " 条记录。" << endl;
    }
}

// ===================== 增删改查 =====================

void addBook(vector<Book>& books) {
    cout << "\n----- 添加图书 -----" << endl;
    Book b;
    while (true) {
        b.id = readLine("请输入图书编号：");
        if (b.id.empty()) {
            cout << "编号不能为空！" << endl;
            continue;
        }
        if (findBookById(books, b.id) >= 0) {
            cout << "该编号已存在，请换一个编号！" << endl;
            continue;
        }
        break;
    }
    b.name = readLine("请输入书名：");
    if (b.name.empty()) b.name = "未命名";
    b.author = readLine("请输入作者：");
    if (b.author.empty()) b.author = "未知";
    b.publisher = readLine("请输入出版社：");
    if (b.publisher.empty()) b.publisher = "未知";
    b.isbn = readLine("请输入 ISBN：");
    if (b.isbn.empty()) b.isbn = "无";

    while (true) {
        b.price = readDouble("请输入价格（元）：");
        if (b.price >= 0) break;
        cout << "价格不能是负数！" << endl;
    }
    while (true) {
        b.total = readInt("请输入馆藏数量：");
        if (b.total > 0) break;
        cout << "馆藏数量必须大于 0！" << endl;
    }
    b.available = b.total;   // 新书默认全部可借
    books.push_back(b);
    saveBooks(BOOK_FILE, books);
    cout << "添加成功！编号 " << b.id << " 的《" << b.name << "》已入库" << endl;
}

void deleteBook(vector<Book>& books) {
    cout << "\n----- 删除图书 -----" << endl;
    if (books.empty()) {
        cout << "（暂无图书可删）" << endl;
        return;
    }
    string id = readLine("请输入要删除的图书编号：");
    int pos = findBookById(books, id);
    if (pos < 0) {
        cout << "没有找到编号为 " << id << " 的图书！" << endl;
        return;
    }
    cout << "要删除的图书信息如下：" << endl;
    showBook(books[pos]);
    string ans = readLine("确认删除吗？（y/n）：");
    if (ans == "y" || ans == "Y" || ans == "yes" || ans == "YES") {
        books.erase(books.begin() + pos);
        saveBooks(BOOK_FILE, books);
        cout << "删除成功！" << endl;
    } else {
        cout << "已取消删除。" << endl;
    }
}

void modifyBook(vector<Book>& books) {
    cout << "\n----- 修改图书信息 -----" << endl;
    if (books.empty()) {
        cout << "（暂无图书可修改）" << endl;
        return;
    }
    string id = readLine("请输入要修改的图书编号：");
    int pos = findBookById(books, id);
    if (pos < 0) {
        cout << "没有找到编号为 " << id << " 的图书！" << endl;
        return;
    }
    cout << "原图书信息：" << endl;
    showBook(books[pos]);
    cout << "（直接回车表示该项保持不变）" << endl;

    Book& b = books[pos];
    string t;
    t = readLine("书名：");   if (!t.empty()) b.name = t;
    t = readLine("作者：");   if (!t.empty()) b.author = t;
    t = readLine("出版社："); if (!t.empty()) b.publisher = t;
    t = readLine("ISBN：");   if (!t.empty()) b.isbn = t;

    t = readLine("价格：");
    if (!t.empty()) {
        double d;
        if (strToDouble(t, d) && d >= 0) b.price = d;
        else cout << "价格输入无效，保持原值。" << endl;
    }
    t = readLine("馆藏数量：");
    if (!t.empty()) {
        int n;
        if (strToInt(t, n) && n >= 0) {
            int diff = n - b.total;   // 馆藏变化，可借数量跟着变
            b.total = n;
            b.available += diff;
            if (b.available < 0) b.available = 0;
            if (b.available > b.total) b.available = b.total;
        } else {
            cout << "数量输入无效，保持原值。" << endl;
        }
    }
    saveBooks(BOOK_FILE, books);
    cout << "修改成功！" << endl;
}

void searchBook(const vector<Book>& books) {
    cout << "\n----- 查找图书 -----" << endl;
    cout << "1. 按编号查找（精确）" << endl;
    cout << "2. 按书名查找（模糊）" << endl;
    cout << "3. 按作者查找（模糊）" << endl;
    cout << "4. 按出版社查找（模糊）" << endl;
    cout << "5. 按 ISBN 查找（模糊）" << endl;
    cout << "0. 返回" << endl;
    int ch = readInt("请选择：");
    if (ch == 0) return;
    if (ch < 1 || ch > 5) {
        cout << "无效选项！" << endl;
        return;
    }
    string key = readLine("请输入关键词：");
    if (key.empty()) {
        cout << "关键词不能为空！" << endl;
        return;
    }
    vector<Book> result;
    for (const auto& b : books) {
        if (ch == 1 && b.id == key) result.push_back(b);
        else if (ch == 2 && b.name.find(key) != string::npos) result.push_back(b);
        else if (ch == 3 && b.author.find(key) != string::npos) result.push_back(b);
        else if (ch == 4 && b.publisher.find(key) != string::npos) result.push_back(b);
        else if (ch == 5 && b.isbn.find(key) != string::npos) result.push_back(b);
    }
    if (result.empty()) {
        cout << "没有找到符合条件的图书。" << endl;
        return;
    }
    cout << "共找到 " << result.size() << " 本：" << endl;
    for (const auto& b : result) {
        showBook(b);
    }
}

void sortBooks(vector<Book>& books, const vector<BorrowRecord>& records) {
    cout << "\n----- 图书排序 -----" << endl;
    cout << "1. 按编号升序" << endl;
    cout << "2. 按价格从低到高" << endl;
    cout << "3. 按馆藏数量从多到少" << endl;
    cout << "4. 按借出次数从多到少" << endl;
    cout << "0. 返回" << endl;
    int ch = readInt("请选择：");
    if (ch == 1) {
        sort(books.begin(), books.end(), [](const Book& a, const Book& b) { return a.id < b.id; });
    } else if (ch == 2) {
        sort(books.begin(), books.end(), [](const Book& a, const Book& b) { return a.price < b.price; });
    } else if (ch == 3) {
        sort(books.begin(), books.end(), [](const Book& a, const Book& b) { return a.total > b.total; });
    } else if (ch == 4) {
        // 借出次数在 lambda 里用引用的 records 现算
        sort(books.begin(), books.end(), [&records](const Book& a, const Book& b) {
            return countBorrowTimes(records, a.id) > countBorrowTimes(records, b.id);
        });
    } else if (ch == 0) {
        return;
    } else {
        cout << "无效选项！" << endl;
        return;
    }
    saveBooks(BOOK_FILE, books);   // 排序结果也保存
    cout << "排序完成！" << endl;
    listAllBooks(books, records);
}

// ===================== 借书 / 还书 / 续借 =====================

void borrowBook(vector<Book>& books, vector<BorrowRecord>& records) {
    cout << "\n----- 借书 -----" << endl;
    if (books.empty()) {
        cout << "（暂无图书可借）" << endl;
        return;
    }
    string id = readLine("请输入图书编号：");
    int pos = findBookById(books, id);
    if (pos < 0) {
        cout << "没有找到编号为 " << id << " 的图书！" << endl;
        return;
    }
    if (books[pos].available <= 0) {
        cout << "《" << books[pos].name << "》已全部借出，暂时无法借阅。" << endl;
        return;
    }
    string borrower = readLine("请输入借书人姓名：");
    if (borrower.empty()) {
        cout << "借书人姓名不能为空！" << endl;
        return;
    }
    // 规则：有逾期未还的书时，不能再借
    string today = getToday();
    for (const auto& r : records) {
        if (r.borrower == borrower && isOverdue(r, today)) {
            cout << "借阅失败：" << borrower << " 有逾期未还的《" << r.bookName
                 << "》（应还日期 " << r.dueDate << "），请先归还再借。" << endl;
            return;
        }
    }
    books[pos].available--;   // 可借数量减一
    BorrowRecord r;
    r.bookId = books[pos].id;
    r.bookName = books[pos].name;
    r.borrower = borrower;
    r.borrowDate = today;
    r.dueDate = addDays(today, BORROW_DAYS);  // 借期 30 天
    r.returnDate = "未归还";
    r.renewCount = 0;
    records.push_back(r);
    saveBooks(BOOK_FILE, books);
    saveBorrowRecords(BORROW_FILE, records);
    cout << "借书成功！《" << r.bookName << "》已借给 " << borrower
         << "，应还日期 " << r.dueDate << "（借期 " << BORROW_DAYS << " 天）。" << endl;
}

void returnBook(vector<Book>& books, vector<BorrowRecord>& records) {
    cout << "\n----- 还书 -----" << endl;
    string id = readLine("请输入图书编号：");
    int pos = findBookById(books, id);
    if (pos < 0) {
        cout << "没有找到编号为 " << id << " 的图书！" << endl;
        return;
    }
    string borrower = readLine("请输入借书人姓名：");
    int found = -1;
    for (size_t i = 0; i < records.size(); ++i) {
        if (records[i].bookId == id && records[i].borrower == borrower
            && records[i].returnDate == "未归还") {
            found = (int)i;
            break;
        }
    }
    if (found < 0) {
        cout << "没有找到对应的未归还借阅记录，请确认编号和姓名是否正确。" << endl;
        return;
    }
    BorrowRecord& r = records[found];
    string today = getToday();
    if (today > r.dueDate) {
        int days = daysBetween(today, r.dueDate);
        double fine = days * FINE_PER_DAY;
        cout << "注意：《" << r.bookName << "》已逾期 " << days << " 天，"
             << "需缴纳罚款 " << fine << " 元（示意金额）。" << endl;
    }
    r.returnDate = today;
    books[pos].available++;
    if (books[pos].available > books[pos].total) books[pos].available = books[pos].total;
    saveBooks(BOOK_FILE, books);
    saveBorrowRecords(BORROW_FILE, records);
    cout << "还书成功！《" << r.bookName << "》已归还（" << today << "）。" << endl;
}

void renewBook(vector<Book>& books, vector<BorrowRecord>& records) {
    cout << "\n----- 续借（延长借书） -----" << endl;
    if (records.empty()) {
        cout << "（暂无借阅记录）" << endl;
        return;
    }
    string id = readLine("请输入图书编号：");
    int pos = findBookById(books, id);
    if (pos < 0) {
        cout << "没有找到编号为 " << id << " 的图书！" << endl;
        return;
    }
    string borrower = readLine("请输入借书人姓名：");
    int found = -1;
    for (size_t i = 0; i < records.size(); ++i) {
        if (records[i].bookId == id && records[i].borrower == borrower
            && records[i].returnDate == "未归还") {
            found = (int)i;
            break;
        }
    }
    if (found < 0) {
        cout << "没有找到对应的未归还借阅记录，请确认编号和姓名是否正确。" << endl;
        return;
    }
    BorrowRecord& r = records[found];
    string today = getToday();

    // 规则一：逾期不能续借
    if (isOverdue(r, today)) {
        cout << "该书已逾期（应还日期 " << r.dueDate << "），逾期图书不能续借，请尽快归还。" << endl;
        return;
    }
    // 规则二：续借次数上限
    if (r.renewCount >= MAX_RENEW) {
        cout << "该书已续借 " << r.renewCount << " 次，达到上限（" << MAX_RENEW
             << " 次），不能再续借了。" << endl;
        return;
    }
    r.dueDate = addDays(r.dueDate, BORROW_DAYS);  // 应还日期往后推 30 天
    r.renewCount++;
    saveBooks(BOOK_FILE, books);
    saveBorrowRecords(BORROW_FILE, records);
    cout << "续借成功！《" << r.bookName << "》新的应还日期是 " << r.dueDate
         << "（已续借 " << r.renewCount << "/" << MAX_RENEW << " 次）。" << endl;
}

// ===================== 统计 / 排行 / 预警 / 帮助 =====================

void showStatistics(const vector<Book>& books, const vector<BorrowRecord>& records) {
    cout << "\n----- 统计信息 -----" << endl;
    if (books.empty()) {
        cout << "（暂无图书）" << endl;
        return;
    }
    string today = getToday();
    int totalKinds = (int)books.size();
    int totalCopies = 0;
    int availableCopies = 0;
    double totalPrice = 0.0;
    int notReturned = 0;
    int overdueCount = 0;
    for (const auto& b : books) {
        totalCopies += b.total;
        availableCopies += b.available;
        totalPrice += b.price;
    }
    for (const auto& r : records) {
        if (r.returnDate == "未归还") notReturned++;
        if (isOverdue(r, today)) overdueCount++;
    }
    cout << "图书种类：" << totalKinds << " 种" << endl;
    cout << "馆藏总量：" << totalCopies << " 本" << endl;
    cout << "可借数量：" << availableCopies << " 本" << endl;
    cout << "已借出：" << (totalCopies - availableCopies) << " 本" << endl;
    cout << "借阅记录总数：" << records.size() << " 条" << endl;
    cout << "未归还记录：" << notReturned << " 条" << endl;
    cout << "逾期未还：" << overdueCount << " 条" << endl;
    double avgPrice = totalPrice / totalKinds;
    avgPrice = floor(avgPrice * 100.0 + 0.5) / 100.0;   // 四舍五入保留两位小数
    cout << "平均单价：" << avgPrice << " 元" << endl;
}

// 热门图书排行：按借阅次数从多到少
void showHotBooks(const vector<Book>& books, const vector<BorrowRecord>& records) {
    cout << "\n----- 热门图书排行 -----" << endl;
    if (records.empty()) {
        cout << "（还没有借阅记录，暂时无法排行）" << endl;
        return;
    }
    // 先把每本书的借阅次数统计出来（记录里书被删了也能统计到）
    struct HotItem {
        string id;
        string name;
        int count;
    };
    vector<HotItem> hot;
    for (const auto& r : records) {
        bool found = false;
        for (auto& h : hot) {
            if (h.id == r.bookId) {
                h.count++;
                found = true;
                break;
            }
        }
        if (!found) {
            HotItem item;
            item.id = r.bookId;
            item.name = r.bookName;
            item.count = 1;
            hot.push_back(item);
        }
    }
    sort(hot.begin(), hot.end(), [](const HotItem& a, const HotItem& b) {
        return a.count > b.count;
    });
    int showCount = (int)hot.size() > 5 ? 5 : (int)hot.size();  // 只显示前 5
    for (int i = 0; i < showCount; ++i) {
        cout << "第 " << (i + 1) << " 名：《" << hot[i].name << "》（" << hot[i].id
             << "）被借 " << hot[i].count << " 次" << endl;
    }
}

// 库存预警：馆藏数量太少要补货
void showStockAlert(const vector<Book>& books) {
    cout << "\n----- 库存预警 -----" << endl;
    if (books.empty()) {
        cout << "（暂无图书）" << endl;
        return;
    }
    int alertCount = 0;
    for (const auto& b : books) {
        if (b.total <= LOW_STOCK) {
            cout << "《" << b.name << "》（" << b.id << "）馆藏仅剩 " << b.total
                 << " 本（可借 " << b.available << " 本），建议补货！" << endl;
            alertCount++;
        }
    }
    if (alertCount == 0) {
        cout << "库存充足，没有需要预警的图书。" << endl;
    } else {
        cout << "共 " << alertCount << " 本需要补货。" << endl;
    }
}

void showHelp() {
    cout << "\n========== 帮助说明 ==========" << endl;
    cout << "1. 借书规则：借期 " << BORROW_DAYS << " 天，到期前可续借，最多续借 "
         << MAX_RENEW << " 次（每次延长 " << BORROW_DAYS << " 天）。" << endl;
    cout << "2. 逾期规则：逾期未还的书不能续借，也不能再借新书；" << endl;
    cout << "   还书时会提示逾期天数和罚款金额（每天 " << FINE_PER_DAY << " 元，示意）。" << endl;
    cout << "3. 数据文件：books.txt 存图书，borrow_records.txt 存借阅记录，" << endl;
    cout << "   程序启动自动加载，每次操作自动保存。" << endl;
    cout << "4. 每个功能操作完都会自动保存，放心使用；" << endl;
    cout << "   想清空数据，直接删除 books.txt 和 borrow_records.txt 即可。" << endl;
    cout << "5. 遇到输入错误不用担心，程序会提示重新输入，不会崩溃。" << endl;
    cout << "=================================" << endl;
}

// ===================== 主菜单 =====================

void bookManageMenu() {
    vector<Book> books = loadBooks(BOOK_FILE);
    vector<BorrowRecord> records = loadBorrowRecords(BORROW_FILE);
    if (books.empty()) {
        cout << "（数据文件 " << BOOK_FILE << " 不存在或为空，本次从空库开始）" << endl;
    } else {
        cout << "已加载 " << books.size() << " 本图书、" << records.size() << " 条借阅记录。" << endl;
    }

    int choice;
    do {
        cout << "\n========== 图书管理系统 ==========" << endl;
        cout << "1. 添加图书" << endl;
        cout << "2. 删除图书" << endl;
        cout << "3. 修改图书信息" << endl;
        cout << "4. 查找图书" << endl;
        cout << "5. 显示全部图书" << endl;
        cout << "6. 图书排序" << endl;
        cout << "7. 借书" << endl;
        cout << "8. 还书" << endl;
        cout << "9. 续借（延长借书）" << endl;
        cout << "10. 全部借阅记录" << endl;
        cout << "11. 按借书人查询" << endl;
        cout << "12. 热门图书排行" << endl;
        cout << "13. 库存预警" << endl;
        cout << "14. 统计信息" << endl;
        cout << "15. 帮助说明" << endl;
        cout << "0. 保存并退出" << endl;
        choice = readInt("请选择：");
        switch (choice) {
        case 1: addBook(books); break;
        case 2: deleteBook(books); break;
        case 3: modifyBook(books); break;
        case 4: searchBook(books); break;
        case 5: listAllBooks(books, records); break;
        case 6: sortBooks(books, records); break;
        case 7: borrowBook(books, records); break;
        case 8: returnBook(books, records); break;
        case 9: renewBook(books, records); break;
        case 10: showAllBorrowRecords(records); break;
        case 11: showMyBorrowRecords(records); break;
        case 12: showHotBooks(books, records); break;
        case 13: showStockAlert(books); break;
        case 14: showStatistics(books, records); break;
        case 15: showHelp(); break;
        case 0:
            saveBooks(BOOK_FILE, books);
            saveBorrowRecords(BORROW_FILE, records);
            cout << "数据已保存，感谢使用！" << endl;
            break;
        default:
            cout << "无效选项，请重新输入" << endl;
        }
    } while (choice != 0);
}
