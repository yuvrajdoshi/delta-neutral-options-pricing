#include <iostream>
#include <cassert>
#include <memory>
#include "strategy/Signal.h"
#include "core/DateTime.h"

using namespace VolatilityArbitrage;
using namespace core;

void testSignalConstructor() {
    std::cout << "Testing Signal constructor..." << std::endl;
    
    DateTime timestamp(2024, 1, 15, 10, 30, 0);
    
    // Test BUY signal
    Signal buySignal(Signal::Type::BUY, 0.8, "AAPL", timestamp);
    assert(buySignal.type == Signal::Type::BUY);
    assert(buySignal.strength == 0.8);
    assert(buySignal.instrumentId == "AAPL");
    assert(buySignal.timestamp.toString() == timestamp.toString());
    
    // Test SELL signal
    Signal sellSignal(Signal::Type::SELL, 0.6, "MSFT", timestamp);
    assert(sellSignal.type == Signal::Type::SELL);
    assert(sellSignal.strength == 0.6);
    assert(sellSignal.instrumentId == "MSFT");
    
    // Test HOLD signal
    Signal holdSignal(Signal::Type::HOLD, 0.0, "GOOGL", timestamp);
    assert(holdSignal.type == Signal::Type::HOLD);
    assert(holdSignal.strength == 0.0);
    assert(holdSignal.instrumentId == "GOOGL");
    
    std::cout << "BUY signal: " << buySignal.toString() << std::endl;
    std::cout << "SELL signal: " << sellSignal.toString() << std::endl;
    std::cout << "HOLD signal: " << holdSignal.toString() << std::endl;
    std::cout << "✓ Signal constructor tests passed!" << std::endl;
}

void testSignalIsActionable() {
    std::cout << "Testing Signal isActionable..." << std::endl;
    
    DateTime timestamp(2024, 1, 15, 10, 30, 0);
    
    // BUY signals should be actionable
    Signal buySignal(Signal::Type::BUY, 0.9, "AAPL", timestamp);
    assert(buySignal.isActionable() == true);
    
    Signal buyWeakSignal(Signal::Type::BUY, 0.1, "AAPL", timestamp);
    assert(buyWeakSignal.isActionable() == true);
    
    // SELL signals should be actionable
    Signal sellSignal(Signal::Type::SELL, 0.7, "MSFT", timestamp);
    assert(sellSignal.isActionable() == true);
    
    Signal sellWeakSignal(Signal::Type::SELL, 0.05, "MSFT", timestamp);
    assert(sellWeakSignal.isActionable() == true);
    
    // HOLD signals should not be actionable
    Signal holdSignal(Signal::Type::HOLD, 0.0, "GOOGL", timestamp);
    assert(holdSignal.isActionable() == false);
    
    Signal holdWithStrength(Signal::Type::HOLD, 0.5, "GOOGL", timestamp);
    assert(holdWithStrength.isActionable() == false);
    
    std::cout << "BUY actionable: " << buySignal.isActionable() << std::endl;
    std::cout << "SELL actionable: " << sellSignal.isActionable() << std::endl;
    std::cout << "HOLD actionable: " << holdSignal.isActionable() << std::endl;
    std::cout << "✓ Signal isActionable tests passed!" << std::endl;
}

void testSignalToString() {
    std::cout << "Testing Signal toString..." << std::endl;
    
    DateTime timestamp(2024, 1, 15, 10, 30, 0);
    
    Signal buySignal(Signal::Type::BUY, 0.85, "AAPL", timestamp);
    std::string buyString = buySignal.toString();
    
    // The string should contain key information
    assert(buyString.find("BUY") != std::string::npos);
    assert(buyString.find("AAPL") != std::string::npos);
    assert(buyString.find("0.85") != std::string::npos);
    
    Signal sellSignal(Signal::Type::SELL, 0.75, "MSFT", timestamp);
    std::string sellString = sellSignal.toString();
    
    assert(sellString.find("SELL") != std::string::npos);
    assert(sellString.find("MSFT") != std::string::npos);
    assert(sellString.find("0.75") != std::string::npos);
    
    Signal holdSignal(Signal::Type::HOLD, 0.0, "GOOGL", timestamp);
    std::string holdString = holdSignal.toString();
    
    assert(holdString.find("HOLD") != std::string::npos);
    assert(holdString.find("GOOGL") != std::string::npos);
    
    std::cout << "BUY toString: " << buyString << std::endl;
    std::cout << "SELL toString: " << sellString << std::endl;
    std::cout << "HOLD toString: " << holdString << std::endl;
    std::cout << "✓ Signal toString tests passed!" << std::endl;
}

void testSignalCopyConstructor() {
    std::cout << "Testing Signal copy constructor..." << std::endl;
    
    DateTime timestamp(2024, 1, 15, 10, 30, 0);
    Signal original(Signal::Type::BUY, 0.92, "TSLA", timestamp);
    
    Signal copy(original);
    
    assert(copy.type == Signal::Type::BUY);
    assert(copy.strength == 0.92);
    assert(copy.instrumentId == "TSLA");
    assert(copy.timestamp.toString() == timestamp.toString());
    assert(copy.isActionable() == true);
    
    std::cout << "Original: " << original.toString() << std::endl;
    std::cout << "Copy: " << copy.toString() << std::endl;
    std::cout << "✓ Signal copy constructor tests passed!" << std::endl;
}

