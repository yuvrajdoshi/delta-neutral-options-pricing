#include "instruments/Derivative.h"
#include <stdexcept>

namespace instruments {

Derivative::Derivative(const std::string& underlyingSymbol, const core::DateTime& expiryDate)
    : underlyingSymbol_(underlyingSymbol), expiryDate_(expiryDate) {
    // Validation can be added here if needed
}

std::string Derivative::getUnderlyingSymbol() const {
    return underlyingSymbol_;
}

core::DateTime Derivative::getExpiryDate() const {
    return expiryDate_;
}

double Derivative::timeToExpiry(const core::DateTime& currentDate) const {
    if (currentDate >= expiryDate_) {
        return 0.0; // Already expired
    }
    
    // Calculate time difference in years
    // This is a simplified calculation - in practice you'd want more accurate day counting
    core::TimeDelta diff = expiryDate_ - currentDate;
    double totalSeconds = diff.totalSeconds();
    double secondsPerYear = 365.25 * 24 * 60 * 60; // Including leap years
    
    return totalSeconds / secondsPerYear;
}

} // namespace instruments
