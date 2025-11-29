#define _CRT_SECURE_NO_WARNINGS
#include <iostream>
#include <vector>
#include <string>
#include <memory>
#include <fstream>
#include <conio.h>
#include <windows.h>
#include <ctime>
#include <sstream>
#include <algorithm>
#include <map>
#include <iomanip>
#include <filesystem>

using namespace std;
namespace fs = std::filesystem;

bool parseDate(const string& s, time_t& outT) {
    struct tm tm {};
    if (s.size() < 10) return false;
    int y, m, d;
    char dash1, dash2;
    stringstream ss(s);
    ss >> y >> dash1 >> m >> dash2 >> d;
    if (!ss || dash1 != '-' || dash2 != '-') return false;
    tm.tm_year = y - 1900;
    tm.tm_mon = m - 1;
    tm.tm_mday = d;
    tm.tm_hour = 0; tm.tm_min = 0; tm.tm_sec = 0;
    tm.tm_isdst = -1;
    outT = mktime(&tm);
    return (outT != -1);
}

string formatDate(time_t t) {
    struct tm* tm = localtime(&t);
    char buf[16];
    strftime(buf, sizeof(buf), "%Y-%m-%d", tm);
    return string(buf);
}

time_t startOfWeek(time_t t) {
    struct tm tm = *localtime(&t);
    int wday = tm.tm_wday; 
    int offset = (wday + 6) % 7; 
    time_t start = t - offset * 24 * 3600;
    struct tm st = *localtime(&start);
    st.tm_hour = 0; st.tm_min = 0; st.tm_sec = 0;
    return mktime(&st);
}

class Transaction {
public:
    double amount = 0.0;
    bool isExpense = true;
    string category;
    string note;
    time_t date = 0;

    Transaction() {}
    Transaction(double a, bool exp, const string& cat, const string& n, time_t d)
        : amount(a), isExpense(exp), category(cat), note(n), date(d) {
    }

    friend ostream& operator<<(ostream& out, const Transaction& t) {
        out << formatDate(t.date) << "\n";
        out << t.amount << "\n";
        out << (t.isExpense ? 1 : 0) << "\n";
        out << t.category << "\n";
        out << t.note << "\n";
        return out;
    }

    friend istream& operator>>(istream& in, Transaction& t) {
        string dates;
        if (!getline(in, dates)) return in;
        time_t dt;
        if (!parseDate(dates, dt)) dt = 0;
        t.date = dt;

        string s;
        if (!getline(in, s)) return in;
        try { t.amount = stod(s); }
        catch (...) { t.amount = 0.0; }

        if (!getline(in, s)) return in;
        try { t.isExpense = (stoi(s) != 0); }
        catch (...) { t.isExpense = true; }

        if (!getline(in, t.category)) return in;
        if (!getline(in, t.note)) return in;
        return in;
    }
};

class Wallet {
private:
    string name;
    vector<Transaction> transactions;
public:
    Wallet() : name("Unnamed") {}
    Wallet(const string& n) : name(n) {}
    string getName() const { return name; }

    void addTransaction(const Transaction& t) { transactions.push_back(t); }
    const vector<Transaction>& getTransactions() const { return transactions; }

    double getBalance() const {
        double bal = 0.0;
        for (const auto& t : transactions) {
            if (t.isExpense) bal -= t.amount;
            else bal += t.amount;
        }
        return bal;
    }

    bool saveToFile(const string& filename) const {
        ofstream out(filename);
        if (!out.is_open()) return false;
        out << name << "\n";
        out << transactions.size() << "\n";
        for (const auto& t : transactions) out << t;
        out.close();
        return true;
    }

    bool loadFromFile(const string& filename) {
        if (!fs::exists(filename)) return false;
        ifstream in(filename);
        if (!in.is_open()) return false;
        string nm;
        getline(in, nm);
        if (!nm.empty()) name = nm;
        string s;
        if (!getline(in, s)) return false;
        int cnt = 0;
        try { cnt = stoi(s); }
        catch (...) { cnt = 0; }
        transactions.clear();
        for (int i = 0; i < cnt; ++i) {
            Transaction tr;
            in >> tr;
            transactions.push_back(tr);
        }
        in.close();
        return true;
    }

