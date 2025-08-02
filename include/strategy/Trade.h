#ifndef TRADE_H
#define TRADE_H

#include "core/DateTime.h"
#include <string>

namespace VolatilityArbitrage {

/**
 * Represents a single trade execution
 */
class Trade {
public:
    enum class Action { BUY, SELL };
    
    // Members
    std::string instrumentId;
    Action action;
    double quantity;
    double price;
    core::DateTime timestamp;
    double transactionCost;
    
    // Constructors
    Trade();
    Trade(const std::string& instrumentId, Action action, double quantity, 
          double price, const core::DateTime& timestamp, double transactionCost = 0.0);
    
    // Methods
    double getValue() const;
    double getNetValue() const; // Value minus transaction cost
    std::string toString() const;
    
    // Comparison operators
    bool operator==(const Trade& other) const;
    bool operator<(const Trade& other) const; // For sorting by timestamp
    
    // Static helper methods
    static std::string actionToString(Action action);
    static Action stringToAction(const std::string& actionStr);
};

} // namespace VolatilityArbitrage

#endif // TRADE_H
