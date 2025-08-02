#include <iostream>
#include <cassert>
#include <memory>
#include "strategy/Portfolio.h"
#include "strategy/Position.h"
#include "instruments/InstrumentFactory.h"
#include "core/MarketData.h"
#include "core/DateTime.h"

using namespace VolatilityArbitrage;
using namespace core;
using namespace instruments;

void testPortfolioConstructor() {
    std::cout << "Testing Portfolio constructor..." << std::endl;
    
    // Test default constructor
    Portfolio portfolio1;
    assert(portfolio1.getCash() == 0.0);
    assert(portfolio1.getPositionCount() == 0);
    
    // Test constructor with initial cash
    Portfolio portfolio2(50000.0);
    assert(portfolio2.getCash() == 50000.0);
    assert(portfolio2.getPositionCount() == 0);
    
    std::cout << "Portfolio1 cash: $" << portfolio1.getCash() << std::endl;
    std::cout << "Portfolio2 cash: $" << portfolio2.getCash() << std::endl;
    std::cout << "✓ Portfolio constructor tests passed!" << std::endl;
}

void testPortfolioAddPosition() {
    std::cout << "Testing Portfolio add position..." << std::endl;
    
    Portfolio portfolio(100000.0);
    
    // Create a position
    auto equity = InstrumentFactory::createEquity("AAPL");
    DateTime entryDate(2024, 1, 15, 10, 30, 0);
    Position position(std::move(equity), 100.0, 150.0, entryDate);
    
    // Add position to portfolio
    portfolio.addPosition(std::move(position));
    
    assert(portfolio.getPositionCount() == 1);
    assert(portfolio.getCash() == 100000.0); // Cash shouldn't change when adding position
    
    // Add another position
    auto equity2 = InstrumentFactory::createEquity("MSFT");
    Position position2(std::move(equity2), 50.0, 300.0, entryDate);
    portfolio.addPosition(std::move(position2));
    
    assert(portfolio.getPositionCount() == 2);
    
    std::cout << "Position count: " << portfolio.getPositionCount() << std::endl;
    std::cout << "✓ Portfolio add position tests passed!" << std::endl;
}

void testPortfolioCashOperations() {
    std::cout << "Testing Portfolio cash operations..." << std::endl;
    
    Portfolio portfolio(25000.0);
    
    // Test adding cash
    portfolio.addCash(5000.0);
    assert(portfolio.getCash() == 30000.0);
    
    // Test removing cash
    portfolio.removeCash(10000.0);
    assert(portfolio.getCash() == 20000.0);
    
    // Test adding more cash
    portfolio.addCash(15000.0);
    assert(portfolio.getCash() == 35000.0);
    
    std::cout << "Final cash: $" << portfolio.getCash() << std::endl;
    std::cout << "✓ Portfolio cash operations tests passed!" << std::endl;
}

void testPortfolioGetPosition() {
    std::cout << "Testing Portfolio get position..." << std::endl;
    
    Portfolio portfolio(100000.0);
    
    // Add positions
    auto equity1 = InstrumentFactory::createEquity("AAPL");
    auto equity2 = InstrumentFactory::createEquity("MSFT");
    DateTime entryDate(2024, 1, 15, 10, 30, 0);
    
    Position pos1(std::move(equity1), 100.0, 150.0, entryDate);
    Position pos2(std::move(equity2), 50.0, 300.0, entryDate);
    
    portfolio.addPosition(std::move(pos1));
    portfolio.addPosition(std::move(pos2));
    
    // Test getting position by index
    assert(portfolio.getPositionCount() == 2);
    
    const Position& firstPosition = portfolio.getPosition(0);
    assert(firstPosition.getQuantity() == 100.0);
    assert(firstPosition.getEntryPrice() == 150.0);
    
    const Position& secondPosition = portfolio.getPosition(1);
    assert(secondPosition.getQuantity() == 50.0);
    assert(secondPosition.getEntryPrice() == 300.0);
    
    std::cout << "First position quantity: " << firstPosition.getQuantity() << std::endl;
    std::cout << "Second position quantity: " << secondPosition.getQuantity() << std::endl;
    std::cout << "✓ Portfolio get position tests passed!" << std::endl;
}

void testPortfolioRemovePosition() {
    std::cout << "Testing Portfolio remove position..." << std::endl;
    
    Portfolio portfolio(100000.0);
    
    // Add positions
    auto equity1 = InstrumentFactory::createEquity("AAPL");
    auto equity2 = InstrumentFactory::createEquity("MSFT");
    DateTime entryDate(2024, 1, 15, 10, 30, 0);
    
    Position pos1(std::move(equity1), 100.0, 150.0, entryDate);
    Position pos2(std::move(equity2), 50.0, 300.0, entryDate);
    
    portfolio.addPosition(std::move(pos1));
    portfolio.addPosition(std::move(pos2));
    
    assert(portfolio.getPositionCount() == 2);
    
    // Remove first position (index 0)
    portfolio.removePosition(0);
    
    assert(portfolio.getPositionCount() == 1);
    
    // The remaining position should be the MSFT one (originally at index 1)
    const Position& remainingPosition = portfolio.getPosition(0);
    assert(remainingPosition.getQuantity() == 50.0);
    assert(remainingPosition.getEntryPrice() == 300.0);
    
    // Remove the last position
    portfolio.removePosition(0);
    
    assert(portfolio.getPositionCount() == 0);
    
    std::cout << "Final position count: " << portfolio.getPositionCount() << std::endl;
    std::cout << "✓ Portfolio remove position tests passed!" << std::endl;
}

