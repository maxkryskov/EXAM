#include "io.h"
#include "classes.h"
// дл€ завантаженн€ ≥ збереженн€ кожного гаманц€ 
bool loadDebit(const std::string& filename, FinanceSystem& fsys) {
    return fsys.getDebit().loadFromFile(filename);
}
bool saveDebit(const std::string& filename, FinanceSystem& fsys) {
    return fsys.getDebit().saveToFile(filename);
}
bool loadCredit(const std::string& filename, FinanceSystem& fsys) {
    return fsys.getCredit().loadFromFile(filename);
}
bool saveCredit(const std::string& filename, FinanceSystem& fsys) {
    return fsys.getCredit().saveToFile(filename);
}
