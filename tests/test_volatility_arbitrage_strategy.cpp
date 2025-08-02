#include <iostream>
#include <cassert>
#include <memory>
#include "strategy/VolatilityArbitrageStrategy.h"
#include "strategy/BacktestParameters.h"
#include "strategy/VolatilitySpreadSignal.h"
#include "strategy/DeltaHedgingStrategy.h"
#include "models/ModelFactory.h"
#include "instruments/InstrumentFactory.h"
#include "core/MarketData.h"
#include "core/DateTime.h"
#include "core/TimeSeries.h"

using namespace VolatilityArbitrage;
using namespace core;
using namespace models;
using namespace instruments;

void testVolatilityArbitrageStrategyConstructor() {
    std::cout << "Testing VolatilityArbitrageStrategy constructor..." << std::endl;
    
    // Create components
    auto volatilityModel = ModelFactory::createGARCHModel(0.1, 0.1, 0.8);
    auto signalGenerator = std::make_unique<VolatilitySpreadSignal>(0.1, 0.05);
    auto hedgingStrategy = std::make_unique<DeltaHedgingStrategy>(0.0, 0.01);
    
    VolatilityArbitrageStrategy strategy(
        std::move(volatilityModel),
        std::move(signalGenerator),
        std::move(hedgingStrategy),
        21  // 21-day holding period
    );
    
    assert(strategy.getHoldingPeriod() == 21);
    
    std::cout << "Holding period: " << strategy.getHoldingPeriod() << " days" << std::endl;
    std::cout << "✓ VolatilityArbitrageStrategy constructor tests passed!" << std::endl;
}

void testVolatilityArbitrageStrategyInitialization() {
    std::cout << "Testing VolatilityArbitrageStrategy initialization..." << std::endl;
    
    // Create strategy
    auto volatilityModel = ModelFactory::createGARCHModel(0.1, 0.1, 0.8);
    auto signalGenerator = std::make_unique<VolatilitySpreadSignal>(0.1, 0.05);
    auto hedgingStrategy = std::make_unique<DeltaHedgingStrategy>(0.0, 0.01);
    
    VolatilityArbitrageStrategy strategy(
        std::move(volatilityModel),
        std::move(signalGenerator),
        std::move(hedgingStrategy)
    );
    
    // Create backtest parameters
    BacktestParameters params;
    DateTime startDate(2024, 1, 1, 9, 30, 0);
    DateTime endDate(2024, 12, 31, 16, 0, 0);
    std::vector<std::string> symbols = {"AAPL", "MSFT"};
    
    params.setStartDate(startDate);
    params.setEndDate(endDate);
    params.setInitialCapital(100000.0);
    params.setSymbols(symbols);
    
    // Initialize strategy
    strategy.initialize(params);
    
    // Verify portfolio initialization
    Portfolio portfolio = strategy.getPortfolio();
    assert(portfolio.getCash() == 100000.0);
    assert(portfolio.getPositionCount() == 0);
    
    std::cout << "Initial cash: $" << portfolio.getCash() << std::endl;
    std::cout << "Initial positions: " << portfolio.getPositionCount() << std::endl;
    std::cout << "✓ VolatilityArbitrageStrategy initialization tests passed!" << std::endl;
}

void testVolatilityArbitrageStrategyProcessBar() {
    std::cout << "Testing VolatilityArbitrageStrategy process bar..." << std::endl;
    
    // Create strategy
    auto volatilityModel = ModelFactory::createGARCHModel(0.1, 0.1, 0.8);
    
    // Calibrate the model first with some sample data
    TimeSeries calibrationData;
    for (int i = 0; i < 20; ++i) {
        DateTime date(2024, 1, i + 1, 0, 0, 0);
        double return_val = 0.01 * (i % 8 - 4) / 8.0;  // Varying returns
        calibrationData.addDataPoint(date, return_val);
    }
    volatilityModel->calibrate(calibrationData);
    
    auto signalGenerator = std::make_unique<VolatilitySpreadSignal>(0.1, 0.05);
    auto hedgingStrategy = std::make_unique<DeltaHedgingStrategy>(0.0, 0.01);
    
    VolatilityArbitrageStrategy strategy(
        std::move(volatilityModel),
        std::move(signalGenerator),
        std::move(hedgingStrategy)
    );
    
    // Initialize strategy
    BacktestParameters params;
    DateTime startDate(2024, 1, 1, 9, 30, 0);
    params.setStartDate(startDate);
    params.setInitialCapital(100000.0);
    std::vector<std::string> symbols = {"AAPL"};
    params.setSymbols(symbols);
    
    strategy.initialize(params);
    
    // Create market data
    DateTime timestamp(2024, 1, 15, 10, 30, 0);
    MarketData data("AAPL", timestamp, 150.0, 151.0, 149.0, 150.5, 1000000);
    data.setAdditionalData("implied_volatility", 0.25);
    data.setAdditionalData("risk_free_rate", 0.05);
    
    // Process first bar
    strategy.processBar(data);
    
    // Strategy should process the bar without error
    Portfolio portfolio = strategy.getPortfolio();
    std::cout << "Portfolio cash after processing bar: $" << portfolio.getCash() << std::endl;
    std::cout << "Position count after processing bar: " << portfolio.getPositionCount() << std::endl;
    
    std::cout << "✓ VolatilityArbitrageStrategy process bar tests passed!" << std::endl;
}

