#include "strategy/BacktestParameters.h"

namespace VolatilityArbitrage {

BacktestParameters::BacktestParameters() 
    : startDate_(DateTime()), endDate_(DateTime()), initialCapital_(100000.0),
      includeTransactionCosts_(false), transactionCostPerTrade_(0.0), 
      transactionCostPercentage_(0.0) {
}

DateTime BacktestParameters::getStartDate() const {
    return startDate_;
}

void BacktestParameters::setStartDate(const DateTime& startDate) {
    startDate_ = startDate;
}

DateTime BacktestParameters::getEndDate() const {
    return endDate_;
}

void BacktestParameters::setEndDate(const DateTime& endDate) {
    endDate_ = endDate;
}

double BacktestParameters::getInitialCapital() const {
    return initialCapital_;
}

void BacktestParameters::setInitialCapital(double capital) {
    initialCapital_ = capital;
}

std::vector<std::string> BacktestParameters::getSymbols() const {
    return symbols_;
}

void BacktestParameters::setSymbols(const std::vector<std::string>& symbols) {
    symbols_ = symbols;
}

bool BacktestParameters::getIncludeTransactionCosts() const {
    return includeTransactionCosts_;
}

void BacktestParameters::setIncludeTransactionCosts(bool include) {
    includeTransactionCosts_ = include;
}

double BacktestParameters::getTransactionCostPerTrade() const {
    return transactionCostPerTrade_;
}

void BacktestParameters::setTransactionCostPerTrade(double cost) {
    transactionCostPerTrade_ = cost;
}

double BacktestParameters::getTransactionCostPercentage() const {
    return transactionCostPercentage_;
}

void BacktestParameters::setTransactionCostPercentage(double percentage) {
    transactionCostPercentage_ = percentage;
}

} // namespace VolatilityArbitrage
