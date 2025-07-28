#include "instruments/Option.h"
#include "core/MarketData.h"
#include "core/DateTime.h"
#include <iostream>
#include <iomanip>
#include <memory>
#include <cmath>

int main() {
    std::cout << "Testing Option Instruments...\n" << std::endl;
    
    try {
        // Create expiry date (30 days from now)
        core::DateTime expiry(2025, 8, 28, 16, 0, 0);
        
        // Create European options
        instruments::EuropeanOption callOption("AAPL", expiry, 150.0, instruments::OptionType::Call);
        instruments::EuropeanOption putOption("AAPL", expiry, 150.0, instruments::OptionType::Put);
        
        // Create American options
        instruments::AmericanOption americanCall("AAPL", expiry, 150.0, instruments::OptionType::Call);
        instruments::AmericanOption americanPut("AAPL", expiry, 150.0, instruments::OptionType::Put);
        
        std::cout << "=== Option Properties ===" << std::endl;
        std::cout << "Call Symbol: " << callOption.getSymbol() << std::endl;
        std::cout << "Put Symbol: " << putOption.getSymbol() << std::endl;
        std::cout << "Strike Price: $" << callOption.getStrike() << std::endl;
        std::cout << "Expiry: " << expiry.toString() << std::endl;
        std::cout << "Call Type: " << static_cast<int>(callOption.getOptionType()) << std::endl;
        std::cout << "Put Type: " << static_cast<int>(putOption.getOptionType()) << std::endl;
        
        // Create market data
        core::DateTime timestamp(2025, 7, 29, 10, 30, 0);
        core::MarketData marketData("AAPL", timestamp, 150.0, 155.0, 149.0, 153.0, 1000000.0);
        
        // Test with implied volatility
        marketData.setAdditionalData("implied_volatility", 0.25); // 25% IV
        
        std::cout << "\n=== European Options Pricing (with 25% IV) ===" << std::endl;
        std::cout << std::fixed << std::setprecision(4);
        
        double callPrice = callOption.price(marketData);
        double putPrice = putOption.price(marketData);
        
        std::cout << "Call Option Price: $" << callPrice << std::endl;
        std::cout << "Put Option Price: $" << putPrice << std::endl;
        
        // Test Put-Call Parity (approximately)
        double underlyingPrice = marketData.getClose();
        double strike = callOption.getStrike();
        double timeToExpiry = callOption.timeToExpiry(timestamp);
        double riskFreeRate = 0.05;
        double theoreticalPutCallParity = callPrice - putPrice - underlyingPrice + strike * std::exp(-riskFreeRate * timeToExpiry);
        std::cout << "Put-Call Parity Check: " << theoreticalPutCallParity << " (should be ~0)" << std::endl;
        
        std::cout << "\n=== American Options Pricing ===" << std::endl;
        double americanCallPrice = americanCall.price(marketData);
        double americanPutPrice = americanPut.price(marketData);
        
        std::cout << "American Call Price: $" << americanCallPrice << std::endl;
        std::cout << "American Put Price: $" << americanPutPrice << std::endl;
        
        std::cout << "\n=== Greeks (European Call) ===" << std::endl;
        std::cout << "Delta: " << callOption.delta(marketData) << std::endl;
        std::cout << "Gamma: " << callOption.gamma(marketData) << std::endl;
        std::cout << "Vega: " << callOption.vega(marketData) << std::endl;
        std::cout << "Theta: " << callOption.theta(marketData) << std::endl;
        std::cout << "Rho: " << callOption.rho(marketData) << std::endl;
        
        std::cout << "\n=== Greeks (European Put) ===" << std::endl;
        std::cout << "Delta: " << putOption.delta(marketData) << std::endl;
        std::cout << "Gamma: " << putOption.gamma(marketData) << std::endl;
        std::cout << "Vega: " << putOption.vega(marketData) << std::endl;
        std::cout << "Theta: " << putOption.theta(marketData) << std::endl;
        std::cout << "Rho: " << putOption.rho(marketData) << std::endl;
        
        std::cout << "\n=== Risk Metrics ===" << std::endl;
        auto callRiskMetrics = callOption.calculateRiskMetrics(marketData);
        std::cout << "Call Risk Metrics Count: " << callRiskMetrics.size() << std::endl;
        for (size_t i = 0; i < callRiskMetrics.size(); ++i) {
            std::cout << "  Metric " << i << ": " << callRiskMetrics[i] << std::endl;
        }
        
        std::cout << "\n=== Test Without Implied Volatility ===" << std::endl;
        core::MarketData marketDataNoIV("AAPL", timestamp, 150.0, 155.0, 149.0, 153.0, 1000000.0);
        // No implied volatility set - should use default 20%
        
        double callPriceNoIV = callOption.price(marketDataNoIV);
        double putPriceNoIV = putOption.price(marketDataNoIV);
        
        std::cout << "Call Price (no IV, using default): $" << callPriceNoIV << std::endl;
        std::cout << "Put Price (no IV, using default): $" << putPriceNoIV << std::endl;
        
        std::cout << "\n=== In-The-Money vs Out-of-The-Money ===" << std::endl;
        
        // ITM Call (strike < current price)
        instruments::EuropeanOption itmCall("AAPL", expiry, 140.0, instruments::OptionType::Call);
        // OTM Call (strike > current price)  
        instruments::EuropeanOption otmCall("AAPL", expiry, 160.0, instruments::OptionType::Call);
        
        std::cout << "ITM Call (Strike $140): $" << itmCall.price(marketData) << std::endl;
        std::cout << "OTM Call (Strike $160): $" << otmCall.price(marketData) << std::endl;
        
        std::cout << "\n=== Time Decay Test ===" << std::endl;
        // Create option close to expiry
        core::DateTime nearExpiry(2025, 7, 30, 16, 0, 0); // Tomorrow
        instruments::EuropeanOption nearExpiryOption("AAPL", nearExpiry, 150.0, instruments::OptionType::Call);
        
        double nearExpiryPrice = nearExpiryOption.price(marketData);
        std::cout << "Near Expiry Option Price: $" << nearExpiryPrice << std::endl;
        std::cout << "Time to Expiry: " << nearExpiryOption.timeToExpiry(timestamp) << " years" << std::endl;
        
        std::cout << "\n=== Clone Test ===" << std::endl;
        auto clonedCall = callOption.clone();
        std::cout << "Cloned Call Symbol: " << clonedCall->getSymbol() << std::endl;
        std::cout << "Cloned Call Price: $" << clonedCall->price(marketData) << std::endl;
        
        std::cout << "\n✅ Option tests completed successfully!" << std::endl;
        
    } catch (const std::exception& e) {
        std::cerr << "❌ Error: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}
