#include "core/VolatilityEstimator.h"
#include "core/TimeSeries.h"
#include "core/MarketData.h"
#include "core/DateTime.h"
#include <iostream>
#include <iomanip>
#include <vector>
#include <random>

int main() {
    std::cout << "Testing VolatilityEstimator...\n" << std::endl;
    
    try {
        // Create sample price data (simulating daily prices)
        std::vector<core::DateTime> timestamps;
        std::vector<double> prices;
        
        // Generate 60 days of sample price data
        core::DateTime startDate(2025, 6, 1);
        double basePrice = 100.0;
        
        // Simple random walk for testing
        std::random_device rd;
        std::mt19937 gen(42); // Fixed seed for reproducible results
        std::normal_distribution<double> returns(0.0, 0.02); // 2% daily volatility
        
        for (int i = 0; i < 60; ++i) {
            // Use TimeDelta for proper date arithmetic
            core::TimeDelta delta(i); // i days
            core::DateTime date = startDate + delta;
            
            if (i > 0) {
                double dailyReturn = returns(gen);
                basePrice *= (1.0 + dailyReturn);
            }
            
            timestamps.push_back(date);
            prices.push_back(basePrice);
        }
        
        core::TimeSeries priceSeries(timestamps, prices, "TestStock");
        
        std::cout << "=== Sample Data ===" << std::endl;
        std::cout << "Number of price points: " << priceSeries.size() << std::endl;
        std::cout << "First price: $" << std::fixed << std::setprecision(2) << prices[0] << std::endl;
        std::cout << "Last price: $" << prices.back() << std::endl;
        std::cout << "Price change: " << std::setprecision(1) << ((prices.back() / prices[0] - 1.0) * 100) << "%" << std::endl;
        
        std::cout << "\n=== Historical Volatility Tests ===" << std::endl;
        std::cout << std::setprecision(4);
        
        // Test different lookback periods
        double vol30 = core::VolatilityEstimator::calculateHistoricalVolatility(priceSeries, 30, true);
        double vol20 = core::VolatilityEstimator::calculateHistoricalVolatility(priceSeries, 20, true);
        double vol10 = core::VolatilityEstimator::calculateHistoricalVolatility(priceSeries, 10, true);
        
        std::cout << "30-day Historical Volatility: " << vol30 * 100 << "%" << std::endl;
        std::cout << "20-day Historical Volatility: " << vol20 * 100 << "%" << std::endl;
        std::cout << "10-day Historical Volatility: " << vol10 * 100 << "%" << std::endl;
        
        // Test non-annualized
        double volDaily = core::VolatilityEstimator::calculateHistoricalVolatility(priceSeries, 30, false);
        std::cout << "Daily Volatility (non-annualized): " << volDaily * 100 << "%" << std::endl;
        std::cout << "Annualization factor check: " << (vol30 / volDaily) << " (should be ~" << std::sqrt(252) << ")" << std::endl;
        
        std::cout << "\n=== EWMA Volatility Tests ===" << std::endl;
        
        double ewmaVol094 = core::VolatilityEstimator::calculateEWMAVolatility(priceSeries, 0.94, true);
        double ewmaVol090 = core::VolatilityEstimator::calculateEWMAVolatility(priceSeries, 0.90, true);
        double ewmaVol080 = core::VolatilityEstimator::calculateEWMAVolatility(priceSeries, 0.80, true);
        
        std::cout << "EWMA Volatility (λ=0.94): " << ewmaVol094 * 100 << "%" << std::endl;
        std::cout << "EWMA Volatility (λ=0.90): " << ewmaVol090 * 100 << "%" << std::endl;
        std::cout << "EWMA Volatility (λ=0.80): " << ewmaVol080 * 100 << "%" << std::endl;
        
        std::cout << "\n=== MarketData Integration Tests ===" << std::endl;
        
        core::DateTime currentTime(2025, 7, 29, 10, 30, 0);
        
        // Test with implied volatility available
        core::MarketData dataWithIV("TEST", currentTime, 99.0, 101.0, 98.0, 100.0, 50000);
        dataWithIV.setAdditionalData("implied_volatility", 0.22); // 22% IV
        
        double volWithIV = core::VolatilityEstimator::getVolatility(
            dataWithIV, priceSeries, core::VolatilityEstimator::VolatilityMethod::IMPLIED_FALLBACK, 30);
        
        std::cout << "Volatility with IV available: " << volWithIV * 100 << "% (should be 22%)" << std::endl;
        
        // Test without implied volatility (fallback to historical)
        core::MarketData dataNoIV("TEST", currentTime, 99.0, 101.0, 98.0, 100.0, 50000);
        
        double volNoIV = core::VolatilityEstimator::getVolatility(
            dataNoIV, priceSeries, core::VolatilityEstimator::VolatilityMethod::IMPLIED_FALLBACK, 30);
        
        std::cout << "Volatility without IV (fallback): " << volNoIV * 100 << "%" << std::endl;
        
        // Test specific methods
        double volHistorical = core::VolatilityEstimator::getVolatility(
            dataNoIV, priceSeries, core::VolatilityEstimator::VolatilityMethod::HISTORICAL, 30);
        
        double volEWMA = core::VolatilityEstimator::getVolatility(
            dataNoIV, priceSeries, core::VolatilityEstimator::VolatilityMethod::EWMA, 30);
        
        std::cout << "Historical Method: " << volHistorical * 100 << "%" << std::endl;
        std::cout << "EWMA Method: " << volEWMA * 100 << "%" << std::endl;
        
        std::cout << "\n=== Implied Volatility Estimation ===" << std::endl;
        
        double estimatedIV30 = core::VolatilityEstimator::estimateImpliedVolatility(
            "TEST", priceSeries, 0.0833, core::VolatilityEstimator::VolatilityMethod::HISTORICAL); // ~30 days
        
        double estimatedIV7 = core::VolatilityEstimator::estimateImpliedVolatility(
            "TEST", priceSeries, 0.0192, core::VolatilityEstimator::VolatilityMethod::HISTORICAL); // ~7 days
        
        std::cout << "Estimated IV (30 days to expiry): " << estimatedIV30 * 100 << "%" << std::endl;
        std::cout << "Estimated IV (7 days to expiry): " << estimatedIV7 * 100 << "%" << std::endl;
        
        std::cout << "\n=== Edge Cases Tests ===" << std::endl;
        
        try {
            // Test with very short series
            core::TimeSeries shortSeries;
            shortSeries.addDataPoint(timestamps[0], prices[0]);
            
            core::VolatilityEstimator::calculateHistoricalVolatility(shortSeries, 30, true);
            std::cout << "❌ Should have thrown exception for short series" << std::endl;
        } catch (const std::exception& e) {
            std::cout << "✅ Correctly caught exception for short series: " << e.what() << std::endl;
        }
        
        try {
            // Test with invalid lambda
            core::VolatilityEstimator::calculateEWMAVolatility(priceSeries, 1.5, true);
            std::cout << "❌ Should have thrown exception for invalid lambda" << std::endl;
        } catch (const std::exception& e) {
            std::cout << "✅ Correctly caught exception for invalid lambda: " << e.what() << std::endl;
        }
        
        // Test bounds checking
        core::MarketData extremeIVData("TEST", currentTime, 99.0, 101.0, 98.0, 100.0, 50000);
        extremeIVData.setAdditionalData("implied_volatility", 5.0); // 500% - should be capped
        
        double boundedVol = core::VolatilityEstimator::getVolatility(
            extremeIVData, priceSeries, core::VolatilityEstimator::VolatilityMethod::IMPLIED_FALLBACK, 30);
        
        std::cout << "Extreme IV test (500% input): " << boundedVol * 100 << "% (should be <= 300%)" << std::endl;
        
        std::cout << "\n✅ VolatilityEstimator tests completed successfully!" << std::endl;
        
    } catch (const std::exception& e) {
        std::cerr << "❌ Error: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}
