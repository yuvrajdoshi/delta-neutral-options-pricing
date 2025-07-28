#include "core/MarketData.h"
#include "core/DateTime.h"
#include <iostream>
#include <vector>

int main() {
    try {
        // Test MarketData creation
        core::DateTime timestamp(2025, 7, 26, 10, 30, 0);
        core::MarketData data("AAPL", timestamp, 150.0, 155.0, 149.0, 153.0, 1000000.0);
        
        std::cout << "Created MarketData for: " << data.getSymbol() << std::endl;
        std::cout << "Timestamp: " << data.getTimestamp().toString() << std::endl;
        std::cout << "OHLCV: " << data.getOpen() << ", " << data.getHigh() << ", " 
                  << data.getLow() << ", " << data.getClose() << ", " << data.getVolume() << std::endl;
        
        // Test additional data
        data.setAdditionalData("implied_volatility", 0.25);
        data.setAdditionalData("bid_ask_spread", 0.05);
        
        std::cout << "Has implied_volatility: " << (data.hasAdditionalData("implied_volatility") ? "Yes" : "No") << std::endl;
        std::cout << "Implied Volatility: " << data.getAdditionalData("implied_volatility") << std::endl;
        
        // Test multiple MarketData for TimeSeries extraction
        std::vector<core::MarketData> marketDataVector;
        for (int i = 0; i < 5; ++i) {
            core::DateTime ts(2025, 7, 26 + i, 10, 30, 0);
            core::MarketData md("AAPL", ts, 150.0 + i, 155.0 + i, 149.0 + i, 153.0 + i, 1000000.0);
            marketDataVector.push_back(md);
        }
        
        // Extract TimeSeries for close prices
        core::TimeSeries closeSeries = core::MarketData::extractTimeSeries(marketDataVector, "close");
        std::cout << "Extracted TimeSeries with " << closeSeries.size() << " data points" << std::endl;
        std::cout << "First close price: " << closeSeries.getValue(0) << std::endl;
        std::cout << "Last close price: " << closeSeries.getValue(closeSeries.size() - 1) << std::endl;
        
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
}
