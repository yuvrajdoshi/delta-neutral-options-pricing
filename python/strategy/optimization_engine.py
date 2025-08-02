"""
Enhanced Risk Management & Parameter Optimization - Layer 8 Component 4
Advanced risk controls and strategy optimization for volatility arbitrage
"""

import pandas as pd
import numpy as np
from datetime import datetime, timedelta
from typing import Dict, List, Optional, Tuple, NamedTuple
import itertools
from scipy.optimize import minimize
import sys
import os

# Add paths for strategy components
sys.path.append(os.path.dirname(__file__))

from real_world_strategy import RealWorldVolatilityArbitrageStrategy, create_default_config, Signal, Position
from backtest_engine import RealWorldBacktestEngine, BacktestResult
from market_data import MarketDataManager

class OptimizationResult(NamedTuple):
    """Result from parameter optimization"""
    params: Dict
    sharpe_ratio: float
    total_return: float
    max_drawdown: float
    win_rate: float
    profit_factor: float
    total_trades: int

class EnhancedRiskManager:
    """
    Advanced risk management system for volatility arbitrage strategy
    """
    
    def __init__(self, config: Dict):
        self.config = config
        
        # Enhanced risk parameters
        self.max_portfolio_var = config.get('max_portfolio_var', 0.02)  # 2% daily VaR
        self.max_single_position_risk = config.get('max_position_risk', 0.05)  # 5% per position
        self.max_correlation = config.get('max_correlation', 0.7)  # Max position correlation
        self.volatility_regime_multiplier = config.get('vol_regime_multiplier', 1.5)
        
        # Dynamic position sizing
        self.kelly_fraction = config.get('kelly_fraction', 0.25)  # 25% of Kelly criterion
        self.min_position_size = config.get('min_position_size', 0.02)  # 2% minimum
        self.max_position_size = config.get('max_position_size', 0.15)  # 15% maximum
        
        # Stop loss and profit taking
        self.stop_loss_pct = config.get('stop_loss_pct', -0.30)  # -30% stop loss
        self.profit_target_pct = config.get('profit_target_pct', 0.50)  # +50% profit target
        self.trailing_stop_pct = config.get('trailing_stop_pct', 0.20)  # 20% trailing stop
        
        print(f"🛡️  Enhanced Risk Manager initialized")
        print(f"   Max portfolio VaR: {self.max_portfolio_var:.1%}")
        print(f"   Position size range: {self.min_position_size:.1%} - {self.max_position_size:.1%}")
        print(f"   Stop loss: {self.stop_loss_pct:.1%}")
        print(f"   Profit target: {self.profit_target_pct:.1%}")
    
    def calculate_kelly_position_size(self, signal: Signal, market_data: pd.DataFrame) -> float:
        """
        Calculate optimal position size using Kelly criterion
        
        Args:
            signal: Trading signal
            market_data: Market data for volatility assessment
            
        Returns:
            Optimal position size as fraction of portfolio
        """
        try:
            # Estimate win probability based on signal strength and confidence
            win_prob = (signal.strength * signal.confidence + 0.3) / 1.3  # Normalize to 0.3-1.0
            win_prob = min(max(win_prob, 0.3), 0.8)  # Cap between 30-80%
            
            # Estimate average win/loss ratio based on volatility spread
            avg_win = abs(signal.expected_return) * 0.6  # Conservative estimate
            avg_loss = abs(signal.expected_return) * 0.4  # Typical loss
            
            # Kelly fraction: f = (bp - q) / b
            # where b = odds received (avg_win/avg_loss), p = win prob, q = loss prob
            if avg_loss > 0:
                odds = avg_win / avg_loss
                kelly_f = (odds * win_prob - (1 - win_prob)) / odds
            else:
                kelly_f = 0.1
            
            # Apply Kelly fraction multiplier for safety
            kelly_f *= self.kelly_fraction
            
            # Clamp to min/max position sizes
            kelly_f = max(min(kelly_f, self.max_position_size), self.min_position_size)
            
            return kelly_f
            
        except Exception as e:
            print(f"⚠️  Kelly calculation error: {e}, using default size")
            return self.min_position_size
    
    def adjust_for_volatility_regime(self, position_size: float, market_data: pd.DataFrame) -> float:
        """
        Adjust position size based on market volatility regime
        
        Args:
            position_size: Base position size
            market_data: Market data for regime detection
            
        Returns:
            Adjusted position size
        """
        try:
            # Check if we're in high volatility regime
            if 'HighVolRegime' in market_data.columns:
                is_high_vol = market_data['HighVolRegime'].iloc[-1]
                if is_high_vol:
                    # Reduce position size in high volatility
                    position_size *= (1 / self.volatility_regime_multiplier)
            
            # Check VIX levels
            if 'VIX' in market_data.columns:
                current_vix = market_data['VIX'].iloc[-1]
                if current_vix > 30:  # High VIX
                    position_size *= 0.7  # Reduce by 30%
                elif current_vix < 15:  # Low VIX
                    position_size *= 1.2  # Increase by 20%
            
            return max(min(position_size, self.max_position_size), self.min_position_size)
            
        except Exception as e:
            print(f"⚠️  Volatility regime adjustment error: {e}")
            return position_size
    
    def validate_signal_enhanced(self, signal: Signal, portfolio: Dict, market_data: pd.DataFrame) -> Tuple[bool, float]:
        """
        Enhanced signal validation with dynamic position sizing
        
        Args:
            signal: Trading signal to validate
            portfolio: Current portfolio state
            market_data: Market data
            
        Returns:
            Tuple of (is_valid, optimal_position_size)
        """
        # Calculate optimal position size
        base_size = self.calculate_kelly_position_size(signal, market_data)
        adjusted_size = self.adjust_for_volatility_regime(base_size, market_data)
        
        # Enhanced risk checks
        
        # 1. Signal quality threshold (more strict)
        if signal.confidence < 0.4:
            print(f"⚠️  Signal confidence too low: {signal.confidence:.2f} < 0.4")
            return False, 0.0
        
        if signal.strength < 0.3:
            print(f"⚠️  Signal strength too low: {signal.strength:.2f} < 0.3")
            return False, 0.0
        
        # 2. Risk estimate check (dynamic based on market conditions)
        risk_threshold = 0.08  # 8% base threshold
        if 'VIX' in market_data.columns:
            vix = market_data['VIX'].iloc[-1]
            if vix > 25:
                risk_threshold = 0.12  # Higher threshold in high vol
            elif vix < 15:
                risk_threshold = 0.06  # Lower threshold in low vol
        
        if signal.risk_estimate > risk_threshold:
            print(f"⚠️  Risk estimate too high: {signal.risk_estimate:.2%} > {risk_threshold:.2%}")
            return False, 0.0
        
        # 3. Portfolio concentration check
        active_positions = len([p for p in portfolio.get('positions', {}).values() if p.is_active])
        max_positions = 8 if market_data.get('VIX', [20]).iloc[-1] < 20 else 5  # More positions in low vol
        
        if active_positions >= max_positions:
            print(f"⚠️  Max positions limit: {active_positions} >= {max_positions}")
            return False, 0.0
        
        # 4. Volatility spread magnitude check
        min_spread = 0.015  # 1.5% minimum spread
        if abs(signal.vol_spread) < min_spread:
            print(f"⚠️  Volatility spread too small: {abs(signal.vol_spread):.2%} < {min_spread:.2%}")
            return False, 0.0
        
        return True, adjusted_size
    
    def should_close_position_enhanced(self, position: Position, market_data: pd.DataFrame, 
                                     days_to_expiry: int) -> Tuple[bool, str]:
        """
        Enhanced position closing logic with multiple exit criteria
        
        Args:
            position: Position to evaluate
            market_data: Current market data
            days_to_expiry: Days until expiry
            
        Returns:
            Tuple of (should_close, reason)
        """
        current_return = position.pnl / (abs(position.quantity) * position.entry_price * 100)
        
        # 1. Time-based exits
        if days_to_expiry <= 3:
            return True, "Expiry approaching"
        
        # 2. Profit target
        if current_return >= self.profit_target_pct:
            return True, f"Profit target hit: {current_return:.1%}"
        
        # 3. Stop loss
        if current_return <= self.stop_loss_pct:
            return True, f"Stop loss hit: {current_return:.1%}"
        
        # 4. Trailing stop (if position is profitable)
        if hasattr(position, 'max_profit'):
            if current_return > 0.1:  # Only apply if >10% profit
                if current_return < position.max_profit - self.trailing_stop_pct:
                    return True, f"Trailing stop: {current_return:.1%} vs max {position.max_profit:.1%}"
        else:
            position.max_profit = current_return
        
        # Update max profit
        if current_return > getattr(position, 'max_profit', 0):
            position.max_profit = current_return
        
        # 5. Volatility regime change
        if 'VIX' in market_data.columns:
            current_vix = market_data['VIX'].iloc[-1]
            entry_vix = getattr(position, 'entry_vix', current_vix)
            
            # Close if VIX changed significantly
            if abs(current_vix - entry_vix) > 10:  # 10 point VIX change
                return True, f"VIX regime change: {entry_vix:.1f} -> {current_vix:.1f}"
        
        # 6. Signal decay (for long-held positions)
        days_held = (market_data.index[-1] - position.entry_time).days
        if days_held > 20 and current_return < 0.05:  # Held >20 days with <5% profit
            return True, "Signal decay - holding too long with low profit"
        
        return False, ""

