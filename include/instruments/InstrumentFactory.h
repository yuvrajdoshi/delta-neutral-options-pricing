#ifndef INSTRUMENT_FACTORY_H
#define INSTRUMENT_FACTORY_H

#include "instruments/Instrument.h"
#include "instruments/Equity.h"
#include "instruments/Option.h"
#include "core/DateTime.h"
#include <map>
#include <string>
#include <any>

namespace instruments {

// Structure for instrument parameters
struct InstrumentParameters {
    InstrumentType type;
    std::string symbol;
    std::map<std::string, std::any> parameters;
};

/**
 * Factory class for creating financial instruments
 */
class InstrumentFactory {
public:
    // Factory methods
    static std::unique_ptr<Instrument> createInstrument(const InstrumentParameters& params);
    static std::unique_ptr<Equity> createEquity(const std::string& symbol, double shares = 1.0);
    static std::unique_ptr<Option> createEuropeanCall(const std::string& underlyingSymbol, 
                                                     const core::DateTime& expiryDate, double strike);
    static std::unique_ptr<Option> createEuropeanPut(const std::string& underlyingSymbol, 
                                                    const core::DateTime& expiryDate, double strike);
    static std::unique_ptr<Option> createAmericanCall(const std::string& underlyingSymbol, 
                                                     const core::DateTime& expiryDate, double strike);
    static std::unique_ptr<Option> createAmericanPut(const std::string& underlyingSymbol, 
                                                    const core::DateTime& expiryDate, double strike);
};

} // namespace instruments

#endif // INSTRUMENT_FACTORY_H
