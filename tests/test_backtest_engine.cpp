#include "strategy/BacktestEngine.h"
#include "strategy/BacktestParameters.h"
#include "strategy/VolatilityArbitrageStrategy.h"
#include "models/GARCHModel.h"
#include "models/BSMPricingModel.h"
#include "strategy/VolatilitySpreadSignal.h"
#include "strategy/DeltaHedgingStrategy.h"
#include "core/MarketData.h"
#include "core/DateTime.h"
#include <iostream>
#include <vector>
#include <random>
#include <iomanip>

using namespace VolatilityArbitrage;

int main() {
    std::cout << "Testing Backtesting Engine...\n" << std::endl;
    
    try {
        // Create the backtesting engine
        BacktestEngine engine;
        
        std::cout << "=== Creating Sample Market Data ===" << std::endl;
        
        // Generate sample market data for testing
        std::vector<core::MarketData> sampleData;
        core::DateTime startDate(2025, 1, 1);
        
        std::random_device rd;
        std::mt19937 gen(42); // Fixed seed for reproducible results
        std::normal_distribution<double> priceDist(0.0, 0.02); // 2% daily volatility
        
        double price = 100.0; // Starting price
        
        for (int i = 0; i < 252; ++i) { // One year of trading days
            core::TimeDelta delta(i);
            core::DateTime currentDate = startDate + delta;
            
            // Generate realistic OHLC data
            double dailyReturn = priceDist(gen);
            double newPrice = price * (1.0 + dailyReturn);
            
            double open = price;
            double high = std::max(open, newPrice) * (1.0 + std::abs(priceDist(gen)) * 0.5);
            double low = std::min(open, newPrice) * (1.0 - std::abs(priceDist(gen)) * 0.5);
            double close = newPrice;
            double volume = 1000000 + gen() % 500000; // Random volume
            
            sampleData.emplace_back("SPY", currentDate, open, high, low, close, volume);
            price = newPrice;
        }
        
        std::cout << "Generated " << sampleData.size() << " data points for SPY" << std::endl;
        std::cout << "Price range: $" << std::fixed << std::setprecision(2) 
                  << sampleData.front().getClose() << " - $" << sampleData.back().getClose() << std::endl;
        
        // Add market data to engine
        engine.addMarketData("SPY", sampleData);
        
        std::cout << "\n=== Setting Up Backtest Parameters ===" << std::endl;
        
        // Create backtest parameters
        BacktestParameters params;
        params.setStartDate(core::DateTime(2025, 1, 1));
        params.setEndDate(core::DateTime(2025, 12, 31));
        params.setInitialCapital(100000.0);
        params.setSymbols({"SPY"});
        params.setIncludeTransactionCosts(true);
        params.setTransactionCostPerTrade(5.0);
        params.setTransactionCostPercentage(0.001); // 0.1%
        
        std::cout << "Backtest period: " << params.getStartDate().toString() 
                  << " to " << params.getEndDate().toString() << std::endl;
        std::cout << "Initial capital: $" << params.getInitialCapital() << std::endl;
        std::cout << "Transaction costs: $" << params.getTransactionCostPerTrade() 
                  << " + " << (params.getTransactionCostPercentage() * 100) << "%" << std::endl;
        
        std::cout << "\n=== Creating Strategy Components ===" << std::endl;
        
        // Create volatility model
        auto volatilityModel = std::make_unique<models::GARCHModel>(0.0001, 0.1, 0.8);
        std::cout << "Created GARCH(1,1) model with ω=0.0001, α=0.1, β=0.8" << std::endl;
        
        // Create signal generator
        auto signalGenerator = std::make_unique<VolatilitySpreadSignal>(0.15, 0.05);
        std::cout << "Created volatility spread signal (entry=15%, exit=5%)" << std::endl;
        
        // Create hedging strategy
        auto hedgingStrategy = std::make_unique<DeltaHedgingStrategy>(0.0, 0.01);
        std::cout << "Created delta hedging strategy (target=0.0, tolerance=1%)" << std::endl;
        
        // Create main strategy
        auto strategy = std::make_unique<VolatilityArbitrageStrategy>(
            std::move(volatilityModel),
            std::move(signalGenerator),
            std::move(hedgingStrategy),
            21 // 21-day holding period
        );
        std::cout << "Created volatility arbitrage strategy (21-day holding period)" << std::endl;
        
        std::cout << "\n=== Running Backtest ===" << std::endl;
        
        // Run the backtest
        auto result = engine.run(std::move(strategy), params);
        
        std::cout << "\n=== Backtest Results ===" << std::endl;
        
        // Display results
        result.printSummary();
        
        std::cout << "\n=== Detailed Analysis ===" << std::endl;
        
        // Get equity curve
        auto equityCurve = result.getEquityCurve();
        std::cout << "Equity curve has " << equityCurve.size() << " data points" << std::endl;
        
        if (!equityCurve.empty()) {
            auto values = equityCurve.getValues();
            std::cout << "Initial value: $" << std::fixed << std::setprecision(2) << values.front() << std::endl;
            std::cout << "Final value: $" << values.back() << std::endl;
            std::cout << "Peak value: $" << *std::max_element(values.begin(), values.end()) << std::endl;
            std::cout << "Trough value: $" << *std::min_element(values.begin(), values.end()) << std::endl;
        }
        
        // Get trades
        auto trades = result.getTrades();
        std::cout << "\nTotal trades executed: " << trades.size() << std::endl;
        
        if (!trades.empty()) {
            std::cout << "First few trades:" << std::endl;
            for (size_t i = 0; i < std::min(size_t(5), trades.size()); ++i) {
                std::cout << "  " << (i+1) << ". " << trades[i].toString() << std::endl;
            }
        }
        
        // Test metric calculations
        std::cout << "\n=== Performance Metrics Validation ===" << std::endl;
        auto allMetrics = result.getAllMetrics();
        std::cout << "Calculated " << allMetrics.size() << " performance metrics:" << std::endl;
        for (const auto& metric : allMetrics) {
            std::cout << "  " << metric.first << ": " << std::fixed << std::setprecision(4) 
                      << metric.second << std::endl;
        }
        
        // Test drawdown analysis
        auto drawdownSeries = result.getDrawdownSeries();
        if (!drawdownSeries.empty()) {
            auto drawdowns = drawdownSeries.getValues();
            double maxDD = *std::min_element(drawdowns.begin(), drawdowns.end());
            std::cout << "\nDrawdown analysis:" << std::endl;
            std::cout << "  Max drawdown from series: " << std::fixed << std::setprecision(2) 
                      << (maxDD * 100) << "%" << std::endl;
            
            auto drawdownPeriods = result.getDrawdownPeriods();
            std::cout << "  Number of drawdown periods: " << drawdownPeriods.size() << std::endl;
        }
        
        std::cout << "\n=== Engine Information ===" << std::endl;
        std::cout << engine.getEngineInfo() << std::endl;
        
        std::cout << "✅ All backtesting engine tests completed successfully!" << std::endl;
        
    } catch (const std::exception& e) {
        std::cerr << "❌ Test failed with error: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}
