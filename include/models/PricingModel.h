#ifndef PRICING_MODEL_H
#define PRICING_MODEL_H

#include "models/Greeks.h"
#include "core/MarketData.h"
#include <memory>
#include <string>

// Forward declarations
namespace instruments {
    class Option;
}

namespace models {

/**
 * Base class for option pricing models
 */
class PricingModel {
public:
    // Virtual destructor
    virtual ~PricingModel() = default;
    
    // Core functionality
    virtual double price(const instruments::Option& option, const core::MarketData& data) const = 0;
    virtual Greeks calculateGreeks(const instruments::Option& option, const core::MarketData& data) const = 0;
    
    // Model information
    virtual std::string getModelName() const = 0;
    
    // Clone pattern
    virtual std::unique_ptr<PricingModel> clone() const = 0;
};

} // namespace models

#endif // PRICING_MODEL_H
