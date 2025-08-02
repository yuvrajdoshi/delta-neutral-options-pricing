#ifndef BACKTEST_RESULT_H
#define BACKTEST_RESULT_H

#include "core/TimeSeries.h"
#include "strategy/Trade.h"
#include <vector>
#include <map>
#include <string>

namespace VolatilityArbitrage {

/**
 * Stores and analyzes backtesting results
 */
class BacktestResult {
private:
    core::TimeSeries equityCurve_;
    std::vector<Trade> trades_;
    std::map<std::string, double> metrics_;
    mutable bool metricsCalculated_;
    
    // Helper methods for metric calculations
    void calculateMetrics() const;
    double calculateSharpeRatio() const;
    double calculateSortinoRatio() const;
    double calculateMaxDrawdown() const;
    double calculateTotalReturn() const;
    double calculateAnnualizedReturn() const;
    double calculateAnnualizedVolatility() const;
    double calculateWinRate() const;
    double calculateProfitFactor() const;
    core::TimeSeries calculateDrawdownSeries() const;
    
public:
    // Constructors
    BacktestResult();
    BacktestResult(const core::TimeSeries& equityCurve, const std::vector<Trade>& trades);
    
    // Accessors
    core::TimeSeries getEquityCurve() const;
    std::vector<Trade> getTrades() const;
    size_t getTradeCount() const;
    
    // Performance metrics
    double getSharpeRatio() const;
    double getSortinoRatio() const;
    double getMaxDrawdown() const;
    double getTotalReturn() const;
    double getAnnualizedReturn() const;
    double getAnnualizedVolatility() const;
    double getWinRate() const;
    double getProfitFactor() const;
    
    // Metrics management
    void setMetric(const std::string& name, double value);
    double getMetric(const std::string& name) const;
    bool hasMetric(const std::string& name) const;
    std::map<std::string, double> getAllMetrics() const;
    
    // Analysis methods
    core::TimeSeries getDrawdownSeries() const;
    std::vector<std::pair<core::DateTime, core::DateTime>> getDrawdownPeriods() const;
    std::map<int, double> getReturnsByMonth() const;
    std::map<int, double> getReturnsByYear() const;
    
    // Modifiers
    void setEquityCurve(const core::TimeSeries& equityCurve);
    void setTrades(const std::vector<Trade>& trades);
    void addTrade(const Trade& trade);
    
    // Summary methods
    std::string getSummary() const;
    void printSummary() const;
};

} // namespace VolatilityArbitrage

#endif // BACKTEST_RESULT_H
