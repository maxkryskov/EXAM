#pragma once
#include <string>
class FinanceSystem;
bool loadDebit(const std::string& filename, FinanceSystem& fsys);
bool saveDebit(const std::string& filename, FinanceSystem& fsys);
bool loadCredit(const std::string& filename, FinanceSystem& fsys);
bool saveCredit(const std::string& filename, FinanceSystem& fsys);
