#ifndef OPTION_H
#define OPTION_H

#include "instruments/Derivative.h"

namespace instruments {

// Enums for option types
enum class OptionType { Call, Put };
enum class ExerciseStyle { European, American };

/**
 * Base Option class
 */
class Option : public Derivative {
protected:
    double strike_;
    OptionType type_;
    ExerciseStyle exerciseStyle_;
    
public:
    // Constructors
    Option(const std::string& underlyingSymbol, const core::DateTime& expiryDate,
           double strike, OptionType type, ExerciseStyle exerciseStyle);
    
    // Accessors
    double getStrike() const;
    OptionType getOptionType() const;
    ExerciseStyle getExerciseStyle() const;
    
    // Instrument interface implementation
    std::string getSymbol() const override;
    InstrumentType getType() const override;
    std::vector<double> calculateRiskMetrics(const core::MarketData& data) const override;
    
    // Greeks calculation (implemented in derived classes)
    double delta(const core::MarketData& data) const override;
    double gamma(const core::MarketData& data) const override;
    double vega(const core::MarketData& data) const override;
    double theta(const core::MarketData& data) const override;
    double rho(const core::MarketData& data) const override;

protected:
    // Helper method for volatility estimation
    double getVolatilityForPricing(const core::MarketData& data) const;
};

/**
 * European Option class
 */
class EuropeanOption : public Option {
public:
    // Constructors
    EuropeanOption(const std::string& underlyingSymbol, const core::DateTime& expiryDate,
                  double strike, OptionType type);
    
    // Pricing using Black-Scholes
    double price(const core::MarketData& data) const override;
    
    // Clone implementation
    std::unique_ptr<Instrument> clone() const override;
};

/**
 * American Option class
 */
class AmericanOption : public Option {
public:
    // Constructors
    AmericanOption(const std::string& underlyingSymbol, const core::DateTime& expiryDate,
                  double strike, OptionType type);
    
    // Pricing using binomial tree or other numerical methods
    double price(const core::MarketData& data) const override;
    
    // Clone implementation
    std::unique_ptr<Instrument> clone() const override;
};

} // namespace instruments

#endif // OPTION_H
