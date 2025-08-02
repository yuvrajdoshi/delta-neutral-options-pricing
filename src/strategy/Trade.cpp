#include "strategy/Trade.h"
#include <sstream>
#include <stdexcept>

namespace VolatilityArbitrage {

Trade::Trade() 
    : instrumentId(""), action(Action::BUY), quantity(0.0), price(0.0), 
      timestamp(core::DateTime()), transactionCost(0.0) {
}

Trade::Trade(const std::string& instrumentId, Action action, double quantity, 
             double price, const core::DateTime& timestamp, double transactionCost)
    : instrumentId(instrumentId), action(action), quantity(quantity), 
      price(price), timestamp(timestamp), transactionCost(transactionCost) {
}

double Trade::getValue() const {
    return quantity * price;
}

double Trade::getNetValue() const {
    double value = getValue();
    if (action == Action::BUY) {
        return -(value + transactionCost); // Cash outflow
    } else {
        return value - transactionCost;    // Cash inflow
    }
}

std::string Trade::toString() const {
    std::ostringstream oss;
    oss << timestamp.toString() << " " 
        << actionToString(action) << " " 
        << quantity << " " << instrumentId 
        << " @ $" << price 
        << " (Cost: $" << transactionCost << ")";
    return oss.str();
}

bool Trade::operator==(const Trade& other) const {
    return instrumentId == other.instrumentId &&
           action == other.action &&
           std::abs(quantity - other.quantity) < 1e-10 &&
           std::abs(price - other.price) < 1e-10 &&
           timestamp == other.timestamp;
}

bool Trade::operator<(const Trade& other) const {
    return timestamp < other.timestamp;
}

std::string Trade::actionToString(Action action) {
    switch (action) {
        case Action::BUY:  return "BUY";
        case Action::SELL: return "SELL";
        default: return "UNKNOWN";
    }
}

Trade::Action Trade::stringToAction(const std::string& actionStr) {
    if (actionStr == "BUY") return Action::BUY;
    if (actionStr == "SELL") return Action::SELL;
    throw std::invalid_argument("Unknown trade action: " + actionStr);
}

} // namespace VolatilityArbitrage
