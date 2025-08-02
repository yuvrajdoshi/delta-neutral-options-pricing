#include <string>
#ifndef GREEKS_H
#define GREEKS_H

#include <string>

namespace models {

/**
 * Greeks structure to hold option sensitivities
 */
struct Greeks {
    double delta;   // Price sensitivity to underlying price
    double gamma;   // Rate of change of delta
    double vega;    // Price sensitivity to volatility
    double theta;   // Price sensitivity to time decay
    double rho;     // Price sensitivity to interest rate
    
    // Constructors
    Greeks();
    Greeks(double delta, double gamma, double vega, double theta, double rho);
    
    // Utility methods
    std::string toString() const;
};

} // namespace models

#endif // GREEKS_H
