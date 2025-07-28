#ifndef TIMESERIES_H
#define TIMESERIES_H

#include "core/DateTime.h"
#include <vector>
#include <string>
#include <utility>

namespace core {

/**
 * TimeSeries class for storing and manipulating time-indexed data
 * Implementation follows the LLD specification
 */
class TimeSeries {
private:
    std::vector<DateTime> timestamps_;
    std::vector<double> values_;
    std::string name_;
    
public:
    // Constructors
    TimeSeries();
    TimeSeries(const std::string& name);
    TimeSeries(const std::vector<DateTime>& timestamps, const std::vector<double>& values, const std::string& name = "");
    
    // Core functionality
    void addDataPoint(const DateTime& timestamp, double value);
    void clear();
    size_t size() const;
    bool empty() const;
    
    // Data access
    double getValue(size_t index) const;
    double getValue(const DateTime& timestamp) const;
    DateTime getTimestamp(size_t index) const;
    std::pair<DateTime, double> getDataPoint(size_t index) const;
    std::vector<double> getValues() const;
    std::vector<DateTime> getTimestamps() const;
    
    // Time series operations
    TimeSeries getSubseries(const DateTime& start, const DateTime& end) const;
    TimeSeries getSubseries(size_t startIndex, size_t endIndex) const;
    
    // Statistical functions
    double mean() const;
    double variance() const;
    double standardDeviation() const;
    double skewness() const;
    double kurtosis() const;
    double autocorrelation(int lag) const;
    
    // Transformations
    TimeSeries diff() const;
    TimeSeries pctChange() const;
    TimeSeries logReturn() const;
    TimeSeries rollingMean(size_t window) const;
    TimeSeries rollingStd(size_t window) const;
    
    // I/O operations
    void saveToCSV(const std::string& filename) const;
    static TimeSeries loadFromCSV(const std::string& filename, const std::string& valueColumn, const std::string& timestampColumn);
    
    // Getter for name
    const std::string& getName() const;
    void setName(const std::string& name);
};

} // namespace core

#endif // TIMESERIES_H