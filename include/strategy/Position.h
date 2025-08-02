#pragma once
#include "instruments/Instrument.h"
#include "core/DateTime.h"
#include "core/MarketData.h"
#include <memory>
#include <map>
#include <string>

namespace VolatilityArbitrage {

class Position {
private:
    std::unique_ptr<instruments::Instrument> instrument_;
    double quantity_;
    double entryPrice_;
    core::DateTime entryDate_;
    std::map<std::string, double> metadata_;
    
public:
    // Constructors
    Position(std::unique_ptr<instruments::Instrument> instrument, double quantity, double entryPrice, const core::DateTime& entryDate);
    
    // Copy and move constructors
    Position(const Position& other);
    Position(Position&& other) noexcept;
    Position& operator=(const Position& other);
    Position& operator=(Position&& other) noexcept;
    
    // Accessors
    const instruments::Instrument& getInstrument() const;
    double getQuantity() const;
    double getEntryPrice() const;
    core::DateTime getEntryDate() const;
    
    // Modifiers
    void setQuantity(double quantity);
    
    // Valuation
    double getValue(const core::MarketData& data) const;
    double getPnL(const core::MarketData& data) const;
    
    // Metadata handling
    void setMetadata(const std::string& key, double value);
    double getMetadata(const std::string& key) const;
    bool hasMetadata(const std::string& key) const;
};

} // namespace VolatilityArbitrage
