#ifndef MODEL_FACTORY_H
#define MODEL_FACTORY_H

#include "models/VolatilityModel.h"
#include "models/PricingModel.h"
#include <memory>

namespace models {

/**
 * Factory class for creating various model instances
 */
class ModelFactory {
public:
    // Factory methods for volatility models
    static std::unique_ptr<VolatilityModel> createGARCHModel(
        double omega = 0.0, double alpha = 0.0, double beta = 0.0);
    
    // Factory methods for pricing models
    static std::unique_ptr<PricingModel> createBSMPricingModel();
    
    // Model validation
    static bool validateGARCHParameters(double omega, double alpha, double beta);
};

} // namespace models

#endif // MODEL_FACTORY_H
