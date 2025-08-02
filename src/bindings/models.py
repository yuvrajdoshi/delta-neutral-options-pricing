"""Volatility modeling and pricing models module."""

import pandas as pd
import numpy as np
from typing import List, Dict, Optional, Union, Tuple, Any
from datetime import datetime
import volatility_arbitrage as va
from .time_series import TimeSeriesWrapper

class VolatilityModelWrapper:
    """Enhanced Python wrapper for volatility models."""
    
    def __init__(self, model: va.VolatilityModel):
        """Initialize volatility model wrapper.
        
        Args:
            model: C++ VolatilityModel object
        """
        self._model = model
    
    @property
    def name(self) -> str:
        """Get model name."""
        return self._model.getModelName()
    
    @property
    def parameters(self) -> Dict[str, float]:
        """Get model parameters."""
        params = self._model.getParameters()
        return dict(params)  # Convert C++ map to Python dict
    
    def calibrate(self, returns: Union[TimeSeriesWrapper, List[float], np.ndarray]) -> bool:
        """Calibrate model to return series.
        
        Args:
            returns: Return time series
        
        Returns:
            True if calibration successful
        """
        if isinstance(returns, TimeSeriesWrapper):
            return self._model.calibrate(returns._ts)
        elif isinstance(returns, (list, np.ndarray)):
            return self._model.calibrate(list(returns))
        else:
            raise ValueError("Returns must be TimeSeriesWrapper, list, or numpy array")
    
    def forecast(self, horizon: int = 1) -> float:
        """Forecast volatility.
        
        Args:
            horizon: Forecast horizon
        
        Returns:
            Forecasted volatility
        """
        return self._model.forecast(horizon)
    
    def forecast_series(self, horizons: List[int]) -> List[float]:
        """Forecast volatility series.
        
        Args:
            horizons: List of forecast horizons
        
        Returns:
            List of forecasted volatilities
        """
        return self._model.forecastSeries(horizons)
    
    def log_likelihood(self, returns: Union[TimeSeriesWrapper, List[float]]) -> float:
        """Calculate log-likelihood."""
        if isinstance(returns, TimeSeriesWrapper):
            return self._model.calculateLogLikelihood(returns._ts)
        else:
            return self._model.calculateLogLikelihood(list(returns))
    
    def aic(self, returns: Union[TimeSeriesWrapper, List[float]]) -> float:
        """Calculate Akaike Information Criterion."""
        if isinstance(returns, TimeSeriesWrapper):
            return self._model.calculateAIC(returns._ts)
        else:
            return self._model.calculateAIC(list(returns))
    
    def bic(self, returns: Union[TimeSeriesWrapper, List[float]]) -> float:
        """Calculate Bayesian Information Criterion."""
        if isinstance(returns, TimeSeriesWrapper):
            return self._model.calculateBIC(returns._ts)
        else:
            return self._model.calculateBIC(list(returns))
    
    def __repr__(self) -> str:
        return f"VolatilityModelWrapper(name='{self.name}', parameters={self.parameters})"


