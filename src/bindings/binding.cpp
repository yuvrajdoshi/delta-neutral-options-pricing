#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <pybind11/numpy.h>

// Core includes
#include "core/TimeSeries.h"
#include "core/DateTime.h"
#include "core/MarketData.h"
#include "core/MathUtils.h"

// Instruments includes
#include "instruments/Instrument.h"
#include "instruments/Equity.h"
#include "instruments/Option.h"
#include "instruments/InstrumentFactory.h"

// Models includes
#include "models/VolatilityModel.h"
#include "models/GARCHModel.h"
#include "models/PricingModel.h"
#include "models/BSMPricingModel.h"
#include "models/Greeks.h"
#include "models/ModelFactory.h"

// Strategy includes
#include "strategy/Signal.h"
#include "strategy/Portfolio.h"
#include "strategy/Position.h"
#include "strategy/Strategy.h"
#include "strategy/VolatilityArbitrageStrategy.h"
#include "strategy/BacktestParameters.h"
#include "strategy/BacktestResult.h"
#include "strategy/BacktestEngine.h"
#include "strategy/Trade.h"

namespace py = pybind11;
using namespace VolatilityArbitrage;

PYBIND11_MODULE(volatility_arbitrage, m) {
    m.doc() = "Volatility Arbitrage Trading System - High-performance C++ library with Python bindings";
    
    // =================================================================
    // CORE LAYER BINDINGS
    // =================================================================
    
    // DateTime class
    py::class_<core::DateTime>(m, "DateTime")
        .def(py::init<>())
        .def(py::init<int, int, int>())
        .def(py::init<int, int, int, int, int, int>())
        .def_static("now", &core::DateTime::now)
        .def_static("fromString", &core::DateTime::fromString)
        .def("year", &core::DateTime::year)
        .def("month", &core::DateTime::month)
        .def("day", &core::DateTime::day)
        .def("hour", &core::DateTime::hour)
        .def("minute", &core::DateTime::minute)
        .def("second", &core::DateTime::second)
        .def("toString", &core::DateTime::toString, py::arg("format") = "%Y-%m-%d %H:%M:%S")
        .def("isValid", &core::DateTime::isValid)
        .def("__eq__", &core::DateTime::operator==)
        .def("__ne__", &core::DateTime::operator!=)
        .def("__lt__", &core::DateTime::operator<)
        .def("__le__", &core::DateTime::operator<=)
        .def("__gt__", &core::DateTime::operator>)
        .def("__ge__", &core::DateTime::operator>=)
        .def("__repr__", [](const core::DateTime& dt) {
            return "DateTime('" + dt.toString() + "')";
        });
    
    // TimeSeries class
    py::class_<core::TimeSeries>(m, "TimeSeries")
        .def(py::init<>())
        .def(py::init<const std::string&>())
        .def(py::init<const std::vector<core::DateTime>&, const std::vector<double>&, const std::string&>(),
             py::arg("timestamps"), py::arg("values"), py::arg("name") = "")
        .def("addDataPoint", &core::TimeSeries::addDataPoint)
        .def("clear", &core::TimeSeries::clear)
        .def("size", &core::TimeSeries::size)
        .def("empty", &core::TimeSeries::empty)
        .def("getValue", py::overload_cast<size_t>(&core::TimeSeries::getValue, py::const_))
        .def("getValue", py::overload_cast<const core::DateTime&>(&core::TimeSeries::getValue, py::const_))
        .def("getTimestamp", &core::TimeSeries::getTimestamp)
        .def("getDataPoint", &core::TimeSeries::getDataPoint)
        .def("getValues", &core::TimeSeries::getValues)
        .def("getTimestamps", &core::TimeSeries::getTimestamps)
        .def("getSubseries", py::overload_cast<const core::DateTime&, const core::DateTime&>(&core::TimeSeries::getSubseries, py::const_))
        .def("getSubseries", py::overload_cast<size_t, size_t>(&core::TimeSeries::getSubseries, py::const_))
        .def("mean", &core::TimeSeries::mean)
        .def("variance", &core::TimeSeries::variance)
        .def("standardDeviation", &core::TimeSeries::standardDeviation)
        .def("skewness", &core::TimeSeries::skewness)
        .def("kurtosis", &core::TimeSeries::kurtosis)
        .def("autocorrelation", &core::TimeSeries::autocorrelation)
        .def("diff", &core::TimeSeries::diff)
        .def("pctChange", &core::TimeSeries::pctChange)
        .def("logReturn", &core::TimeSeries::logReturn)
        .def("rollingMean", &core::TimeSeries::rollingMean)
        .def("rollingStd", &core::TimeSeries::rollingStd)
        .def("saveToCSV", &core::TimeSeries::saveToCSV)
        .def_static("loadFromCSV", &core::TimeSeries::loadFromCSV)
        .def("__len__", &core::TimeSeries::size)
        .def("__repr__", [](const core::TimeSeries& ts) {
            return "TimeSeries(size=" + std::to_string(ts.size()) + ")";
        });
    
    // MarketData class
    py::class_<core::MarketData>(m, "MarketData")
        .def(py::init<>())
        .def(py::init<const std::string&, const core::DateTime&, double, double, double, double, double>())
        .def("getSymbol", &core::MarketData::getSymbol)
        .def("getTimestamp", &core::MarketData::getTimestamp)
        .def("getOpen", &core::MarketData::getOpen)
        .def("getHigh", &core::MarketData::getHigh)
        .def("getLow", &core::MarketData::getLow)
        .def("getClose", &core::MarketData::getClose)
        .def("getVolume", &core::MarketData::getVolume)
        .def("setAdditionalData", &core::MarketData::setAdditionalData)
        .def("getAdditionalData", &core::MarketData::getAdditionalData)
        .def("hasAdditionalData", &core::MarketData::hasAdditionalData)
        .def_static("fromCSV", &core::MarketData::fromCSV)
        .def_static("extractTimeSeries", &core::MarketData::extractTimeSeries, 
                   py::arg("data"), py::arg("field") = "close")
        .def("__repr__", [](const core::MarketData& md) {
            return "MarketData(" + md.getSymbol() + ", " + md.getTimestamp().toString() + 
                   ", close=" + std::to_string(md.getClose()) + ")";
        });
    
    // Math utilities
    py::module_ math_module = m.def_submodule("Math", "Mathematical utilities");
    math_module.def("mean", &core::mean);
    math_module.def("variance", &core::variance);
    math_module.def("standardDeviation", &core::standardDeviation);
    math_module.def("skewness", &core::skewness);
    math_module.def("kurtosis", &core::kurtosis);
    math_module.def("correlation", &core::correlation);
    math_module.def("normalPDF", &core::normalPDF, py::arg("x"), py::arg("mean") = 0.0, py::arg("stdDev") = 1.0);
    math_module.def("normalCDF", &core::normalCDF, py::arg("x"), py::arg("mean") = 0.0, py::arg("stdDev") = 1.0);
    math_module.def("normalInverseCDF", &core::normalInverseCDF, py::arg("p"), py::arg("mean") = 0.0, py::arg("stdDev") = 1.0);
    
    // =================================================================
    // INSTRUMENTS LAYER BINDINGS
    // =================================================================
    
    // InstrumentType enum
    py::enum_<instruments::InstrumentType>(m, "InstrumentType")
        .value("EQUITY", instruments::InstrumentType::EQUITY)
        .value("EUROPEAN_OPTION", instruments::InstrumentType::EUROPEAN_OPTION)
        .value("AMERICAN_OPTION", instruments::InstrumentType::AMERICAN_OPTION)
        .value("FUTURE", instruments::InstrumentType::FUTURE)
        .value("BOND", instruments::InstrumentType::BOND)
        .export_values();
    
    // OptionType enum
    py::enum_<instruments::OptionType>(m, "OptionType")
        .value("Call", instruments::OptionType::Call)
        .value("Put", instruments::OptionType::Put)
        .export_values();
    
    // ExerciseStyle enum
    py::enum_<instruments::ExerciseStyle>(m, "ExerciseStyle")
        .value("European", instruments::ExerciseStyle::European)
        .value("American", instruments::ExerciseStyle::American)
        .export_values();
    
    // Instrument base class
    py::class_<instruments::Instrument>(m, "Instrument")
        .def("price", &instruments::Instrument::price)
        .def("getSymbol", &instruments::Instrument::getSymbol)
        .def("getType", &instruments::Instrument::getType)
        .def("calculateRiskMetrics", &instruments::Instrument::calculateRiskMetrics);
    
    // Equity class
    py::class_<instruments::Equity, instruments::Instrument>(m, "Equity")
        .def(py::init<const std::string&, double>(), py::arg("symbol"), py::arg("shares") = 1.0)
        .def("getShares", &instruments::Equity::getShares)
        .def("setShares", &instruments::Equity::setShares);
    
    // Derivative base class
    py::class_<instruments::Derivative, instruments::Instrument>(m, "Derivative")
        .def("getUnderlyingSymbol", &instruments::Derivative::getUnderlyingSymbol)
        .def("getExpiryDate", &instruments::Derivative::getExpiryDate);
    
    // Option class
    py::class_<instruments::Option, instruments::Derivative>(m, "Option")
        .def("getStrike", &instruments::Option::getStrike)
        .def("getOptionType", &instruments::Option::getOptionType)
        .def("getExerciseStyle", &instruments::Option::getExerciseStyle)
        .def("delta", &instruments::Option::delta)
        .def("gamma", &instruments::Option::gamma)
        .def("vega", &instruments::Option::vega)
        .def("theta", &instruments::Option::theta)
        .def("rho", &instruments::Option::rho);
    
    // InstrumentFactory
    py::class_<instruments::InstrumentFactory>(m, "InstrumentFactory")
        .def_static("createEquity", &instruments::InstrumentFactory::createEquity, 
                   py::arg("symbol"), py::arg("shares") = 1.0)
        .def_static("createEuropeanCall", &instruments::InstrumentFactory::createEuropeanCall)
        .def_static("createEuropeanPut", &instruments::InstrumentFactory::createEuropeanPut)
        .def_static("createAmericanCall", &instruments::InstrumentFactory::createAmericanCall)
        .def_static("createAmericanPut", &instruments::InstrumentFactory::createAmericanPut);
    
    // =================================================================
    // MODELS LAYER BINDINGS
    // =================================================================
    
    // Greeks class
    py::class_<models::Greeks>(m, "Greeks")
        .def(py::init<double, double, double, double, double>())
        .def_readwrite("delta", &models::Greeks::delta)
        .def_readwrite("gamma", &models::Greeks::gamma)
        .def_readwrite("vega", &models::Greeks::vega)
        .def_readwrite("theta", &models::Greeks::theta)
        .def_readwrite("rho", &models::Greeks::rho)
        .def("__repr__", [](const models::Greeks& g) {
            return "Greeks(delta=" + std::to_string(g.delta) + 
                   ", gamma=" + std::to_string(g.gamma) + 
                   ", vega=" + std::to_string(g.vega) + 
                   ", theta=" + std::to_string(g.theta) + 
                   ", rho=" + std::to_string(g.rho) + ")";
        });
    
    // VolatilityModel base class
    py::class_<models::VolatilityModel>(m, "VolatilityModel")
        .def("calibrate", &models::VolatilityModel::calibrate)
        .def("forecast", &models::VolatilityModel::forecast)
        .def("forecastSeries", &models::VolatilityModel::forecastSeries)
        .def("getModelName", &models::VolatilityModel::getModelName)
        .def("getParameters", &models::VolatilityModel::getParameters)
        .def("calculateLogLikelihood", &models::VolatilityModel::calculateLogLikelihood)
        .def("calculateAIC", &models::VolatilityModel::calculateAIC)
        .def("calculateBIC", &models::VolatilityModel::calculateBIC);
    
    // GARCHModel class
    py::class_<models::GARCHModel, models::VolatilityModel>(m, "GARCHModel")
        .def(py::init<>())
        .def(py::init<double, double, double>())
        .def("getOmega", &models::GARCHModel::getOmega)
        .def("getAlpha", &models::GARCHModel::getAlpha)
        .def("getBeta", &models::GARCHModel::getBeta)
        .def("getLastVariance", &models::GARCHModel::getLastVariance)
        .def("getLongRunVariance", &models::GARCHModel::getLongRunVariance)
        .def("isStationary", &models::GARCHModel::isStationary)
        .def("isCalibrated", &models::GARCHModel::isCalibrated);
    
    // PricingModel base class
    py::class_<models::PricingModel>(m, "PricingModel")
        .def("price", &models::PricingModel::price)
        .def("calculateGreeks", &models::PricingModel::calculateGreeks)
        .def("getModelName", &models::PricingModel::getModelName);
    
    // BSMPricingModel class
    py::class_<models::BSMPricingModel, models::PricingModel>(m, "BSMPricingModel")
        .def(py::init<>());
    
    // ModelFactory
    py::class_<models::ModelFactory>(m, "ModelFactory")
        .def_static("createGARCHModel", &models::ModelFactory::createGARCHModel,
                   py::arg("omega") = 0.0, py::arg("alpha") = 0.0, py::arg("beta") = 0.0)
        .def_static("createBSMPricingModel", &models::ModelFactory::createBSMPricingModel);
    
    // =================================================================
    // STRATEGY LAYER BINDINGS
    // =================================================================
    
    // Signal class
    py::enum_<Signal::Type>(m, "SignalType")
        .value("BUY", Signal::Type::BUY)
        .value("SELL", Signal::Type::SELL)
        .value("HOLD", Signal::Type::HOLD)
        .export_values();
    
    py::class_<Signal>(m, "Signal")
        .def(py::init<>())
        .def(py::init<Signal::Type, double, const std::string&, const core::DateTime&>())
        .def_readwrite("type", &Signal::type)
        .def_readwrite("strength", &Signal::strength)
        .def_readwrite("instrumentId", &Signal::instrumentId)
        .def_readwrite("timestamp", &Signal::timestamp)
        .def_readwrite("metadata", &Signal::metadata)
        .def("isActionable", &Signal::isActionable)
        .def("toString", &Signal::toString)
        .def("__repr__", [](const Signal& s) {
            return "Signal(" + s.toString() + ")";
        });
    
    // Trade class
    py::enum_<Trade::Action>(m, "TradeAction")
        .value("BUY", Trade::Action::BUY)
        .value("SELL", Trade::Action::SELL)
        .export_values();
    
    py::class_<Trade>(m, "Trade")
        .def(py::init<>())
        .def(py::init<const std::string&, Trade::Action, double, double, const core::DateTime&, double>(),
             py::arg("instrumentId"), py::arg("action"), py::arg("quantity"), 
             py::arg("price"), py::arg("timestamp"), py::arg("transactionCost") = 0.0)
        .def_readwrite("instrumentId", &Trade::instrumentId)
        .def_readwrite("action", &Trade::action)
        .def_readwrite("quantity", &Trade::quantity)
        .def_readwrite("price", &Trade::price)
        .def_readwrite("timestamp", &Trade::timestamp)
        .def_readwrite("transactionCost", &Trade::transactionCost)
        .def("getValue", &Trade::getValue)
        .def("getNetValue", &Trade::getNetValue)
        .def("toString", &Trade::toString)
        .def("__repr__", [](const Trade& t) {
            return "Trade(" + t.toString() + ")";
        });
    
    // BacktestParameters class
    py::class_<BacktestParameters>(m, "BacktestParameters")
        .def(py::init<>())
        .def("getStartDate", &BacktestParameters::getStartDate)
        .def("setStartDate", &BacktestParameters::setStartDate)
        .def("getEndDate", &BacktestParameters::getEndDate)
        .def("setEndDate", &BacktestParameters::setEndDate)
        .def("getInitialCapital", &BacktestParameters::getInitialCapital)
        .def("setInitialCapital", &BacktestParameters::setInitialCapital)
        .def("getSymbols", &BacktestParameters::getSymbols)
        .def("setSymbols", &BacktestParameters::setSymbols)
        .def("getIncludeTransactionCosts", &BacktestParameters::getIncludeTransactionCosts)
        .def("setIncludeTransactionCosts", &BacktestParameters::setIncludeTransactionCosts)
        .def("getTransactionCostPerTrade", &BacktestParameters::getTransactionCostPerTrade)
        .def("setTransactionCostPerTrade", &BacktestParameters::setTransactionCostPerTrade)
        .def("getTransactionCostPercentage", &BacktestParameters::getTransactionCostPercentage)
        .def("setTransactionCostPercentage", &BacktestParameters::setTransactionCostPercentage);
    
    // BacktestResult class
    py::class_<BacktestResult>(m, "BacktestResult")
        .def(py::init<>())
        .def(py::init<const core::TimeSeries&, const std::vector<Trade>&>())
        .def("getEquityCurve", &BacktestResult::getEquityCurve)
        .def("getTrades", &BacktestResult::getTrades)
        .def("getTradeCount", &BacktestResult::getTradeCount)
        .def("getSharpeRatio", &BacktestResult::getSharpeRatio)
        .def("getSortinoRatio", &BacktestResult::getSortinoRatio)
        .def("getMaxDrawdown", &BacktestResult::getMaxDrawdown)
        .def("getTotalReturn", &BacktestResult::getTotalReturn)
        .def("getAnnualizedReturn", &BacktestResult::getAnnualizedReturn)
        .def("getAnnualizedVolatility", &BacktestResult::getAnnualizedVolatility)
        .def("getWinRate", &BacktestResult::getWinRate)
        .def("getProfitFactor", &BacktestResult::getProfitFactor)
        .def("setMetric", &BacktestResult::setMetric)
        .def("getMetric", &BacktestResult::getMetric)
        .def("hasMetric", &BacktestResult::hasMetric)
        .def("getAllMetrics", &BacktestResult::getAllMetrics)
        .def("getDrawdownSeries", &BacktestResult::getDrawdownSeries)
        .def("getDrawdownPeriods", &BacktestResult::getDrawdownPeriods)
        .def("getReturnsByMonth", &BacktestResult::getReturnsByMonth)
        .def("getReturnsByYear", &BacktestResult::getReturnsByYear)
        .def("getSummary", &BacktestResult::getSummary)
        .def("printSummary", &BacktestResult::printSummary);
    
    // BacktestEngine class
    py::class_<BacktestEngine>(m, "BacktestEngine")
        .def(py::init<>())
        .def("loadMarketData", &BacktestEngine::loadMarketData)
        .def("addMarketData", &BacktestEngine::addMarketData)
        .def("clearMarketData", &BacktestEngine::clearMarketData)
        .def("hasMarketData", &BacktestEngine::hasMarketData)
        .def("getAvailableSymbols", &BacktestEngine::getAvailableSymbols)
        .def("run", py::overload_cast<const Strategy&, const BacktestParameters&>(&BacktestEngine::run))
        .def("runParameterSweep", &BacktestEngine::runParameterSweep)
        .def("runMonteCarloSimulation", &BacktestEngine::runMonteCarloSimulation, 
             py::arg("strategy"), py::arg("params"), py::arg("numSimulations") = 1000)
        .def("getEngineInfo", &BacktestEngine::getEngineInfo);
}
