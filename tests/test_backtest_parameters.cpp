#include <iostream>
#include <cassert>
#include <vector>
#include "strategy/BacktestParameters.h"
#include "core/DateTime.h"

using namespace VolatilityArbitrage;
using namespace core;

void testBacktestParametersConstructor() {
    std::cout << "Testing BacktestParameters constructor..." << std::endl;
    
    BacktestParameters params;
    
    // Test default values
    assert(params.getInitialCapital() == 100000.0);
    assert(params.getIncludeTransactionCosts() == false);
    assert(params.getTransactionCostPerTrade() == 0.0);
    assert(params.getTransactionCostPercentage() == 0.0);
    assert(params.getSymbols().empty());
    
    std::cout << "✓ BacktestParameters constructor tests passed!" << std::endl;
}

void testBacktestParametersDates() {
    std::cout << "Testing BacktestParameters date functionality..." << std::endl;
    
    BacktestParameters params;
    
    DateTime startDate(2024, 1, 1, 9, 30, 0);
    DateTime endDate(2024, 12, 31, 16, 0, 0);
    
    params.setStartDate(startDate);
    params.setEndDate(endDate);
    
    assert(params.getStartDate().year() == 2024);
    assert(params.getStartDate().month() == 1);
    assert(params.getStartDate().day() == 1);
    
    assert(params.getEndDate().year() == 2024);
    assert(params.getEndDate().month() == 12);
    assert(params.getEndDate().day() == 31);
    
    std::cout << "Start Date: " << params.getStartDate().toString() << std::endl;
    std::cout << "End Date: " << params.getEndDate().toString() << std::endl;
    
    std::cout << "✓ BacktestParameters date tests passed!" << std::endl;
}

void testBacktestParametersCapital() {
    std::cout << "Testing BacktestParameters capital functionality..." << std::endl;
    
    BacktestParameters params;
    
    // Test setting different capital amounts
    params.setInitialCapital(50000.0);
    assert(params.getInitialCapital() == 50000.0);
    
    params.setInitialCapital(1000000.0);
    assert(params.getInitialCapital() == 1000000.0);
    
    // Test edge cases
    params.setInitialCapital(0.0);
    assert(params.getInitialCapital() == 0.0);
    
    std::cout << "Final capital: " << params.getInitialCapital() << std::endl;
    
    std::cout << "✓ BacktestParameters capital tests passed!" << std::endl;
}

void testBacktestParametersSymbols() {
    std::cout << "Testing BacktestParameters symbols functionality..." << std::endl;
    
    BacktestParameters params;
    
    // Test empty symbols
    assert(params.getSymbols().empty());
    
    // Test setting symbols
    std::vector<std::string> symbols = {"AAPL", "MSFT", "GOOGL", "TSLA"};
    params.setSymbols(symbols);
    
    auto retrievedSymbols = params.getSymbols();
    assert(retrievedSymbols.size() == 4);
    assert(retrievedSymbols[0] == "AAPL");
    assert(retrievedSymbols[1] == "MSFT");
    assert(retrievedSymbols[2] == "GOOGL");
    assert(retrievedSymbols[3] == "TSLA");
    
    // Test single symbol
    std::vector<std::string> singleSymbol = {"SPY"};
    params.setSymbols(singleSymbol);
    
    auto singleRetrieved = params.getSymbols();
    assert(singleRetrieved.size() == 1);
    assert(singleRetrieved[0] == "SPY");
    
    std::cout << "Symbols count: " << singleRetrieved.size() << std::endl;
    std::cout << "First symbol: " << singleRetrieved[0] << std::endl;
    
    std::cout << "✓ BacktestParameters symbols tests passed!" << std::endl;
}

void testBacktestParametersTransactionCosts() {
    std::cout << "Testing BacktestParameters transaction costs functionality..." << std::endl;
    
    BacktestParameters params;
    
    // Test default transaction costs
    assert(params.getIncludeTransactionCosts() == false);
    assert(params.getTransactionCostPerTrade() == 0.0);
    assert(params.getTransactionCostPercentage() == 0.0);
    
    // Test enabling transaction costs
    params.setIncludeTransactionCosts(true);
    assert(params.getIncludeTransactionCosts() == true);
    
    // Test setting fixed cost per trade
    params.setTransactionCostPerTrade(9.99);
    assert(params.getTransactionCostPerTrade() == 9.99);
    
    // Test setting percentage cost
    params.setTransactionCostPercentage(0.001); // 0.1%
    assert(params.getTransactionCostPercentage() == 0.001);
    
    // Test disabling transaction costs
    params.setIncludeTransactionCosts(false);
    assert(params.getIncludeTransactionCosts() == false);
    
    std::cout << "Transaction costs enabled: " << params.getIncludeTransactionCosts() << std::endl;
    std::cout << "Cost per trade: $" << params.getTransactionCostPerTrade() << std::endl;
    std::cout << "Cost percentage: " << params.getTransactionCostPercentage() * 100 << "%" << std::endl;
    
    std::cout << "✓ BacktestParameters transaction costs tests passed!" << std::endl;
}

void testBacktestParametersCompleteSetup() {
    std::cout << "Testing BacktestParameters complete setup..." << std::endl;
    
    BacktestParameters params;
    
    // Set up a complete backtest configuration
    DateTime startDate(2023, 1, 1, 9, 30, 0);
    DateTime endDate(2023, 12, 31, 16, 0, 0);
    std::vector<std::string> symbols = {"AAPL", "MSFT", "GOOGL"};
    
    params.setStartDate(startDate);
    params.setEndDate(endDate);
    params.setInitialCapital(250000.0);
    params.setSymbols(symbols);
    params.setIncludeTransactionCosts(true);
    params.setTransactionCostPerTrade(5.0);
    params.setTransactionCostPercentage(0.0005); // 0.05%
    
    // Verify all settings
    assert(params.getStartDate().year() == 2023);
    assert(params.getEndDate().year() == 2023);
    assert(params.getInitialCapital() == 250000.0);
    assert(params.getSymbols().size() == 3);
    assert(params.getIncludeTransactionCosts() == true);
    assert(params.getTransactionCostPerTrade() == 5.0);
    assert(params.getTransactionCostPercentage() == 0.0005);
    
    std::cout << "Complete backtest setup verified:" << std::endl;
    std::cout << "  Period: " << params.getStartDate().toString() << " to " << params.getEndDate().toString() << std::endl;
    std::cout << "  Capital: $" << params.getInitialCapital() << std::endl;
    std::cout << "  Symbols: " << params.getSymbols().size() << " instruments" << std::endl;
    std::cout << "  Transaction costs: " << (params.getIncludeTransactionCosts() ? "enabled" : "disabled") << std::endl;
    
    std::cout << "✓ BacktestParameters complete setup tests passed!" << std::endl;
}

int main() {
    std::cout << "Running BacktestParameters tests..." << std::endl;
    
    try {
        testBacktestParametersConstructor();
        testBacktestParametersDates();
        testBacktestParametersCapital();
        testBacktestParametersSymbols();
        testBacktestParametersTransactionCosts();
        testBacktestParametersCompleteSetup();
        
        std::cout << "\n✅ All BacktestParameters tests passed!" << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "❌ Test failed: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}
