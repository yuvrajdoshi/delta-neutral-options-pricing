"""Main volatility arbitrage Python package."""

# Import core C++ module
try:
    import volatility_arbitrage as _va
except ImportError as e:
    raise ImportError(f"Failed to import C++ module: {e}. Make sure the library is compiled and installed.")

# Re-export C++ classes for direct access
from volatility_arbitrage import *

# Import enhanced Python wrappers
from .time_series import TimeSeriesWrapper, MarketDataWrapper
from .instruments import InstrumentWrapper, EquityWrapper, OptionWrapper, PortfolioWrapper
from .models import (VolatilityModelWrapper, GARCHModelWrapper, 
                     PricingModelWrapper, BSMPricingModelWrapper)
from .strategy import (SignalWrapper, TradeWrapper, BacktestParametersWrapper,
                      BacktestResultWrapper, BacktestEngineWrapper)

# Version information
__version__ = "1.0.0"
__author__ = "Volatility Arbitrage Development Team"
__email__ = "dev@volatilityarbitrage.com"

# Package metadata
__all__ = [
    # Core C++ classes (re-exported)
    'DateTime', 'TimeSeries', 'MarketData',
    'InstrumentType', 'OptionType', 'ExerciseStyle',
    'Instrument', 'Equity', 'Option', 'InstrumentFactory',
    'Greeks', 'VolatilityModel', 'GARCHModel', 'PricingModel', 'BSMPricingModel', 'ModelFactory',
    'Signal', 'SignalType', 'Trade', 'TradeAction', 'BacktestParameters', 'BacktestResult', 'BacktestEngine',
    'Math',
    
    # Enhanced Python wrappers
    'TimeSeriesWrapper', 'MarketDataWrapper',
    'InstrumentWrapper', 'EquityWrapper', 'OptionWrapper', 'PortfolioWrapper',
    'VolatilityModelWrapper', 'GARCHModelWrapper', 'PricingModelWrapper', 'BSMPricingModelWrapper',
    'SignalWrapper', 'TradeWrapper', 'BacktestParametersWrapper', 'BacktestResultWrapper', 'BacktestEngineWrapper',
]

# Convenience functions
def create_garch_model(omega: float = 0.0, alpha: float = 0.0, beta: float = 0.0) -> GARCHModelWrapper:
    """Create GARCH model with enhanced Python interface.
    
    Args:
        omega: Omega parameter
        alpha: Alpha parameter
        beta: Beta parameter
    
    Returns:
        GARCHModelWrapper instance
    """
    return GARCHModelWrapper(omega, alpha, beta)

def create_bsm_model() -> BSMPricingModelWrapper:
    """Create Black-Scholes-Merton pricing model with enhanced Python interface.
    
    Returns:
        BSMPricingModelWrapper instance
    """
    return BSMPricingModelWrapper()

def create_equity(symbol: str, shares: float = 1.0) -> EquityWrapper:
    """Create equity instrument with enhanced Python interface.
    
    Args:
        symbol: Equity symbol
        shares: Number of shares
    
    Returns:
        EquityWrapper instance
    """
    return EquityWrapper(symbol, shares)

def create_option(underlying: str, strike: float, expiry: DateTime, 
                 option_type: str = "call", exercise_style: str = "european") -> OptionWrapper:
    """Create option instrument with enhanced Python interface.
    
    Args:
        underlying: Underlying symbol
        strike: Strike price
        expiry: Expiration date
        option_type: "call" or "put"
        exercise_style: "european" or "american"
    
    Returns:
        OptionWrapper instance
    """
    # Convert string parameters to enums
    if option_type.lower() == "call":
        opt_type = OptionType.Call
    else:
        opt_type = OptionType.Put
    
    if exercise_style.lower() == "european":
        ex_style = ExerciseStyle.European
    else:
        ex_style = ExerciseStyle.American
    
    return OptionWrapper(underlying, strike, expiry, opt_type, ex_style)

def create_portfolio(initial_capital: float = 1000000.0) -> PortfolioWrapper:
    """Create portfolio with enhanced Python interface.
    
    Args:
        initial_capital: Initial portfolio capital
    
    Returns:
        PortfolioWrapper instance
    """
    return PortfolioWrapper(initial_capital)

def create_backtest_engine() -> BacktestEngineWrapper:
    """Create backtesting engine with enhanced Python interface.
    
    Returns:
        BacktestEngineWrapper instance
    """
    return BacktestEngineWrapper()

# Package information
def get_package_info() -> dict:
    """Get package information.
    
    Returns:
        Dictionary with package metadata
    """
    return {
        'name': 'volatility_arbitrage',
        'version': __version__,
        'author': __author__,
        'email': __email__,
        'description': 'High-performance volatility arbitrage trading system',
        'features': [
            'GARCH volatility modeling',
            'Black-Scholes-Merton option pricing',
            'Portfolio management',
            'Advanced backtesting engine',
            'Risk metrics calculation',
            'Monte Carlo simulation',
            'Parameter optimization'
        ],
        'requirements': [
            'numpy >= 1.20.0',
            'pandas >= 1.3.0',
            'pybind11 >= 2.6.0'
        ]
    }

# Module initialization
def _initialize_module():
    """Initialize the module and perform any necessary setup."""
    # Verify that C++ module is properly loaded
    try:
        # Test basic functionality
        test_dt = DateTime.now()
        test_ts = TimeSeries("test")
        
        # Module loaded successfully
        return True
    except Exception as e:
        print(f"Warning: Module initialization failed: {e}")
        return False

# Initialize on import
_module_initialized = _initialize_module()

if not _module_initialized:
    print("Warning: Volatility Arbitrage module may not function correctly.")
    print("Please check that the C++ library is properly compiled and installed.")
