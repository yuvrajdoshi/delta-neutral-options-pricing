#include <iostream>
#include <cassert>
#include <memory>
#include "strategy/Signal.h"
#include "strategy/VolatilitySpreadSignal.h"
#include "strategy/Position.h"
#include "strategy/Portfolio.h"
#include "strategy/DeltaHedgingStrategy.h"
#include "instruments/InstrumentFactory.h"
#include "models/ModelFactory.h"
#include "core/MarketData.h"
#include "core/TimeSeries.h"

using namespace VolatilityArbitrage;
using namespace instruments;
using namespace models;
using namespace core;

void testSignal() {
    std::cout << "Testing Signal class..." << std::endl;
    
    DateTime now(2024, 1, 15, 10, 30, 0);
    Signal signal(Signal::Type::BUY, 0.8, "AAPL", now);
    
    assert(signal.type == Signal::Type::BUY);
    assert(signal.strength == 0.8);
    assert(signal.instrumentId == "AAPL");
    assert(signal.isActionable() == true);
    
    Signal holdSignal(Signal::Type::HOLD, 0.0, "MSFT", now);
    assert(holdSignal.isActionable() == false);
    
    std::cout << "Signal: " << signal.toString() << std::endl;
    std::cout << "✓ Signal tests passed!" << std::endl;
}

void testVolatilitySpreadSignal() {
    std::cout << "Testing VolatilitySpreadSignal class..." << std::endl;
    
    VolatilitySpreadSignal signalGen(0.1, 0.05);
    
    // Create a European call option
    DateTime expiry(2024, 6, 15, 0, 0, 0);
    auto option = InstrumentFactory::createEuropeanCall("AAPL", expiry, 150.0);
    
    // Create GARCH model
    auto garchModel = ModelFactory::createGARCHModel(0.1, 0.1, 0.8);
    
    // Create market data with implied volatility
    DateTime timestamp(2024, 1, 15, 10, 30, 0);
    MarketData data("AAPL", timestamp, 145.0, 146.0, 144.0, 145.5, 1000000);
    data.setAdditionalData("implied_volatility", 0.25);
    
    // Set up some returns data for GARCH model using TimeSeries
    TimeSeries returns;
    for (int i = 0; i < 20; ++i) {
        DateTime date(2024, 1, i + 1, 0, 0, 0);
        double return_val = 0.001 * (i % 10 - 5); // Small random returns
        returns.addDataPoint(date, return_val);
    }
    
    garchModel->calibrate(returns);
    
    Signal signal = signalGen.generateSignal(*option, *garchModel, data);
    
    assert(signal.instrumentId == option->getSymbol());
    std::cout << "Generated signal: " << signal.toString() << std::endl;
    std::cout << "✓ VolatilitySpreadSignal tests passed!" << std::endl;
}

void testPosition() {
    std::cout << "Testing Position class..." << std::endl;
    
    // Create an equity instrument (1 share by default)
    auto equity = InstrumentFactory::createEquity("AAPL");
    DateTime entryDate(2024, 1, 15, 10, 30, 0);
    
    Position position(std::move(equity), 100.0, 145.0, entryDate);
    
    assert(position.getQuantity() == 100.0);
    assert(position.getEntryPrice() == 145.0);
    assert(position.getEntryDate().toString() == entryDate.toString());
    
    // Test copy constructor
    Position copyPosition(position);
    assert(copyPosition.getQuantity() == 100.0);
    assert(copyPosition.getEntryPrice() == 145.0);
    
    // Test valuation
    DateTime currentDate(2024, 1, 16, 10, 30, 0);
    MarketData data("AAPL", currentDate, 150.0, 151.0, 149.0, 150.0, 1000000);
    
    double value = position.getValue(data);
    double pnl = position.getPnL(data);
    
    std::cout << "Position value: " << value << std::endl;
    std::cout << "Position P&L: " << pnl << std::endl;
    
    assert(pnl == 100.0 * (150.0 - 145.0)); // 500.0
    
    // Test metadata
    position.setMetadata("test_key", 42.0);
    assert(position.hasMetadata("test_key"));
    assert(position.getMetadata("test_key") == 42.0);
    
    std::cout << "✓ Position tests passed!" << std::endl;
}

void testPortfolio() {
    std::cout << "Testing Portfolio class..." << std::endl;
    
    Portfolio portfolio(10000.0); // Start with $10,000 cash
    
    // Create positions (1 share each by default)
    auto equity1 = InstrumentFactory::createEquity("AAPL");
    
    DateTime entryDate(2024, 1, 15, 10, 30, 0);
    Position pos1(std::move(equity1), 100.0, 145.0, entryDate);  // 100 shares of AAPL
    
    portfolio.addPosition(std::move(pos1));
    
    assert(portfolio.getPositionCount() == 1);
    assert(portfolio.getCash() == 10000.0);
    
    // Test portfolio valuation
    DateTime currentDate(2024, 1, 16, 10, 30, 0);
    MarketData dataAAPL("AAPL", currentDate, 150.0, 151.0, 149.0, 150.0, 1000000);
    
    // For AAPL position
    double totalPnL = portfolio.getTotalPnL(dataAAPL);
    std::cout << "Portfolio P&L: " << totalPnL << std::endl;
    
    std::cout << "✓ Portfolio tests passed!" << std::endl;
}

void testDeltaHedgingStrategy() {
    std::cout << "Testing DeltaHedgingStrategy class..." << std::endl;
    
    DeltaHedgingStrategy hedgeStrategy(0.0, 0.01); // Target delta-neutral with 1% tolerance
    
    Portfolio portfolio(10000.0);
    
    // Create an option position
    DateTime expiry(2024, 6, 15, 0, 0, 0);
    auto option = InstrumentFactory::createEuropeanCall("AAPL", expiry, 150.0);
    
    DateTime entryDate(2024, 1, 15, 10, 30, 0);
    Position optionPos(std::move(option), 10.0, 5.0, entryDate); // 10 contracts at $5 each
    
    portfolio.addPosition(std::move(optionPos));
    
    // Create market data
    DateTime currentDate(2024, 1, 16, 10, 30, 0);
    MarketData data("AAPL", currentDate, 150.0, 151.0, 149.0, 150.0, 1000000);
    data.setAdditionalData("risk_free_rate", 0.05);
    data.setAdditionalData("implied_volatility", 0.25);
    
    std::cout << "Portfolio delta before hedge: " << portfolio.calculateDelta(data) << std::endl;
    
    // Apply hedge
    hedgeStrategy.applyHedge(portfolio, data);
    
    std::cout << "Portfolio delta after hedge: " << portfolio.calculateDelta(data) << std::endl;
    std::cout << "Position count after hedge: " << portfolio.getPositionCount() << std::endl;
    
    std::cout << "✓ DeltaHedgingStrategy tests passed!" << std::endl;
}

int main() {
    std::cout << "Running Strategy Layer tests..." << std::endl;
    
    try {
        testSignal();
        testVolatilitySpreadSignal();
        testPosition();
        testPortfolio();
        testDeltaHedgingStrategy();
        
        std::cout << "\n✅ All Strategy Layer tests passed!" << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "❌ Test failed: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}
