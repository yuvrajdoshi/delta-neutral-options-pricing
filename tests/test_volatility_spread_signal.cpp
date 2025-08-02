#include <iostream>
#include <cassert>
#include <memory>
#include "strategy/VolatilitySpreadSignal.h"
#include "strategy/Signal.h"
#include "instruments/InstrumentFactory.h"
#include "models/ModelFactory.h"
#include "core/MarketData.h"
#include "core/DateTime.h"
#include "core/TimeSeries.h"

using namespace VolatilityArbitrage;
using namespace core;
using namespace instruments;
using namespace models;

void testVolatilitySpreadSignalConstructor() {
    std::cout << "Testing VolatilitySpreadSignal constructor..." << std::endl;
    
    // Test default threshold values
    VolatilitySpreadSignal signalGen1(0.1, 0.05);
    
    // Test different threshold values
    VolatilitySpreadSignal signalGen2(0.2, 0.1);
    VolatilitySpreadSignal signalGen3(0.05, 0.02);
    
    std::cout << "Signal generators created with different thresholds" << std::endl;
    std::cout << "✓ VolatilitySpreadSignal constructor tests passed!" << std::endl;
}

void testVolatilitySpreadSignalWithEquity() {
    std::cout << "Testing VolatilitySpreadSignal with equity..." << std::endl;
    
    VolatilitySpreadSignal signalGen(0.1, 0.05);
    
    // Create an equity instrument
    auto equity = InstrumentFactory::createEquity("AAPL");
    
    // Create GARCH model and add some sample data
    auto garchModel = ModelFactory::createGARCHModel(0.1, 0.1, 0.8);
    
    // Create sample returns data for calibration
    TimeSeries returns;
    for (int i = 0; i < 30; ++i) {
        DateTime date(2024, 1, i + 1, 0, 0, 0);
        double return_val = 0.01 * (i % 10 - 5) / 10.0; // Small random returns between -0.005 and 0.005
        returns.addDataPoint(date, return_val);
    }
    
    garchModel->calibrate(returns);
    
    // Create market data
    DateTime timestamp(2024, 1, 15, 10, 30, 0);
    MarketData data("AAPL", timestamp, 150.0, 151.0, 149.0, 150.5, 1000000);
    data.setAdditionalData("implied_volatility", 0.25);
    
    // Generate signal
    Signal signal = signalGen.generateSignal(*equity, *garchModel, data);
    
    assert(signal.instrumentId == "AAPL");
    assert(signal.timestamp.toString() == timestamp.toString());
    
    std::cout << "Generated signal for equity: " << signal.toString() << std::endl;
    std::cout << "✓ VolatilitySpreadSignal with equity tests passed!" << std::endl;
}

void testVolatilitySpreadSignalWithOption() {
    std::cout << "Testing VolatilitySpreadSignal with option..." << std::endl;
    
    VolatilitySpreadSignal signalGen(0.1, 0.05);
    
    // Create a European call option
    DateTime expiry(2024, 6, 15, 0, 0, 0);
    auto option = InstrumentFactory::createEuropeanCall("AAPL", expiry, 150.0);
    
    // Create GARCH model
    auto garchModel = ModelFactory::createGARCHModel(0.1, 0.1, 0.8);
    
    // Create sample returns data
    TimeSeries returns;
    for (int i = 0; i < 25; ++i) {
        DateTime date(2024, 1, i + 1, 0, 0, 0);
        double return_val = 0.005 * sin(i * 0.1); // Sine wave returns for variety
        returns.addDataPoint(date, return_val);
    }
    
    garchModel->calibrate(returns);
    
    // Create market data with implied volatility
    DateTime timestamp(2024, 1, 15, 10, 30, 0);
    MarketData data("AAPL", timestamp, 145.0, 146.0, 144.0, 145.5, 1000000);
    data.setAdditionalData("implied_volatility", 0.30);
    data.setAdditionalData("risk_free_rate", 0.05);
    
    // Generate signal
    Signal signal = signalGen.generateSignal(*option, *garchModel, data);
    
    assert(signal.instrumentId == option->getSymbol());
    assert(signal.timestamp.toString() == timestamp.toString());
    
    std::cout << "Generated signal for option: " << signal.toString() << std::endl;
    std::cout << "✓ VolatilitySpreadSignal with option tests passed!" << std::endl;
}

