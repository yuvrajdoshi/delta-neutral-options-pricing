#include "strategy/Position.h"
#include "instruments/InstrumentFactory.h"

namespace VolatilityArbitrage {

Position::Position(std::unique_ptr<instruments::Instrument> instrument, double quantity, double entryPrice, const core::DateTime& entryDate)
    : instrument_(std::move(instrument)), quantity_(quantity), entryPrice_(entryPrice), entryDate_(entryDate) {
}

Position::Position(const Position& other)
    : instrument_(instruments::InstrumentFactory::clone(*other.instrument_)),
      quantity_(other.quantity_),
      entryPrice_(other.entryPrice_),
      entryDate_(other.entryDate_),
      metadata_(other.metadata_) {
}

Position::Position(Position&& other) noexcept
    : instrument_(std::move(other.instrument_)),
      quantity_(other.quantity_),
      entryPrice_(other.entryPrice_),
      entryDate_(other.entryDate_),
      metadata_(std::move(other.metadata_)) {
}

Position& Position::operator=(const Position& other) {
    if (this != &other) {
        instrument_ = instruments::InstrumentFactory::clone(*other.instrument_);
        quantity_ = other.quantity_;
        entryPrice_ = other.entryPrice_;
        entryDate_ = other.entryDate_;
        metadata_ = other.metadata_;
    }
    return *this;
}

Position& Position::operator=(Position&& other) noexcept {
    if (this != &other) {
        instrument_ = std::move(other.instrument_);
        quantity_ = other.quantity_;
        entryPrice_ = other.entryPrice_;
        entryDate_ = other.entryDate_;
        metadata_ = std::move(other.metadata_);
    }
    return *this;
}

const instruments::Instrument& Position::getInstrument() const {
    return *instrument_;
}

double Position::getQuantity() const {
    return quantity_;
}

double Position::getEntryPrice() const {
    return entryPrice_;
}

core::DateTime Position::getEntryDate() const {
    return entryDate_;
}

void Position::setQuantity(double quantity) {
    quantity_ = quantity;
}

double Position::getValue(const core::MarketData& data) const {
    return quantity_ * instrument_->price(data);
}

double Position::getPnL(const core::MarketData& data) const {
    double currentPrice = instrument_->price(data);
    return quantity_ * (currentPrice - entryPrice_);
}

void Position::setMetadata(const std::string& key, double value) {
    metadata_[key] = value;
}

double Position::getMetadata(const std::string& key) const {
    auto it = metadata_.find(key);
    if (it != metadata_.end()) {
        return it->second;
    }
    return 0.0; // Default value for missing keys
}

bool Position::hasMetadata(const std::string& key) const {
    return metadata_.find(key) != metadata_.end();
}

} // namespace VolatilityArbitrage