    void clear() { transactions.clear(); }
};

class FinanceSystem {
private:
    Wallet debit;
    Wallet credit;
    vector<string> categories;
public:
    FinanceSystem() : debit("Debit"), credit("Credit") {
        categories = { "Food", "Transport", "Utilities", "Internet", "Shopping", "Restaurants", "Other" };
    }

    Wallet& getDebit() { return debit; }
    Wallet& getCredit() { return credit; }
    const vector<string>& getCategories() const { return categories; }

    void addCategory(const string& c) {
        if (c.empty()) return;
        if (find(categories.begin(), categories.end(), c) == categories.end())
            categories.push_back(c);
    }

    bool addTransactionToWallet(const string& walletName, const Transaction& t) {
        if (walletName == "Debit") debit.addTransaction(t);
        else credit.addTransaction(t);
        return true;
    }

    bool loadAll(const string& debitFile, const string& creditFile, const string& catFile) {
        debit.loadFromFile(debitFile);
        credit.loadFromFile(creditFile);
        if (fs::exists(catFile)) {
            ifstream in(catFile);
            if (in.is_open()) {
                categories.clear();
                string line;
                while (getline(in, line)) { if (!line.empty()) categories.push_back(line); }
            }
        }
        return true;
    }

    bool saveAll(const string& debitFile, const string& creditFile, const string& catFile) {
        debit.saveToFile(debitFile);
        credit.saveToFile(creditFile);
        ofstream out(catFile);
        if (!out.is_open()) return false;
        for (auto& c : categories) out << c << "\n";
        return true;
    }

    double sumTransactionsInRange(const Wallet& w, time_t start, time_t end, bool expensesOnly = true) {
        double sum = 0.0;
        for (const auto& t : w.getTransactions()) {
            if (t.date < start || t.date > end) continue;
            if (expensesOnly && !t.isExpense) continue;
            if (!expensesOnly && t.isExpense) continue;
            sum += t.amount;
        }
        return sum;
    }

    vector<pair<double, string>> topTransactionsInRange(time_t start, time_t end, int N = 3) {
        vector<pair<double, string>> all;
        auto collect = [&](const Wallet& w, const string& wname) {
            for (const auto& t : w.getTransactions()) {
                if (t.date < start || t.date > end) continue;
                if (!t.isExpense) continue;
                string desc = wname + ": " + t.category + " - " + t.note + " (" + formatDate(t.date) + ")";
                all.push_back({ t.amount, desc });
            }
            };
        collect(debit, "Debit"); collect(credit, "Credit");
        sort(all.begin(), all.end(), [](auto& a, auto& b) { return a.first > b.first; });
        if ((int)all.size() > N) all.resize(N);
        return all;
    }

    vector<pair<string, double>> topCategoriesInRange(time_t start, time_t end, int N = 3) {
        map<string, double> sums;
        auto collect = [&](const Wallet& w) {
            for (const auto& t : w.getTransactions()) {
                if (t.date < start || t.date > end) continue;
                if (!t.isExpense) continue;
                sums[t.category] += t.amount;
            }
            };
        collect(debit); collect(credit);
        vector<pair<string, double>> vec;
        for (auto& p : sums) vec.push_back(p);
        sort(vec.begin(), vec.end(), [](auto& a, auto& b) { return a.second > b.second; });
        if ((int)vec.size() > N) vec.resize(N);
        return vec;
    }

    vector<Transaction> getTransactionsInRange(const Wallet& w, time_t start, time_t end) {
        vector<Transaction> res;
        for (const auto& t : w.getTransactions()) if (t.date >= start && t.date <= end) res.push_back(t);
        return res;
    }
};

