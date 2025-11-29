#define _CRT_SECURE_NO_WARNINGS
#include "interactive.h"
#include "utils.h"
#include "menu.h"
#include <iostream>
#include <sstream>
#include <iomanip>
#include <fstream>
#include <conio.h>

using namespace std;

void pausePress(); 
// меню вибору гаманц€
string chooseWalletInteractive() {
    vector<string> opts = { "Debit", "Credit", "Cancel" };
    int sel = runArrowMenu(opts, "Choose wallet:");
    if (sel == 2) return "";
    return opts[sel];
}
// меню вибору категор≥њ стр≥лочками 
string chooseCategoryInteractive(FinanceSystem& fsys) {
    while (true) {
        const auto& cats = fsys.getCategories();
        vector<string> menu;
        for (const auto& c : cats) menu.push_back(c);
        menu.push_back("Add new category");
        menu.push_back("Back");

        int sel = runArrowMenu(menu, "Choose category:");
        if (sel == (int)menu.size() - 1) {
            return "";
        }
        if (sel == (int)menu.size() - 2) {
            system("cls");
            cout << "Enter new category name: ";
            string newc;
            getline(cin, newc);
            if (newc.empty()) getline(cin, newc);
            if (!newc.empty()) fsys.addCategory(newc);
            continue;
        }
        return menu[sel];
    }
}
// введенн€ суми транзакц≥њ
bool readAmount(double& amount) {
    cout << "Enter amount (positive number): ";
    string s; getline(cin, s);
    if (s.empty()) getline(cin, s);
    try { amount = stod(s); if (amount <= 0) return false; return true; }
    catch (...) { return false; }
}
// введенн€ дати у формат≥ YYYY-MM-DD
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
// коли додаЇмо витрату, виб≥р гаманц€, сума, дата, категор≥€, нотатки
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
// коли додаЇмо дох≥д, виб≥р гаманц€, сума, дата, нотатки
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
// виводить баланс карток
void showBalances(FinanceSystem& fsys) {
    system("cls");
    cout << "Wallet balances:\n";
    cout << "  Debit : " << fixed << setprecision(2) << fsys.getDebit().getBalance() << "\n";
    cout << "  Credit: " << fixed << setprecision(2) << fsys.getCredit().getBalance() << "\n";
    pausePress();
}
// виводить вс≥ транзакц≥њ конкретного гаманц€ 
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
// перегл€д та додаванн€ категор≥й
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
// генеруЇ зв≥т на день, тиждень, м≥с€ць (загальн≥ витрати, топ 3 транзакц≥њ, топ 3 категор≥њ
void generateAndSaveReport(FinanceSystem& fsys, const string& outFilename) {
    system("cls"); cout << "Generate report\n";
    time_t ref = readDateInteractive();

    struct tm tmref;
#ifdef _WIN32
    localtime_s(&tmref, &ref);
#else
    tmref = *localtime(&ref);
#endif
    tmref.tm_hour = 0; tmref.tm_min = 0; tmref.tm_sec = 0;
    time_t dayStart = mktime(&tmref);
    time_t dayEnd = dayStart + 24 * 3600 - 1;

    time_t weekStart = startOfWeek(ref);
    time_t weekEnd = weekStart + 7 * 24 * 3600 - 1;

#ifdef _WIN32
    struct tm tmm;
    localtime_s(&tmm, &ref);
#else
    struct tm tmm = *localtime(&ref);
#endif
    tmm.tm_mday = 1; tmm.tm_hour = 0; tmm.tm_min = 0; tmm.tm_sec = 0;
    time_t monthStart = mktime(&tmm);
#ifdef _WIN32
    struct tm tnext = tmm;
#else
    struct tm tnext = tmm;
#endif
    tnext.tm_mon += 1;
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