void testPortfolioTotalValue() {
    std::cout << "Testing Portfolio total value..." << std::endl;
    
    Portfolio portfolio(50000.0);
    
    // Add only AAPL position to avoid symbol mismatch
    auto equity1 = InstrumentFactory::createEquity("AAPL");
    DateTime entryDate(2024, 1, 15, 10, 30, 0);
    
    Position pos1(std::move(equity1), 100.0, 150.0, entryDate); // 100 shares at $150
    
    portfolio.addPosition(std::move(pos1));
    
    // Create market data for AAPL
    DateTime currentDate(2024, 1, 16, 10, 30, 0);
    MarketData aaplData("AAPL", currentDate, 155.0, 156.0, 154.0, 155.0, 1000000);
    
    // Test total value calculation
    double totalValue = portfolio.getTotalValue(aaplData);
    
    // Expected: cash (50000) + position value (100 * 155 = 15500) = 65500
    double expectedValue = 50000.0 + (100.0 * 155.0);
    assert(totalValue == expectedValue);
    
    std::cout << "Portfolio total value: $" << totalValue << std::endl;
    std::cout << "Portfolio cash: $" << portfolio.getCash() << std::endl;
    
    std::cout << "✓ Portfolio total value tests passed!" << std::endl;
}

void testPortfolioPnL() {
    std::cout << "Testing Portfolio P&L calculation..." << std::endl;
    
    Portfolio portfolio(100000.0);
    
    // Add a position
    auto equity = InstrumentFactory::createEquity("AAPL");
    DateTime entryDate(2024, 1, 15, 10, 30, 0);
    Position position(std::move(equity), 100.0, 150.0, entryDate); // 100 shares at $150
    
    portfolio.addPosition(std::move(position));
    
    // Create market data with price increase
    DateTime currentDate(2024, 1, 16, 10, 30, 0);
    MarketData data("AAPL", currentDate, 160.0, 161.0, 159.0, 160.0, 1000000);
    
    // Calculate P&L
    double totalPnL = portfolio.getTotalPnL(data);
    
    // Expected P&L: 100 shares * (160 - 150) = $1000
    assert(totalPnL == 1000.0);
    
    std::cout << "Total P&L: $" << totalPnL << std::endl;
    std::cout << "✓ Portfolio P&L tests passed!" << std::endl;
}

void testPortfolioDelta() {
    std::cout << "Testing Portfolio delta calculation..." << std::endl;
    
    Portfolio portfolio(100000.0);
    
    // Add an equity position (delta = quantity for equities)
    auto equity = InstrumentFactory::createEquity("AAPL");
    DateTime entryDate(2024, 1, 15, 10, 30, 0);
    Position position(std::move(equity), 100.0, 150.0, entryDate);
    
    portfolio.addPosition(std::move(position));
    
    // Create market data
    DateTime currentDate(2024, 1, 16, 10, 30, 0);
    MarketData data("AAPL", currentDate, 160.0, 161.0, 159.0, 160.0, 1000000);
    data.setAdditionalData("risk_free_rate", 0.05);
    
    // Calculate delta
    double delta = portfolio.calculateDelta(data);
    
    // For equity, delta should equal the quantity
    assert(delta == 100.0);
    
    std::cout << "Portfolio delta: " << delta << std::endl;
    std::cout << "✓ Portfolio delta tests passed!" << std::endl;
}

void testPortfolioCopyConstructor() {
    std::cout << "Testing Portfolio copy constructor..." << std::endl;
    
    Portfolio original(75000.0);
    
    // Add position to original
    auto equity = InstrumentFactory::createEquity("AAPL");
    DateTime entryDate(2024, 1, 15, 10, 30, 0);
    Position position(std::move(equity), 100.0, 150.0, entryDate);
    original.addPosition(std::move(position));
    
    // Copy portfolio
    Portfolio copy(original);
    
    assert(copy.getCash() == 75000.0);
    assert(copy.getPositionCount() == 1);
    
    // Test that positions are copied
    const Position& originalPos = original.getPosition(0);
    const Position& copiedPos = copy.getPosition(0);
    assert(originalPos.getQuantity() == copiedPos.getQuantity());
    assert(originalPos.getEntryPrice() == copiedPos.getEntryPrice());
    
    // Verify positions are independent by modifying cash
    copy.addCash(-25000.0);  // Remove cash from copy
    assert(original.getCash() == 75000.0);
    assert(copy.getCash() == 50000.0);
    
    std::cout << "Original cash: $" << original.getCash() << std::endl;
    std::cout << "Copy cash: $" << copy.getCash() << std::endl;
    std::cout << "✓ Portfolio copy constructor tests passed!" << std::endl;
}

int main() {
    std::cout << "Running Portfolio tests..." << std::endl;
    
    try {
        testPortfolioConstructor();
        testPortfolioAddPosition();
        testPortfolioCashOperations();
        testPortfolioGetPosition();
        testPortfolioRemovePosition();
        testPortfolioTotalValue();
        testPortfolioPnL();
        testPortfolioDelta();
        testPortfolioCopyConstructor();
        
        std::cout << "\n✅ All Portfolio tests passed!" << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "❌ Test failed: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}
