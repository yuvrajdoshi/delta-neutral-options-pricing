#include <iostream>
#include <cassert>
#include <memory>
#include "strategy/Signal.h"
#include "core/DateTime.h"

using namespace VolatilityArbitrage;
using namespace core;

void testSignalBasic() {
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

int main() {
    std::cout << "Running basic Strategy Layer tests..." << std::endl;
    
    try {
        testSignalBasic();
        std::cout << "\n✅ Basic Strategy Layer tests passed!" << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "❌ Test failed: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}
