#define _CRT_SECURE_NO_WARNINGS
#include "utils.h"
#include <sstream>
#include <iomanip>

using namespace std;
// перетворює рядок YYYY-MM-DD у time_t
bool parseDate(const string& s, time_t& outT) {
    struct tm tm {};
    if (s.size() < 10) return false;
    int y, m, d; char dash1, dash2;
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
// перетворює time_t у рядок YYYY-MM-DD
string formatDate(time_t t) {
    struct tm tm_buf;
#ifdef _WIN32
    localtime_s(&tm_buf, &t);
    struct tm* tm_ptr = &tm_buf;
#else
    struct tm* tm_ptr = localtime(&t);
    if (!tm_ptr) tm_ptr = &tm_buf; // fallback (tm_buf currently zeros)
#endif
    char buf[16];
    strftime(buf, sizeof(buf), "%Y-%m-%d", tm_ptr);
    return string(buf);
}
// обчислює дату понеділка того тижня для заданої дати
time_t startOfWeek(time_t t) {
    struct tm tm_struct;
#ifdef _WIN32
    localtime_s(&tm_struct, &t);
    struct tm tm = tm_struct;
#else
    struct tm tm = *localtime(&t);
#endif

    int wday = tm.tm_wday; 
    int offset = (wday + 6) % 7;
    time_t start = t - offset * 24 * 3600;

    struct tm st_struct;
#ifdef _WIN32
    localtime_s(&st_struct, &start);
    st_struct.tm_hour = 0; st_struct.tm_min = 0; st_struct.tm_sec = 0;
    return mktime(&st_struct);
#else
    struct tm st = *localtime(&start);
    st.tm_hour = 0; st.tm_min = 0; st.tm_sec = 0;
    return mktime(&st);
#endif
}
