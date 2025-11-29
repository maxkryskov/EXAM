#pragma once

#include <string>
#include <vector>
#include <fstream>
#include <filesystem>
#include <algorithm>
#include <map>

using namespace std;
namespace fs = std::filesystem;

bool parseDate(const string& s, time_t& outT);
string formatDate(time_t t);
time_t startOfWeek(time_t t);
// транзакції
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
// гаманець 
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
// фінанси
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
        collect(debit, "Debit");
        collect(credit, "Credit");
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
        for (const auto& t : w.getTransactions()) {
            if (t.date >= start && t.date <= end) res.push_back(t);
        }
        return res;
    }
};
