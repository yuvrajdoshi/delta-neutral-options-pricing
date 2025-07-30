#ifndef VOLATILITY_MODEL_H
#define VOLATILITY_MODEL_H

#include "core/TimeSeries.h"
#include <string>
#include <map>
#include <memory>

namespace models {

/**
 * Base class for volatility models
 */
class VolatilityModel {
public:
    // Virtual destructor
    virtual ~VolatilityModel() = default;
    
    // Core functionality
    virtual void calibrate(const core::TimeSeries& returns) = 0;
    virtual double forecast(int horizon) const = 0;
    virtual core::TimeSeries forecastSeries(int horizon) const = 0;
    
    // Model information
    virtual std::string getModelName() const = 0;
    virtual std::map<std::string, double> getParameters() const = 0;
    
    // Validation
    virtual double calculateLogLikelihood(const core::TimeSeries& returns) const = 0;
    virtual double calculateAIC() const = 0;
    virtual double calculateBIC() const = 0;
    
    // Clone pattern
    virtual std::unique_ptr<VolatilityModel> clone() const = 0;
};

} // namespace models

#endif // VOLATILITY_MODEL_H
