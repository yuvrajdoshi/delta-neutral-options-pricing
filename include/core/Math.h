#ifndef MATH_H
#define MATH_H

#include <vector>
#include <functional>

namespace core {
    // Statistical functions
    double mean(const std::vector<double>& values);
    double variance(const std::vector<double>& values);
    double standardDeviation(const std::vector<double>& values);
    double skewness(const std::vector<double>& values);
    double kurtosis(const std::vector<double>& values);
    double correlation(const std::vector<double>& x, const std::vector<double>& y);
    
    // Probability distributions
    double normalPDF(double x, double mean = 0.0, double stdDev = 1.0);
    double normalCDF(double x, double mean = 0.0, double stdDev = 1.0);
    double normalInverseCDF(double p, double mean = 0.0, double stdDev = 1.0);
    
    // Optimization utilities
    struct OptimizationResult {
        std::vector<double> parameters;
        double objectiveValue;
        bool converged;
        int iterations;
    };
    
    OptimizationResult gradientDescent(
        const std::function<double(const std::vector<double>&)>& objective,
        const std::function<std::vector<double>(const std::vector<double>&)>& gradient,
        const std::vector<double>& initialParams,
        double learningRate = 0.01,
        double tolerance = 1e-6,
        int maxIterations = 1000
    );
    
    OptimizationResult newtonRaphson(
        const std::function<double(const std::vector<double>&)>& objective,
        const std::function<std::vector<double>(const std::vector<double>&)>& gradient,
        const std::function<std::vector<std::vector<double>>(const std::vector<double>&)>& hessian,
        const std::vector<double>& initialParams,
        double tolerance = 1e-6,
        int maxIterations = 100
    );
}

#endif