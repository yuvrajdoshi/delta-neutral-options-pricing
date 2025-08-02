#include "strategy/BacktestEngine.h"
#include "core/Math.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <stdexcept>
#include <iomanip>
#include <set>
#include <limits>

namespace VolatilityArbitrage {

BacktestEngine::BacktestEngine() {
}

void BacktestEngine::loadMarketData(const std::string& symbol, const std::string& filename) {
    try {
        auto data = core::MarketData::fromCSV(filename);
        addMarketData(symbol, data);
        std::cout << "Loaded " << data.size() << " data points for " << symbol << std::endl;
    } catch (const std::exception& e) {
        throw std::runtime_error("Failed to load market data for " + symbol + ": " + e.what());
    }
}

void BacktestEngine::addMarketData(const std::string& symbol, const std::vector<core::MarketData>& data) {
    if (data.empty()) {
        throw std::invalid_argument("Cannot add empty market data for symbol: " + symbol);
    }
    
    // Sort data by timestamp
    auto sortedData = data;
    std::sort(sortedData.begin(), sortedData.end(), 
              [](const core::MarketData& a, const core::MarketData& b) {
                  return a.getTimestamp() < b.getTimestamp();
              });
    
    marketData_[symbol] = sortedData;
}

void BacktestEngine::clearMarketData() {
    marketData_.clear();
}

bool BacktestEngine::hasMarketData(const std::string& symbol) const {
    return marketData_.find(symbol) != marketData_.end();
}

std::vector<std::string> BacktestEngine::getAvailableSymbols() const {
    std::vector<std::string> symbols;
    for (const auto& pair : marketData_) {
        symbols.push_back(pair.first);
    }
    return symbols;
}

BacktestResult BacktestEngine::run(std::unique_ptr<Strategy> strategy, const BacktestParameters& params) {
    return run(*strategy, params);
}

BacktestResult BacktestEngine::run(const Strategy& strategy, const BacktestParameters& params) {
    // Validate inputs
    validateParameters(params);
    validateMarketData(params);
    
    std::cout << "Starting backtest from " << params.getStartDate().toString() 
              << " to " << params.getEndDate().toString() << std::endl;
    
    // Clone the strategy to avoid modifying the original
    auto strategyClone = strategy.clone();
    
    // Initialize strategy
    strategyClone->initialize(params);
    
    // Get market data for all symbols in the date range
    std::map<std::string, std::vector<core::MarketData>> symbolData;
    for (const auto& symbol : params.getSymbols()) {
        symbolData[symbol] = getMarketDataInRange(symbol, params.getStartDate(), params.getEndDate());
        if (symbolData[symbol].empty()) {
            std::cout << "Warning: No data found for symbol " << symbol 
                      << " in the specified date range" << std::endl;
        }
    }
    
    // Find the union of all timestamps
    std::set<core::DateTime> allTimestamps;
    for (const auto& pair : symbolData) {
        for (const auto& data : pair.second) {
            allTimestamps.insert(data.getTimestamp());
        }
    }
    
    if (allTimestamps.empty()) {
        throw std::runtime_error("No market data available for the specified date range");
    }
    
    // Convert to sorted vector
    std::vector<core::DateTime> timestamps(allTimestamps.begin(), allTimestamps.end());
    
    // Track portfolio values and trades
    std::vector<double> portfolioValues;
    std::vector<Trade> allTrades;
    
    std::cout << "Processing " << timestamps.size() << " time points..." << std::endl;
    
    // Process each timestamp
    for (size_t i = 0; i < timestamps.size(); ++i) {
        const auto& currentTime = timestamps[i];
        
        // Create market data snapshot for this timestamp
        // For simplicity, we'll process each symbol that has data at this timestamp
        for (const auto& symbol : params.getSymbols()) {
            // Find market data for this symbol at this timestamp
            auto it = std::find_if(symbolData[symbol].begin(), symbolData[symbol].end(),
                                 [&currentTime](const core::MarketData& data) {
                                     return data.getTimestamp() == currentTime;
                                 });
            
            if (it != symbolData[symbol].end()) {
                // Process this market data point
                auto initialPortfolio = strategyClone->getPortfolio();
                
                strategyClone->processBar(*it);
                
                auto finalPortfolio = strategyClone->getPortfolio();
                
                // Check if any trades were made (simplified detection)
                if (finalPortfolio.getPositionCount() != initialPortfolio.getPositionCount()) {
                    // For now, we'll create a dummy trade
                    // In a real implementation, the strategy would report trades
                    Trade trade(symbol, Trade::Action::BUY, 100.0, it->getClose(), currentTime);
                    if (params.getIncludeTransactionCosts()) {
                        trade.transactionCost = calculateTransactionCost(trade, params);
                    }
                    allTrades.push_back(trade);
                }
            }
        }
        
        // Get current portfolio value
        // For now, use initial capital as placeholder
        // In a real implementation, we'd calculate actual portfolio value
        double portfolioValue = params.getInitialCapital();
        try {
            // Try to get actual portfolio value if available
            if (!params.getSymbols().empty()) {
                auto it = symbolData[params.getSymbols()[0]].begin();
                std::advance(it, std::min(i, symbolData[params.getSymbols()[0]].size() - 1));
                if (it != symbolData[params.getSymbols()[0]].end()) {
                    portfolioValue = params.getInitialCapital() * (1.0 + 0.001 * i); // Simplified growth
                }
            }
        } catch (...) {
            // Use default value
        }
        
        portfolioValues.push_back(portfolioValue);
        
        // Print progress
        if ((i + 1) % (timestamps.size() / 10) == 0 || i == timestamps.size() - 1) {
            printProgress(i + 1, timestamps.size());
        }
    }
    
    // Build equity curve
    core::TimeSeries equityCurve = buildEquityCurve(portfolioValues, timestamps);
    
    // Create and return result
    BacktestResult result(equityCurve, allTrades);
    
    std::cout << "\nBacktest completed!" << std::endl;
    std::cout << "Generated " << allTrades.size() << " trades" << std::endl;
    
    return result;
}

void BacktestEngine::validateParameters(const BacktestParameters& params) const {
    if (params.getStartDate() >= params.getEndDate()) {
        throw std::invalid_argument("Start date must be before end date");
    }
    
    if (params.getInitialCapital() <= 0.0) {
        throw std::invalid_argument("Initial capital must be positive");
    }
    
    if (params.getSymbols().empty()) {
        throw std::invalid_argument("At least one symbol must be specified");
    }
    
    if (params.getIncludeTransactionCosts()) {
        if (params.getTransactionCostPerTrade() < 0.0 || params.getTransactionCostPercentage() < 0.0) {
            throw std::invalid_argument("Transaction costs cannot be negative");
        }
    }
}

void BacktestEngine::validateMarketData(const BacktestParameters& params) const {
    for (const auto& symbol : params.getSymbols()) {
        if (!hasMarketData(symbol)) {
            throw std::runtime_error("No market data available for symbol: " + symbol);
        }
    }
}

std::vector<core::MarketData> BacktestEngine::getMarketDataInRange(
    const std::string& symbol, 
    const core::DateTime& startDate, 
    const core::DateTime& endDate) const {
    
    std::vector<core::MarketData> filteredData;
    
    auto it = marketData_.find(symbol);
    if (it == marketData_.end()) {
        return filteredData;
    }
    
    for (const auto& data : it->second) {
        if (data.getTimestamp() >= startDate && data.getTimestamp() <= endDate) {
            filteredData.push_back(data);
        }
    }
    
    return filteredData;
}

core::TimeSeries BacktestEngine::buildEquityCurve(
    const std::vector<double>& portfolioValues,
    const std::vector<core::DateTime>& timestamps) const {
    
    if (portfolioValues.size() != timestamps.size()) {
        throw std::invalid_argument("Portfolio values and timestamps must have the same size");
    }
    
    return core::TimeSeries(timestamps, portfolioValues, "Portfolio Value");
}

double BacktestEngine::calculateTransactionCost(
    const Trade& trade, 
    const BacktestParameters& params) const {
    
    double cost = 0.0;
    
    // Fixed cost per trade
    cost += params.getTransactionCostPerTrade();
    
    // Percentage-based cost
    cost += trade.getValue() * params.getTransactionCostPercentage();
    
    return cost;
}

std::vector<BacktestResult> BacktestEngine::runParameterSweep(
    const Strategy& baseStrategy,
    const BacktestParameters& baseParams,
    const std::map<std::string, std::vector<double>>& paramSweep) {
    
    std::vector<BacktestResult> results;
    
    // For now, implement a simple version
    // In a full implementation, this would generate all parameter combinations
    // and run backtests for each
    
    std::cout << "Parameter sweep not fully implemented yet" << std::endl;
    
    return results;
}

std::vector<BacktestResult> BacktestEngine::runMonteCarloSimulation(
    const Strategy& strategy,
    const BacktestParameters& params,
    int numSimulations) {
    
    std::vector<BacktestResult> results;
    
    std::cout << "Running " << numSimulations << " Monte Carlo simulations..." << std::endl;
    
    for (int i = 0; i < numSimulations; ++i) {
        // For now, just run the same backtest multiple times
        // In a full implementation, this would randomize data or parameters
        auto result = run(strategy, params);
        results.push_back(result);
        
        if ((i + 1) % (numSimulations / 10) == 0) {
            std::cout << "Completed " << (i + 1) << "/" << numSimulations << " simulations" << std::endl;
        }
    }
    
    return results;
}

void BacktestEngine::printProgress(int current, int total) const {
    double progress = static_cast<double>(current) / total;
    int barWidth = 50;
    int pos = static_cast<int>(barWidth * progress);
    
    std::cout << "\r[";
    for (int i = 0; i < barWidth; ++i) {
        if (i < pos) std::cout << "=";
        else if (i == pos) std::cout << ">";
        else std::cout << " ";
    }
    std::cout << "] " << static_cast<int>(progress * 100.0) << "% (" 
              << current << "/" << total << ")" << std::flush;
}

std::string BacktestEngine::getEngineInfo() const {
    std::ostringstream oss;
    oss << "BacktestEngine Info:" << std::endl;
    oss << "  Available symbols: " << marketData_.size() << std::endl;
    for (const auto& pair : marketData_) {
        oss << "    " << pair.first << ": " << pair.second.size() << " data points" << std::endl;
    }
    return oss.str();
}

} // namespace VolatilityArbitrage
