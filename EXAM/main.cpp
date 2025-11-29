#define _CRT_SECURE_NO_WARNINGS
#include <iostream>
#include <vector>
#include <string>
#include <conio.h>
#include <windows.h>

#include "classes.h"
#include "utils.h"
#include "menu.h"
#include "interactive.h"
#include "io.h"

using namespace std;

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