int runArrowMenu(const vector<string>& items, const string& title = "") {
    int selected = 0;
    while (true) {
        system("cls");
        if (!title.empty()) cout << title << "\n\n";
        for (int i = 0; i < (int)items.size(); ++i) {
            if (i == selected) cout << " > " << items[i] << "\n";
            else cout << "   " << items[i] << "\n";
        }
        int c = _getch();
        if (c == 224) {
            int d = _getch();
            if (d == 80 && selected < (int)items.size() - 1) selected++;
            else if (d == 72 && selected > 0) selected--;
        }
        else if (c == 13) {
            return selected;
        }
    }
}

void pausePress() { cout << "\nPress any key..."; _getch(); }

string chooseWalletInteractive() {
    vector<string> opts = { "Debit", "Credit", "Cancel" };
    int sel = runArrowMenu(opts, "Choose wallet:");
    if (sel == 2) return "";
    return opts[sel];
}

string chooseCategoryInteractive(FinanceSystem& fsys) {
    while (true) {
        system("cls");
        cout << "Categories:\n";
        const auto& cats = fsys.getCategories();
        for (size_t i = 0; i < cats.size(); ++i) cout << "  " << (i + 1) << ") " << cats[i] << "\n";
        cout << "\nEnter number to choose, 'a' to add new category, or 0 to cancel: ";
        string line; getline(cin, line);
        if (line.empty()) getline(cin, line);
        if (line == "0") return "";
        if (line == "a" || line == "A") {
            cout << "Enter new category name: ";
            string newc; getline(cin, newc);
            if (!newc.empty()) fsys.addCategory(newc);
            continue;
        }
        try {
            int idx = stoi(line);
            if (idx >= 1 && idx <= (int)cats.size()) return cats[idx - 1];
        }
        catch (...) {}
        cout << "Invalid input. Press key to try again."; _getch();
    }
}

bool readAmount(double& amount) {
    cout << "Enter amount (positive number): ";
    string s; getline(cin, s);
    if (s.empty()) getline(cin, s);
    try { amount = stod(s); if (amount <= 0) return false; return true; }
    catch (...) { return false; }
}

time_t readDateInteractive() {
    while (true) {
        cout << "Enter date (YYYY-MM-DD): ";
        string ds; getline(cin, ds);
        if (ds.empty()) getline(cin, ds);
        time_t t;
        if (parseDate(ds, t)) return t;
        cout << "Invalid date format. Try again.\n";
    }
}

void addExpenseInteractive(FinanceSystem& fsys) {
    system("cls"); cout << "Add Expense\n";
    string walletName = chooseWalletInteractive(); if (walletName.empty()) return;
    double amount; if (!readAmount(amount)) { cout << "Invalid amount.\n"; pausePress(); return; }
    time_t date = readDateInteractive();
    string category = chooseCategoryInteractive(fsys); if (category.empty()) category = "Other";
    cout << "Optional note (enter to skip): ";
    string note; getline(cin, note);

    system("cls");
    cout << "Please confirm transaction:\n";
    cout << " Wallet: " << walletName << "\n";
    cout << " Amount: -" << fixed << setprecision(2) << amount << "\n";
    cout << " Date  : " << formatDate(date) << "\n";
    cout << " Category: " << category << "\n";
    cout << " Note: " << note << "\n";
    cout << "\nConfirm? (y/n): ";
    char c = _getch();
    cout << c << "\n";
    if (c != 'y' && c != 'Y') { cout << "Cancelled.\n"; pausePress(); return; }

    Transaction tr(amount, true, category, note, date);
    fsys.addTransactionToWallet(walletName, tr);
    cout << "Expense added.\n"; pausePress();
}