class ParameterOptimizer:
    """
    Parameter optimization system for volatility arbitrage strategy
    """
    
    def __init__(self):
        self.optimization_results = []
        
    def define_parameter_space(self) -> Dict[str, List]:
        """
        Define the parameter space for optimization
        
        Returns:
            Dictionary of parameter ranges
        """
        return {
            'entry_threshold': [0.015, 0.02, 0.025, 0.03, 0.035, 0.04],  # 1.5% to 4%
            'max_position_size': [0.08, 0.10, 0.12, 0.15],  # 8% to 15%
            'lookback_period': [40, 50, 60, 80],  # Days for GARCH
            'max_positions': [4, 5, 6, 8],  # Max concurrent positions
            'stop_loss_pct': [-0.25, -0.30, -0.35, -0.40],  # Stop loss levels
            'profit_target_pct': [0.40, 0.50, 0.60, 0.75],  # Profit targets
            'kelly_fraction': [0.15, 0.20, 0.25, 0.30],  # Kelly multiplier
        }
    
    def create_optimized_config(self, params: Dict) -> Dict:
        """
        Create strategy config from optimization parameters
        
        Args:
            params: Parameter dictionary
            
        Returns:
            Complete strategy configuration
        """
        config = create_default_config()
        
        # Update with optimized parameters
        config.update(params)
        
        # Add enhanced risk management parameters
        config.update({
            'max_portfolio_var': 0.025,
            'max_position_risk': 0.06,
            'max_correlation': 0.7,
            'vol_regime_multiplier': 1.4,
            'min_position_size': 0.02,
            'trailing_stop_pct': 0.15,
        })
        
        return config
    
    def objective_function(self, metrics: Dict) -> float:
        """
        Multi-objective optimization function
        
        Args:
            metrics: Performance metrics
            
        Returns:
            Objective score (higher is better)
        """
        if not metrics or metrics.get('total_trades', 0) < 5:
            return -999  # Penalize strategies with too few trades
        
        # Multi-objective scoring
        sharpe = metrics.get('sharpe_ratio', 0)
        returns = metrics.get('annualized_return', 0)
        drawdown = abs(metrics.get('max_drawdown', -1))
        win_rate = metrics.get('win_rate', 0)
        profit_factor = metrics.get('profit_factor', 0)
        trades = metrics.get('total_trades', 0)
        
        # Weighted objective function
        score = (
            sharpe * 0.35 +           # 35% weight on risk-adjusted returns
            returns * 2.0 * 0.25 +    # 25% weight on absolute returns
            (1 - drawdown/0.2) * 0.20 + # 20% weight on drawdown control
            win_rate * 0.10 +         # 10% weight on win rate
            min(profit_factor, 3) * 0.10  # 10% weight on profit factor (capped)
        )
        
        # Bonus for reasonable number of trades
        if 10 <= trades <= 50:
            score *= 1.1
        elif trades < 5:
            score *= 0.5
        
        return score
    
    def grid_search_optimization(self, start_date: str, end_date: str, 
                               max_combinations: int = 50) -> List[OptimizationResult]:
        """
        Grid search parameter optimization
        
        Args:
            start_date: Backtest start date
            end_date: Backtest end date
            max_combinations: Maximum parameter combinations to test
            
        Returns:
            List of optimization results sorted by performance
        """
        print(f"🔍 Starting grid search optimization...")
        print(f"   Date range: {start_date} to {end_date}")
        print(f"   Max combinations: {max_combinations}")
        
        param_space = self.define_parameter_space()
        
        # Generate all parameter combinations
        param_names = list(param_space.keys())
        param_values = list(param_space.values())
        
        all_combinations = list(itertools.product(*param_values))
        
        # Limit combinations if too many
        if len(all_combinations) > max_combinations:
            print(f"   Sampling {max_combinations} from {len(all_combinations)} combinations")
            import random
            all_combinations = random.sample(all_combinations, max_combinations)
        
        results = []
        
        for i, param_combo in enumerate(all_combinations):
            # Create parameter dictionary
            params = dict(zip(param_names, param_combo))
            
            try:
                # Create optimized config
                config = self.create_optimized_config(params)
                
                # Run backtest
                engine = RealWorldBacktestEngine(config)
                backtest_result = engine.run_backtest(start_date, end_date, 
                                                    test_volatility_threshold=params['entry_threshold'])
                
                # Calculate metrics
                metrics = backtest_result.calculate_metrics()
                
                if metrics:
                    # Calculate objective score
                    score = self.objective_function(metrics)
                    
                    result = OptimizationResult(
                        params=params,
                        sharpe_ratio=metrics.get('sharpe_ratio', 0),
                        total_return=metrics.get('total_return', 0),
                        max_drawdown=metrics.get('max_drawdown', 0),
                        win_rate=metrics.get('win_rate', 0),
                        profit_factor=metrics.get('profit_factor', 0),
                        total_trades=metrics.get('total_trades', 0)
                    )
                    
                    results.append((score, result))
                    
                    # Progress update
                    if (i + 1) % 10 == 0:
                        best_score = max(results, key=lambda x: x[0])[0] if results else 0
                        print(f"   Progress: {i+1}/{len(all_combinations)} - Best score: {best_score:.3f}")
                
            except Exception as e:
                print(f"   ⚠️  Error testing combination {i+1}: {e}")
                continue
        
        # Sort by score (descending)
        results.sort(key=lambda x: x[0], reverse=True)
        
        # Return just the OptimizationResult objects
        optimization_results = [result for score, result in results]
        
        print(f"✅ Optimization completed! Tested {len(results)} combinations")
        
        return optimization_results
    
    def generate_optimization_report(self, results: List[OptimizationResult]) -> str:
        """
        Generate comprehensive optimization report
        
        Args:
            results: List of optimization results
            
        Returns:
            Formatted optimization report
        """
        if not results:
            return "No optimization results to display"
        
        best = results[0]
        
        report = f"""
🎯 PARAMETER OPTIMIZATION REPORT
{'='*60}

🏆 BEST CONFIGURATION FOUND:
   Entry Threshold:        {best.params['entry_threshold']:.1%}
   Max Position Size:      {best.params['max_position_size']:.1%}
   Lookback Period:        {best.params['lookback_period']} days
   Max Positions:          {best.params['max_positions']}
   Stop Loss:              {best.params['stop_loss_pct']:.1%}
   Profit Target:          {best.params['profit_target_pct']:.1%}
   Kelly Fraction:         {best.params['kelly_fraction']:.1%}

📊 BEST PERFORMANCE METRICS:
   Sharpe Ratio:           {best.sharpe_ratio:>10.2f}
   Total Return:           {best.total_return:>10.2%}
   Max Drawdown:           {best.max_drawdown:>10.2%}
   Win Rate:               {best.win_rate:>10.1%}
   Profit Factor:          {best.profit_factor:>10.2f}
   Total Trades:           {best.total_trades:>10}

📈 TOP 5 CONFIGURATIONS:
{'   Rank':<6} {'Sharpe':<8} {'Return':<8} {'Drawdown':<10} {'Trades':<8} {'Entry Thresh':<12}
{'-'*60}
"""
        
        for i, result in enumerate(results[:5], 1):
            report += f"   {i:<6} {result.sharpe_ratio:<8.2f} {result.total_return:<8.1%} "
            report += f"{result.max_drawdown:<10.1%} {result.total_trades:<8} "
            report += f"{result.params['entry_threshold']:<12.1%}\n"
        
        # Parameter sensitivity analysis
        report += f"\n🔬 PARAMETER SENSITIVITY ANALYSIS:\n"
        
        # Analyze impact of each parameter
        param_impact = {}
        for param in ['entry_threshold', 'max_position_size', 'lookback_period']:
            param_values = [r.params[param] for r in results[:10]]  # Top 10
            param_impact[param] = {
                'min': min(param_values),
                'max': max(param_values),
                'avg': sum(param_values) / len(param_values)
            }
        
        for param, stats in param_impact.items():
            report += f"   {param.replace('_', ' ').title():<20}: "
            if param == 'entry_threshold':
                report += f"{stats['avg']:.2%} (range: {stats['min']:.2%} - {stats['max']:.2%})\n"
            elif param == 'max_position_size':
                report += f"{stats['avg']:.1%} (range: {stats['min']:.1%} - {stats['max']:.1%})\n"
            else:
                report += f"{stats['avg']:.1f} (range: {stats['min']} - {stats['max']})\n"
        
        return report

