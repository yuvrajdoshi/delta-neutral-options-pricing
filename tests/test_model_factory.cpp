#include "models/ModelFactory.h"
#include "models/GARCHModel.h"
#include "models/BSMPricingModel.h"
#include "instruments/Option.h"
#include "core/MarketData.h"
#include "core/DateTime.h"
#include "core/TimeSeries.h"
#include <iostream>
#include <iomanip>
#include <vector>
#include <cmath>

int main() {
    std::cout << "Testing Model Factory...\n" << std::endl;
    
    try {
        // Test GARCH model creation
        std::cout << "=== GARCH Model Factory Tests ===" << std::endl;
        
        // Test default GARCH model
        auto defaultGarch = models::ModelFactory::createGARCHModel();
        std::cout << "Default GARCH created: " << defaultGarch->getModelName() << std::endl;
        std::cout << "Is calibrated: " << (dynamic_cast<models::GARCHModel*>(defaultGarch.get())->isCalibrated() ? "Yes" : "No") << std::endl;
        
        // Test parameterized GARCH model
        auto paramGarch = models::ModelFactory::createGARCHModel(0.0001, 0.1, 0.8);
        std::cout << "\nParameterized GARCH created" << std::endl;
        auto garchPtr = dynamic_cast<models::GARCHModel*>(paramGarch.get());
        std::cout << "Omega: " << garchPtr->getOmega() << std::endl;
        std::cout << "Alpha: " << garchPtr->getAlpha() << std::endl;
        std::cout << "Beta: " << garchPtr->getBeta() << std::endl;
        std::cout << "Is stationary: " << (garchPtr->isStationary() ? "Yes" : "No") << std::endl;
        
        // Test BSM model creation
        std::cout << "\n=== BSM Pricing Model Factory Tests ===" << std::endl;
        auto bsmModel = models::ModelFactory::createBSMPricingModel();
        std::cout << "BSM model created: " << bsmModel->getModelName() << std::endl;
        
        // Test parameter validation
        std::cout << "\n=== Parameter Validation Tests ===" << std::endl;
        
        // Valid parameters
        bool valid1 = models::ModelFactory::validateGARCHParameters(0.0001, 0.1, 0.8);
        std::cout << "Valid params (0.0001, 0.1, 0.8): " << (valid1 ? "✅ PASS" : "❌ FAIL") << std::endl;
        
        // Default parameters (all zeros)
        bool valid2 = models::ModelFactory::validateGARCHParameters(0.0, 0.0, 0.0);
        std::cout << "Default params (0.0, 0.0, 0.0): " << (valid2 ? "✅ PASS" : "❌ FAIL") << std::endl;
        
        // Invalid - negative omega
        bool invalid1 = models::ModelFactory::validateGARCHParameters(-0.1, 0.1, 0.8);
        std::cout << "Invalid omega (-0.1, 0.1, 0.8): " << (invalid1 ? "❌ FAIL" : "✅ PASS") << std::endl;
        
        // Invalid - alpha >= 1
        bool invalid2 = models::ModelFactory::validateGARCHParameters(0.0001, 1.1, 0.8);
        std::cout << "Invalid alpha (0.0001, 1.1, 0.8): " << (invalid2 ? "❌ FAIL" : "✅ PASS") << std::endl;
        
        // Invalid - non-stationary (alpha + beta >= 1)
        bool invalid3 = models::ModelFactory::validateGARCHParameters(0.0001, 0.5, 0.6);
        std::cout << "Non-stationary (0.0001, 0.5, 0.6): " << (invalid3 ? "❌ FAIL" : "✅ PASS") << std::endl;
        
        // Test error handling in factory
        std::cout << "\n=== Factory Error Handling ===" << std::endl;
        
        try {
            auto invalidModel = models::ModelFactory::createGARCHModel(-0.1, 0.1, 0.8);
            std::cout << "❌ Should have caught invalid parameters" << std::endl;
        } catch (const std::exception& e) {
            std::cout << "✅ Correctly caught error: " << e.what() << std::endl;
        }
        
        // Test polymorphic usage
        std::cout << "\n=== Polymorphic Usage Test ===" << std::endl;
        
        // Create models using factory
        std::vector<std::unique_ptr<models::VolatilityModel>> volModels;
        volModels.push_back(models::ModelFactory::createGARCHModel(0.0001, 0.05, 0.9));
        volModels.push_back(models::ModelFactory::createGARCHModel(0.0002, 0.1, 0.85));
        
        std::vector<std::unique_ptr<models::PricingModel>> pricingModels;
        pricingModels.push_back(models::ModelFactory::createBSMPricingModel());
        pricingModels.push_back(models::ModelFactory::createBSMPricingModel());
        
        std::cout << "Created " << volModels.size() << " volatility models:" << std::endl;
        for (size_t i = 0; i < volModels.size(); ++i) {
            std::cout << "  Model " << (i+1) << ": " << volModels[i]->getModelName() << std::endl;
        }
        
        std::cout << "\nCreated " << pricingModels.size() << " pricing models:" << std::endl;
        for (size_t i = 0; i < pricingModels.size(); ++i) {
            std::cout << "  Model " << (i+1) << ": " << pricingModels[i]->getModelName() << std::endl;
        }
        
        // Test actual functionality with created models
        std::cout << "\n=== Functional Test with Factory Models ===" << std::endl;
        
        // Create test data for volatility model
        std::vector<core::DateTime> timestamps;
        std::vector<double> returns;
        
        core::DateTime startDate(2025, 1, 1);
        for (int i = 0; i < 50; ++i) {
            core::TimeDelta delta(i);
            timestamps.push_back(startDate + delta);
            returns.push_back(0.01 * std::sin(i * 0.1) + 0.005 * (i % 3 - 1)); // Simple synthetic returns
        }
        
        core::TimeSeries returnSeries(timestamps, returns, "TestReturns");
        
        // Test volatility model
        auto testGarch = models::ModelFactory::createGARCHModel(0.0001, 0.1, 0.8);
        testGarch->calibrate(returnSeries);
        double forecast = testGarch->forecast(1);
        std::cout << "GARCH 1-day forecast: " << std::fixed << std::setprecision(4) << forecast * 100 << "%" << std::endl;
        
        // Test pricing model
        auto testBSM = models::ModelFactory::createBSMPricingModel();
        
        core::DateTime timestamp(2025, 7, 29, 10, 0, 0);
        core::DateTime expiry(2025, 8, 29, 16, 0, 0);
        core::MarketData data("AAPL", timestamp, 150.0, 155.0, 149.0, 153.0, 1000000.0);
        data.setAdditionalData("implied_volatility", 0.25);
        
        instruments::EuropeanOption testOption("AAPL", expiry, 153.0, instruments::OptionType::Call);
        double optionPrice = testBSM->price(testOption, data);
        std::cout << "BSM option price: $" << optionPrice << std::endl;
        
        // Test cloning through factory models
        std::cout << "\n=== Clone Test through Factory ===" << std::endl;
        auto clonedGarch = testGarch->clone();
        auto clonedBSM = testBSM->clone();
        
        std::cout << "Original GARCH forecast: " << testGarch->forecast(1) * 100 << "%" << std::endl;
        std::cout << "Cloned GARCH forecast: " << clonedGarch->forecast(1) * 100 << "%" << std::endl;
        std::cout << "GARCH forecasts match: " << (std::abs(testGarch->forecast(1) - clonedGarch->forecast(1)) < 1e-10 ? "✅ PASS" : "❌ FAIL") << std::endl;
        
        double originalPrice = testBSM->price(testOption, data);
        double clonedPrice = clonedBSM->price(testOption, data);
        std::cout << "Original BSM price: $" << originalPrice << std::endl;
        std::cout << "Cloned BSM price: $" << clonedPrice << std::endl;
        std::cout << "BSM prices match: " << (std::abs(originalPrice - clonedPrice) < 1e-10 ? "✅ PASS" : "❌ FAIL") << std::endl;
        
        // Test model collections
        std::cout << "\n=== Model Collection Test ===" << std::endl;
        
        // Create a collection of different models
        std::vector<std::unique_ptr<models::VolatilityModel>> modelPortfolio;
        modelPortfolio.push_back(models::ModelFactory::createGARCHModel(0.0001, 0.05, 0.90));
        modelPortfolio.push_back(models::ModelFactory::createGARCHModel(0.0002, 0.10, 0.85));
        modelPortfolio.push_back(models::ModelFactory::createGARCHModel(0.0003, 0.15, 0.80));
        
        std::cout << "Model Portfolio Performance:" << std::endl;
        for (size_t i = 0; i < modelPortfolio.size(); ++i) {
            modelPortfolio[i]->calibrate(returnSeries);
            double forecast = modelPortfolio[i]->forecast(1);
            double aic = modelPortfolio[i]->calculateAIC();
            
            auto params = modelPortfolio[i]->getParameters();
            std::cout << "  Model " << (i+1) << ": Forecast=" << std::setprecision(3) << forecast*100 
                     << "%, AIC=" << std::setprecision(1) << aic 
                     << ", Alpha=" << std::setprecision(3) << params["alpha"] 
                     << ", Beta=" << params["beta"] << std::endl;
        }
        
        std::cout << "\n✅ Model Factory tests completed successfully!" << std::endl;
        
    } catch (const std::exception& e) {
        std::cout << "❌ Error: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}
