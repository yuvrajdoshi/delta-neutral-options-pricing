#include "models/GARCHModel.h"
#include "core/TimeSeries.h"
#include "core/DateTime.h"
#include <iostream>
#include <iomanip>
#include <vector>
#include <random>

int main() {
    std::cout << "Testing GARCH Model...\n" << std::endl;
    
    try {
        // Create sample return data
        std::vector<core::DateTime> timestamps;
        std::vector<double> returns;
        
        // Generate 100 days of sample returns with volatility clustering
        core::DateTime startDate(2025, 1, 1);
        std::random_device rd;
        std::mt19937 gen(42); // Fixed seed for reproducible results
        std::normal_distribution<double> normalDist(0.0, 1.0);
        
        double variance = 0.01; // Initial variance
        
        for (int i = 0; i < 100; ++i) {
            core::TimeDelta delta(i);
            timestamps.push_back(startDate + delta);
            
            // Generate return with time-varying volatility (simple GARCH simulation)
            double epsilon = normalDist(gen);
            double returnValue = std::sqrt(variance) * epsilon;
            returns.push_back(returnValue);
            
            // Update variance for next period (simple GARCH process)
            variance = 0.0001 + 0.1 * returnValue * returnValue + 0.8 * variance;
        }
        
        core::TimeSeries returnSeries(timestamps, returns, "TestReturns");
        
        std::cout << "=== Sample Data ===" << std::endl;
        std::cout << "Number of returns: " << returnSeries.size() << std::endl;
        std::cout << "Mean return: " << std::fixed << std::setprecision(6) << returnSeries.mean() << std::endl;
        std::cout << "Return volatility: " << returnSeries.standardDeviation() << std::endl;
        
        // Test GARCH model creation
        std::cout << "\n=== GARCH Model Creation ===" << std::endl;
        models::GARCHModel garch1; // Default constructor
        std::cout << "Default GARCH model created" << std::endl;
        std::cout << "Model name: " << garch1.getModelName() << std::endl;
        std::cout << "Is calibrated: " << (garch1.isCalibrated() ? "Yes" : "No") << std::endl;
        
        // Test with specific parameters
        models::GARCHModel garch2(0.0001, 0.1, 0.8);
        std::cout << "\nParameterized GARCH model created" << std::endl;
        std::cout << "Omega: " << garch2.getOmega() << std::endl;
        std::cout << "Alpha: " << garch2.getAlpha() << std::endl;
        std::cout << "Beta: " << garch2.getBeta() << std::endl;
        std::cout << "Is stationary: " << (garch2.isStationary() ? "Yes" : "No") << std::endl;
        
        // Test calibration
        std::cout << "\n=== GARCH Model Calibration ===" << std::endl;
        garch1.calibrate(returnSeries);
        std::cout << "Model calibrated successfully" << std::endl;
        std::cout << "Is calibrated: " << (garch1.isCalibrated() ? "Yes" : "No") << std::endl;
        
        auto params = garch1.getParameters();
        std::cout << "\nCalibrated Parameters:" << std::endl;
        for (const auto& param : params) {
            std::cout << "  " << param.first << ": " << std::setprecision(6) << param.second << std::endl;
        }
        
        // Test forecasting
        std::cout << "\n=== GARCH Forecasting ===" << std::endl;
        double forecast1 = garch1.forecast(1);
        double forecast5 = garch1.forecast(5);
        double forecast10 = garch1.forecast(10);
        
        std::cout << "1-day forecast: " << std::setprecision(4) << forecast1 * 100 << "%" << std::endl;
        std::cout << "5-day forecast: " << forecast5 * 100 << "%" << std::endl;
        std::cout << "10-day forecast: " << forecast10 * 100 << "%" << std::endl;
        
        // Test forecast series
        auto forecastSeries = garch1.forecastSeries(5);
        std::cout << "\nForecast Series (5 days):" << std::endl;
        for (size_t i = 0; i < forecastSeries.size(); ++i) {
            std::cout << "  Day " << (i+1) << ": " << forecastSeries.getValue(i) * 100 << "%" << std::endl;
        }
        
        // Test model validation
        std::cout << "\n=== Model Validation ===" << std::endl;
        double logLikelihood = garch1.calculateLogLikelihood(returnSeries);
        double aic = garch1.calculateAIC();
        double bic = garch1.calculateBIC();
        
        std::cout << "Log Likelihood: " << std::setprecision(2) << logLikelihood << std::endl;
        std::cout << "AIC: " << aic << std::endl;
        std::cout << "BIC: " << bic << std::endl;
        
        // Test cloning
        std::cout << "\n=== Clone Test ===" << std::endl;
        auto clonedModel = garch1.clone();
        std::cout << "Model cloned successfully" << std::endl;
        std::cout << "Original forecast: " << garch1.forecast(1) * 100 << "%" << std::endl;
        std::cout << "Cloned forecast: " << clonedModel->forecast(1) * 100 << "%" << std::endl;
        std::cout << "Forecasts match: " << (std::abs(garch1.forecast(1) - clonedModel->forecast(1)) < 1e-10 ? "Yes" : "No") << std::endl;
        
        // Test error handling
        std::cout << "\n=== Error Handling Tests ===" << std::endl;
        
        try {
            models::GARCHModel invalidModel(-0.1, 0.1, 0.8); // Negative omega
            std::cout << "❌ Should have caught negative omega error" << std::endl;
        } catch (const std::exception& e) {
            std::cout << "✅ Correctly caught omega error: " << e.what() << std::endl;
        }
        
        try {
            models::GARCHModel invalidModel2(0.0001, 0.5, 0.6); // Alpha + Beta >= 1
            std::cout << "❌ Should have caught stationarity error" << std::endl;
        } catch (const std::exception& e) {
            std::cout << "✅ Correctly caught stationarity error: " << e.what() << std::endl;
        }
        
        try {
            models::GARCHModel uncalibratedModel;
            double forecast = uncalibratedModel.forecast(1); // Should fail
            std::cout << "❌ Should have caught uncalibrated model error" << std::endl;
        } catch (const std::exception& e) {
            std::cout << "✅ Correctly caught uncalibrated error: " << e.what() << std::endl;
        }
        
        std::cout << "\n✅ GARCH Model tests completed successfully!" << std::endl;
        
    } catch (const std::exception& e) {
        std::cout << "❌ Error: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}
