"""Strategy and backtesting module."""

import pandas as pd
import numpy as np
from typing import List, Dict, Optional, Union, Tuple, Any, Callable
from datetime import datetime, date
import volatility_arbitrage as va
from .time_series import TimeSeriesWrapper, MarketDataWrapper
from .instruments import InstrumentWrapper, PortfolioWrapper

class SignalWrapper:
    """Enhanced Python wrapper for trading signals."""
    
    def __init__(self, signal_type: str = "HOLD", strength: float = 0.0, 
                 instrument_id: str = "", timestamp: Optional[datetime] = None):
        """Initialize signal wrapper.
        
        Args:
            signal_type: Signal type ("BUY", "SELL", "HOLD")
            strength: Signal strength
            instrument_id: Instrument identifier
            timestamp: Signal timestamp
        """
        # Convert string to enum
        if signal_type.upper() == "BUY":
            sig_type = va.SignalType.BUY
        elif signal_type.upper() == "SELL":
            sig_type = va.SignalType.SELL
        else:
            sig_type = va.SignalType.HOLD
        
        # Convert timestamp
        if timestamp is None:
            dt = va.DateTime.now()
        elif isinstance(timestamp, datetime):
            dt = va.DateTime(timestamp.year, timestamp.month, timestamp.day,
                           timestamp.hour, timestamp.minute, timestamp.second,
                           timestamp.microsecond // 1000)
        else:
            dt = timestamp
        
        self._signal = va.Signal(sig_type, strength, instrument_id, dt)
    
    @property
    def type(self) -> str:
        """Get signal type as string."""
        sig_type = self._signal.type
        if sig_type == va.SignalType.BUY:
            return "BUY"
        elif sig_type == va.SignalType.SELL:
            return "SELL"
        else:
            return "HOLD"
    
    @property
    def strength(self) -> float:
        """Get signal strength."""
        return self._signal.strength
    
    @property
    def instrument_id(self) -> str:
        """Get instrument ID."""
        return self._signal.instrumentId
    
    @property
    def timestamp(self) -> datetime:
        """Get timestamp as Python datetime."""
        dt = self._signal.timestamp
        return datetime(dt.year(), dt.month(), dt.day(),
                       dt.hour(), dt.minute(), dt.second(),
                       dt.millisecond() * 1000)
    
    def is_actionable(self) -> bool:
        """Check if signal is actionable."""
        return self._signal.isActionable()
    
    def __repr__(self) -> str:
        return f"SignalWrapper(type='{self.type}', strength={self.strength}, instrument='{self.instrument_id}')"


class TradeWrapper:
    """Enhanced Python wrapper for trade records."""
    
    def __init__(self, instrument_id: str, action: str, quantity: float, 
                 price: float, timestamp: Optional[datetime] = None, 
                 transaction_cost: float = 0.0):
        """Initialize trade wrapper.
        
        Args:
            instrument_id: Instrument identifier
            action: Trade action ("BUY" or "SELL")
            quantity: Trade quantity
            price: Trade price
            timestamp: Trade timestamp
            transaction_cost: Transaction cost
        """
        # Convert action string to enum
        if action.upper() == "BUY":
            trade_action = va.TradeAction.BUY
        else:
            trade_action = va.TradeAction.SELL
        
        # Convert timestamp
        if timestamp is None:
            dt = va.DateTime.now()
        elif isinstance(timestamp, datetime):
            dt = va.DateTime(timestamp.year, timestamp.month, timestamp.day,
                           timestamp.hour, timestamp.minute, timestamp.second,
                           timestamp.microsecond // 1000)
        else:
            dt = timestamp
        
        self._trade = va.Trade(instrument_id, trade_action, quantity, price, dt, transaction_cost)
    
    @property
    def instrument_id(self) -> str:
        """Get instrument ID."""
        return self._trade.instrumentId
    
    @property
    def action(self) -> str:
        """Get trade action as string."""
        if self._trade.action == va.TradeAction.BUY:
            return "BUY"
        else:
            return "SELL"
    
    @property
    def quantity(self) -> float:
        """Get trade quantity."""
        return self._trade.quantity
    
    @property
    def price(self) -> float:
        """Get trade price."""
        return self._trade.price
    
    @property
    def timestamp(self) -> datetime:
        """Get timestamp as Python datetime."""
        dt = self._trade.timestamp
        return datetime(dt.year(), dt.month(), dt.day(),
                       dt.hour(), dt.minute(), dt.second(),
                       dt.millisecond() * 1000)
    
    @property
    def transaction_cost(self) -> float:
        """Get transaction cost."""
        return self._trade.transactionCost
    
    @property
    def value(self) -> float:
        """Get trade value."""
        return self._trade.getValue()
    
    @property
    def net_value(self) -> float:
        """Get net trade value (after costs)."""
        return self._trade.getNetValue()
    
    def to_dict(self) -> Dict[str, Any]:
        """Convert to dictionary."""
        return {
            'instrument_id': self.instrument_id,
            'action': self.action,
            'quantity': self.quantity,
            'price': self.price,
            'timestamp': self.timestamp,
            'transaction_cost': self.transaction_cost,
            'value': self.value,
            'net_value': self.net_value
        }
    
    def __repr__(self) -> str:
        return (f"TradeWrapper({self.action} {self.quantity} {self.instrument_id} "
                f"@ ${self.price:.2f})")


class BacktestParametersWrapper:
    """Enhanced Python wrapper for backtest parameters."""
    
    def __init__(self, start_date: Union[datetime, date, str], 
                 end_date: Union[datetime, date, str],
                 initial_capital: float = 1000000.0,
                 symbols: Optional[List[str]] = None,
                 include_transaction_costs: bool = True,
                 transaction_cost_per_trade: float = 5.0,
                 transaction_cost_percentage: float = 0.001):
        """Initialize backtest parameters wrapper.
        
        Args:
            start_date: Start date for backtest
            end_date: End date for backtest
            initial_capital: Initial capital
            symbols: List of symbols to trade
            include_transaction_costs: Whether to include transaction costs
            transaction_cost_per_trade: Fixed cost per trade
            transaction_cost_percentage: Percentage cost per trade
        """
        self._params = va.BacktestParameters()
        
        # Convert dates
        start_dt = self._convert_date(start_date)
        end_dt = self._convert_date(end_date)
        
        # Set parameters
        self._params.setStartDate(start_dt)
        self._params.setEndDate(end_dt)
        self._params.setInitialCapital(initial_capital)
        
        if symbols:
            self._params.setSymbols(symbols)
        
        self._params.setIncludeTransactionCosts(include_transaction_costs)
        self._params.setTransactionCostPerTrade(transaction_cost_per_trade)
        self._params.setTransactionCostPercentage(transaction_cost_percentage)
    
    def _convert_date(self, date_input: Union[datetime, date, str]) -> va.DateTime:
        """Convert various date formats to DateTime."""
        if isinstance(date_input, str):
            return va.DateTime.fromString(date_input)
        elif isinstance(date_input, datetime):
            return va.DateTime(date_input.year, date_input.month, date_input.day,
                             date_input.hour, date_input.minute, date_input.second,
                             date_input.microsecond // 1000)
        elif isinstance(date_input, date):
            return va.DateTime(date_input.year, date_input.month, date_input.day)
        else:
            return date_input
    
    @property
    def start_date(self) -> datetime:
        """Get start date."""
        dt = self._params.getStartDate()
        return datetime(dt.year(), dt.month(), dt.day())
    
    @property
    def end_date(self) -> datetime:
        """Get end date."""
        dt = self._params.getEndDate()
        return datetime(dt.year(), dt.month(), dt.day())
    
    @property
    def initial_capital(self) -> float:
        """Get initial capital."""
        return self._params.getInitialCapital()
    
    @property
    def symbols(self) -> List[str]:
        """Get symbols."""
        return self._params.getSymbols()
    
    def __repr__(self) -> str:
        return (f"BacktestParametersWrapper(start={self.start_date.date()}, "
                f"end={self.end_date.date()}, capital=${self.initial_capital:,.0f})")


class BacktestResultWrapper:
    """Enhanced Python wrapper for backtest results."""
    
    def __init__(self, result: va.BacktestResult):
        """Initialize backtest result wrapper.
        
        Args:
            result: C++ BacktestResult object
        """
        self._result = result
    
    @property
    def equity_curve(self) -> TimeSeriesWrapper:
        """Get equity curve."""
        return TimeSeriesWrapper(self._result.getEquityCurve())
    
    @property
    def trades(self) -> List[TradeWrapper]:
        """Get trades as wrapper objects."""
        cpp_trades = self._result.getTrades()
        return [TradeWrapper(t.instrumentId, 
                           "BUY" if t.action == va.TradeAction.BUY else "SELL",
                           t.quantity, t.price, 
                           datetime(t.timestamp.year(), t.timestamp.month(), t.timestamp.day(),
                                  t.timestamp.hour(), t.timestamp.minute(), t.timestamp.second(),
                                  t.timestamp.millisecond() * 1000),
                           t.transactionCost) 
                for t in cpp_trades]
    
    @property
    def trade_count(self) -> int:
        """Get trade count."""
        return self._result.getTradeCount()
    
    # Performance metrics
    @property
    def sharpe_ratio(self) -> float:
        """Get Sharpe ratio."""
        return self._result.getSharpeRatio()
    
    @property
    def sortino_ratio(self) -> float:
        """Get Sortino ratio."""
        return self._result.getSortinoRatio()
    
    @property
    def max_drawdown(self) -> float:
        """Get maximum drawdown."""
        return self._result.getMaxDrawdown()
    
    @property
    def total_return(self) -> float:
        """Get total return."""
        return self._result.getTotalReturn()
    
    @property
    def annualized_return(self) -> float:
        """Get annualized return."""
        return self._result.getAnnualizedReturn()
    
    @property
    def annualized_volatility(self) -> float:
        """Get annualized volatility."""
        return self._result.getAnnualizedVolatility()
    
    @property
    def win_rate(self) -> float:
        """Get win rate."""
        return self._result.getWinRate()
    
    @property
    def profit_factor(self) -> float:
        """Get profit factor."""
        return self._result.getProfitFactor()
    
    def get_metric(self, name: str) -> Optional[float]:
        """Get custom metric."""
        if self._result.hasMetric(name):
            return self._result.getMetric(name)
        return None
    
    @property
    def all_metrics(self) -> Dict[str, float]:
        """Get all metrics."""
        return dict(self._result.getAllMetrics())
    
    @property
    def drawdown_series(self) -> TimeSeriesWrapper:
        """Get drawdown series."""
        return TimeSeriesWrapper(self._result.getDrawdownSeries())
    
    def trades_df(self) -> pd.DataFrame:
        """Get trades as DataFrame."""
        if not self.trades:
            return pd.DataFrame()
        
        return pd.DataFrame([trade.to_dict() for trade in self.trades])
    
    def equity_curve_df(self) -> pd.DataFrame:
        """Get equity curve as DataFrame."""
        equity_ts = self.equity_curve
        series = equity_ts.to_pandas()
        return pd.DataFrame({'equity': series})
    
    def performance_summary(self) -> Dict[str, float]:
        """Get performance summary."""
        return {
            'total_return': self.total_return,
            'annualized_return': self.annualized_return,
            'annualized_volatility': self.annualized_volatility,
            'sharpe_ratio': self.sharpe_ratio,
            'sortino_ratio': self.sortino_ratio,
            'max_drawdown': self.max_drawdown,
            'win_rate': self.win_rate,
            'profit_factor': self.profit_factor,
            'trade_count': self.trade_count
        }
    
    def print_summary(self):
        """Print performance summary."""
        self._result.printSummary()
    
    def __repr__(self) -> str:
        return (f"BacktestResultWrapper(return={self.total_return:.2%}, "
                f"sharpe={self.sharpe_ratio:.3f}, trades={self.trade_count})")


class BacktestEngineWrapper:
    """Enhanced Python wrapper for backtesting engine."""
    
    def __init__(self):
        """Initialize backtest engine wrapper."""
        self._engine = va.BacktestEngine()
    
    def load_market_data(self, symbol: str, filename: str):
        """Load market data from CSV file.
        
        Args:
            symbol: Symbol identifier
            filename: CSV filename
        """
        self._engine.loadMarketData(symbol, filename)
    
    def add_market_data(self, symbol: str, data: Union[List[va.MarketData], MarketDataWrapper]):
        """Add market data for symbol.
        
        Args:
            symbol: Symbol identifier
            data: Market data
        """
        if isinstance(data, MarketDataWrapper):
            for md in data._data:
                self._engine.addMarketData(symbol, md)
        else:
            for md in data:
                self._engine.addMarketData(symbol, md)
    
    def clear_market_data(self, symbol: Optional[str] = None):
        """Clear market data.
        
        Args:
            symbol: Symbol to clear (if None, clear all)
        """
        if symbol:
            self._engine.clearMarketData(symbol)
        else:
            self._engine.clearMarketData()
    
    def has_market_data(self, symbol: str) -> bool:
        """Check if market data exists for symbol."""
        return self._engine.hasMarketData(symbol)
    
    @property
    def available_symbols(self) -> List[str]:
        """Get available symbols."""
        return self._engine.getAvailableSymbols()
    
    def run(self, strategy: va.Strategy, parameters: BacktestParametersWrapper) -> BacktestResultWrapper:
        """Run backtest.
        
        Args:
            strategy: Trading strategy
            parameters: Backtest parameters
        
        Returns:
            Backtest results
        """
        result = self._engine.run(strategy, parameters._params)
        return BacktestResultWrapper(result)
    
    def run_parameter_sweep(self, strategy: va.Strategy, base_params: BacktestParametersWrapper,
                           parameter_ranges: Dict[str, List[Any]]) -> List[BacktestResultWrapper]:
        """Run parameter sweep.
        
        Args:
            strategy: Trading strategy
            base_params: Base parameters
            parameter_ranges: Parameter ranges to sweep
        
        Returns:
            List of backtest results
        """
        # Convert parameter ranges to C++ format
        cpp_ranges = {}
        for param, values in parameter_ranges.items():
            cpp_ranges[param] = [str(v) for v in values]  # Convert to strings
        
        results = self._engine.runParameterSweep(strategy, base_params._params, cpp_ranges)
        return [BacktestResultWrapper(result) for result in results]
    
    def run_monte_carlo(self, strategy: va.Strategy, parameters: BacktestParametersWrapper,
                       num_simulations: int = 1000) -> List[BacktestResultWrapper]:
        """Run Monte Carlo simulation.
        
        Args:
            strategy: Trading strategy
            parameters: Backtest parameters
            num_simulations: Number of simulations
        
        Returns:
            List of backtest results
        """
        results = self._engine.runMonteCarloSimulation(strategy, parameters._params, num_simulations)
        return [BacktestResultWrapper(result) for result in results]
    
    def analyze_parameter_sweep(self, results: List[BacktestResultWrapper]) -> pd.DataFrame:
        """Analyze parameter sweep results.
        
        Args:
            results: List of backtest results
        
        Returns:
            DataFrame with analysis
        """
        if not results:
            return pd.DataFrame()
        
        summary_data = []
        for i, result in enumerate(results):
            summary = result.performance_summary()
            summary['run_id'] = i
            summary_data.append(summary)
        
        return pd.DataFrame(summary_data)
    
    def analyze_monte_carlo(self, results: List[BacktestResultWrapper]) -> Dict[str, Any]:
        """Analyze Monte Carlo simulation results.
        
        Args:
            results: List of backtest results
        
        Returns:
            Dictionary with statistical analysis
        """
        if not results:
            return {}
        
        # Extract metrics
        returns = [r.total_return for r in results]
        sharpe_ratios = [r.sharpe_ratio for r in results]
        max_drawdowns = [r.max_drawdown for r in results]
        
        return {
            'return_stats': {
                'mean': np.mean(returns),
                'std': np.std(returns),
                'min': np.min(returns),
                'max': np.max(returns),
                'percentiles': {
                    '5%': np.percentile(returns, 5),
                    '25%': np.percentile(returns, 25),
                    '50%': np.percentile(returns, 50),
                    '75%': np.percentile(returns, 75),
                    '95%': np.percentile(returns, 95)
                }
            },
            'sharpe_stats': {
                'mean': np.mean(sharpe_ratios),
                'std': np.std(sharpe_ratios),
                'min': np.min(sharpe_ratios),
                'max': np.max(sharpe_ratios)
            },
            'drawdown_stats': {
                'mean': np.mean(max_drawdowns),
                'std': np.std(max_drawdowns),
                'min': np.min(max_drawdowns),
                'max': np.max(max_drawdowns)
            },
            'num_simulations': len(results)
        }
    
    def __repr__(self) -> str:
        return f"BacktestEngineWrapper(symbols={len(self.available_symbols)})"
