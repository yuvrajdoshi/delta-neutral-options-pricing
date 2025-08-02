#include "models/GARCHModel.h"
#include "core/MathUtils.h"
#include "core/DateTime.h"
#include <cmath>
#include <stdexcept>
#include <algorithm>

namespace models {

GARCHModel::GARCHModel() 
    : omega_(0.0), alpha_(0.0), beta_(0.0), lastVariance_(0.0), 
      longRunVariance_(0.0), isCalibrated_(false), numParameters_(3), logLikelihood_(0.0) {
}

GARCHModel::GARCHModel(double omega, double alpha, double beta)
    : omega_(omega), alpha_(alpha), beta_(beta), lastVariance_(0.0),
      longRunVariance_(0.0), isCalibrated_(false), numParameters_(3), logLikelihood_(0.0) {
    validateParameters();
    longRunVariance_ = omega / (1.0 - alpha - beta);
}

void GARCHModel::calibrate(const core::TimeSeries& returns) {
    if (returns.size() < 10) {
        throw std::invalid_argument("Need at least 10 observations for GARCH calibration");
    }
    
    // Simple estimation using method of moments as starting point
    // In practice, you'd use Maximum Likelihood Estimation
    std::vector<double> returnValues = returns.getValues();
    
    // Calculate unconditional variance
    double variance = core::variance(returnValues);
    
    // Initial parameter estimates (simplified)
    omega_ = variance * 0.1;  // 10% of unconditional variance
    alpha_ = 0.1;             // Standard initial value
    beta_ = 0.8;              // Standard initial value
    
    // Ensure parameters are valid
    if (alpha_ + beta_ >= 1.0) {
        alpha_ = 0.05;
        beta_ = 0.9;
    }
    
    longRunVariance_ = omega_ / (1.0 - alpha_ - beta_);
    
    // Calculate log likelihood
    logLikelihood_ = calculateLogLikelihood(returns);
    
    // Set last variance to unconditional variance
    lastVariance_ = longRunVariance_;
    
    isCalibrated_ = true;
}

double GARCHModel::forecast(int horizon) const {
    if (!isCalibrated_) {
        throw std::runtime_error("Model must be calibrated before forecasting");
    }
    
    if (horizon <= 0) {
        throw std::invalid_argument("Horizon must be positive");
    }
    
    // GARCH(1,1) forecast: converges to long-run variance
    double lambda = alpha_ + beta_;
    double forecast = longRunVariance_ + std::pow(lambda, horizon) * (lastVariance_ - longRunVariance_);
    
    return std::sqrt(forecast); // Return volatility (standard deviation)
}

core::TimeSeries GARCHModel::forecastSeries(int horizon) const {
    if (!isCalibrated_) {
        throw std::runtime_error("Model must be calibrated before forecasting");
    }
    
    std::vector<core::DateTime> timestamps;
    std::vector<double> forecasts;
    
    // Generate timestamps (daily for simplicity)
    core::DateTime startDate = core::DateTime::now();
    
    for (int i = 1; i <= horizon; ++i) {
        core::TimeDelta delta(i);
        timestamps.push_back(startDate + delta);
        forecasts.push_back(forecast(i));
    }
    
    return core::TimeSeries(timestamps, forecasts, "GARCH_Forecast");
}

std::string GARCHModel::getModelName() const {
    return "GARCH(1,1)";
}

std::map<std::string, double> GARCHModel::getParameters() const {
    std::map<std::string, double> params;
    params["omega"] = omega_;
    params["alpha"] = alpha_;
    params["beta"] = beta_;
    params["long_run_variance"] = longRunVariance_;
    params["last_variance"] = lastVariance_;
    return params;
}

double GARCHModel::calculateLogLikelihood(const core::TimeSeries& returns) const {
    if (!isCalibrated_ && (omega_ == 0.0 && alpha_ == 0.0 && beta_ == 0.0)) {
        return -std::numeric_limits<double>::infinity();
    }
    
    std::vector<double> returnValues = returns.getValues();
    if (returnValues.empty()) {
        return -std::numeric_limits<double>::infinity();
    }
    
    double logLikelihood = 0.0;
    double variance = longRunVariance_; // Initialize with long-run variance
    
    for (size_t i = 1; i < returnValues.size(); ++i) {
        // Update variance using GARCH equation
        variance = calculateNextVariance(variance, returnValues[i-1]);
        
        // Calculate log likelihood contribution
        if (variance > 0.0) {
            logLikelihood += -0.5 * (std::log(2.0 * M_PI) + std::log(variance) + 
                                   (returnValues[i] * returnValues[i]) / variance);
        }
    }
    
    return logLikelihood;
}

double GARCHModel::calculateAIC() const {
    if (!isCalibrated_) {
        return std::numeric_limits<double>::infinity();
    }
    return -2.0 * logLikelihood_ + 2.0 * numParameters_;
}

double GARCHModel::calculateBIC() const {
    if (!isCalibrated_) {
        return std::numeric_limits<double>::infinity();
    }
    // Note: We'd need the sample size for proper BIC calculation
    // Using a default sample size of 100 for now
    int n = 100;
    return -2.0 * logLikelihood_ + numParameters_ * std::log(n);
}

std::unique_ptr<VolatilityModel> GARCHModel::clone() const {
    auto cloned = std::make_unique<GARCHModel>(omega_, alpha_, beta_);
    cloned->lastVariance_ = lastVariance_;
    cloned->longRunVariance_ = longRunVariance_;
    cloned->isCalibrated_ = isCalibrated_;
    cloned->logLikelihood_ = logLikelihood_;
    return cloned;
}

double GARCHModel::getOmega() const {
    return omega_;
}

double GARCHModel::getAlpha() const {
    return alpha_;
}

double GARCHModel::getBeta() const {
    return beta_;
}

double GARCHModel::getLastVariance() const {
    return lastVariance_;
}

double GARCHModel::getLongRunVariance() const {
    return longRunVariance_;
}

bool GARCHModel::isStationary() const {
    return (alpha_ + beta_) < 1.0;
}

bool GARCHModel::isCalibrated() const {
    return isCalibrated_;
}

double GARCHModel::calculateNextVariance(double lastVariance, double lastReturn) const {
    return omega_ + alpha_ * lastReturn * lastReturn + beta_ * lastVariance;
}

void GARCHModel::validateParameters() const {
    if (omega_ < 0.0) {
        throw std::invalid_argument("Omega must be non-negative");
    }
    if (alpha_ < 0.0 || alpha_ >= 1.0) {
        throw std::invalid_argument("Alpha must be in [0, 1)");
    }
    if (beta_ < 0.0 || beta_ >= 1.0) {
        throw std::invalid_argument("Beta must be in [0, 1)");
    }
    if (alpha_ + beta_ >= 1.0) {
        throw std::invalid_argument("Alpha + Beta must be less than 1 for stationarity");
    }
}

} // namespace models