void addTopUpInteractive(FinanceSystem& fsys) {
    system("cls"); cout << "Top-up (income)\n";
    string walletName = chooseWalletInteractive(); if (walletName.empty()) return;
    double amount; if (!readAmount(amount)) { cout << "Invalid amount.\n"; pausePress(); return; }
    time_t date = readDateInteractive();
    cout << "Optional note: "; string note; getline(cin, note);

    system("cls");
    cout << "Please confirm transaction:\n";
    cout << " Wallet: " << walletName << "\n";
    cout << " Amount: +" << fixed << setprecision(2) << amount << "\n";
    cout << " Date  : " << formatDate(date) << "\n";
    cout << " Note  : " << note << "\n";
    cout << "\nConfirm? (y/n): ";
    char c = _getch(); cout << c << "\n";
    if (c != 'y' && c != 'Y') { cout << "Cancelled.\n"; pausePress(); return; }

    Transaction tr(amount, false, "Income", note, date);
    fsys.addTransactionToWallet(walletName, tr);
    cout << "Top-up added.\n"; pausePress();
}

void showBalances(FinanceSystem& fsys) {
    system("cls");
    cout << "Wallet balances:\n";
    cout << "  Debit : " << fixed << setprecision(2) << fsys.getDebit().getBalance() << "\n";
    cout << "  Credit: " << fixed << setprecision(2) << fsys.getCredit().getBalance() << "\n";
    pausePress();
}

void listTransactionsWallet(const Wallet& w) {
    system("cls");
    cout << "Transactions for wallet: " << w.getName() << "\n";
    const auto& tx = w.getTransactions();
    for (size_t i = 0; i < tx.size(); ++i) {
        cout << i + 1 << ") [" << formatDate(tx[i].date) << "] ";
        cout << (tx[i].isExpense ? "-" : "+") << fixed << setprecision(2) << tx[i].amount;
        cout << " | " << tx[i].category << " | " << tx[i].note << "\n";
    }
    pausePress();
}

void manageCategoriesInteractive(FinanceSystem& fsys) {
    while (true) {
        system("cls");
        cout << "Categories:\n";
        const auto& cats = fsys.getCategories();
        for (size_t i = 0; i < cats.size(); ++i) cout << "  " << (i + 1) << ") " << cats[i] << "\n";
        cout << "\nOptions: (a)dd new, (b)ack\nChoose: ";
        string s; getline(cin, s);
        if (s == "a" || s == "A") {
            cout << "Enter new category name: ";
            string c; getline(cin, c);
            if (!c.empty()) { fsys.addCategory(c); cout << "Added.\n"; pausePress(); }
        }
        else break;
    }
}

void generateAndSaveReport(FinanceSystem& fsys, const string& outFilename) {
    system("cls"); cout << "Generate report\n";
    time_t ref = readDateInteractive();

    struct tm tmref = *localtime(&ref);
    tmref.tm_hour = 0; tmref.tm_min = 0; tmref.tm_sec = 0;
    time_t dayStart = mktime(&tmref);
    time_t dayEnd = dayStart + 24 * 3600 - 1;

    time_t weekStart = startOfWeek(ref);
    time_t weekEnd = weekStart + 7 * 24 * 3600 - 1;

    struct tm tmm = *localtime(&ref);
    tmm.tm_mday = 1; tmm.tm_hour = 0; tmm.tm_min = 0; tmm.tm_sec = 0;
    time_t monthStart = mktime(&tmm);
    struct tm tnext = tmm; tnext.tm_mon += 1;
    time_t monthEnd = mktime(&tnext) - 1;

    double dayExpenses = fsys.sumTransactionsInRange(fsys.getDebit(), dayStart, dayEnd) + fsys.sumTransactionsInRange(fsys.getCredit(), dayStart, dayEnd);
    double weekExpenses = fsys.sumTransactionsInRange(fsys.getDebit(), weekStart, weekEnd) + fsys.sumTransactionsInRange(fsys.getCredit(), weekStart, weekEnd);
    double monthExpenses = fsys.sumTransactionsInRange(fsys.getDebit(), monthStart, monthEnd) + fsys.sumTransactionsInRange(fsys.getCredit(), monthStart, monthEnd);

    auto topWeek = fsys.topTransactionsInRange(weekStart, weekEnd, 3);
    auto topMonth = fsys.topTransactionsInRange(monthStart, monthEnd, 3);
    auto topCatWeek = fsys.topCategoriesInRange(weekStart, weekEnd, 3);
    auto topCatMonth = fsys.topCategoriesInRange(monthStart, monthEnd, 3);

    ofstream out(outFilename, ios::app);
    if (!out.is_open()) cout << "Warning: could not open report file for writing.\n";

    auto writeLine = [&](const string& line) {
        cout << line << "\n";
        if (out.is_open()) out << line << "\n";
        };

    writeLine("===== Report for reference date: " + formatDate(ref) + " =====");
    writeLine("Day expenses: " + to_string(dayExpenses));
    writeLine("Week expenses: " + to_string(weekExpenses));
    writeLine("Month expenses: " + to_string(monthExpenses));
    writeLine("");
    writeLine("Top 3 transactions this week:");
    for (auto& p : topWeek) writeLine("  " + to_string(p.first) + " - " + p.second);
    writeLine("");
    writeLine("Top 3 transactions this month:");
    for (auto& p : topMonth) writeLine("  " + to_string(p.first) + " - " + p.second);
    writeLine("");
    writeLine("Top 3 categories this week:");
    for (auto& p : topCatWeek) writeLine("  " + p.first + " - " + to_string(p.second));
    writeLine("");
    writeLine("Top 3 categories this month:");
    for (auto& p : topCatMonth) writeLine("  " + p.first + " - " + to_string(p.second));
    writeLine("===== End of report =====");
    if (out.is_open()) out << "\n";
    if (out.is_open()) out.close();

    cout << "\nReport generated and appended to " << outFilename << "\n";
    pausePress();
}

