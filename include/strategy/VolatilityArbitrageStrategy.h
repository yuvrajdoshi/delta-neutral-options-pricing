#pragma once

#include "strategy/Strategy.h"
#include "strategy/SignalGenerator.h"
#include "strategy/HedgingStrategy.h"
#include "models/VolatilityModel.h"
#include <memory>
#include <map>
#include <string>

namespace VolatilityArbitrage {

class VolatilityArbitrageStrategy : public Strategy {
private:
    std::unique_ptr<models::VolatilityModel> volatilityModel_;
    std::unique_ptr<SignalGenerator> signalGenerator_;
    std::unique_ptr<HedgingStrategy> hedgingStrategy_;
    Portfolio portfolio_;
    std::map<std::string, Position> activePositions_;
    int holdingPeriod_;
    std::map<std::string, int> daysInPosition_;
    
public:
    // Constructors
    VolatilityArbitrageStrategy(
        std::unique_ptr<models::VolatilityModel> volatilityModel,
        std::unique_ptr<SignalGenerator> signalGenerator,
        std::unique_ptr<HedgingStrategy> hedgingStrategy,
        int holdingPeriod = 30
    );
    
    // Strategy interface implementation
    void initialize(const BacktestParameters& params) override;
    void processBar(const core::MarketData& data) override;
    Portfolio getPortfolio() const override;
    std::unique_ptr<Strategy> clone() const override;
    
    // Accessors
    int getHoldingPeriod() const;
    void setHoldingPeriod(int days);
    
private:
    // Helper methods
    void processSignal(const Signal& signal, const core::MarketData& data);
    void updatePositions(const core::MarketData& data);
    void applyHedging(const core::MarketData& data);
};

} // namespace VolatilityArbitrage