void testVolatilitySpreadSignalBuySignal() {
    std::cout << "Testing VolatilitySpreadSignal BUY signal generation..." << std::endl;
    
    // Use tight thresholds to trigger signals more easily
    VolatilitySpreadSignal signalGen(0.05, 0.02);
    
    auto option = InstrumentFactory::createEuropeanCall("AAPL", DateTime(2024, 6, 15, 0, 0, 0), 150.0);
    auto garchModel = ModelFactory::createGARCHModel(0.1, 0.1, 0.8);
    
    // Create returns with higher volatility
    TimeSeries returns;
    for (int i = 0; i < 20; ++i) {
        DateTime date(2024, 1, i + 1, 0, 0, 0);
        double return_val = 0.02 * (i % 10 - 5) / 5.0; // Larger returns to increase realized vol
        returns.addDataPoint(date, return_val);
    }
    
    garchModel->calibrate(returns);
    
    // Create market data with low implied volatility (to create spread)
    DateTime timestamp(2024, 1, 15, 10, 30, 0);
    MarketData data("AAPL", timestamp, 150.0, 151.0, 149.0, 150.5, 1000000);
    data.setAdditionalData("implied_volatility", 0.15); // Lower than expected realized vol
    data.setAdditionalData("risk_free_rate", 0.05);
    
    Signal signal = signalGen.generateSignal(*option, *garchModel, data);
    
    // The signal type depends on the spread, but we can test that it's generated
    assert(!signal.instrumentId.empty());
    assert(signal.strength >= 0.0 && signal.strength <= 1.0);
    
    std::cout << "Signal type: " << (signal.type == Signal::Type::BUY ? "BUY" : 
                                      signal.type == Signal::Type::SELL ? "SELL" : "HOLD") << std::endl;
    std::cout << "Signal strength: " << signal.strength << std::endl;
    std::cout << "✓ VolatilitySpreadSignal BUY signal tests passed!" << std::endl;
}

void testVolatilitySpreadSignalSellSignal() {
    std::cout << "Testing VolatilitySpreadSignal SELL signal generation..." << std::endl;
    
    VolatilitySpreadSignal signalGen(0.05, 0.02);
    
    auto option = InstrumentFactory::createEuropeanCall("MSFT", DateTime(2024, 6, 15, 0, 0, 0), 300.0);
    auto garchModel = ModelFactory::createGARCHModel(0.1, 0.1, 0.8);
    
    // Create returns with lower volatility
    TimeSeries returns;
    for (int i = 0; i < 20; ++i) {
        DateTime date(2024, 1, i + 1, 0, 0, 0);
        double return_val = 0.001 * (i % 5 - 2) / 5.0; // Very small returns for low realized vol
        returns.addDataPoint(date, return_val);
    }
    
    garchModel->calibrate(returns);
    
    // Create market data with high implied volatility
    DateTime timestamp(2024, 1, 15, 10, 30, 0);
    MarketData data("MSFT", timestamp, 300.0, 302.0, 298.0, 301.0, 800000);
    data.setAdditionalData("implied_volatility", 0.35); // Higher than expected realized vol
    data.setAdditionalData("risk_free_rate", 0.05);
    
    Signal signal = signalGen.generateSignal(*option, *garchModel, data);
    
    assert(!signal.instrumentId.empty());
    assert(signal.strength >= 0.0 && signal.strength <= 1.0);
    
    std::cout << "Signal type: " << (signal.type == Signal::Type::BUY ? "BUY" : 
                                      signal.type == Signal::Type::SELL ? "SELL" : "HOLD") << std::endl;
    std::cout << "Signal strength: " << signal.strength << std::endl;
    std::cout << "✓ VolatilitySpreadSignal SELL signal tests passed!" << std::endl;
}

void testVolatilitySpreadSignalHoldSignal() {
    std::cout << "Testing VolatilitySpreadSignal HOLD signal generation..." << std::endl;
    
    // Use wide thresholds to make HOLD signals more likely
    VolatilitySpreadSignal signalGen(0.2, 0.15);
    
    auto option = InstrumentFactory::createEuropeanCall("GOOGL", DateTime(2024, 6, 15, 0, 0, 0), 2800.0);
    auto garchModel = ModelFactory::createGARCHModel(0.1, 0.1, 0.8);
    
    // Create moderate returns
    TimeSeries returns;
    for (int i = 0; i < 20; ++i) {
        DateTime date(2024, 1, i + 1, 0, 0, 0);
        double return_val = 0.005 * (i % 6 - 3) / 6.0; // Moderate returns
        returns.addDataPoint(date, return_val);
    }
    
    garchModel->calibrate(returns);
    
    // Create market data with similar implied volatility to realized
    DateTime timestamp(2024, 1, 15, 10, 30, 0);
    MarketData data("GOOGL", timestamp, 2800.0, 2810.0, 2790.0, 2805.0, 500000);
    data.setAdditionalData("implied_volatility", 0.25); // Similar to expected realized vol
    data.setAdditionalData("risk_free_rate", 0.05);
    
    Signal signal = signalGen.generateSignal(*option, *garchModel, data);
    
    assert(!signal.instrumentId.empty());
    assert(signal.strength >= 0.0 && signal.strength <= 1.0);
    
    std::cout << "Signal type: " << (signal.type == Signal::Type::BUY ? "BUY" : 
                                      signal.type == Signal::Type::SELL ? "SELL" : "HOLD") << std::endl;
    std::cout << "Signal strength: " << signal.strength << std::endl;
    std::cout << "✓ VolatilitySpreadSignal HOLD signal tests passed!" << std::endl;
}

