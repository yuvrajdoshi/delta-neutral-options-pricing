#include "models/BSMPricingModel.h"
#include "instruments/Option.h"
#include "core/MarketData.h"
#include "core/DateTime.h"
#include <iostream>
#include <iomanip>
#include <cmath>

int main() {
    std::cout << "Testing BSM Pricing Model...\n" << std::endl;
    
    try {
        // Create market data
        core::DateTime timestamp(2025, 7, 29, 10, 0, 0);
        core::MarketData data("AAPL", timestamp, 150.0, 155.0, 149.0, 153.0, 1000000.0);
        data.setAdditionalData("implied_volatility", 0.25); // 25% IV
        
        // Create options with different properties
        core::DateTime expiry1(2025, 8, 29, 16, 0, 0); // 1 month
        core::DateTime expiry2(2025, 10, 29, 16, 0, 0); // 3 months
        
        instruments::EuropeanOption callATM("AAPL", expiry1, 153.0, instruments::OptionType::Call);
        instruments::EuropeanOption putATM("AAPL", expiry1, 153.0, instruments::OptionType::Put);
        instruments::EuropeanOption callOTM("AAPL", expiry1, 160.0, instruments::OptionType::Call);
        instruments::EuropeanOption putOTM("AAPL", expiry1, 145.0, instruments::OptionType::Put);
        instruments::EuropeanOption callLongTerm("AAPL", expiry2, 153.0, instruments::OptionType::Call);
        
        // Create BSM pricing model
        std::cout << "=== BSM Pricing Model Creation ===" << std::endl;
        models::BSMPricingModel bsm;
        std::cout << "Model name: " << bsm.getModelName() << std::endl;
        
        // Test option pricing
        std::cout << "\n=== Option Pricing Tests ===" << std::endl;
        std::cout << std::fixed << std::setprecision(4);
        
        double callATMPrice = bsm.price(callATM, data);
        double putATMPrice = bsm.price(putATM, data);
        double callOTMPrice = bsm.price(callOTM, data);
        double putOTMPrice = bsm.price(putOTM, data);
        double callLongTermPrice = bsm.price(callLongTerm, data);
        
        std::cout << "ATM Call (Strike $153): $" << callATMPrice << std::endl;
        std::cout << "ATM Put (Strike $153): $" << putATMPrice << std::endl;
        std::cout << "OTM Call (Strike $160): $" << callOTMPrice << std::endl;
        std::cout << "OTM Put (Strike $145): $" << putOTMPrice << std::endl;
        std::cout << "Long-term Call (3 months): $" << callLongTermPrice << std::endl;
        
        // Verify put-call parity: C - P = S - K*e^(-rT)
        double S = data.getClose();
        double K = 153.0;
        double T = callATM.timeToExpiry(timestamp);
        double r = 0.05; // Assuming 5% risk-free rate
        
        double putCallParityLHS = callATMPrice - putATMPrice;
        double putCallParityRHS = S - K * std::exp(-r * T);
        double putCallParityDiff = std::abs(putCallParityLHS - putCallParityRHS);
        
        std::cout << "\n=== Put-Call Parity Verification ===" << std::endl;
        std::cout << "C - P = " << putCallParityLHS << std::endl;
        std::cout << "S - K*e^(-rT) = " << putCallParityRHS << std::endl;
        std::cout << "Difference: " << putCallParityDiff << " (should be ~0)" << std::endl;
        std::cout << "Put-Call Parity: " << (putCallParityDiff < 0.01 ? "✅ PASS" : "❌ FAIL") << std::endl;
        
        // Test Greeks calculation
        std::cout << "\n=== Greeks Calculation ===" << std::endl;
        models::Greeks callGreeks = bsm.calculateGreeks(callATM, data);
        models::Greeks putGreeks = bsm.calculateGreeks(putATM, data);
        
        std::cout << "ATM Call Greeks:" << std::endl;
        std::cout << "  Delta: " << std::setprecision(4) << callGreeks.delta << std::endl;
        std::cout << "  Gamma: " << callGreeks.gamma << std::endl;
        std::cout << "  Vega: " << callGreeks.vega << std::endl;
        std::cout << "  Theta: " << callGreeks.theta << std::endl;
        std::cout << "  Rho: " << callGreeks.rho << std::endl;
        
        std::cout << "\nATM Put Greeks:" << std::endl;
        std::cout << "  Delta: " << putGreeks.delta << std::endl;
        std::cout << "  Gamma: " << putGreeks.gamma << std::endl;
        std::cout << "  Vega: " << putGreeks.vega << std::endl;
        std::cout << "  Theta: " << putGreeks.theta << std::endl;
        std::cout << "  Rho: " << putGreeks.rho << std::endl;
        
        // Greeks string representation test
        std::cout << "\nGreeks toString: " << callGreeks.toString() << std::endl;
        
        // Test relationship: Call Delta - Put Delta = 1
        double deltaDiff = callGreeks.delta - putGreeks.delta;
        std::cout << "\n=== Greeks Relationships ===" << std::endl;
        std::cout << "Call Delta - Put Delta = " << deltaDiff << " (should be ~1.0)" << std::endl;
        std::cout << "Delta Relationship: " << (std::abs(deltaDiff - 1.0) < 0.01 ? "✅ PASS" : "❌ FAIL") << std::endl;
        
        // Gamma should be the same for calls and puts
        std::cout << "Call Gamma: " << callGreeks.gamma << std::endl;
        std::cout << "Put Gamma: " << putGreeks.gamma << std::endl;
        std::cout << "Gamma Relationship: " << (std::abs(callGreeks.gamma - putGreeks.gamma) < 0.001 ? "✅ PASS" : "❌ FAIL") << std::endl;
        
        // Test with different volatilities
        std::cout << "\n=== Volatility Sensitivity Test ===" << std::endl;
        core::MarketData lowVolData = data;
        lowVolData.setAdditionalData("implied_volatility", 0.15); // 15% IV
        
        core::MarketData highVolData = data;
        highVolData.setAdditionalData("implied_volatility", 0.35); // 35% IV
        
        double lowVolPrice = bsm.price(callATM, lowVolData);
        double highVolPrice = bsm.price(callATM, highVolData);
        
        std::cout << "Call price @ 15% vol: $" << lowVolPrice << std::endl;
        std::cout << "Call price @ 25% vol: $" << callATMPrice << std::endl;
        std::cout << "Call price @ 35% vol: $" << highVolPrice << std::endl;
        std::cout << "Volatility monotonicity: " << (lowVolPrice < callATMPrice && callATMPrice < highVolPrice ? "✅ PASS" : "❌ FAIL") << std::endl;
        
        // Test without implied volatility (fallback to default)
        std::cout << "\n=== Volatility Fallback Test ===" << std::endl;
        core::MarketData noIVData("AAPL", timestamp, 150.0, 155.0, 149.0, 153.0, 1000000.0);
        // No implied volatility set
        
        double fallbackPrice = bsm.price(callATM, noIVData);
        std::cout << "Price without IV (using default 20%): $" << fallbackPrice << std::endl;
        
        // Test expired option
        std::cout << "\n=== Expired Option Test ===" << std::endl;
        core::DateTime pastExpiry(2025, 6, 29, 16, 0, 0); // Past date
        instruments::EuropeanOption expiredCall("AAPL", pastExpiry, 150.0, instruments::OptionType::Call);
        instruments::EuropeanOption expiredPut("AAPL", pastExpiry, 160.0, instruments::OptionType::Put);
        
        double expiredCallPrice = bsm.price(expiredCall, data);
        double expiredPutPrice = bsm.price(expiredPut, data);
        
        std::cout << "Expired ITM Call (Strike $150): $" << expiredCallPrice << " (should be $3.00)" << std::endl;
        std::cout << "Expired ITM Put (Strike $160): $" << expiredPutPrice << " (should be $7.00)" << std::endl;
        
        // Verify intrinsic values
        double expectedCallIntrinsic = std::max(0.0, S - 150.0);
        double expectedPutIntrinsic = std::max(0.0, 160.0 - S);
        
        std::cout << "Call intrinsic value check: " << (std::abs(expiredCallPrice - expectedCallIntrinsic) < 0.01 ? "✅ PASS" : "❌ FAIL") << std::endl;
        std::cout << "Put intrinsic value check: " << (std::abs(expiredPutPrice - expectedPutIntrinsic) < 0.01 ? "✅ PASS" : "❌ FAIL") << std::endl;
        
        // Test cloning
        std::cout << "\n=== Clone Test ===" << std::endl;
        auto clonedModel = bsm.clone();
        double originalPrice = bsm.price(callATM, data);
        double clonedPrice = clonedModel->price(callATM, data);
        
        std::cout << "Original model price: $" << originalPrice << std::endl;
        std::cout << "Cloned model price: $" << clonedPrice << std::endl;
        std::cout << "Prices match: " << (std::abs(originalPrice - clonedPrice) < 1e-10 ? "✅ PASS" : "❌ FAIL") << std::endl;
        
        std::cout << "\n✅ BSM Pricing Model tests completed successfully!" << std::endl;
        
    } catch (const std::exception& e) {
        std::cout << "❌ Error: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}
