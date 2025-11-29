#define _CRT_SECURE_NO_WARNINGS
#include "menu.h"
#include <iostream>
#include <conio.h>
using namespace std;
// меню, робота з стрілками і enter, відображає опції у консолі
int runArrowMenu(const vector<string>& items, const string& title) {
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