void testVolatilityArbitrageStrategyHoldingPeriod() {
    std::cout << "Testing VolatilityArbitrageStrategy holding period..." << std::endl;
    
    auto volatilityModel = ModelFactory::createGARCHModel(0.1, 0.1, 0.8);
    auto signalGenerator = std::make_unique<VolatilitySpreadSignal>(0.1, 0.05);
    auto hedgingStrategy = std::make_unique<DeltaHedgingStrategy>(0.0, 0.01);
    
    VolatilityArbitrageStrategy strategy(
        std::move(volatilityModel),
        std::move(signalGenerator),
        std::move(hedgingStrategy),
        15  // 15-day holding period
    );
    
    assert(strategy.getHoldingPeriod() == 15);
    
    // Test setting new holding period
    strategy.setHoldingPeriod(30);
    assert(strategy.getHoldingPeriod() == 30);
    
    strategy.setHoldingPeriod(5);
    assert(strategy.getHoldingPeriod() == 5);
    
    std::cout << "Final holding period: " << strategy.getHoldingPeriod() << " days" << std::endl;
    std::cout << "✓ VolatilityArbitrageStrategy holding period tests passed!" << std::endl;
}

void testVolatilityArbitrageStrategyClone() {
    std::cout << "Testing VolatilityArbitrageStrategy clone..." << std::endl;
    
    // Create original strategy
    auto volatilityModel = ModelFactory::createGARCHModel(0.1, 0.1, 0.8);
    auto signalGenerator = std::make_unique<VolatilitySpreadSignal>(0.1, 0.05);
    auto hedgingStrategy = std::make_unique<DeltaHedgingStrategy>(0.0, 0.01);
    
    VolatilityArbitrageStrategy originalStrategy(
        std::move(volatilityModel),
        std::move(signalGenerator),
        std::move(hedgingStrategy),
        25
    );
    
    // Initialize original
    BacktestParameters params;
    params.setInitialCapital(50000.0);
    originalStrategy.initialize(params);
    
    // Clone strategy
    auto clonedStrategy = originalStrategy.clone();
    
    // Verify clone exists and has the same holding period
    assert(clonedStrategy != nullptr);
    
    // Cast to check specific properties (this is a bit of a hack for testing)
    auto* clonedVolStrategy = dynamic_cast<VolatilityArbitrageStrategy*>(clonedStrategy.get());
    assert(clonedVolStrategy != nullptr);
    assert(clonedVolStrategy->getHoldingPeriod() == 25);
    
    std::cout << "Original holding period: " << originalStrategy.getHoldingPeriod() << std::endl;
    std::cout << "Cloned holding period: " << clonedVolStrategy->getHoldingPeriod() << std::endl;
    std::cout << "✓ VolatilityArbitrageStrategy clone tests passed!" << std::endl;
}

void testVolatilityArbitrageStrategyWithMultipleSymbols() {
    std::cout << "Testing VolatilityArbitrageStrategy with multiple symbols..." << std::endl;
    
    auto volatilityModel = ModelFactory::createGARCHModel(0.1, 0.1, 0.8);
    
    // Calibrate the model first
    TimeSeries calibrationData;
    for (int i = 0; i < 20; ++i) {
        DateTime date(2024, 1, i + 1, 0, 0, 0);
        double return_val = 0.01 * (i % 6 - 3) / 6.0;  // Varying returns
        calibrationData.addDataPoint(date, return_val);
    }
    volatilityModel->calibrate(calibrationData);
    
    auto signalGenerator = std::make_unique<VolatilitySpreadSignal>(0.1, 0.05);
    auto hedgingStrategy = std::make_unique<DeltaHedgingStrategy>(0.0, 0.01);
    
    VolatilityArbitrageStrategy strategy(
        std::move(volatilityModel),
        std::move(signalGenerator),
        std::move(hedgingStrategy)
    );
    
    // Initialize with multiple symbols
    BacktestParameters params;
    params.setInitialCapital(200000.0);
    std::vector<std::string> symbols = {"AAPL", "MSFT", "GOOGL"};
    params.setSymbols(symbols);
    strategy.initialize(params);
    
    // Process bars for different symbols
    DateTime timestamp(2024, 1, 15, 10, 30, 0);
    
    // AAPL data
    MarketData aaplData("AAPL", timestamp, 150.0, 151.0, 149.0, 150.5, 1000000);
    aaplData.setAdditionalData("implied_volatility", 0.25);
    aaplData.setAdditionalData("risk_free_rate", 0.05);
    strategy.processBar(aaplData);
    
    // MSFT data
    MarketData msftData("MSFT", timestamp, 300.0, 302.0, 298.0, 301.0, 800000);
    msftData.setAdditionalData("implied_volatility", 0.22);
    msftData.setAdditionalData("risk_free_rate", 0.05);
    strategy.processBar(msftData);
    
    Portfolio portfolio = strategy.getPortfolio();
    std::cout << "Portfolio cash after processing multiple symbols: $" << portfolio.getCash() << std::endl;
    std::cout << "Position count: " << portfolio.getPositionCount() << std::endl;
    
    std::cout << "✓ VolatilityArbitrageStrategy multiple symbols tests passed!" << std::endl;
}

int main() {
    std::cout << "Running VolatilityArbitrageStrategy tests..." << std::endl;
    
    try {
        testVolatilityArbitrageStrategyConstructor();
        testVolatilityArbitrageStrategyInitialization();
        testVolatilityArbitrageStrategyProcessBar();
        testVolatilityArbitrageStrategyHoldingPeriod();
        testVolatilityArbitrageStrategyClone();
        testVolatilityArbitrageStrategyWithMultipleSymbols();
        
        std::cout << "\n✅ All VolatilityArbitrageStrategy tests passed!" << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "❌ Test failed: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}
