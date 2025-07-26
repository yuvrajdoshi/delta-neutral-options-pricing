#include "include/core/Math.h"
#include <iostream>
#include <vector>
#include <iomanip>

int main() {
    std::cout << "Testing Math Utilities...\n" << std::endl;
    
    // Test data
    std::vector<double> data = {1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0, 9.0, 10.0};
    std::vector<double> data2 = {2.0, 4.0, 6.0, 8.0, 10.0, 12.0, 14.0, 16.0, 18.0, 20.0};
    
    std::cout << std::fixed << std::setprecision(6);
    
    // Test statistical functions
    std::cout << "=== Statistical Functions ===" << std::endl;
    std::cout << "Data: ";
    for (const auto& val : data) {
        std::cout << val << " ";
    }
    std::cout << std::endl;
    
    try {
        std::cout << "Mean: " << core::mean(data) << std::endl;
        std::cout << "Variance: " << core::variance(data) << std::endl;
        std::cout << "Standard Deviation: " << core::standardDeviation(data) << std::endl;
        std::cout << "Skewness: " << core::skewness(data) << std::endl;
        std::cout << "Kurtosis: " << core::kurtosis(data) << std::endl;
        std::cout << "Correlation with data2: " << core::correlation(data, data2) << std::endl;
    } catch (const std::exception& e) {
        std::cout << "Error in statistical functions: " << e.what() << std::endl;
    }
    
    std::cout << "\n=== Probability Distributions ===" << std::endl;
    try {
        double x = 0.0;
        std::cout << "Normal PDF at x=0: " << core::normalPDF(x) << std::endl;
        std::cout << "Normal CDF at x=0: " << core::normalCDF(x) << std::endl;
        
        double p = 0.5;
        std::cout << "Normal Inverse CDF at p=0.5: " << core::normalInverseCDF(p) << std::endl;
        
        // Test with non-standard parameters
        std::cout << "Normal PDF at x=1 (mean=0, std=2): " << core::normalPDF(1.0, 0.0, 2.0) << std::endl;
        std::cout << "Normal CDF at x=1 (mean=0, std=2): " << core::normalCDF(1.0, 0.0, 2.0) << std::endl;
    } catch (const std::exception& e) {
        std::cout << "Error in probability functions: " << e.what() << std::endl;
    }
    
    std::cout << "\n=== Optimization Functions ===" << std::endl;
    try {
        // Simple quadratic function: f(x) = (x-2)^2 + 1
        // Minimum should be at x = 2
        auto objective = [](const std::vector<double>& params) -> double {
            double x = params[0];
            return (x - 2.0) * (x - 2.0) + 1.0;
        };
        
        auto gradient = [](const std::vector<double>& params) -> std::vector<double> {
            double x = params[0];
            return {2.0 * (x - 2.0)};
        };
        
        auto hessian = [](const std::vector<double>& params) -> std::vector<std::vector<double>> {
            return {{2.0}};
        };
        
        std::vector<double> initialParams = {0.0};
        
        // Test gradient descent
        auto result_gd = core::gradientDescent(objective, gradient, initialParams, 0.1, 1e-6, 1000);
        std::cout << "Gradient Descent Result:" << std::endl;
        std::cout << "  Parameter: " << result_gd.parameters[0] << std::endl;
        std::cout << "  Objective Value: " << result_gd.objectiveValue << std::endl;
        std::cout << "  Converged: " << (result_gd.converged ? "Yes" : "No") << std::endl;
        std::cout << "  Iterations: " << result_gd.iterations << std::endl;
        
        // Test Newton-Raphson
        auto result_nr = core::newtonRaphson(objective, gradient, hessian, initialParams, 1e-6, 100);
        std::cout << "\nNewton-Raphson Result:" << std::endl;
        std::cout << "  Parameter: " << result_nr.parameters[0] << std::endl;
        std::cout << "  Objective Value: " << result_nr.objectiveValue << std::endl;
        std::cout << "  Converged: " << (result_nr.converged ? "Yes" : "No") << std::endl;
        std::cout << "  Iterations: " << result_nr.iterations << std::endl;
        
    } catch (const std::exception& e) {
        std::cout << "Error in optimization functions: " << e.what() << std::endl;
    }
    
    std::cout << "\nMath utilities test completed!" << std::endl;
    return 0;
}
