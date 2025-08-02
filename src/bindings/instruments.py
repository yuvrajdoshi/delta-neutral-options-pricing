"""Instrument and portfolio management module."""

import pandas as pd
import numpy as np
from typing import List, Dict, Optional, Union, Any
from datetime import datetime
import volatility_arbitrage as va
from .time_series import TimeSeriesWrapper

class InstrumentWrapper:
    """Enhanced Python wrapper for financial instruments."""
    
    def __init__(self, instrument: va.Instrument):
        """Initialize instrument wrapper.
        
        Args:
            instrument: C++ Instrument object
        """
        self._instrument = instrument
    
    @property
    def symbol(self) -> str:
        """Get instrument symbol."""
        return self._instrument.getSymbol()
    
    @property
    def type(self) -> va.InstrumentType:
        """Get instrument type."""
        return self._instrument.getType()
    
    def price(self, market_data: va.MarketData) -> float:
        """Calculate instrument price."""
        return self._instrument.price(market_data)
    
    def risk_metrics(self, market_data: va.MarketData) -> Dict[str, float]:
        """Calculate risk metrics."""
        metrics = self._instrument.calculateRiskMetrics(market_data)
        return dict(metrics)  # Convert C++ map to Python dict
    
    def __repr__(self) -> str:
        return f"InstrumentWrapper(symbol='{self.symbol}', type={self.type})"


class EquityWrapper(InstrumentWrapper):
    """Enhanced Python wrapper for equity instruments."""
    
    def __init__(self, symbol: str, shares: float = 1.0):
        """Initialize equity wrapper.
        
        Args:
            symbol: Equity symbol
            shares: Number of shares
        """
        equity = va.InstrumentFactory.createEquity(symbol, shares)
        super().__init__(equity)
        self._equity = equity
    
    @property
    def shares(self) -> float:
        """Get number of shares."""
        return self._equity.getShares()
    
    @shares.setter
    def shares(self, value: float):
        """Set number of shares."""
        self._equity.setShares(value)
    
    def __repr__(self) -> str:
        return f"EquityWrapper(symbol='{self.symbol}', shares={self.shares})"


class OptionWrapper(InstrumentWrapper):
    """Enhanced Python wrapper for option instruments."""
    
    def __init__(self, underlying: str, strike: float, expiry: va.DateTime, 
                 option_type: va.OptionType, exercise_style: va.ExerciseStyle = va.ExerciseStyle.European):
        """Initialize option wrapper.
        
        Args:
            underlying: Underlying symbol
            strike: Strike price
            expiry: Expiration date
            option_type: Call or Put
            exercise_style: European or American
        """
        if option_type == va.OptionType.Call and exercise_style == va.ExerciseStyle.European:
            option = va.InstrumentFactory.createEuropeanCall(underlying, strike, expiry)
        elif option_type == va.OptionType.Put and exercise_style == va.ExerciseStyle.European:
            option = va.InstrumentFactory.createEuropeanPut(underlying, strike, expiry)
        elif option_type == va.OptionType.Call and exercise_style == va.ExerciseStyle.American:
            option = va.InstrumentFactory.createAmericanCall(underlying, strike, expiry)
        elif option_type == va.OptionType.Put and exercise_style == va.ExerciseStyle.American:
            option = va.InstrumentFactory.createAmericanPut(underlying, strike, expiry)
        else:
            raise ValueError(f"Invalid option configuration: {option_type}, {exercise_style}")
        
        super().__init__(option)
        self._option = option
        self._underlying = underlying
        self._strike = strike
        self._expiry = expiry
        self._option_type = option_type
        self._exercise_style = exercise_style
    
    @property
    def underlying(self) -> str:
        """Get underlying symbol."""
        return self._underlying
    
    @property
    def strike(self) -> float:
        """Get strike price."""
        return self._option.getStrike()
    
    @property
    def expiry(self) -> va.DateTime:
        """Get expiration date."""
        return self._expiry
    
    @property
    def option_type(self) -> va.OptionType:
        """Get option type."""
        return self._option.getOptionType()
    
    @property
    def exercise_style(self) -> va.ExerciseStyle:
        """Get exercise style."""
        return self._option.getExerciseStyle()
    
    def greeks(self, spot: float, volatility: float, risk_free_rate: float, 
               dividend_yield: float = 0.0) -> va.Greeks:
        """Calculate option Greeks.
        
        Args:
            spot: Current spot price
            volatility: Implied volatility
            risk_free_rate: Risk-free rate
            dividend_yield: Dividend yield
        
        Returns:
            Greeks object with delta, gamma, vega, theta, rho
        """
        return self._option.calculateGreeks(spot, volatility, risk_free_rate, dividend_yield)
    
    def delta(self, spot: float, volatility: float, risk_free_rate: float, 
              dividend_yield: float = 0.0) -> float:
        """Calculate delta."""
        return self._option.delta(spot, volatility, risk_free_rate, dividend_yield)
    
    def gamma(self, spot: float, volatility: float, risk_free_rate: float, 
              dividend_yield: float = 0.0) -> float:
        """Calculate gamma."""
        return self._option.gamma(spot, volatility, risk_free_rate, dividend_yield)
    
    def vega(self, spot: float, volatility: float, risk_free_rate: float, 
             dividend_yield: float = 0.0) -> float:
        """Calculate vega."""
        return self._option.vega(spot, volatility, risk_free_rate, dividend_yield)
    
    def theta(self, spot: float, volatility: float, risk_free_rate: float, 
              dividend_yield: float = 0.0) -> float:
        """Calculate theta."""
        return self._option.theta(spot, volatility, risk_free_rate, dividend_yield)
    
    def rho(self, spot: float, volatility: float, risk_free_rate: float, 
            dividend_yield: float = 0.0) -> float:
        """Calculate rho."""
        return self._option.rho(spot, volatility, risk_free_rate, dividend_yield)
    
    def __repr__(self) -> str:
        return (f"OptionWrapper(underlying='{self.underlying}', strike={self.strike}, "
                f"type={self.option_type}, style={self.exercise_style})")


