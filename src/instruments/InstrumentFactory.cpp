#include "instruments/InstrumentFactory.h"
#include <stdexcept>

namespace instruments {

std::unique_ptr<Instrument> InstrumentFactory::createInstrument(const InstrumentParameters& params) {
    switch (params.type) {
        case InstrumentType::EQUITY: {
            auto symbolIt = params.parameters.find("symbol");
            auto sharesIt = params.parameters.find("shares");
            
            if (symbolIt == params.parameters.end()) {
                throw std::invalid_argument("Symbol parameter required for equity");
            }
            
            std::string symbol = std::any_cast<std::string>(symbolIt->second);
            double shares = 1.0;
            
            if (sharesIt != params.parameters.end()) {
                shares = std::any_cast<double>(sharesIt->second);
            }
            
            return createEquity(symbol, shares);
        }
        
        case InstrumentType::EUROPEAN_OPTION: {
            auto symbolIt = params.parameters.find("underlying_symbol");
            auto expiryIt = params.parameters.find("expiry");
            auto strikeIt = params.parameters.find("strike");
            auto typeIt = params.parameters.find("option_type");
            
            if (symbolIt == params.parameters.end() ||
                expiryIt == params.parameters.end() ||
                strikeIt == params.parameters.end() ||
                typeIt == params.parameters.end()) {
                throw std::invalid_argument("Missing required parameters for European option");
            }
            
            std::string symbol = std::any_cast<std::string>(symbolIt->second);
            core::DateTime expiry = std::any_cast<core::DateTime>(expiryIt->second);
            double strike = std::any_cast<double>(strikeIt->second);
            OptionType optionType = std::any_cast<OptionType>(typeIt->second);
            
            if (optionType == OptionType::Call) {
                return createEuropeanCall(symbol, expiry, strike);
            } else {
                return createEuropeanPut(symbol, expiry, strike);
            }
        }
        
        case InstrumentType::AMERICAN_OPTION: {
            auto symbolIt = params.parameters.find("underlying_symbol");
            auto expiryIt = params.parameters.find("expiry");
            auto strikeIt = params.parameters.find("strike");
            auto typeIt = params.parameters.find("option_type");
            
            if (symbolIt == params.parameters.end() ||
                expiryIt == params.parameters.end() ||
                strikeIt == params.parameters.end() ||
                typeIt == params.parameters.end()) {
                throw std::invalid_argument("Missing required parameters for American option");
            }
            
            std::string symbol = std::any_cast<std::string>(symbolIt->second);
            core::DateTime expiry = std::any_cast<core::DateTime>(expiryIt->second);
            double strike = std::any_cast<double>(strikeIt->second);
            OptionType optionType = std::any_cast<OptionType>(typeIt->second);
            
            if (optionType == OptionType::Call) {
                return createAmericanCall(symbol, expiry, strike);
            } else {
                return createAmericanPut(symbol, expiry, strike);
            }
        }
        
        default:
            throw std::invalid_argument("Unsupported instrument type");
    }
}

std::unique_ptr<Equity> InstrumentFactory::createEquity(const std::string& symbol, double shares) {
    return std::make_unique<Equity>(symbol, shares);
}

std::unique_ptr<Option> InstrumentFactory::createEuropeanCall(const std::string& underlyingSymbol,
                                                             const core::DateTime& expiryDate, 
                                                             double strike) {
    return std::make_unique<EuropeanOption>(underlyingSymbol, expiryDate, strike, OptionType::Call);
}

std::unique_ptr<Option> InstrumentFactory::createEuropeanPut(const std::string& underlyingSymbol,
                                                            const core::DateTime& expiryDate, 
                                                            double strike) {
    return std::make_unique<EuropeanOption>(underlyingSymbol, expiryDate, strike, OptionType::Put);
}

std::unique_ptr<Option> InstrumentFactory::createAmericanCall(const std::string& underlyingSymbol,
                                                             const core::DateTime& expiryDate, 
                                                             double strike) {
    return std::make_unique<AmericanOption>(underlyingSymbol, expiryDate, strike, OptionType::Call);
}

std::unique_ptr<Option> InstrumentFactory::createAmericanPut(const std::string& underlyingSymbol,
                                                            const core::DateTime& expiryDate, 
                                                            double strike) {
    return std::make_unique<AmericanOption>(underlyingSymbol, expiryDate, strike, OptionType::Put);
}

} // namespace instruments
