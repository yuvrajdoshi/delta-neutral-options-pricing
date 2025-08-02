#include "strategy/BacktestResult.h"
#include "core/Math.h"
#include <algorithm>
#include <cmath>
#include <iostream>
#include <iomanip>
#include <sstream>
#include <limits>

namespace VolatilityArbitrage {

BacktestResult::BacktestResult() 
    : equityCurve_(), trades_(), metrics_(), metricsCalculated_(false) {
}

BacktestResult::BacktestResult(const core::TimeSeries& equityCurve, const std::vector<Trade>& trades)
    : equityCurve_(equityCurve), trades_(trades), metrics_(), metricsCalculated_(false) {
}

core::TimeSeries BacktestResult::getEquityCurve() const {
    return equityCurve_;
}

std::vector<Trade> BacktestResult::getTrades() const {
    return trades_;
}

size_t BacktestResult::getTradeCount() const {
    return trades_.size();
}

double BacktestResult::getSharpeRatio() const {
    if (!metricsCalculated_) {
        calculateMetrics();
    }
    return getMetric("sharpe_ratio");
}

double BacktestResult::getSortinoRatio() const {
    if (!metricsCalculated_) {
        calculateMetrics();
    }
    return getMetric("sortino_ratio");
}

double BacktestResult::getMaxDrawdown() const {
    if (!metricsCalculated_) {
        calculateMetrics();
    }
    return getMetric("max_drawdown");
}

double BacktestResult::getTotalReturn() const {
    if (!metricsCalculated_) {
        calculateMetrics();
    }
    return getMetric("total_return");
}

double BacktestResult::getAnnualizedReturn() const {
    if (!metricsCalculated_) {
        calculateMetrics();
    }
    return getMetric("annualized_return");
}

double BacktestResult::getAnnualizedVolatility() const {
    if (!metricsCalculated_) {
        calculateMetrics();
    }
    return getMetric("annualized_volatility");
}

double BacktestResult::getWinRate() const {
    if (!metricsCalculated_) {
        calculateMetrics();
    }
    return getMetric("win_rate");
}

double BacktestResult::getProfitFactor() const {
    if (!metricsCalculated_) {
        calculateMetrics();
    }
    return getMetric("profit_factor");
}

void BacktestResult::setMetric(const std::string& name, double value) {
    metrics_[name] = value;
}

double BacktestResult::getMetric(const std::string& name) const {
    auto it = metrics_.find(name);
    if (it != metrics_.end()) {
        return it->second;
    }
    return 0.0; // Return 0 if metric not found
}

bool BacktestResult::hasMetric(const std::string& name) const {
    return metrics_.find(name) != metrics_.end();
}

std::map<std::string, double> BacktestResult::getAllMetrics() const {
    if (!metricsCalculated_) {
        calculateMetrics();
    }
    return metrics_;
}

core::TimeSeries BacktestResult::getDrawdownSeries() const {
    if (!metricsCalculated_) {
        calculateMetrics();
    }
    return calculateDrawdownSeries();
}

void BacktestResult::setEquityCurve(const core::TimeSeries& equityCurve) {
    equityCurve_ = equityCurve;
    metricsCalculated_ = false; // Invalidate cached metrics
}

void BacktestResult::setTrades(const std::vector<Trade>& trades) {
    trades_ = trades;
    metricsCalculated_ = false; // Invalidate cached metrics
}

void BacktestResult::addTrade(const Trade& trade) {
    trades_.push_back(trade);
    metricsCalculated_ = false; // Invalidate cached metrics
}

void BacktestResult::calculateMetrics() const {
    if (equityCurve_.empty()) {
        metricsCalculated_ = true;
        return;
    }
    
    // Calculate all metrics
    const_cast<std::map<std::string, double>&>(metrics_)["sharpe_ratio"] = calculateSharpeRatio();
    const_cast<std::map<std::string, double>&>(metrics_)["sortino_ratio"] = calculateSortinoRatio();
    const_cast<std::map<std::string, double>&>(metrics_)["max_drawdown"] = calculateMaxDrawdown();
    const_cast<std::map<std::string, double>&>(metrics_)["total_return"] = calculateTotalReturn();
    const_cast<std::map<std::string, double>&>(metrics_)["annualized_return"] = calculateAnnualizedReturn();
    const_cast<std::map<std::string, double>&>(metrics_)["annualized_volatility"] = calculateAnnualizedVolatility();
    const_cast<std::map<std::string, double>&>(metrics_)["win_rate"] = calculateWinRate();
    const_cast<std::map<std::string, double>&>(metrics_)["profit_factor"] = calculateProfitFactor();
    
    metricsCalculated_ = true;
}

double BacktestResult::calculateSharpeRatio() const {
    if (equityCurve_.size() < 2) return 0.0;
    
    // Calculate returns
    auto returns = equityCurve_.pctChange();
    std::vector<double> returnValues = returns.getValues();
    
    // Remove the first NaN value
    if (!returnValues.empty()) {
        returnValues.erase(returnValues.begin());
    }
    
    if (returnValues.empty()) return 0.0;
    
    double meanReturn = core::mean(returnValues);
    double stdReturn = core::standardDeviation(returnValues);
    
    if (stdReturn == 0.0) return 0.0;
    
    // Assuming daily returns, annualize
    return (meanReturn * std::sqrt(252.0)) / (stdReturn * std::sqrt(252.0));
}

double BacktestResult::calculateSortinoRatio() const {
    if (equityCurve_.size() < 2) return 0.0;
    
    // Calculate returns
    auto returns = equityCurve_.pctChange();
    std::vector<double> returnValues = returns.getValues();
    
    // Remove the first NaN value
    if (!returnValues.empty()) {
        returnValues.erase(returnValues.begin());
    }
    
    if (returnValues.empty()) return 0.0;
    
    double meanReturn = core::mean(returnValues);
    
    // Calculate downside deviation
    std::vector<double> negativeReturns;
    for (double ret : returnValues) {
        if (ret < 0.0) {
            negativeReturns.push_back(ret * ret);
        }
    }
    
    if (negativeReturns.empty()) return 0.0;
    
    double downsideVariance = core::mean(negativeReturns);
    double downsideDeviation = std::sqrt(downsideVariance);
    
    if (downsideDeviation == 0.0) return 0.0;
    
    // Annualize
    return (meanReturn * std::sqrt(252.0)) / (downsideDeviation * std::sqrt(252.0));
}

double BacktestResult::calculateMaxDrawdown() const {
    if (equityCurve_.empty()) return 0.0;
    
    std::vector<double> values = equityCurve_.getValues();
    double peak = values[0];
    double maxDrawdown = 0.0;
    
    for (size_t i = 1; i < values.size(); ++i) {
        if (values[i] > peak) {
            peak = values[i];
        }
        
        double drawdown = (peak - values[i]) / peak;
        if (drawdown > maxDrawdown) {
            maxDrawdown = drawdown;
        }
    }
    
    return maxDrawdown;
}

double BacktestResult::calculateTotalReturn() const {
    if (equityCurve_.empty()) return 0.0;
    
    std::vector<double> values = equityCurve_.getValues();
    return (values.back() - values.front()) / values.front();
}

double BacktestResult::calculateAnnualizedReturn() const {
    if (equityCurve_.size() < 2) return 0.0;
    
    double totalReturn = calculateTotalReturn();
    
    // Calculate time period in years
    std::vector<core::DateTime> timestamps = equityCurve_.getTimestamps();
    core::TimeDelta period = timestamps.back() - timestamps.front();
    double years = period.days() / 365.25;
    
    if (years <= 0.0) return 0.0;
    
    return std::pow(1.0 + totalReturn, 1.0 / years) - 1.0;
}

double BacktestResult::calculateAnnualizedVolatility() const {
    if (equityCurve_.size() < 2) return 0.0;
    
    auto returns = equityCurve_.pctChange();
    std::vector<double> returnValues = returns.getValues();
    
    // Remove the first NaN value
    if (!returnValues.empty()) {
        returnValues.erase(returnValues.begin());
    }
    
    if (returnValues.empty()) return 0.0;
    
    double stdReturn = core::standardDeviation(returnValues);
    return stdReturn * std::sqrt(252.0); // Annualize assuming daily returns
}

double BacktestResult::calculateWinRate() const {
    if (trades_.empty()) return 0.0;
    
    int winningTrades = 0;
    for (const auto& trade : trades_) {
        if (trade.getNetValue() > 0.0) {
            winningTrades++;
        }
    }
    
    return static_cast<double>(winningTrades) / trades_.size();
}

double BacktestResult::calculateProfitFactor() const {
    if (trades_.empty()) return 0.0;
    
    double grossProfit = 0.0;
    double grossLoss = 0.0;
    
    for (const auto& trade : trades_) {
        double netValue = trade.getNetValue();
        if (netValue > 0.0) {
            grossProfit += netValue;
        } else {
            grossLoss += std::abs(netValue);
        }
    }
    
    if (grossLoss == 0.0) {
        return grossProfit > 0.0 ? std::numeric_limits<double>::infinity() : 0.0;
    }
    
    return grossProfit / grossLoss;
}

core::TimeSeries BacktestResult::calculateDrawdownSeries() const {
    if (equityCurve_.empty()) return core::TimeSeries();
    
    std::vector<double> values = equityCurve_.getValues();
    std::vector<core::DateTime> timestamps = equityCurve_.getTimestamps();
    std::vector<double> drawdowns;
    
    double peak = values[0];
    drawdowns.push_back(0.0);
    
    for (size_t i = 1; i < values.size(); ++i) {
        if (values[i] > peak) {
            peak = values[i];
        }
        
        double drawdown = (peak - values[i]) / peak;
        drawdowns.push_back(-drawdown); // Negative for drawdown
    }
    
    return core::TimeSeries(timestamps, drawdowns, "Drawdown");
}

std::string BacktestResult::getSummary() const {
    if (!metricsCalculated_) {
        calculateMetrics();
    }
    
    std::ostringstream oss;
    oss << std::fixed << std::setprecision(4);
    oss << "=== Backtest Results Summary ===" << std::endl;
    oss << "Total Return: " << (getTotalReturn() * 100) << "%" << std::endl;
    oss << "Annualized Return: " << (getAnnualizedReturn() * 100) << "%" << std::endl;
    oss << "Annualized Volatility: " << (getAnnualizedVolatility() * 100) << "%" << std::endl;
    oss << "Sharpe Ratio: " << getSharpeRatio() << std::endl;
    oss << "Sortino Ratio: " << getSortinoRatio() << std::endl;
    oss << "Max Drawdown: " << (getMaxDrawdown() * 100) << "%" << std::endl;
    oss << "Win Rate: " << (getWinRate() * 100) << "%" << std::endl;
    oss << "Profit Factor: " << getProfitFactor() << std::endl;
    oss << "Total Trades: " << getTradeCount() << std::endl;
    
    return oss.str();
}

void BacktestResult::printSummary() const {
    std::cout << getSummary() << std::endl;
}

std::vector<std::pair<core::DateTime, core::DateTime>> BacktestResult::getDrawdownPeriods() const {
    // Implementation for identifying drawdown periods
    // This is a simplified version - could be enhanced
    std::vector<std::pair<core::DateTime, core::DateTime>> periods;
    
    if (equityCurve_.empty()) return periods;
    
    auto drawdownSeries = calculateDrawdownSeries();
    std::vector<double> drawdowns = drawdownSeries.getValues();
    std::vector<core::DateTime> timestamps = drawdownSeries.getTimestamps();
    
    bool inDrawdown = false;
    core::DateTime drawdownStart;
    
    for (size_t i = 0; i < drawdowns.size(); ++i) {
        if (drawdowns[i] < -0.01 && !inDrawdown) { // Start of drawdown (1% threshold)
            inDrawdown = true;
            drawdownStart = timestamps[i];
        } else if (drawdowns[i] >= 0.0 && inDrawdown) { // End of drawdown
            inDrawdown = false;
            periods.push_back({drawdownStart, timestamps[i]});
        }
    }
    
    // Handle case where backtest ends in drawdown
    if (inDrawdown) {
        periods.push_back({drawdownStart, timestamps.back()});
    }
    
    return periods;
}

std::map<int, double> BacktestResult::getReturnsByMonth() const {
    std::map<int, double> monthlyReturns;
    
    if (equityCurve_.size() < 2) return monthlyReturns;
    
    auto returns = equityCurve_.pctChange();
    std::vector<double> returnValues = returns.getValues();
    std::vector<core::DateTime> timestamps = returns.getTimestamps();
    
    for (size_t i = 1; i < returnValues.size(); ++i) {
        int month = timestamps[i].month();
        monthlyReturns[month] += returnValues[i];
    }
    
    return monthlyReturns;
}

std::map<int, double> BacktestResult::getReturnsByYear() const {
    std::map<int, double> yearlyReturns;
    
    if (equityCurve_.size() < 2) return yearlyReturns;
    
    auto returns = equityCurve_.pctChange();
    std::vector<double> returnValues = returns.getValues();
    std::vector<core::DateTime> timestamps = returns.getTimestamps();
    
    for (size_t i = 1; i < returnValues.size(); ++i) {
        int year = timestamps[i].year();
        yearlyReturns[year] += returnValues[i];
    }
    
    return yearlyReturns;
}

} // namespace VolatilityArbitrage
