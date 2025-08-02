#ifndef MARKETDATA_H
#define MARKETDATA_H
#include "core/DateTime.h"
#include "core/TimeSeries.h"
#include <vector>
#include <string>
#include <map>
#include <utility>

namespace core {
    class MarketData {
    private:
        std::string symbol_;
        core::DateTime timestamp_;
        double open_;
        double high_;
        double low_;
        double close_;
        double volume_;
        std::map<std::string, double> additionalData_;  // For implied volatility, etc.
        
    public:
        // Constructors
        MarketData();
        MarketData(const std::string& symbol, const DateTime& timestamp, 
                double open, double high, double low, double close, double volume);
        
        // Accessors
        std::string getSymbol() const;
        DateTime getTimestamp() const;
        double getOpen() const;
        double getHigh() const;
        double getLow() const;
        double getClose() const;
        double getVolume() const;
        
        // Additional data handling
        void setAdditionalData(const std::string& key, double value);
        double getAdditionalData(const std::string& key) const;
        bool hasAdditionalData(const std::string& key) const;
        
        // Static methods
        static std::vector<MarketData> fromCSV(const std::string& filename);
        static TimeSeries extractTimeSeries(const std::vector<MarketData>& data, 
                                        const std::string& field = "close");
    };
}
#endif // MARKETDATA_H
