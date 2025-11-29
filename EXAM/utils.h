#pragma once
#include <string>
#include <ctime>

bool parseDate(const std::string& s, time_t& outT);
std::string formatDate(time_t t);
time_t startOfWeek(time_t t);
