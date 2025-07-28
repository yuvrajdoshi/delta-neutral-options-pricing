#include "core/MarketData.h"
#include "core/TimeSeries.h"
#include <stdexcept>
#include <algorithm>
#include <cmath>
#include <fstream>
#include <sstream>
#include <numeric>

namespace core {

// Constructors
MarketData::MarketData() 
    : symbol_(""), timestamp_(DateTime()), open_(0.0), high_(0.0), low_(0.0), close_(0.0), volume_(0.0) {}

MarketData::MarketData(const std::string& symbol, const DateTime& timestamp, 
                      double open, double high, double low, double close, double volume)
    : symbol_(symbol), timestamp_(timestamp), open_(open), high_(high), 
      low_(low), close_(close), volume_(volume) {}

// Accessors
std::string MarketData::getSymbol() const {
    return symbol_; 
}

DateTime MarketData::getTimestamp() const {
    return timestamp_;
}

double MarketData::getOpen() const {
    return open_;
}

double MarketData::getHigh() const {
    return high_;
}

double MarketData::getLow() const {
    return low_;
}

double MarketData::getClose() const {
    return close_;
}

double MarketData::getVolume() const {
    return volume_;
}

// Additional data handling
void MarketData::setAdditionalData(const std::string& key, double value) {
    additionalData_[key] = value;
}

double MarketData::getAdditionalData(const std::string& key) const {
    auto it = additionalData_.find(key);
    if (it != additionalData_.end()) {
        return it->second;
    }
    throw std::invalid_argument("Key not found: " + key);
}

bool MarketData::hasAdditionalData(const std::string& key) const {
    return additionalData_.find(key) != additionalData_.end();
}

// Static methods
std::vector<MarketData> MarketData::fromCSV(const std::string& filename) {
    std::vector<MarketData> marketDataList;
    std::ifstream file(filename);
    
    if (!file.is_open()) {
        throw std::runtime_error("Unable to open file: " + filename);
    }
    
    std::string line;
    // Skip header line if present
    bool isFirstLine = true;
    
    while (std::getline(file, line)) {
        // Skip empty lines
        if (line.empty()) continue;
        
        // Skip header line (assuming first line contains headers like "Symbol,Date,Open,High,Low,Close,Volume")
        if (isFirstLine && (line.find("Symbol") != std::string::npos || line.find("symbol") != std::string::npos)) {
            isFirstLine = false;
            continue;
        }
        isFirstLine = false;
        
        std::istringstream ss(line);
        std::string symbol;
        std::string timestampStr;
        double open, high, low, close, volume;

        // Parse the CSV line (format: symbol,timestamp,open,high,low,close,volume)
        if (std::getline(ss, symbol, ',') &&
            std::getline(ss, timestampStr, ',') &&
            (ss >> open) && ss.get() == ',' &&
            (ss >> high) && ss.get() == ',' &&
            (ss >> low) && ss.get() == ',' &&
            (ss >> close) && ss.get() == ',' &&
            (ss >> volume)) {

            try {
                // Parse the timestamp string into a DateTime object    
                // Assuming the timestamp format is "YYYY-MM-DD HH:MM:SS"
                DateTime timestamp = DateTime::fromString(timestampStr, "%Y-%m-%d %H:%M:%S");
                MarketData data(symbol, timestamp, open, high, low, close, volume);
                marketDataList.push_back(data);
            } catch (const std::exception& e) {
                // Skip invalid lines and continue processing
                continue;
            }
        }
    }
    
    file.close();
    return marketDataList;
}

TimeSeries MarketData::extractTimeSeries(const std::vector<MarketData>& data, 
                                        const std::string& field) {
    std::vector<DateTime> timestamps;
    std::vector<double> values;
    
    for (const auto& entry : data) {
        timestamps.push_back(entry.getTimestamp());
        if (field == "open") {
            values.push_back(entry.getOpen());
        } else if (field == "high") {
            values.push_back(entry.getHigh());
        } else if (field == "low") {
            values.push_back(entry.getLow());
        } else if (field == "close") {
            values.push_back(entry.getClose());
        } else if (field == "volume") {
            values.push_back(entry.getVolume());
        } else {
            throw std::invalid_argument("Invalid field specified for TimeSeries extraction: " + field);
        }
    }
    
    return TimeSeries(timestamps, values, field);
}

} // namespace core