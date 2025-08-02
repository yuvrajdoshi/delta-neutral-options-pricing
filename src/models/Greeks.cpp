#include "models/Greeks.h"
#include <sstream>
#include <iomanip>

namespace models {

Greeks::Greeks() : delta(0.0), gamma(0.0), vega(0.0), theta(0.0), rho(0.0) {
}

Greeks::Greeks(double delta, double gamma, double vega, double theta, double rho)
    : delta(delta), gamma(gamma), vega(vega), theta(theta), rho(rho) {
}

std::string Greeks::toString() const {
    std::ostringstream oss;
    oss << std::fixed << std::setprecision(4);
    oss << "Greeks[Delta=" << delta 
        << ", Gamma=" << gamma 
        << ", Vega=" << vega 
        << ", Theta=" << theta 
        << ", Rho=" << rho << "]";
    return oss.str();
}

} // namespace models
