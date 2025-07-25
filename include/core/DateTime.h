#ifndef DATETIME_H
#define DATETIME_H

#include <string>

namespace core {

// Forward declaration
class TimeDelta;

/**
 * TimeDelta class for representing time differences
 */
class TimeDelta {
private:
    long long seconds_;  // Total seconds for the time difference

public:
    // Constructors
    TimeDelta();
    TimeDelta(long long seconds);
    TimeDelta(int days, int hours = 0, int minutes = 0, int seconds = 0);

    // Accessors
    long long totalSeconds() const;
    int days() const;
    int hours() const;
    int minutes() const;
    int seconds() const;

    // Arithmetic operators
    TimeDelta operator+(const TimeDelta& other) const;
    TimeDelta operator-(const TimeDelta& other) const;
    TimeDelta operator*(double factor) const;
    TimeDelta operator/(double factor) const;
    TimeDelta& operator+=(const TimeDelta& other);
    TimeDelta& operator-=(const TimeDelta& other);

    // Comparison operators
    bool operator==(const TimeDelta& other) const;
    bool operator!=(const TimeDelta& other) const;
    bool operator<(const TimeDelta& other) const;
    bool operator<=(const TimeDelta& other) const;
    bool operator>(const TimeDelta& other) const;
    bool operator>=(const TimeDelta& other) const;

    // Unary operators
    TimeDelta operator-() const;

    // String representation
    std::string toString() const;
};

/**
 * DateTime class for representing date and time according to LLD specification
 */
class DateTime {
private:
    int year_;
    int month_;
    int day_;
    int hour_;
    int minute_;
    int second_;

    // Helper methods
    bool isValidDate(int year, int month, int day) const;
    bool isValidTime(int hour, int minute, int second) const;
    bool isLeapYear(int year) const;
    int daysInMonth(int year, int month) const;

public:
    // Constructors
    DateTime();
    DateTime(int year, int month, int day);
    DateTime(int year, int month, int day, int hour, int minute, int second = 0);
    
    // Static constructors
    static DateTime now();
    static DateTime fromString(const std::string& dateTimeStr, const std::string& format);
    
    // Accessors
    int year() const;
    int month() const;
    int day() const;
    int hour() const;
    int minute() const;
    int second() const;
    
    // Modifiers
    void setYear(int year);
    void setMonth(int month);
    void setDay(int day);
    void setHour(int hour);
    void setMinute(int minute);
    void setSecond(int second);
    
    // Operations
    std::string toString(const std::string& format = "%Y-%m-%d %H:%M:%S") const;
    bool isValid() const;
    
    // Comparison operators
    bool operator==(const DateTime& other) const;
    bool operator!=(const DateTime& other) const;
    bool operator<(const DateTime& other) const;
    bool operator<=(const DateTime& other) const;
    bool operator>(const DateTime& other) const;
    bool operator>=(const DateTime& other) const;
    
    // Arithmetic operators
    DateTime operator+(const TimeDelta& delta) const;
    DateTime operator-(const TimeDelta& delta) const;
    TimeDelta operator-(const DateTime& other) const;
};

// Global operators for TimeDelta
TimeDelta operator*(double factor, const TimeDelta& delta);

} // namespace core

#endif // DATETIME_H