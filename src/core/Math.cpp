#include "core/Math.h"
#include <cmath>
#include <numeric>
#include <stdexcept>
#include <algorithm>

namespace core {

// Statistical functions
double mean(const std::vector<double>& values) {
    if (values.empty()) {
        throw std::invalid_argument("Cannot calculate mean of empty vector");
    }
    
    double sum = std::accumulate(values.begin(), values.end(), 0.0);
    return sum / values.size();
}

double variance(const std::vector<double>& values) {
    if (values.size() <= 1) {
        throw std::invalid_argument("Cannot calculate variance with less than 2 values");
    }
    
    double m = mean(values);
    double sum_sq_diff = 0.0;
    
    for (const auto& value : values) {
        double diff = value - m;
        sum_sq_diff += diff * diff;
    }
    
    return sum_sq_diff / (values.size() - 1); // Sample variance (Bessel's correction)
}

double standardDeviation(const std::vector<double>& values) {
    return std::sqrt(variance(values));
}

double skewness(const std::vector<double>& values) {
    if (values.size() < 3) {
        throw std::invalid_argument("Cannot calculate skewness with less than 3 values");
    }
    
    double m = mean(values);
    double std_dev = standardDeviation(values);
    
    if (std_dev == 0.0) {
        return 0.0; // All values are the same
    }
    
    double sum_cubed_deviations = 0.0;
    for (const auto& value : values) {
        double normalized_deviation = (value - m) / std_dev;
        sum_cubed_deviations += normalized_deviation * normalized_deviation * normalized_deviation;
    }
    
    size_t n = values.size();
    return (n / ((n - 1.0) * (n - 2.0))) * sum_cubed_deviations;
}

double kurtosis(const std::vector<double>& values) {
    if (values.size() < 4) {
        throw std::invalid_argument("Cannot calculate kurtosis with less than 4 values");
    }
    
    double m = mean(values);
    double std_dev = standardDeviation(values);
    
    if (std_dev == 0.0) {
        return 0.0; // All values are the same
    }
    
    double sum_fourth_deviations = 0.0;
    for (const auto& value : values) {
        double normalized_deviation = (value - m) / std_dev;
        double fourth_power = normalized_deviation * normalized_deviation * normalized_deviation * normalized_deviation;
        sum_fourth_deviations += fourth_power;
    }
    
    size_t n = values.size();
    double excess_kurtosis = (n * (n + 1.0) / ((n - 1.0) * (n - 2.0) * (n - 3.0))) * sum_fourth_deviations 
                            - (3.0 * (n - 1.0) * (n - 1.0) / ((n - 2.0) * (n - 3.0)));
    
    return excess_kurtosis; // Returns excess kurtosis (kurtosis - 3)
}

double correlation(const std::vector<double>& x, const std::vector<double>& y) {
    if (x.size() != y.size()) {
        throw std::invalid_argument("Vectors must have the same size for correlation calculation");
    }
    
    if (x.size() < 2) {
        throw std::invalid_argument("Cannot calculate correlation with less than 2 values");
    }
    
    double mean_x = mean(x);
    double mean_y = mean(y);
    
    double numerator = 0.0;
    double sum_sq_x = 0.0;
    double sum_sq_y = 0.0;
    
    for (size_t i = 0; i < x.size(); ++i) {
        double diff_x = x[i] - mean_x;
        double diff_y = y[i] - mean_y;
        
        numerator += diff_x * diff_y;
        sum_sq_x += diff_x * diff_x;
        sum_sq_y += diff_y * diff_y;
    }
    
    double denominator = std::sqrt(sum_sq_x * sum_sq_y);
    
    if (denominator == 0.0) {
        return 0.0; // One or both series have no variance
    }
    
    return numerator / denominator;
}

// Probability distributions
double normalPDF(double x, double mean, double stdDev) {
    if (stdDev <= 0.0) {
        throw std::invalid_argument("Standard deviation must be positive");
    }
    
    double coefficient = 1.0 / (stdDev * std::sqrt(2.0 * M_PI));
    double exponent = -0.5 * std::pow((x - mean) / stdDev, 2.0);
    
    return coefficient * std::exp(exponent);
}

double normalCDF(double x, double mean, double stdDev) {
    if (stdDev <= 0.0) {
        throw std::invalid_argument("Standard deviation must be positive");
    }
    
    // Standardize the value
    double z = (x - mean) / stdDev;
    
    // Use the error function for standard normal CDF
    // CDF(z) = 0.5 * (1 + erf(z / sqrt(2)))
    return 0.5 * (1.0 + std::erf(z / std::sqrt(2.0)));
}

double normalInverseCDF(double p, double mean, double stdDev) {
    if (p <= 0.0 || p >= 1.0) {
        throw std::invalid_argument("Probability must be between 0 and 1 (exclusive)");
    }
    
    if (stdDev <= 0.0) {
        throw std::invalid_argument("Standard deviation must be positive");
    }
    
    // Beasley-Springer-Moro algorithm approximation for inverse normal CDF
    // This is a simplified implementation - in production, you'd use a more sophisticated algorithm
    
    double a[4] = {2.50662823884, -18.61500062529, 41.39119773534, -25.44106049637};
    double b[4] = {-8.47351093090, 23.08336743743, -21.06224101826, 3.13082909833};
    double c[9] = {0.3374754822726147, 0.9761690190917186, 0.1607979714918209,
                   0.0276438810333863, 0.0038405729373609, 0.0003951896511919,
                   0.0000321767881768, 0.0000002888167364, 0.0000003960315187};
    
    double y = p - 0.5;
    double r, x;
    
    if (std::abs(y) < 0.42) {
        r = y * y;
        x = y * (((a[3] * r + a[2]) * r + a[1]) * r + a[0]) /
            ((((b[3] * r + b[2]) * r + b[1]) * r + b[0]) * r + 1.0);
    } else {
        r = p;
        if (y > 0.0) r = 1.0 - p;
        r = std::log(-std::log(r));
        x = c[0] + r * (c[1] + r * (c[2] + r * (c[3] + r * (c[4] + r * (c[5] + r * (c[6] + r * (c[7] + r * c[8])))))));
        if (y < 0.0) x = -x;
    }
    
    return mean + stdDev * x;
}

// Optimization utilities
OptimizationResult gradientDescent(
    const std::function<double(const std::vector<double>&)>& objective,
    const std::function<std::vector<double>(const std::vector<double>&)>& gradient,
    const std::vector<double>& initialParams,
    double learningRate,
    double tolerance,
    int maxIterations) {
    
    if (initialParams.empty()) {
        throw std::invalid_argument("Initial parameters cannot be empty");
    }
    
    if (learningRate <= 0.0) {
        throw std::invalid_argument("Learning rate must be positive");
    }
    
    if (tolerance <= 0.0) {
        throw std::invalid_argument("Tolerance must be positive");
    }
    
    std::vector<double> params = initialParams;
    OptimizationResult result;
    result.parameters = params;
    result.converged = false;
    result.iterations = 0;
    
    double previousObjectiveValue = objective(params);
    
    for (int iter = 0; iter < maxIterations; ++iter) {
        result.iterations = iter + 1;
        
        // Calculate gradient
        std::vector<double> grad = gradient(params);
        
        // Update parameters
        for (size_t i = 0; i < params.size(); ++i) {
            params[i] -= learningRate * grad[i];
        }
        
        // Calculate new objective value
        double currentObjectiveValue = objective(params);
        
        // Check for convergence
        if (std::abs(currentObjectiveValue - previousObjectiveValue) < tolerance) {
            result.converged = true;
            break;
        }
        
        previousObjectiveValue = currentObjectiveValue;
    }
    
    result.parameters = params;
    result.objectiveValue = objective(params);
    
    return result;
}

OptimizationResult newtonRaphson(
    const std::function<double(const std::vector<double>&)>& objective,
    const std::function<std::vector<double>(const std::vector<double>&)>& gradient,
    const std::function<std::vector<std::vector<double>>(const std::vector<double>&)>& hessian,
    const std::vector<double>& initialParams,
    double tolerance,
    int maxIterations) {
    
    if (initialParams.empty()) {
        throw std::invalid_argument("Initial parameters cannot be empty");
    }
    
    if (tolerance <= 0.0) {
        throw std::invalid_argument("Tolerance must be positive");
    }
    
    std::vector<double> params = initialParams;
    OptimizationResult result;
    result.parameters = params;
    result.converged = false;
    result.iterations = 0;
    
    for (int iter = 0; iter < maxIterations; ++iter) {
        result.iterations = iter + 1;
        
        // Calculate gradient and hessian
        std::vector<double> grad = gradient(params);
        std::vector<std::vector<double>> hess = hessian(params);
        
        // Check gradient norm for convergence
        double gradNorm = 0.0;
        for (const auto& g : grad) {
            gradNorm += g * g;
        }
        gradNorm = std::sqrt(gradNorm);
        
        if (gradNorm < tolerance) {
            result.converged = true;
            break;
        }
        
        // Solve Hessian * delta = -gradient for delta (simplified implementation)
        // In practice, you'd use a proper linear algebra library for matrix inversion
        // This is a simplified implementation for demonstration
        
        size_t n = params.size();
        std::vector<double> delta(n, 0.0);
        
        // Simple diagonal approximation (not a full Newton-Raphson step)
        for (size_t i = 0; i < n; ++i) {
            if (std::abs(hess[i][i]) > 1e-12) {
                delta[i] = -grad[i] / hess[i][i];
            } else {
                delta[i] = -grad[i] * 0.01; // Fallback to gradient descent step
            }
        }
        
        // Update parameters
        for (size_t i = 0; i < params.size(); ++i) {
            params[i] += delta[i];
        }
    }
    
    result.parameters = params;
    result.objectiveValue = objective(params);
    
    return result;
}

} // namespace core