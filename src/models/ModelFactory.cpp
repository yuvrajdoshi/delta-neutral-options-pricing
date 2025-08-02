#include "models/ModelFactory.h"
#include "models/GARCHModel.h"
#include "models/BSMPricingModel.h"
#include <stdexcept>

namespace models {

std::unique_ptr<VolatilityModel> ModelFactory::createGARCHModel(double omega, double alpha, double beta) {
    if (!validateGARCHParameters(omega, alpha, beta)) {
        throw std::invalid_argument("Invalid GARCH parameters");
    }
    
    if (omega == 0.0 && alpha == 0.0 && beta == 0.0) {
        // Use default parameters
        return std::make_unique<GARCHModel>();
    } else {
        return std::make_unique<GARCHModel>(omega, alpha, beta);
    }
}

std::unique_ptr<PricingModel> ModelFactory::createBSMPricingModel() {
    return std::make_unique<BSMPricingModel>();
}

bool ModelFactory::validateGARCHParameters(double omega, double alpha, double beta) {
    // Allow default values (0.0) to pass validation
    if (omega == 0.0 && alpha == 0.0 && beta == 0.0) {
        return true;
    }
    
    // Check parameter bounds
    if (omega < 0.0) return false;
    if (alpha < 0.0 || alpha >= 1.0) return false;
    if (beta < 0.0 || beta >= 1.0) return false;
    
    // Check stationarity condition
    if (alpha + beta >= 1.0) return false;
    
    return true;
}

} // namespace models
