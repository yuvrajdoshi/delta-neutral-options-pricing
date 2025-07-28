#include "instruments/InstrumentFactory.h"
#include "instruments/Equity.h"
#include "instruments/Option.h"
#include "core/MarketData.h"
#include "core/DateTime.h"
#include <iostream>
#include <iomanip>

int main() {
    std::cout << "Testing InstrumentFactory...\n" << std::endl;
    
    try {
        core::DateTime expiry(2025, 8, 28, 16, 0, 0);
        core::DateTime currentTime(2025, 7, 29, 10, 30, 0);
        core::MarketData marketData("AAPL", currentTime, 150.0, 155.0, 149.0, 153.0, 1000000.0);
        marketData.setAdditionalData("implied_volatility", 0.25);
        
        std::cout << "=== Creating Equity ===" << std::endl;
        
        auto equity = instruments::InstrumentFactory::createEquity("AAPL", 100.0);
        std::cout << "Created Equity: " << equity->getSymbol() << std::endl;
        std::cout << "Equity Type: " << static_cast<int>(equity->getType()) << std::endl;
        std::cout << "Equity Price: $" << std::fixed << std::setprecision(2) << equity->price(marketData) << std::endl;
        
        std::cout << "\n=== Creating European Options ===" << std::endl;
        
        auto europeanCall = instruments::InstrumentFactory::createEuropeanCall(
            "AAPL", expiry, 150.0);
        
        auto europeanPut = instruments::InstrumentFactory::createEuropeanPut(
            "AAPL", expiry, 150.0);
        
        std::cout << "European Call: " << europeanCall->getSymbol() << std::endl;
        std::cout << "European Put: " << europeanPut->getSymbol() << std::endl;
        std::cout << "Call Price: $" << std::setprecision(4) << europeanCall->price(marketData) << std::endl;
        std::cout << "Put Price: $" << europeanPut->price(marketData) << std::endl;
        
        std::cout << "\n=== Creating American Options ===" << std::endl;
        
        auto americanCall = instruments::InstrumentFactory::createAmericanCall(
            "AAPL", expiry, 150.0);
        
        auto americanPut = instruments::InstrumentFactory::createAmericanPut(
            "AAPL", expiry, 150.0);
        
        std::cout << "American Call: " << americanCall->getSymbol() << std::endl;
        std::cout << "American Put: " << americanPut->getSymbol() << std::endl;
        std::cout << "American Call Price: $" << americanCall->price(marketData) << std::endl;
        std::cout << "American Put Price: $" << americanPut->price(marketData) << std::endl;
        
        std::cout << "\n=== Multiple Strikes Test ===" << std::endl;
        
        std::vector<double> strikes = {140.0, 145.0, 150.0, 155.0, 160.0};
        
        std::cout << "Strike\tCall Price\tPut Price" << std::endl;
        std::cout << "------\t----------\t---------" << std::endl;
        
        for (double strike : strikes) {
            auto call = instruments::InstrumentFactory::createEuropeanCall(
                "AAPL", expiry, strike);
            auto put = instruments::InstrumentFactory::createEuropeanPut(
                "AAPL", expiry, strike);
            
            std::cout << "$" << std::setprecision(0) << strike 
                      << "\t$" << std::setprecision(4) << call->price(marketData)
                      << "\t\t$" << put->price(marketData) << std::endl;
        }
        
        std::cout << "\n=== Polymorphism Test ===" << std::endl;
        
        // Store different instruments in same container
        std::vector<std::unique_ptr<instruments::Instrument>> portfolio;
        
        portfolio.push_back(instruments::InstrumentFactory::createEquity("AAPL", 50.0));
        portfolio.push_back(instruments::InstrumentFactory::createAmericanCall(
            "AAPL", expiry, 150.0));
        portfolio.push_back(instruments::InstrumentFactory::createAmericanPut(
            "AAPL", expiry, 150.0));
        
        std::cout << "Portfolio Contents:" << std::endl;
        double totalValue = 0.0;
        
        for (size_t i = 0; i < portfolio.size(); ++i) {
            double price = portfolio[i]->price(marketData);
            totalValue += price;
            
            std::cout << i + 1 << ". " << portfolio[i]->getSymbol() 
                      << " (Type: " << static_cast<int>(portfolio[i]->getType()) 
                      << ") - $" << std::setprecision(4) << price << std::endl;
        }
        
        std::cout << "Total Portfolio Value: $" << totalValue << std::endl;
        
        std::cout << "\n=== Risk Metrics Test ===" << std::endl;
        
        for (size_t i = 0; i < portfolio.size(); ++i) {
            std::cout << "\n" << portfolio[i]->getSymbol() << " Risk Metrics:" << std::endl;
            auto metrics = portfolio[i]->calculateRiskMetrics(marketData);
            
            for (size_t j = 0; j < metrics.size(); ++j) {
                std::cout << "  Metric " << j << ": " << metrics[j] << std::endl;
            }
        }
        
        std::cout << "\n=== Option Greeks Test ===" << std::endl;
        
        // Test Greeks for different option types
        auto testCall = instruments::InstrumentFactory::createEuropeanCall(
            "AAPL", expiry, 150.0);
        
        // Cast to check if it's a derivative (for Greeks)
        auto* derivative = dynamic_cast<instruments::Derivative*>(testCall.get());
        if (derivative) {
            std::cout << "Option Greeks:" << std::endl;
            std::cout << "Delta: " << derivative->delta(marketData) << std::endl;
            std::cout << "Gamma: " << derivative->gamma(marketData) << std::endl;
            std::cout << "Vega: " << derivative->vega(marketData) << std::endl;
            std::cout << "Theta: " << derivative->theta(marketData) << std::endl;
            std::cout << "Rho: " << derivative->rho(marketData) << std::endl;
        }
        
        std::cout << "\n=== Clone Test ===" << std::endl;

        auto originalCall = instruments::InstrumentFactory::createEuropeanCall(
            "AAPL", expiry, 150.0);
        auto clonedCall = originalCall->clone();
        
        std::cout << "Original Call Symbol: " << originalCall->getSymbol() << std::endl;
        std::cout << "Cloned Call Symbol: " << clonedCall->getSymbol() << std::endl;
        std::cout << "Original Price: $" << originalCall->price(marketData) << std::endl;
        std::cout << "Cloned Price: $" << clonedCall->price(marketData) << std::endl;
        std::cout << "Prices Match: " << (originalCall->price(marketData) == clonedCall->price(marketData) ? "Yes" : "No") << std::endl;
        
        std::cout << "\n=== Error Handling Test ===" << std::endl;
        
        try {
            // Test with invalid strike
            auto invalidOption = instruments::InstrumentFactory::createEuropeanCall(
                "AAPL", expiry, -100.0);
            std::cout << "❌ Should have thrown exception for negative strike" << std::endl;
        } catch (const std::exception& e) {
            std::cout << "✅ Correctly caught exception for negative strike: " << e.what() << std::endl;
        }
        
        try {
            // Test with invalid shares
            auto invalidEquity = instruments::InstrumentFactory::createEquity("AAPL", -50.0);
            std::cout << "❌ Should have thrown exception for negative shares" << std::endl;
        } catch (const std::exception& e) {
            std::cout << "✅ Correctly caught exception for negative shares: " << e.what() << std::endl;
        }
        
        std::cout << "\n✅ InstrumentFactory tests completed successfully!" << std::endl;
        
    } catch (const std::exception& e) {
        std::cerr << "❌ Error: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}
