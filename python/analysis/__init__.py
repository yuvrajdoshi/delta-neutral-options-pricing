"""
Python Analysis Framework for Volatility Arbitrage Trading System
Layer 7: Advanced Analytics, Visualization, and Optimization

This module provides:
- StrategyAnalyzer: Comprehensive performance analysis
- StrategyVisualizer: Advanced visualization capabilities  
- ParameterOptimizer: Grid search and Bayesian optimization
"""

from .analyzer import StrategyAnalyzer
from .visualizer import StrategyVisualizer
from .optimizer import ParameterOptimizer

__all__ = [
    'StrategyAnalyzer',
    'StrategyVisualizer', 
    'ParameterOptimizer'
]

__version__ = '1.0.0'
