#include "core/DateTime.h"
#include <chrono>
#include <iomanip>
#include <sstream>
#include <stdexcept>
#include <cmath>
#include <ctime>

namespace core {

// TimeDelta implementation

TimeDelta::TimeDelta() : seconds_(0) {}

TimeDelta::TimeDelta(long long seconds) : seconds_(seconds) {}

TimeDelta::TimeDelta(int days, int hours, int minutes, int seconds) {
    seconds_ = static_cast<long long>(days) * 24 * 60 * 60 +
               static_cast<long long>(hours) * 60 * 60 +
               static_cast<long long>(minutes) * 60 +
               static_cast<long long>(seconds);
}

long long TimeDelta::totalSeconds() const {
    return seconds_;
}

int TimeDelta::days() const {
    return static_cast<int>(seconds_ / (24 * 60 * 60));
}

int TimeDelta::hours() const {
    return static_cast<int>((seconds_ % (24 * 60 * 60)) / (60 * 60));
}

int TimeDelta::minutes() const {
    return static_cast<int>((seconds_ % (60 * 60)) / 60);
}

int TimeDelta::seconds() const {
    return static_cast<int>(seconds_ % 60);
}

TimeDelta TimeDelta::operator+(const TimeDelta& other) const {
    return TimeDelta(seconds_ + other.seconds_);
}

TimeDelta TimeDelta::operator-(const TimeDelta& other) const {
    return TimeDelta(seconds_ - other.seconds_);
}

TimeDelta TimeDelta::operator*(double factor) const {
    return TimeDelta(static_cast<long long>(seconds_ * factor));
}

TimeDelta TimeDelta::operator/(double factor) const {
    if (factor == 0.0) {
        throw std::invalid_argument("Division by zero");
    }
    return TimeDelta(static_cast<long long>(seconds_ / factor));
}

TimeDelta& TimeDelta::operator+=(const TimeDelta& other) {
    seconds_ += other.seconds_;
    return *this;
}

TimeDelta& TimeDelta::operator-=(const TimeDelta& other) {
    seconds_ -= other.seconds_;
    return *this;
}

bool TimeDelta::operator==(const TimeDelta& other) const {
    return seconds_ == other.seconds_;
}

bool TimeDelta::operator!=(const TimeDelta& other) const {
    return !(*this == other);
}

bool TimeDelta::operator<(const TimeDelta& other) const {
    return seconds_ < other.seconds_;
}

bool TimeDelta::operator<=(const TimeDelta& other) const {
    return seconds_ <= other.seconds_;
}

bool TimeDelta::operator>(const TimeDelta& other) const {
    return seconds_ > other.seconds_;
}

bool TimeDelta::operator>=(const TimeDelta& other) const {
    return seconds_ >= other.seconds_;
}

TimeDelta TimeDelta::operator-() const {
    return TimeDelta(-seconds_);
}

std::string TimeDelta::toString() const {
    std::ostringstream oss;
    long long abs_seconds = std::abs(seconds_);
    
    if (seconds_ < 0) oss << "-";
    
    int d = static_cast<int>(abs_seconds / (24 * 60 * 60));
    int h = static_cast<int>((abs_seconds % (24 * 60 * 60)) / (60 * 60));
    int m = static_cast<int>((abs_seconds % (60 * 60)) / 60);
    int s = static_cast<int>(abs_seconds % 60);
    
    if (d > 0) oss << d << " days, ";
    oss << std::setfill('0') << std::setw(2) << h << ":"
        << std::setw(2) << m << ":" << std::setw(2) << s;
    
    return oss.str();
}

// DateTime implementation

DateTime::DateTime() {
    auto now = std::chrono::system_clock::now();
    auto time_t = std::chrono::system_clock::to_time_t(now);
    
    std::tm* tm = std::localtime(&time_t);
    year_ = tm->tm_year + 1900;
    month_ = tm->tm_mon + 1;
    day_ = tm->tm_mday;
    hour_ = tm->tm_hour;
    minute_ = tm->tm_min;
    second_ = tm->tm_sec;
}

DateTime::DateTime(int year, int month, int day) 
    : DateTime(year, month, day, 0, 0, 0) {}

DateTime::DateTime(int year, int month, int day, int hour, int minute, int second)
    : year_(year), month_(month), day_(day), hour_(hour), minute_(minute), second_(second) {
    
    if (!isValidDate(year_, month_, day_) || !isValidTime(hour_, minute_, second_)) {
        throw std::invalid_argument("Invalid date or time values");
    }
}

// Static constructors
DateTime DateTime::now() {
    return DateTime();
}

DateTime DateTime::fromString(const std::string& dateTimeStr, const std::string& format) {
    // Simple implementation for basic formats
    // This is a simplified version - in a full implementation, you'd use strptime or similar
    std::istringstream iss(dateTimeStr);
    int year, month, day, hour = 0, minute = 0, second = 0;
    char delimiter;
    
    if (format == "%Y-%m-%d %H:%M:%S" || format == "%Y-%m-%d") {
        iss >> year >> delimiter >> month >> delimiter >> day;
        if (format == "%Y-%m-%d %H:%M:%S" && iss.peek() == ' ') {
            iss >> hour >> delimiter >> minute >> delimiter >> second;
        }
    } else {
        throw std::invalid_argument("Unsupported date format");
    }
    
    return DateTime(year, month, day, hour, minute, second);
}

// Accessors
int DateTime::year() const { return year_; }
int DateTime::month() const { return month_; }
int DateTime::day() const { return day_; }
int DateTime::hour() const { return hour_; }
int DateTime::minute() const { return minute_; }
int DateTime::second() const { return second_; }

// Modifiers
void DateTime::setYear(int year) {
    if (!isValidDate(year, month_, day_)) {
        throw std::invalid_argument("Invalid year");
    }
    year_ = year;
}

void DateTime::setMonth(int month) {
    if (!isValidDate(year_, month, day_)) {
        throw std::invalid_argument("Invalid month");
    }
    month_ = month;
}

void DateTime::setDay(int day) {
    if (!isValidDate(year_, month_, day)) {
        throw std::invalid_argument("Invalid day");
    }
    day_ = day;
}

void DateTime::setHour(int hour) {
    if (!isValidTime(hour, minute_, second_)) {
        throw std::invalid_argument("Invalid hour");
    }
    hour_ = hour;
}

void DateTime::setMinute(int minute) {
    if (!isValidTime(hour_, minute, second_)) {
        throw std::invalid_argument("Invalid minute");
    }
    minute_ = minute;
}

void DateTime::setSecond(int second) {
    if (!isValidTime(hour_, minute_, second)) {
        throw std::invalid_argument("Invalid second");
    }
    second_ = second;
}

// Operations
std::string DateTime::toString(const std::string& format) const {
    if (format == "%Y-%m-%d %H:%M:%S") {
        std::ostringstream oss;
        oss << std::setfill('0') 
            << std::setw(4) << year_ << "-"
            << std::setw(2) << month_ << "-"
            << std::setw(2) << day_ << " "
            << std::setw(2) << hour_ << ":"
            << std::setw(2) << minute_ << ":"
            << std::setw(2) << second_;
        return oss.str();
    } else if (format == "%Y-%m-%d") {
        std::ostringstream oss;
        oss << std::setfill('0') 
            << std::setw(4) << year_ << "-"
            << std::setw(2) << month_ << "-"
            << std::setw(2) << day_;
        return oss.str();
    } else {
        // Default format
        std::ostringstream oss;
        oss << std::setfill('0') 
            << std::setw(4) << year_ << "-"
            << std::setw(2) << month_ << "-"
            << std::setw(2) << day_ << " "
            << std::setw(2) << hour_ << ":"
            << std::setw(2) << minute_ << ":"
            << std::setw(2) << second_;
        return oss.str();
    }
}

bool DateTime::isValid() const {
    return isValidDate(year_, month_, day_) && isValidTime(hour_, minute_, second_);
}

// Comparison operators
bool DateTime::operator==(const DateTime& other) const {
    return year_ == other.year_ && month_ == other.month_ && day_ == other.day_ &&
           hour_ == other.hour_ && minute_ == other.minute_ && second_ == other.second_;
}

bool DateTime::operator!=(const DateTime& other) const {
    return !(*this == other);
}

bool DateTime::operator<(const DateTime& other) const {
    if (year_ != other.year_) return year_ < other.year_;
    if (month_ != other.month_) return month_ < other.month_;
    if (day_ != other.day_) return day_ < other.day_;
    if (hour_ != other.hour_) return hour_ < other.hour_;
    if (minute_ != other.minute_) return minute_ < other.minute_;
    return second_ < other.second_;
}

bool DateTime::operator<=(const DateTime& other) const {
    return *this < other || *this == other;
}

bool DateTime::operator>(const DateTime& other) const {
    return !(*this <= other);
}

bool DateTime::operator>=(const DateTime& other) const {
    return !(*this < other);
}

// Arithmetic operators
DateTime DateTime::operator+(const TimeDelta& delta) const {
    // Convert to seconds since epoch, add delta, convert back
    std::tm tm = {};
    tm.tm_year = year_ - 1900;
    tm.tm_mon = month_ - 1;
    tm.tm_mday = day_;
    tm.tm_hour = hour_;
    tm.tm_min = minute_;
    tm.tm_sec = second_;
    
    std::time_t time = std::mktime(&tm);
    time += delta.totalSeconds();
    
    std::tm* result_tm = std::localtime(&time);
    return DateTime(result_tm->tm_year + 1900, result_tm->tm_mon + 1, result_tm->tm_mday,
                   result_tm->tm_hour, result_tm->tm_min, result_tm->tm_sec);
}

DateTime DateTime::operator-(const TimeDelta& delta) const {
    // Convert to seconds since epoch, subtract delta, convert back
    std::tm tm = {};
    tm.tm_year = year_ - 1900;
    tm.tm_mon = month_ - 1;
    tm.tm_mday = day_;
    tm.tm_hour = hour_;
    tm.tm_min = minute_;
    tm.tm_sec = second_;
    
    std::time_t time = std::mktime(&tm);
    time -= delta.totalSeconds();
    
    std::tm* result_tm = std::localtime(&time);
    return DateTime(result_tm->tm_year + 1900, result_tm->tm_mon + 1, result_tm->tm_mday,
                   result_tm->tm_hour, result_tm->tm_min, result_tm->tm_sec);
}

TimeDelta DateTime::operator-(const DateTime& other) const {
    std::tm tm1 = {};
    tm1.tm_year = year_ - 1900;
    tm1.tm_mon = month_ - 1;
    tm1.tm_mday = day_;
    tm1.tm_hour = hour_;
    tm1.tm_min = minute_;
    tm1.tm_sec = second_;
    
    std::tm tm2 = {};
    tm2.tm_year = other.year_ - 1900;
    tm2.tm_mon = other.month_ - 1;
    tm2.tm_mday = other.day_;
    tm2.tm_hour = other.hour_;
    tm2.tm_min = other.minute_;
    tm2.tm_sec = other.second_;
    
    std::time_t time1 = std::mktime(&tm1);
    std::time_t time2 = std::mktime(&tm2);
    
    return TimeDelta(static_cast<long long>(time1 - time2));
}

// Helper methods
bool DateTime::isValidDate(int year, int month, int day) const {
    if (year < 1900 || year > 3000) return false;
    if (month < 1 || month > 12) return false;
    if (day < 1 || day > daysInMonth(year, month)) return false;
    return true;
}

bool DateTime::isValidTime(int hour, int minute, int second) const {
    return hour >= 0 && hour < 24 &&
           minute >= 0 && minute < 60 &&
           second >= 0 && second < 60;
}

bool DateTime::isLeapYear(int year) const {
    return (year % 4 == 0 && year % 100 != 0) || (year % 400 == 0);
}

int DateTime::daysInMonth(int year, int month) const {
    static const int days[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
    if (month == 2 && isLeapYear(year)) return 29;
    return days[month - 1];
}

// Global operators
TimeDelta operator*(double factor, const TimeDelta& delta) {
    return delta * factor;
}

} // namespace core