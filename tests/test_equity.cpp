#include "instruments/Equity.h"
#include "core/MarketData.h"
#include "core/DateTime.h"
#include <iostream>
#include <iomanip>

int main() {
    std::cout << "Testing Equity Instrument...\n" << std::endl;
    
    try {
        // Create test equity
        instruments::Equity apple("AAPL", 100.0);
        
        std::cout << "=== Basic Equity Properties ===" << std::endl;
        std::cout << "Symbol: " << apple.getSymbol() << std::endl;
        std::cout << "Shares: " << apple.getShares() << std::endl;
        std::cout << "Type: " << static_cast<int>(apple.getType()) << std::endl;
        
        // Create market data
        core::DateTime timestamp(2025, 7, 29, 10, 30, 0);
        core::MarketData marketData("AAPL", timestamp, 150.0, 155.0, 149.0, 153.0, 1000000.0);
        
        std::cout << "\n=== Pricing ===" << std::endl;
        double price = apple.price(marketData);
        std::cout << "Price per share: $" << std::fixed << std::setprecision(2) 
                  << marketData.getClose() << std::endl;
        std::cout << "Total position value: $" << price << std::endl;
        
        std::cout << "\n=== Risk Metrics ===" << std::endl;
        auto riskMetrics = apple.calculateRiskMetrics(marketData);
        std::cout << "Number of risk metrics: " << riskMetrics.size() << std::endl;
        for (size_t i = 0; i < riskMetrics.size(); ++i) {
            std::cout << "Metric " << i << ": " << riskMetrics[i] << std::endl;
        }
        
        std::cout << "\n=== Modify Shares ===" << std::endl;
        apple.setShares(200.0);
        std::cout << "New shares: " << apple.getShares() << std::endl;
        std::cout << "New position value: $" << apple.price(marketData) << std::endl;
        
        std::cout << "\n=== Clone Test ===" << std::endl;
        auto cloned = apple.clone();
        std::cout << "Cloned symbol: " << cloned->getSymbol() << std::endl;
        std::cout << "Cloned price: $" << cloned->price(marketData) << std::endl;
        
        std::cout << "\n✅ Equity tests completed successfully!" << std::endl;
        
    } catch (const std::exception& e) {
        std::cerr << "❌ Error: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}