void testSignalAssignmentOperator() {
    std::cout << "Testing Signal assignment operator..." << std::endl;
    
    DateTime timestamp1(2024, 1, 15, 10, 30, 0);
    DateTime timestamp2(2024, 1, 16, 11, 0, 0);
    
    Signal signal1(Signal::Type::BUY, 0.8, "AAPL", timestamp1);
    Signal signal2(Signal::Type::SELL, 0.6, "MSFT", timestamp2);
    
    // Test assignment
    signal1 = signal2;
    
    assert(signal1.type == Signal::Type::SELL);
    assert(signal1.strength == 0.6);
    assert(signal1.instrumentId == "MSFT");
    assert(signal1.timestamp.toString() == timestamp2.toString());
    
    std::cout << "After assignment: " << signal1.toString() << std::endl;
    std::cout << "✓ Signal assignment operator tests passed!" << std::endl;
}

void testSignalStrengthValues() {
    std::cout << "Testing Signal strength values..." << std::endl;
    
    DateTime timestamp(2024, 1, 15, 10, 30, 0);
    
    // Test minimum strength
    Signal minSignal(Signal::Type::BUY, 0.0, "TEST", timestamp);
    assert(minSignal.strength == 0.0);
    
    // Test maximum strength
    Signal maxSignal(Signal::Type::SELL, 1.0, "TEST", timestamp);
    assert(maxSignal.strength == 1.0);
    
    // Test intermediate values
    Signal midSignal(Signal::Type::BUY, 0.5, "TEST", timestamp);
    assert(midSignal.strength == 0.5);
    
    // Test high precision
    Signal preciseSignal(Signal::Type::SELL, 0.123456, "TEST", timestamp);
    assert(preciseSignal.strength == 0.123456);
    
    std::cout << "Min strength: " << minSignal.strength << std::endl;
    std::cout << "Max strength: " << maxSignal.strength << std::endl;
    std::cout << "Mid strength: " << midSignal.strength << std::endl;
    std::cout << "Precise strength: " << preciseSignal.strength << std::endl;
    std::cout << "✓ Signal strength values tests passed!" << std::endl;
}

void testSignalWithDifferentSymbols() {
    std::cout << "Testing Signal with different symbols..." << std::endl;
    
    DateTime timestamp(2024, 1, 15, 10, 30, 0);
    
    // Test with various symbol formats
    Signal stockSignal(Signal::Type::BUY, 0.8, "AAPL", timestamp);
    Signal etfSignal(Signal::Type::SELL, 0.7, "SPY", timestamp);
    Signal cryptoSignal(Signal::Type::BUY, 0.9, "BTC-USD", timestamp);
    Signal forexSignal(Signal::Type::SELL, 0.6, "EUR/USD", timestamp);
    Signal indexSignal(Signal::Type::HOLD, 0.0, "^GSPC", timestamp);
    
    assert(stockSignal.instrumentId == "AAPL");
    assert(etfSignal.instrumentId == "SPY");
    assert(cryptoSignal.instrumentId == "BTC-USD");
    assert(forexSignal.instrumentId == "EUR/USD");
    assert(indexSignal.instrumentId == "^GSPC");
    
    std::cout << "Stock signal: " << stockSignal.instrumentId << std::endl;
    std::cout << "ETF signal: " << etfSignal.instrumentId << std::endl;
    std::cout << "Crypto signal: " << cryptoSignal.instrumentId << std::endl;
    std::cout << "Forex signal: " << forexSignal.instrumentId << std::endl;
    std::cout << "Index signal: " << indexSignal.instrumentId << std::endl;
    std::cout << "✓ Signal with different symbols tests passed!" << std::endl;
}

void testSignalTimestamps() {
    std::cout << "Testing Signal timestamps..." << std::endl;
    
    // Test with different timestamps
    DateTime morning(2024, 1, 15, 9, 30, 0);
    DateTime midday(2024, 1, 15, 12, 0, 0);
    DateTime afternoon(2024, 1, 15, 16, 0, 0);
    DateTime afterHours(2024, 1, 15, 18, 30, 0);
    
    Signal morningSignal(Signal::Type::BUY, 0.8, "AAPL", morning);
    Signal middaySignal(Signal::Type::SELL, 0.7, "AAPL", midday);
    Signal afternoonSignal(Signal::Type::BUY, 0.6, "AAPL", afternoon);
    Signal afterHoursSignal(Signal::Type::HOLD, 0.0, "AAPL", afterHours);
    
    assert(morningSignal.timestamp.hour() == 9);
    assert(middaySignal.timestamp.hour() == 12);
    assert(afternoonSignal.timestamp.hour() == 16);
    assert(afterHoursSignal.timestamp.hour() == 18);
    
    std::cout << "Morning signal: " << morningSignal.timestamp.toString() << std::endl;
    std::cout << "Midday signal: " << middaySignal.timestamp.toString() << std::endl;
    std::cout << "Afternoon signal: " << afternoonSignal.timestamp.toString() << std::endl;
    std::cout << "After hours signal: " << afterHoursSignal.timestamp.toString() << std::endl;
    std::cout << "✓ Signal timestamps tests passed!" << std::endl;
}

int main() {
    std::cout << "Running Signal tests..." << std::endl;
    
    try {
        testSignalConstructor();
        testSignalIsActionable();
        testSignalToString();
        testSignalCopyConstructor();
        testSignalAssignmentOperator();
        testSignalStrengthValues();
        testSignalWithDifferentSymbols();
        testSignalTimestamps();
        
        std::cout << "\n✅ All Signal tests passed!" << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "❌ Test failed: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}
