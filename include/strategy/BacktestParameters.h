#pragma once

#include "core/DateTime.h"
#include <vector>
#include <string>

namespace VolatilityArbitrage {

class BacktestParameters {
private:
    core::DateTime startDate_;
    core::DateTime endDate_;
    double initialCapital_;
    std::vector<std::string> symbols_;
    bool includeTransactionCosts_;
    double transactionCostPerTrade_;
    double transactionCostPercentage_;
    
public:
    // Constructors
    BacktestParameters();
    
    // Accessors and modifiers
    core::DateTime getStartDate() const;
    void setStartDate(const core::DateTime& startDate);
    core::DateTime getEndDate() const;
    void setEndDate(const core::DateTime& endDate);
    double getInitialCapital() const;
    void setInitialCapital(double capital);
    std::vector<std::string> getSymbols() const;
    void setSymbols(const std::vector<std::string>& symbols);
    bool getIncludeTransactionCosts() const;
    void setIncludeTransactionCosts(bool include);
    double getTransactionCostPerTrade() const;
    void setTransactionCostPerTrade(double cost);
    double getTransactionCostPercentage() const;
    void setTransactionCostPercentage(double percentage);
};

} // namespace VolatilityArbitrage