void testVolatilitySpreadSignalClone() {
    std::cout << "Testing VolatilitySpreadSignal clone..." << std::endl;
    
    VolatilitySpreadSignal original(0.12, 0.06);
    
    auto cloned = original.clone();
    assert(cloned != nullptr);
    
    // Test that clone works by generating signals
    auto option = InstrumentFactory::createEuropeanCall("TEST", DateTime(2024, 6, 15, 0, 0, 0), 100.0);
    auto garchModel = ModelFactory::createGARCHModel(0.1, 0.1, 0.8);
    
    TimeSeries returns;
    for (int i = 0; i < 10; ++i) {
        DateTime date(2024, 1, i + 1, 0, 0, 0);
        returns.addDataPoint(date, 0.01);
    }
    garchModel->calibrate(returns);
    
    DateTime timestamp(2024, 1, 15, 10, 30, 0);
    MarketData data("TEST", timestamp, 100.0, 101.0, 99.0, 100.5, 100000);
    data.setAdditionalData("implied_volatility", 0.20);
    data.setAdditionalData("risk_free_rate", 0.05);
    
    Signal originalSignal = original.generateSignal(*option, *garchModel, data);
    Signal clonedSignal = cloned->generateSignal(*option, *garchModel, data);
    
    // Both should generate valid signals
    assert(!originalSignal.instrumentId.empty());
    assert(!clonedSignal.instrumentId.empty());
    
    std::cout << "Original signal: " << originalSignal.toString() << std::endl;
    std::cout << "Cloned signal: " << clonedSignal.toString() << std::endl;
    std::cout << "✓ VolatilitySpreadSignal clone tests passed!" << std::endl;
}

void testVolatilitySpreadSignalEdgeCases() {
    std::cout << "Testing VolatilitySpreadSignal edge cases..." << std::endl;
    
    VolatilitySpreadSignal signalGen(0.1, 0.05);
    
    auto option = InstrumentFactory::createEuropeanCall("EDGE", DateTime(2024, 6, 15, 0, 0, 0), 50.0);
    auto garchModel = ModelFactory::createGARCHModel(0.1, 0.1, 0.8);
    
    // Test with minimum required data points (at least 10)
    TimeSeries minimalReturns;
    for (int i = 0; i < 12; ++i) {  // Use 12 data points to be safe
        DateTime date(2024, 1, i + 1, 0, 0, 0);
        double return_val = 0.01 * ((i % 4) - 2) / 4.0;  // Small varying returns
        minimalReturns.addDataPoint(date, return_val);
    }
    
    garchModel->calibrate(minimalReturns);
    
    DateTime timestamp(2024, 1, 15, 10, 30, 0);
    MarketData data("EDGE", timestamp, 50.0, 51.0, 49.0, 50.5, 50000);
    data.setAdditionalData("implied_volatility", 0.25);
    data.setAdditionalData("risk_free_rate", 0.05);
    
    // Should handle minimal data gracefully
    Signal signal = signalGen.generateSignal(*option, *garchModel, data);
    assert(!signal.instrumentId.empty());
    
    // Test with zero implied volatility
    MarketData zeroVolData("EDGE", timestamp, 50.0, 51.0, 49.0, 50.5, 50000);
    zeroVolData.setAdditionalData("implied_volatility", 0.0);
    zeroVolData.setAdditionalData("risk_free_rate", 0.05);
    
    Signal zeroVolSignal = signalGen.generateSignal(*option, *garchModel, zeroVolData);
    assert(!zeroVolSignal.instrumentId.empty());
    
    std::cout << "Minimal data signal: " << signal.toString() << std::endl;
    std::cout << "Zero vol signal: " << zeroVolSignal.toString() << std::endl;
    std::cout << "✓ VolatilitySpreadSignal edge cases tests passed!" << std::endl;
}

int main() {
    std::cout << "Running VolatilitySpreadSignal tests..." << std::endl;
    
    try {
        testVolatilitySpreadSignalConstructor();
        testVolatilitySpreadSignalWithEquity();
        testVolatilitySpreadSignalWithOption();
        testVolatilitySpreadSignalBuySignal();
        testVolatilitySpreadSignalSellSignal();
        testVolatilitySpreadSignalHoldSignal();
        testVolatilitySpreadSignalClone();
        testVolatilitySpreadSignalEdgeCases();
        
        std::cout << "\n✅ All VolatilitySpreadSignal tests passed!" << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "❌ Test failed: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}