def run_enhanced_optimization():
    """Run complete parameter optimization and enhanced risk management"""
    print("🚀 Enhanced Risk Management & Parameter Optimization")
    print("=" * 60)
    
    # Initialize optimizer
    optimizer = ParameterOptimizer()
    
    # Run optimization on different time periods
    test_periods = [
        ('2024-08-01', '2025-08-02', 'Last 12 months'),
        ('2025-01-01', '2025-08-02', 'YTD 2025'),
    ]
    
    all_results = {}
    
    for start_date, end_date, period_name in test_periods:
        print(f"\n📅 Optimizing for period: {period_name}")
        
        try:
            # Run optimization
            results = optimizer.grid_search_optimization(start_date, end_date, max_combinations=30)
            
            if results:
                # Generate and display report
                report = optimizer.generate_optimization_report(results)
                print(report)
                
                all_results[period_name] = results[0]  # Store best result
                
                # Test best configuration
                print(f"\n🧪 TESTING BEST CONFIGURATION:")
                best_config = optimizer.create_optimized_config(results[0].params)
                
                # Create enhanced strategy with best parameters
                engine = RealWorldBacktestEngine(best_config)
                backtest_result = engine.run_backtest(
                    start_date, end_date, 
                    test_volatility_threshold=results[0].params['entry_threshold']
                )
                
                # Display results
                final_report = engine.generate_performance_report(backtest_result)
                print(final_report)
                
            else:
                print(f"   ❌ No valid results for {period_name}")
                
        except Exception as e:
            print(f"   ❌ Optimization error for {period_name}: {e}")
    
    # Summary of best configurations across periods
    if all_results:
        print(f"\n🏆 OPTIMIZATION SUMMARY ACROSS PERIODS")
        print("=" * 60)
        
        for period, result in all_results.items():
            print(f"\n{period}:")
            print(f"   Best Sharpe Ratio: {result.sharpe_ratio:.2f}")
            print(f"   Entry Threshold: {result.params['entry_threshold']:.1%}")
            print(f"   Position Size: {result.params['max_position_size']:.1%}")
            print(f"   Total Trades: {result.total_trades}")

def main():
    """Main optimization execution"""
    run_enhanced_optimization()

if __name__ == "__main__":
    main()
