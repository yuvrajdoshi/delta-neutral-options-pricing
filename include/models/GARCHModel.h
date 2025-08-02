#ifndef GARCH_MODEL_H
#define GARCH_MODEL_H

#include "models/VolatilityModel.h"

namespace models {

/**
 * GARCH(1,1) volatility model implementation
 */
class GARCHModel : public VolatilityModel {
private:
    double omega_;     // Long-run variance intercept
    double alpha_;     // ARCH parameter (reaction to past shocks)
    double beta_;      // GARCH parameter (persistence of volatility)
    double lastVariance_;  // Last estimated conditional variance
    double longRunVariance_;  // Long-run average variance
    bool isCalibrated_;
    int numParameters_; // For AIC/BIC calculation
    double logLikelihood_; // Cached log likelihood
    
public:
    // Constructors
    GARCHModel();
    GARCHModel(double omega, double alpha, double beta);
    
    // VolatilityModel interface implementation
    void calibrate(const core::TimeSeries& returns) override;
    double forecast(int horizon) const override;
    core::TimeSeries forecastSeries(int horizon) const override;
    std::string getModelName() const override;
    std::map<std::string, double> getParameters() const override;
    double calculateLogLikelihood(const core::TimeSeries& returns) const override;
    double calculateAIC() const override;
    double calculateBIC() const override;
    std::unique_ptr<VolatilityModel> clone() const override;
    
    // GARCH-specific methods
    double getOmega() const;
    double getAlpha() const;
    double getBeta() const;
    double getLastVariance() const;
    double getLongRunVariance() const;
    bool isStationary() const;
    bool isCalibrated() const;
    
private:
    // Helper methods
    double calculateNextVariance(double lastVariance, double lastReturn) const;
    void validateParameters() const;
    void estimateParameters(const core::TimeSeries& returns);
};

} // namespace models

#endif // GARCH_MODEL_H
