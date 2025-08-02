#include "strategy/VolatilityArbitrageStrategy.h"
#include "instruments/InstrumentFactory.h"
#include "core/DateTime.h"

namespace VolatilityArbitrage {

VolatilityArbitrageStrategy::VolatilityArbitrageStrategy(
    std::unique_ptr<VolatilityModel> volatilityModel,
    std::unique_ptr<SignalGenerator> signalGenerator,
    std::unique_ptr<HedgingStrategy> hedgingStrategy,
    int holdingPeriod)
    : volatilityModel_(std::move(volatilityModel)),
      signalGenerator_(std::move(signalGenerator)),
      hedgingStrategy_(std::move(hedgingStrategy)),
      portfolio_(0.0),
      holdingPeriod_(holdingPeriod) {
}

void VolatilityArbitrageStrategy::initialize(const BacktestParameters& params) {
    // Initialize portfolio with initial capital
    portfolio_ = Portfolio(params.getInitialCapital());
    
    // Clear any existing positions
    activePositions_.clear();
    daysInPosition_.clear();
}

void VolatilityArbitrageStrategy::processBar(const MarketData& data) {
    // Update positions (check for exits due to holding period)
    updatePositions(data);
    
    // For each symbol we're tracking, try to find option instruments to trade
    // For now, we'll assume we have access to the underlying and some options
    // In a real system, this would query available options from market data
    
    // For demo purposes, let's create a European call option at-the-money
    DateTime expiry = data.getTimestamp();
    expiry.addDays(30); // 30 days to expiry
    
    auto option = instruments::InstrumentFactory::createEuropeanCall(
        data.getSymbol(), expiry, data.getClose());
    
    // Generate signal for this option
    Signal signal = signalGenerator_->generateSignal(*option, *volatilityModel_, data);
    
    // Process the signal
    if (signal.isActionable()) {
        processSignal(signal, data);
    }
    
    // Apply hedging
    applyHedging(data);
}

Portfolio VolatilityArbitrageStrategy::getPortfolio() const {
    return portfolio_;
}

std::unique_ptr<Strategy> VolatilityArbitrageStrategy::clone() const {
    return std::make_unique<VolatilityArbitrageStrategy>(
        volatilityModel_->clone(),
        signalGenerator_->clone(),
        hedgingStrategy_->clone(),
        holdingPeriod_
    );
}

int VolatilityArbitrageStrategy::getHoldingPeriod() const {
    return holdingPeriod_;
}

void VolatilityArbitrageStrategy::setHoldingPeriod(int days) {
    holdingPeriod_ = days;
}

void VolatilityArbitrageStrategy::processSignal(const Signal& signal, const MarketData& data) {
    // Check if we already have a position in this instrument
    auto it = activePositions_.find(signal.instrumentId);
    
    if (it != activePositions_.end()) {
        // We already have a position - for now, we'll skip
        // In a more sophisticated system, we might adjust the position
        return;
    }
    
    // Determine position size based on signal strength and available capital
    double availableCash = portfolio_.getCash();
    double maxRiskPerTrade = availableCash * 0.05; // Risk 5% of capital per trade
    
    // For options, let's assume we want to buy/sell a fixed number of contracts
    double quantity = 10.0; // 10 contracts
    
    if (signal.type == Signal::Type::BUY || signal.type == Signal::Type::SELL) {
        // Create the instrument for this signal
        // For demo, assume it's an option with known characteristics
        DateTime expiry = data.getTimestamp();
        expiry.addDays(30);
        
        std::unique_ptr<instruments::Instrument> instrument;
        if (signal.type == Signal::Type::BUY) {
            instrument = instruments::InstrumentFactory::createEuropeanCall(
                data.getSymbol(), expiry, data.getClose());
        } else {
            // For SELL signal, we might want to sell calls or buy puts
            // For simplicity, let's sell calls (short position)
            instrument = instruments::InstrumentFactory::createEuropeanCall(
                data.getSymbol(), expiry, data.getClose());
            quantity = -quantity; // Negative quantity for short position
        }
        
        // Calculate the cost of this position
        double instrumentPrice = instrument->price(data);
        double totalCost = std::abs(quantity) * instrumentPrice;
        
        // Check if we have enough capital
        if (totalCost <= availableCash) {
            // Create and add the position
            Position position(std::move(instrument), quantity, instrumentPrice, data.getTimestamp());
            
            // Add metadata
            position.setMetadata("signal_strength", signal.strength);
            position.setMetadata("entry_signal_type", static_cast<double>(signal.type));
            
            // Add to portfolio
            portfolio_.addPosition(std::move(position));
            
            // Update cash
            if (quantity > 0) {
                portfolio_.removeCash(totalCost); // Buying costs cash
            } else {
                portfolio_.addCash(totalCost); // Selling adds cash
            }
            
            // Track this as an active position
            daysInPosition_[signal.instrumentId] = 0;
        }
    }
}

void VolatilityArbitrageStrategy::updatePositions(const MarketData& data) {
    // Update days in position for all active positions
    for (auto& pair : daysInPosition_) {
        pair.second++;
    }
    
    // Check if any positions should be closed due to holding period
    std::vector<size_t> positionsToClose;
    
    for (size_t i = 0; i < portfolio_.getPositionCount(); ++i) {
        const Position& position = portfolio_.getPosition(i);
        const std::string& instrumentId = position.getInstrument().getSymbol();
        
        auto daysIt = daysInPosition_.find(instrumentId);
        if (daysIt != daysInPosition_.end() && daysIt->second >= holdingPeriod_) {
            positionsToClose.push_back(i);
        }
    }
    
    // Close positions (in reverse order to maintain indices)
    for (auto it = positionsToClose.rbegin(); it != positionsToClose.rend(); ++it) {
        const Position& position = portfolio_.getPosition(*it);
        
        // Calculate the proceeds from closing
        double currentPrice = position.getInstrument().price(data);
        double proceeds = position.getQuantity() * currentPrice;
        
        // Add proceeds to cash (consider the sign of quantity)
        if (position.getQuantity() > 0) {
            portfolio_.addCash(proceeds); // Selling long position adds cash
        } else {
            portfolio_.removeCash(-proceeds); // Covering short position costs cash
        }
        
        // Remove from portfolio
        portfolio_.removePosition(*it);
        
        // Remove from tracking
        const std::string& instrumentId = position.getInstrument().getSymbol();
        daysInPosition_.erase(instrumentId);
    }
}

void VolatilityArbitrageStrategy::applyHedging(const MarketData& data) {
    if (hedgingStrategy_) {
        hedgingStrategy_->applyHedge(portfolio_, data);
    }
}

} // namespace VolatilityArbitrage
