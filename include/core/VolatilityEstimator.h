#ifndef VOLATILITY_ESTIMATOR_H
#define VOLATILITY_ESTIMATOR_H

#include "core/TimeSeries.h"
#include "core/MarketData.h"
#include <vector>
#include <string>

namespace core {

/**
 * VolatilityEstimator - Handles multiple methods for estimating volatility
 * when implied volatility is not available in market data
 */
class VolatilityEstimator {
public:
    enum class VolatilityMethod {
        HISTORICAL,          // Historical volatility from price returns
        EWMA,               // Exponentially Weighted Moving Average
        GARCH,              // GARCH(1,1) model
        IMPLIED_FALLBACK    // Use implied vol if available, else historical
    };

    // Calculate historical volatility from price series
    static double calculateHistoricalVolatility(
        const TimeSeries& prices, 
        int lookbackDays = 30,
        bool annualized = true
    );

    // EWMA volatility estimation
    static double calculateEWMAVolatility(
        const TimeSeries& prices,
        double lambda = 0.94,  // RiskMetrics standard
        bool annualized = true
    );

    // Get volatility with fallback logic
    static double getVolatility(
        const MarketData& marketData,
        const TimeSeries& historicalPrices,
        VolatilityMethod method = VolatilityMethod::IMPLIED_FALLBACK,
        int lookbackDays = 30
    );

    // Estimate volatility for option pricing when IV is missing
    static double estimateImpliedVolatility(
        const std::string& symbol,
        const TimeSeries& underlyingPrices,
        double timeToExpiry,
        VolatilityMethod method = VolatilityMethod::HISTORICAL
    );

private:
    static constexpr double TRADING_DAYS_PER_YEAR = 252.0;
    static constexpr double DEFAULT_MIN_VOLATILITY = 0.05;  // 5% minimum
    static constexpr double DEFAULT_MAX_VOLATILITY = 3.0;   // 300% maximum
};

} // namespace core

#endif // VOLATILITY_ESTIMATOR_H
