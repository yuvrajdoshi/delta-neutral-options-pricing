#include "core/TimeSeries.h"
#include <stdexcept>
#include <algorithm>
#include <cmath>
#include <fstream>
#include <sstream>
#include <numeric>

namespace core {

// Constructors
TimeSeries::TimeSeries() : name_("") {}

TimeSeries::TimeSeries(const std::string& name) : name_(name) {}

TimeSeries::TimeSeries(const std::vector<DateTime>& timestamps, const std::vector<double>& values, const std::string& name)
    : timestamps_(timestamps), values_(values), name_(name) {
    if (timestamps_.size() != values_.size()) {
        throw std::invalid_argument("Timestamps and values vectors must have the same size");
    }
}

// Core functionality
void TimeSeries::addDataPoint(const DateTime& timestamp, double value) {
    // Check if timestamp already exists
    auto it = std::find(timestamps_.begin(), timestamps_.end(), timestamp);
    if (it != timestamps_.end()) {
        // Update existing value
        size_t index = std::distance(timestamps_.begin(), it);
        values_[index] = value;
    } else {
        // Find insertion point to maintain chronological order
        auto insert_pos = std::lower_bound(timestamps_.begin(), timestamps_.end(), timestamp);
        size_t index = std::distance(timestamps_.begin(), insert_pos);
        
        timestamps_.insert(insert_pos, timestamp);
        values_.insert(values_.begin() + index, value);
    }
}

void TimeSeries::clear() {
    timestamps_.clear();
    values_.clear();
}

size_t TimeSeries::size() const {
    return timestamps_.size();
}

bool TimeSeries::empty() const {
    return timestamps_.empty();
}

// Data access
double TimeSeries::getValue(size_t index) const {
    if (index >= values_.size()) {
        throw std::out_of_range("Index out of range");
    }
    return values_[index];
}

double TimeSeries::getValue(const DateTime& timestamp) const {
    auto it = std::find(timestamps_.begin(), timestamps_.end(), timestamp);
    if (it == timestamps_.end()) {
        throw std::invalid_argument("Timestamp not found in time series");
    }
    size_t index = std::distance(timestamps_.begin(), it);
    return values_[index];
}

DateTime TimeSeries::getTimestamp(size_t index) const {
    if (index >= timestamps_.size()) {
        throw std::out_of_range("Index out of range");
    }
    return timestamps_[index];
}

std::pair<DateTime, double> TimeSeries::getDataPoint(size_t index) const {
    if (index >= timestamps_.size()) {
        throw std::out_of_range("Index out of range");
    }
    return std::make_pair(timestamps_[index], values_[index]);
}

std::vector<double> TimeSeries::getValues() const {
    return values_;
}

std::vector<DateTime> TimeSeries::getTimestamps() const {
    return timestamps_;
}

// Time series operations
TimeSeries TimeSeries::getSubseries(const DateTime& start, const DateTime& end) const {
    std::vector<DateTime> subTimestamps;
    std::vector<double> subValues;
    
    for (size_t i = 0; i < timestamps_.size(); ++i) {
        if (timestamps_[i] >= start && timestamps_[i] <= end) {
            subTimestamps.push_back(timestamps_[i]);
            subValues.push_back(values_[i]);
        }
    }
    
    return TimeSeries(subTimestamps, subValues, name_ + "_subseries");
}

TimeSeries TimeSeries::getSubseries(size_t startIndex, size_t endIndex) const {
    if (startIndex >= timestamps_.size() || endIndex >= timestamps_.size() || startIndex > endIndex) {
        throw std::out_of_range("Invalid index range");
    }
    
    std::vector<DateTime> subTimestamps(timestamps_.begin() + startIndex, timestamps_.begin() + endIndex + 1);
    std::vector<double> subValues(values_.begin() + startIndex, values_.begin() + endIndex + 1);
    
    return TimeSeries(subTimestamps, subValues, name_ + "_subseries");
}

// Statistical functions
double TimeSeries::mean() const {
    if (values_.empty()) {
        throw std::runtime_error("Cannot calculate mean of empty time series");
    }
    
    return std::accumulate(values_.begin(), values_.end(), 0.0) / values_.size();
}

double TimeSeries::variance() const {
    if (values_.size() < 2) {
        throw std::runtime_error("Cannot calculate variance with less than 2 data points");
    }
    
    double m = mean();
    double sum = 0.0;
    
    for (double value : values_) {
        sum += (value - m) * (value - m);
    }
    
    return sum / (values_.size() - 1);  // Sample variance
}

double TimeSeries::standardDeviation() const {
    return std::sqrt(variance());
}

double TimeSeries::skewness() const {
    if (values_.size() < 3) {
        throw std::runtime_error("Cannot calculate skewness with less than 3 data points");
    }
    
    double m = mean();
    double std_dev = standardDeviation();
    double sum = 0.0;
    
    for (double value : values_) {
        sum += std::pow((value - m) / std_dev, 3);
    }
    
    size_t n = values_.size();
    return (n / ((n - 1.0) * (n - 2.0))) * sum;
}

double TimeSeries::kurtosis() const {
    if (values_.size() < 4) {
        throw std::runtime_error("Cannot calculate kurtosis with less than 4 data points");
    }
    
    double m = mean();
    double std_dev = standardDeviation();
    double sum = 0.0;
    
    for (double value : values_) {
        sum += std::pow((value - m) / std_dev, 4);
    }
    
    size_t n = values_.size();
    double numerator = n * (n + 1) / ((n - 1.0) * (n - 2.0) * (n - 3.0)) * sum;
    double correction = 3.0 * (n - 1) * (n - 1) / ((n - 2.0) * (n - 3.0));
    
    return numerator - correction;  // Excess kurtosis
}

double TimeSeries::autocorrelation(int lag) const {
    if (lag < 0 || static_cast<size_t>(lag) >= values_.size()) {
        throw std::invalid_argument("Invalid lag value");
    }
    
    if (values_.size() <= static_cast<size_t>(lag)) {
        throw std::runtime_error("Not enough data points for the specified lag");
    }
    
    double m = mean();
    double numerator = 0.0;
    double denominator = 0.0;
    
    size_t n = values_.size() - lag;
    
    for (size_t i = 0; i < n; ++i) {
        numerator += (values_[i] - m) * (values_[i + lag] - m);
    }
    
    for (size_t i = 0; i < values_.size(); ++i) {
        denominator += (values_[i] - m) * (values_[i] - m);
    }
    
    return numerator / denominator;
}

// Transformations
TimeSeries TimeSeries::diff() const {
    if (values_.size() < 2) {
        throw std::runtime_error("Cannot calculate differences with less than 2 data points");
    }
    
    std::vector<DateTime> diffTimestamps(timestamps_.begin() + 1, timestamps_.end());
    std::vector<double> diffValues;
    
    for (size_t i = 1; i < values_.size(); ++i) {
        diffValues.push_back(values_[i] - values_[i - 1]);
    }
    
    return TimeSeries(diffTimestamps, diffValues, name_ + "_diff");
}

TimeSeries TimeSeries::pctChange() const {
    if (values_.size() < 2) {
        throw std::runtime_error("Cannot calculate percentage changes with less than 2 data points");
    }
    
    std::vector<DateTime> pctTimestamps(timestamps_.begin() + 1, timestamps_.end());
    std::vector<double> pctValues;
    
    for (size_t i = 1; i < values_.size(); ++i) {
        if (values_[i - 1] == 0.0) {
            throw std::runtime_error("Cannot calculate percentage change with zero denominator");
        }
        pctValues.push_back((values_[i] - values_[i - 1]) / values_[i - 1]);
    }
    
    return TimeSeries(pctTimestamps, pctValues, name_ + "_pctchange");
}

TimeSeries TimeSeries::logReturn() const {
    if (values_.size() < 2) {
        throw std::runtime_error("Cannot calculate log returns with less than 2 data points");
    }
    
    std::vector<DateTime> logTimestamps(timestamps_.begin() + 1, timestamps_.end());
    std::vector<double> logValues;
    
    for (size_t i = 1; i < values_.size(); ++i) {
        if (values_[i - 1] <= 0.0 || values_[i] <= 0.0) {
            throw std::runtime_error("Cannot calculate log return with non-positive values");
        }
        logValues.push_back(std::log(values_[i] / values_[i - 1]));
    }
    
    return TimeSeries(logTimestamps, logValues, name_ + "_logreturn");
}

TimeSeries TimeSeries::rollingMean(size_t window) const {
    if (window == 0 || window > values_.size()) {
        throw std::invalid_argument("Invalid window size");
    }
    
    std::vector<DateTime> rollingTimestamps(timestamps_.begin() + window - 1, timestamps_.end());
    std::vector<double> rollingValues;
    
    for (size_t i = window - 1; i < values_.size(); ++i) {
        double sum = 0.0;
        for (size_t j = i - window + 1; j <= i; ++j) {
            sum += values_[j];
        }
        rollingValues.push_back(sum / window);
    }
    
    return TimeSeries(rollingTimestamps, rollingValues, name_ + "_rolling_mean");
}

TimeSeries TimeSeries::rollingStd(size_t window) const {
    if (window == 0 || window > values_.size()) {
        throw std::invalid_argument("Invalid window size");
    }
    
    std::vector<DateTime> rollingTimestamps(timestamps_.begin() + window - 1, timestamps_.end());
    std::vector<double> rollingValues;
    
    for (size_t i = window - 1; i < values_.size(); ++i) {
        // Calculate mean for this window
        double sum = 0.0;
        for (size_t j = i - window + 1; j <= i; ++j) {
            sum += values_[j];
        }
        double windowMean = sum / window;
        
        // Calculate variance for this window
        double variance = 0.0;
        for (size_t j = i - window + 1; j <= i; ++j) {
            variance += (values_[j] - windowMean) * (values_[j] - windowMean);
        }
        variance /= (window - 1);  // Sample variance
        
        rollingValues.push_back(std::sqrt(variance));
    }
    
    return TimeSeries(rollingTimestamps, rollingValues, name_ + "_rolling_std");
}

// I/O operations
void TimeSeries::saveToCSV(const std::string& filename) const {
    std::ofstream file(filename);
    if (!file.is_open()) {
        throw std::runtime_error("Cannot open file for writing: " + filename);
    }
    
    file << "timestamp,value\n";
    for (size_t i = 0; i < timestamps_.size(); ++i) {
        file << timestamps_[i].toString() << "," << values_[i] << "\n";
    }
    
    file.close();
}

TimeSeries TimeSeries::loadFromCSV(const std::string& filename, const std::string& valueColumn, const std::string& timestampColumn) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        throw std::runtime_error("Cannot open file for reading: " + filename);
    }
    
    std::vector<DateTime> timestamps;
    std::vector<double> values;
    std::string line;
    
    // Read header line
    if (!std::getline(file, line)) {
        throw std::runtime_error("Empty file or cannot read header");
    }
    
    // Parse header to find column indices
    std::stringstream headerStream(line);
    std::string columnName;
    int timestampIndex = -1;
    int valueIndex = -1;
    int columnIndex = 0;
    
    while (std::getline(headerStream, columnName, ',')) {
        if (columnName == timestampColumn) {
            timestampIndex = columnIndex;
        }
        if (columnName == valueColumn) {
            valueIndex = columnIndex;
        }
        columnIndex++;
    }
    
    if (timestampIndex == -1 || valueIndex == -1) {
        throw std::runtime_error("Required columns not found in CSV file");
    }
    
    // Read data lines
    while (std::getline(file, line)) {
        std::stringstream lineStream(line);
        std::string cell;
        std::vector<std::string> cells;
        
        while (std::getline(lineStream, cell, ',')) {
            cells.push_back(cell);
        }
        
        if (cells.size() > static_cast<size_t>(std::max(timestampIndex, valueIndex))) {
            try {
                DateTime timestamp = DateTime::fromString(cells[timestampIndex], "%Y-%m-%d %H:%M:%S");
                double value = std::stod(cells[valueIndex]);
                
                timestamps.push_back(timestamp);
                values.push_back(value);
            } catch (const std::exception& e) {
                // Skip invalid lines
                continue;
            }
        }
    }
    
    file.close();
    return TimeSeries(timestamps, values, filename);
}

// Getter/Setter for name
const std::string& TimeSeries::getName() const {
    return name_;
}

void TimeSeries::setName(const std::string& name) {
    name_ = name;
}

} // namespace core