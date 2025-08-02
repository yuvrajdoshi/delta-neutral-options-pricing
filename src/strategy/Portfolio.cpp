#include "strategy/Portfolio.h"
#include "instruments/Option.h"
#include <stdexcept>

namespace VolatilityArbitrage {

Portfolio::Portfolio(double initialCash) : cash_(initialCash) {
}

void Portfolio::addPosition(Position position) {
    positions_.push_back(std::move(position));
}

void Portfolio::removePosition(size_t index) {
    if (index >= positions_.size()) {
        throw std::out_of_range("Position index out of range");
    }
    positions_.erase(positions_.begin() + index);
}

void Portfolio::updatePosition(size_t index, double newQuantity) {
    if (index >= positions_.size()) {
        throw std::out_of_range("Position index out of range");
    }
    positions_[index].setQuantity(newQuantity);
}

Position& Portfolio::getPosition(size_t index) {
    if (index >= positions_.size()) {
        throw std::out_of_range("Position index out of range");
    }
    return positions_[index];
}

const Position& Portfolio::getPosition(size_t index) const {
    if (index >= positions_.size()) {
        throw std::out_of_range("Position index out of range");
    }
    return positions_[index];
}

size_t Portfolio::getPositionCount() const {
    return positions_.size();
}

double Portfolio::getCash() const {
    return cash_;
}

void Portfolio::addCash(double amount) {
    cash_ += amount;
}

void Portfolio::removeCash(double amount) {
    cash_ -= amount;
}

double Portfolio::getTotalValue(const core::MarketData& data) const {
    double totalValue = cash_;
    
    for (const auto& position : positions_) {
        totalValue += position.getValue(data);
    }
    
    return totalValue;
}

double Portfolio::getTotalPnL(const core::MarketData& data) const {
    double totalPnL = 0.0;
    
    for (const auto& position : positions_) {
        totalPnL += position.getPnL(data);
    }
    
    return totalPnL;
}

double Portfolio::calculateDelta(const core::MarketData& data) const {
    double totalDelta = 0.0;
    
    for (const auto& position : positions_) {
        const instruments::Instrument& instrument = position.getInstrument();
        
        // Try to cast to Option to get delta
        const instruments::Option* option = dynamic_cast<const instruments::Option*>(&instrument);
        if (option) {
            totalDelta += position.getQuantity() * option->delta(data);
        } else {
            // For equity instruments, delta is 1
            totalDelta += position.getQuantity();
        }
    }
    
    return totalDelta;
}

double Portfolio::calculateGamma(const core::MarketData& data) const {
    double totalGamma = 0.0;
    
    for (const auto& position : positions_) {
        const instruments::Instrument& instrument = position.getInstrument();
        
        // Try to cast to Option to get gamma
        const instruments::Option* option = dynamic_cast<const instruments::Option*>(&instrument);
        if (option) {
            totalGamma += position.getQuantity() * option->gamma(data);
        }
        // Equity instruments have zero gamma
    }
    
    return totalGamma;
}

double Portfolio::calculateVega(const core::MarketData& data) const {
    double totalVega = 0.0;
    
    for (const auto& position : positions_) {
        const instruments::Instrument& instrument = position.getInstrument();
        
        // Try to cast to Option to get vega
        const instruments::Option* option = dynamic_cast<const instruments::Option*>(&instrument);
        if (option) {
            totalVega += position.getQuantity() * option->vega(data);
        }
        // Equity instruments have zero vega
    }
    
    return totalVega;
}

double Portfolio::calculateTheta(const core::MarketData& data) const {
    double totalTheta = 0.0;
    
    for (const auto& position : positions_) {
        const instruments::Instrument& instrument = position.getInstrument();
        
        // Try to cast to Option to get theta
        const instruments::Option* option = dynamic_cast<const instruments::Option*>(&instrument);
        if (option) {
            totalTheta += position.getQuantity() * option->theta(data);
        }
        // Equity instruments have zero theta
    }
    
    return totalTheta;
}

} // namespace VolatilityArbitrage