class GARCHModelWrapper(VolatilityModelWrapper):
    """Enhanced Python wrapper for GARCH model."""
    
    def __init__(self, omega: float = 0.0, alpha: float = 0.0, beta: float = 0.0):
        """Initialize GARCH model wrapper.
        
        Args:
            omega: Omega parameter
            alpha: Alpha parameter  
            beta: Beta parameter
        """
        if omega == 0.0 and alpha == 0.0 and beta == 0.0:
            garch = va.ModelFactory.createGARCHModel()
        else:
            garch = va.ModelFactory.createGARCHModel(omega, alpha, beta)
        
        super().__init__(garch)
        self._garch = garch
    
    @property
    def omega(self) -> float:
        """Get omega parameter."""
        return self._garch.getOmega()
    
    @property
    def alpha(self) -> float:
        """Get alpha parameter."""
        return self._garch.getAlpha()
    
    @property
    def beta(self) -> float:
        """Get beta parameter."""
        return self._garch.getBeta()
    
    @property
    def last_variance(self) -> float:
        """Get last estimated variance."""
        return self._garch.getLastVariance()
    
    @property
    def long_run_variance(self) -> float:
        """Get long-run variance."""
        return self._garch.getLongRunVariance()
    
    @property
    def is_stationary(self) -> bool:
        """Check if model is stationary."""
        return self._garch.isStationary()
    
    @property
    def is_calibrated(self) -> bool:
        """Check if model is calibrated."""
        return self._garch.isCalibrated()
    
    def fit(self, returns: Union[TimeSeriesWrapper, pd.Series, np.ndarray]) -> Dict[str, Any]:
        """Fit GARCH model and return results.
        
        Args:
            returns: Return time series
        
        Returns:
            Dictionary with fit results
        """
        # Convert to appropriate format
        if isinstance(returns, pd.Series):
            returns_ts = TimeSeriesWrapper(returns)
        elif isinstance(returns, np.ndarray):
            returns_ts = TimeSeriesWrapper(pd.Series(returns))
        else:
            returns_ts = returns
        
        # Calibrate model
        success = self.calibrate(returns_ts)
        
        # Calculate information criteria
        ll = self.log_likelihood(returns_ts)
        aic = self.aic(returns_ts)
        bic = self.bic(returns_ts)
        
        return {
            'success': success,
            'omega': self.omega,
            'alpha': self.alpha,
            'beta': self.beta,
            'log_likelihood': ll,
            'aic': aic,
            'bic': bic,
            'is_stationary': self.is_stationary,
            'long_run_variance': self.long_run_variance
        }
    
    def simulate(self, n_steps: int, random_seed: Optional[int] = None) -> np.ndarray:
        """Simulate GARCH process.
        
        Args:
            n_steps: Number of steps to simulate
            random_seed: Random seed for reproducibility
        
        Returns:
            Simulated return series
        """
        if random_seed is not None:
            np.random.seed(random_seed)
        
        if not self.is_calibrated:
            raise ValueError("Model must be calibrated before simulation")
        
        returns = np.zeros(n_steps)
        variances = np.zeros(n_steps)
        
        # Initialize with long-run variance
        variances[0] = self.long_run_variance
        
        for t in range(n_steps):
            # Generate return
            returns[t] = np.sqrt(variances[t]) * np.random.standard_normal()
            
            # Update variance for next period
            if t < n_steps - 1:
                variances[t + 1] = (self.omega + 
                                  self.alpha * returns[t]**2 + 
                                  self.beta * variances[t])
        
        return returns
    
    def __repr__(self) -> str:
        return (f"GARCHModelWrapper(omega={self.omega:.6f}, alpha={self.alpha:.6f}, "
                f"beta={self.beta:.6f}, stationary={self.is_stationary})")


class PricingModelWrapper:
    """Enhanced Python wrapper for pricing models."""
    
    def __init__(self, model: va.PricingModel):
        """Initialize pricing model wrapper.
        
        Args:
            model: C++ PricingModel object
        """
        self._model = model
    
    @property
    def name(self) -> str:
        """Get model name."""
        return self._model.getModelName()
    
    def price(self, instrument: va.Instrument, spot: float, volatility: float,
              risk_free_rate: float, dividend_yield: float = 0.0, 
              time_to_expiry: Optional[float] = None) -> float:
        """Price instrument.
        
        Args:
            instrument: Instrument to price
            spot: Current spot price
            volatility: Volatility
            risk_free_rate: Risk-free rate
            dividend_yield: Dividend yield
            time_to_expiry: Time to expiry (for options)
        
        Returns:
            Instrument price
        """
        if time_to_expiry is not None:
            return self._model.price(instrument, spot, volatility, risk_free_rate, 
                                   dividend_yield, time_to_expiry)
        else:
            return self._model.price(instrument, spot, volatility, risk_free_rate, 
                                   dividend_yield)
    
    def greeks(self, instrument: va.Instrument, spot: float, volatility: float,
               risk_free_rate: float, dividend_yield: float = 0.0,
               time_to_expiry: Optional[float] = None) -> va.Greeks:
        """Calculate Greeks.
        
        Args:
            instrument: Instrument
            spot: Current spot price
            volatility: Volatility
            risk_free_rate: Risk-free rate
            dividend_yield: Dividend yield
            time_to_expiry: Time to expiry (for options)
        
        Returns:
            Greeks object
        """
        if time_to_expiry is not None:
            return self._model.calculateGreeks(instrument, spot, volatility, risk_free_rate,
                                             dividend_yield, time_to_expiry)
        else:
            return self._model.calculateGreeks(instrument, spot, volatility, risk_free_rate,
                                             dividend_yield)
    
    def __repr__(self) -> str:
        return f"PricingModelWrapper(name='{self.name}')"