bool loadDebit(const string& filename, FinanceSystem& fsys) {
    return fsys.getDebit().loadFromFile(filename);
}
bool saveDebit(const string& filename, FinanceSystem& fsys) {
    return fsys.getDebit().saveToFile(filename);
}
bool loadCredit(const string& filename, FinanceSystem& fsys) {
    return fsys.getCredit().loadFromFile(filename);
}
bool saveCredit(const string& filename, FinanceSystem& fsys) {
    return fsys.getCredit().saveToFile(filename);
}

int main() {
#ifdef _WIN32
    SetConsoleOutputCP(65001);
    SetConsoleCP(65001);
#endif

    FinanceSystem fsys;
    const string debitFile = "debit.txt";
    const string creditFile = "credit.txt";
    const string catFile = "categories.txt";
    const string reportFile = "reports.txt";

    fsys.loadAll(debitFile, creditFile, catFile);

    while (true) {
        vector<string> mainMenu = {
            "Show balances",
            "Add expense",
            "Top-up (income)",
            "View transactions (Debit)",
            "View transactions (Credit)",
            "Reports (generate & save)",
            "Manage categories",
            "Save all",
            "Exit"
        };
        int choice = runArrowMenu(mainMenu, "=== Personal Finance Manager ===");
        if (choice == 0) showBalances(fsys);
        else if (choice == 1) addExpenseInteractive(fsys);
        else if (choice == 2) addTopUpInteractive(fsys);
        else if (choice == 3) listTransactionsWallet(fsys.getDebit());
        else if (choice == 4) listTransactionsWallet(fsys.getCredit());
        else if (choice == 5) generateAndSaveReport(fsys, reportFile);
        else if (choice == 6) manageCategoriesInteractive(fsys);
        else if (choice == 7) {
            if (fsys.saveAll(debitFile, creditFile, catFile)) cout << "Saved successfully.\n";
            else cout << "Save failed.\n";
            pausePress();
        }
        else {
            cout << "Exit selected. Save data before exit? (y/n): ";
            char c = _getch();
            cout << c << "\n";
            if (c == 'y' || c == 'Y') {
                fsys.saveAll(debitFile, creditFile, catFile);
                cout << "Saved.\n";
            }
            cout << "Bye.\n";
            break;
        }
    }

    return 0;
}