class PortfolioWrapper:
    """Enhanced Python wrapper for portfolio management."""
    
    def __init__(self, initial_capital: float = 1000000.0):
        """Initialize portfolio wrapper.
        
        Args:
            initial_capital: Initial portfolio capital
        """
        self._portfolio = va.Portfolio()
        self._portfolio.setInitialCapital(initial_capital)
        self._positions = {}  # symbol -> position info
    
    def add_position(self, instrument: Union[InstrumentWrapper, va.Instrument], 
                    quantity: float, entry_price: float = 0.0):
        """Add position to portfolio.
        
        Args:
            instrument: Instrument to add
            quantity: Position quantity
            entry_price: Entry price (optional)
        """
        if isinstance(instrument, InstrumentWrapper):
            inst = instrument._instrument
            symbol = instrument.symbol
        else:
            inst = instrument
            symbol = instrument.getSymbol()
        
        position = va.Position(inst, quantity, entry_price)
        self._portfolio.addPosition(position)
        self._positions[symbol] = {
            'instrument': instrument,
            'quantity': quantity,
            'entry_price': entry_price
        }
    
    def remove_position(self, symbol: str):
        """Remove position from portfolio."""
        self._portfolio.removePosition(symbol)
        if symbol in self._positions:
            del self._positions[symbol]
    
    def update_position(self, symbol: str, quantity: float):
        """Update position quantity."""
        self._portfolio.updatePosition(symbol, quantity)
        if symbol in self._positions:
            self._positions[symbol]['quantity'] = quantity
    
    def get_position(self, symbol: str) -> Optional[Dict[str, Any]]:
        """Get position information."""
        return self._positions.get(symbol)
    
    def get_positions(self) -> Dict[str, Dict[str, Any]]:
        """Get all positions."""
        return self._positions.copy()
    
    @property
    def initial_capital(self) -> float:
        """Get initial capital."""
        return self._portfolio.getInitialCapital()
    
    @property
    def cash(self) -> float:
        """Get current cash balance."""
        return self._portfolio.getCash()
    
    def market_value(self, market_data: Dict[str, va.MarketData]) -> float:
        """Calculate total portfolio market value.
        
        Args:
            market_data: Dict of symbol -> MarketData
        
        Returns:
            Total portfolio market value
        """
        total_value = self.cash
        
        for symbol, pos_info in self._positions.items():
            if symbol in market_data:
                instrument = pos_info['instrument']
                quantity = pos_info['quantity']
                
                if isinstance(instrument, InstrumentWrapper):
                    price = instrument.price(market_data[symbol])
                else:
                    price = instrument.price(market_data[symbol])
                
                total_value += quantity * price
        
        return total_value
    
    def pnl(self, market_data: Dict[str, va.MarketData]) -> float:
        """Calculate portfolio P&L.
        
        Args:
            market_data: Dict of symbol -> MarketData
        
        Returns:
            Total portfolio P&L
        """
        return self.market_value(market_data) - self.initial_capital
    
    def positions_summary(self, market_data: Optional[Dict[str, va.MarketData]] = None) -> pd.DataFrame:
        """Get positions summary as DataFrame.
        
        Args:
            market_data: Optional market data for current valuations
        
        Returns:
            DataFrame with position details
        """
        if not self._positions:
            return pd.DataFrame()
        
        records = []
        for symbol, pos_info in self._positions.items():
            record = {
                'symbol': symbol,
                'quantity': pos_info['quantity'],
                'entry_price': pos_info['entry_price']
            }
            
            if market_data and symbol in market_data:
                instrument = pos_info['instrument']
                if isinstance(instrument, InstrumentWrapper):
                    current_price = instrument.price(market_data[symbol])
                else:
                    current_price = instrument.price(market_data[symbol])
                
                record['current_price'] = current_price
                record['market_value'] = pos_info['quantity'] * current_price
                record['unrealized_pnl'] = pos_info['quantity'] * (current_price - pos_info['entry_price'])
            
            records.append(record)
        
        return pd.DataFrame(records)
    
    def __repr__(self) -> str:
        return f"PortfolioWrapper(positions={len(self._positions)}, cash=${self.cash:,.2f})"
