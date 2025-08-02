#include <iostream>
#include <cstdlib>
#include <string>
#include <vector>

// Function to run a test and return the result
bool runTest(const std::string& testName) {
    std::cout << "\n" << std::string(50, '=') << std::endl;
    std::cout << "Running " << testName << "..." << std::endl;
    std::cout << std::string(50, '=') << std::endl;
    
    int result = std::system(("./" + testName).c_str());
    
    if (result == 0) {
        std::cout << "✅ " << testName << " PASSED" << std::endl;
        return true;
    } else {
        std::cout << "❌ " << testName << " FAILED (exit code: " << result << ")" << std::endl;
        return false;
    }
}

int main() {
    std::cout << "🚀 Running Comprehensive Strategy Test Suite" << std::endl;
    std::cout << "=============================================" << std::endl;
    
    // List of all strategy-related tests
    std::vector<std::string> strategyTests = {
        "test_signal",
        "test_backtest_parameters",
        "test_position",
        "test_portfolio", 
        "test_volatility_spread_signal",
        "test_delta_hedging_strategy",
        "test_volatility_arbitrage_strategy"
    };
    
    int totalTests = strategyTests.size();
    int passedTests = 0;
    std::vector<std::string> failedTests;
    
    // Run each test
    for (const auto& testName : strategyTests) {
        if (runTest(testName)) {
            passedTests++;
        } else {
            failedTests.push_back(testName);
        }
    }
    
    // Print summary
    std::cout << "\n" << std::string(60, '=') << std::endl;
    std::cout << "STRATEGY TEST SUITE SUMMARY" << std::endl;
    std::cout << std::string(60, '=') << std::endl;
    
    std::cout << "Total tests: " << totalTests << std::endl;
    std::cout << "Passed: " << passedTests << " ✅" << std::endl;
    std::cout << "Failed: " << (totalTests - passedTests) << " ❌" << std::endl;
    std::cout << "Success rate: " << (passedTests * 100.0 / totalTests) << "%" << std::endl;
    
    if (!failedTests.empty()) {
        std::cout << "\nFailed tests:" << std::endl;
        for (const auto& test : failedTests) {
            std::cout << "  - " << test << std::endl;
        }
    }
    
    std::cout << std::string(60, '=') << std::endl;
    
    if (passedTests == totalTests) {
        std::cout << "🎉 ALL STRATEGY TESTS PASSED! 🎉" << std::endl;
        return 0;
    } else {
        std::cout << "⚠️  Some tests failed. Please check the output above." << std::endl;
        return 1;
    }
}
