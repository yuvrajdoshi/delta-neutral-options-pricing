#ifndef BACKTEST_ENGINE_H
#define BACKTEST_ENGINE_H

#include "strategy/BacktestResult.h"
#include "strategy/BacktestParameters.h"
#include "strategy/Strategy.h"
#include "core/MarketData.h"
#include <map>
#include <vector>
#include <string>
#include <memory>

namespace VolatilityArbitrage {

/**
 * Main backtesting engine for running strategy simulations
 */
class BacktestEngine {
private:
    std::map<std::string, std::vector<core::MarketData>> marketData_;
    
    // Helper methods
    void validateParameters(const BacktestParameters& params) const;
    void validateMarketData(const BacktestParameters& params) const;
    std::vector<core::MarketData> getMarketDataInRange(
        const std::string& symbol, 
        const core::DateTime& startDate, 
        const core::DateTime& endDate) const;
    core::TimeSeries buildEquityCurve(
        const std::vector<double>& portfolioValues,
        const std::vector<core::DateTime>& timestamps) const;
    double calculateTransactionCost(
        const Trade& trade, 
        const BacktestParameters& params) const;
    
public:
    // Constructors
    BacktestEngine();
    
    // Data management
    void loadMarketData(const std::string& symbol, const std::string& filename);
    void addMarketData(const std::string& symbol, const std::vector<core::MarketData>& data);
    void clearMarketData();
    bool hasMarketData(const std::string& symbol) const;
    std::vector<std::string> getAvailableSymbols() const;
    
    // Backtesting
    BacktestResult run(
        std::unique_ptr<Strategy> strategy,
        const BacktestParameters& params);
    
    BacktestResult run(
        const Strategy& strategy,
        const BacktestParameters& params);
    
    // Analysis
    std::vector<BacktestResult> runParameterSweep(
        const Strategy& baseStrategy,
        const BacktestParameters& baseParams,
        const std::map<std::string, std::vector<double>>& paramSweep);
    
    // Monte Carlo simulation
    std::vector<BacktestResult> runMonteCarloSimulation(
        const Strategy& strategy,
        const BacktestParameters& params,
        int numSimulations = 1000);
    
    // Utility methods
    void printProgress(int current, int total) const;
    std::string getEngineInfo() const;
};

} // namespace VolatilityArbitrage

#endif // BACKTEST_ENGINE_H
