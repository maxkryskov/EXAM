#pragma once
#include <string>
#include "classes.h"

using namespace std;

string chooseWalletInteractive();
string chooseCategoryInteractive(FinanceSystem& fsys);
bool readAmount(double& amount);
time_t readDateInteractive();
void addExpenseInteractive(FinanceSystem& fsys);
void addTopUpInteractive(FinanceSystem& fsys);
void showBalances(FinanceSystem& fsys);
void listTransactionsWallet(const Wallet& w);
void manageCategoriesInteractive(FinanceSystem& fsys);
void generateAndSaveReport(FinanceSystem& fsys, const string& outFilename);