class BSMPricingModelWrapper(PricingModelWrapper):
    """Enhanced Python wrapper for Black-Scholes-Merton pricing model."""
    
    def __init__(self):
        """Initialize BSM pricing model wrapper."""
        bsm = va.ModelFactory.createBSMPricingModel()
        super().__init__(bsm)
        self._bsm = bsm
    
    def price_option(self, option_type: str, spot: float, strike: float, 
                    time_to_expiry: float, volatility: float, risk_free_rate: float,
                    dividend_yield: float = 0.0) -> float:
        """Price European option using BSM formula.
        
        Args:
            option_type: 'call' or 'put'
            spot: Current spot price
            strike: Strike price
            time_to_expiry: Time to expiry in years
            volatility: Volatility (annualized)
            risk_free_rate: Risk-free rate (annualized)
            dividend_yield: Dividend yield (annualized)
        
        Returns:
            Option price
        """
        # Create temporary option
        expiry = va.DateTime.now()  # Placeholder
        opt_type = va.OptionType.Call if option_type.lower() == 'call' else va.OptionType.Put
        
        from .instruments import OptionWrapper
        option_wrapper = OptionWrapper("TEMP", strike, expiry, opt_type)
        
        return self.price(option_wrapper._instrument, spot, volatility, risk_free_rate,
                         dividend_yield, time_to_expiry)
    
    def greeks_option(self, option_type: str, spot: float, strike: float,
                     time_to_expiry: float, volatility: float, risk_free_rate: float,
                     dividend_yield: float = 0.0) -> Dict[str, float]:
        """Calculate option Greeks using BSM formula.
        
        Args:
            option_type: 'call' or 'put'
            spot: Current spot price
            strike: Strike price
            time_to_expiry: Time to expiry in years
            volatility: Volatility (annualized)
            risk_free_rate: Risk-free rate (annualized)
            dividend_yield: Dividend yield (annualized)
        
        Returns:
            Dictionary with Greeks
        """
        # Create temporary option
        expiry = va.DateTime.now()  # Placeholder
        opt_type = va.OptionType.Call if option_type.lower() == 'call' else va.OptionType.Put
        
        from .instruments import OptionWrapper
        option_wrapper = OptionWrapper("TEMP", strike, expiry, opt_type)
        
        greeks = self.greeks(option_wrapper._instrument, spot, volatility, risk_free_rate,
                           dividend_yield, time_to_expiry)
        
        return {
            'delta': greeks.delta,
            'gamma': greeks.gamma,
            'vega': greeks.vega,
            'theta': greeks.theta,
            'rho': greeks.rho
        }
    
    def implied_volatility(self, option_type: str, market_price: float, spot: float,
                          strike: float, time_to_expiry: float, risk_free_rate: float,
                          dividend_yield: float = 0.0, max_iterations: int = 100,
                          tolerance: float = 1e-6) -> Optional[float]:
        """Calculate implied volatility using Newton-Raphson method.
        
        Args:
            option_type: 'call' or 'put'
            market_price: Observed market price
            spot: Current spot price
            strike: Strike price
            time_to_expiry: Time to expiry in years
            risk_free_rate: Risk-free rate (annualized)
            dividend_yield: Dividend yield (annualized)
            max_iterations: Maximum iterations for solver
            tolerance: Convergence tolerance
        
        Returns:
            Implied volatility or None if not found
        """
        # Initial guess
        vol = 0.2
        
        for i in range(max_iterations):
            # Calculate price and vega
            price = self.price_option(option_type, spot, strike, time_to_expiry,
                                    vol, risk_free_rate, dividend_yield)
            
            greeks = self.greeks_option(option_type, spot, strike, time_to_expiry,
                                      vol, risk_free_rate, dividend_yield)
            vega = greeks['vega']
            
            # Newton-Raphson update
            price_diff = price - market_price
            
            if abs(price_diff) < tolerance:
                return vol
            
            if abs(vega) < 1e-10:  # Avoid division by zero
                return None
            
            vol = vol - price_diff / vega
            
            # Ensure volatility stays positive
            if vol <= 0:
                vol = 0.01
        
        return None  # Convergence failed
    
    def __repr__(self) -> str:
        return "BSMPricingModelWrapper(Black-Scholes-Merton)"
