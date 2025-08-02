#ifndef BSM_PRICING_MODEL_H
#define BSM_PRICING_MODEL_H

#include "models/PricingModel.h"

namespace models {

/**
 * Black-Scholes-Merton pricing model implementation
 */
class BSMPricingModel : public PricingModel {
public:
    // Constructors
    BSMPricingModel();
    
    // PricingModel interface implementation
    double price(const instruments::Option& option, const core::MarketData& data) const override;
    Greeks calculateGreeks(const instruments::Option& option, const core::MarketData& data) const override;
    std::string getModelName() const override;
    std::unique_ptr<PricingModel> clone() const override;
    
private:
    // Helper methods
    double calculateD1(double S, double K, double T, double r, double sigma) const;
    double calculateD2(double d1, double sigma, double T) const;
    double calculateCallPrice(double S, double K, double T, double r, double sigma) const;
    double calculatePutPrice(double S, double K, double T, double r, double sigma) const;
    
    // Greeks calculation helpers
    double calculateDelta(double S, double K, double T, double r, double sigma, bool isCall) const;
    double calculateGamma(double S, double K, double T, double r, double sigma) const;
    double calculateVega(double S, double K, double T, double r, double sigma) const;
    double calculateTheta(double S, double K, double T, double r, double sigma, bool isCall) const;
    double calculateRho(double S, double K, double T, double r, double sigma, bool isCall) const;
    
    // Utility methods
    double getVolatility(const instruments::Option& option, const core::MarketData& data) const;
    double getRiskFreeRate() const; // Default risk-free rate
};

} // namespace models

#endif // BSM_PRICING_MODEL_H
