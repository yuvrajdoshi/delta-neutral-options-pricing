"""
Strategy Analysis Module - Layer 7 Component 1
Comprehensive performance analysis for volatility arbitrage strategies
"""

import numpy as np
import pandas as pd
from typing import Dict, List, Any, Optional
from datetime import datetime
import warnings

class StrategyAnalyzer:
    """
    Advanced strategy performance analyzer providing extended metrics
    beyond basic Sharpe/Sortino ratios
    """
    
    def __init__(self, backtest_result):
        """
        Initialize analyzer with backtest result
        
        Args:
            backtest_result: BacktestResult object from C++ layer
        """
        self.result = backtest_result
        self._cached_metrics = None
        
    def calculate_performance_metrics(self) -> Dict[str, float]:
        """
        Calculate comprehensive performance metrics
        
        Returns:
            Dictionary containing all performance metrics
        """
        if self._cached_metrics is not None:
            return self._cached_metrics
            
        # Get base metrics from C++ layer
        metrics = {}
        try:
            metrics.update(self.result.get_performance_metrics())
        except Exception as e:
            warnings.warn(f"Could not get base metrics: {e}")
            # Provide defaults
            metrics = {
                'total_return': 0.0,
                'sharpe_ratio': 0.0,
                'sortino_ratio': 0.0,
                'max_drawdown': 0.0
            }
        
        # Get equity curve for additional calculations
        try:
            equity_curve = self.result.get_equity_curve()
            if hasattr(equity_curve, 'values'):
                equity_values = equity_curve.values
            else:
                equity_values = np.array(equity_curve)
                
            # Calculate returns
            if len(equity_values) > 1:
                returns = np.diff(equity_values) / equity_values[:-1]
                returns = returns[np.isfinite(returns)]  # Remove inf/nan
                
                if len(returns) > 0:
                    # Additional risk-adjusted metrics
                    total_return = (equity_values[-1] / equity_values[0]) - 1.0
                    max_dd = abs(metrics.get('max_drawdown', 0.0))
                    
                    # Calmar Ratio (Total Return / Max Drawdown)
                    metrics['calmar_ratio'] = total_return / max_dd if max_dd > 0 else 0.0
                    
                    # Volatility (annualized)
                    metrics['volatility'] = np.std(returns) * np.sqrt(252)
                    
                    # Higher moments
                    metrics['skewness'] = self._calculate_skewness(returns)
                    metrics['kurtosis'] = self._calculate_kurtosis(returns)
                    
                    # Additional metrics
                    metrics['var_95'] = np.percentile(returns, 5)  # 95% VaR
                    metrics['cvar_95'] = np.mean(returns[returns <= metrics['var_95']])  # CVaR
                    
                    # Win rate metrics
                    positive_returns = returns[returns > 0]
                    negative_returns = returns[returns < 0]
                    
                    metrics['win_rate'] = len(positive_returns) / len(returns) if len(returns) > 0 else 0.0
                    metrics['avg_win'] = np.mean(positive_returns) if len(positive_returns) > 0 else 0.0
                    metrics['avg_loss'] = np.mean(negative_returns) if len(negative_returns) > 0 else 0.0
                    
                    # Profit factor
                    total_wins = np.sum(positive_returns) if len(positive_returns) > 0 else 0.0
                    total_losses = abs(np.sum(negative_returns)) if len(negative_returns) > 0 else 0.0
                    metrics['profit_factor'] = total_wins / total_losses if total_losses > 0 else float('inf')
                    
        except Exception as e:
            warnings.warn(f"Could not calculate extended metrics: {e}")
            
        self._cached_metrics = metrics
        return metrics
        
    def analyze_drawdowns(self) -> Dict[str, Any]:
        """
        Perform detailed drawdown analysis
        
        Returns:
            Dictionary containing drawdown statistics
        """
        try:
            equity_curve = self.result.get_equity_curve()
            if hasattr(equity_curve, 'values'):
                equity_values = equity_curve.values
            else:
                equity_values = np.array(equity_curve)
                
            # Calculate running maximum and drawdown
            running_max = np.maximum.accumulate(equity_values)
            drawdown_series = (equity_values - running_max) / running_max
            
            # Find drawdown periods
            major_drawdowns = []
            threshold = -0.05  # 5% drawdown threshold
            
            in_drawdown = False
            start_idx = 0
            
            for i, dd in enumerate(drawdown_series):
                if dd <= threshold and not in_drawdown:
                    # Starting a major drawdown
                    in_drawdown = True
                    start_idx = i
                elif dd > threshold and in_drawdown:
                    # Ending a major drawdown
                    in_drawdown = False
                    end_idx = i - 1
                    
                    # Find the peak drawdown in this period
                    period_drawdowns = drawdown_series[start_idx:end_idx+1]
                    min_dd = np.min(period_drawdowns)
                    min_idx = start_idx + np.argmin(period_drawdowns)
                    
                    major_drawdowns.append({
                        'start_idx': start_idx,
                        'end_idx': end_idx,
                        'peak_idx': min_idx,
                        'duration': end_idx - start_idx + 1,
                        'peak_drawdown': min_dd
                    })
            
            # Handle case where we end in drawdown
            if in_drawdown:
                end_idx = len(drawdown_series) - 1
                period_drawdowns = drawdown_series[start_idx:end_idx+1]
                min_dd = np.min(period_drawdowns)
                min_idx = start_idx + np.argmin(period_drawdowns)
                
                major_drawdowns.append({
                    'start_idx': start_idx,
                    'end_idx': end_idx,
                    'peak_idx': min_idx,
                    'duration': end_idx - start_idx + 1,
                    'peak_drawdown': min_dd
                })
            
            # Calculate average drawdown length
            drawdown_periods = []
            current_length = 0
            
            for dd in drawdown_series:
                if dd < 0:
                    current_length += 1
                else:
                    if current_length > 0:
                        drawdown_periods.append(current_length)
                        current_length = 0
                        
            if current_length > 0:
                drawdown_periods.append(current_length)
                
            avg_drawdown_length = np.mean(drawdown_periods) if drawdown_periods else 0
            
            return {
                'max_drawdown': np.min(drawdown_series),
                'avg_drawdown': np.mean(drawdown_series[drawdown_series < 0]) if np.any(drawdown_series < 0) else 0.0,
                'avg_drawdown_length': avg_drawdown_length,
                'major_drawdowns': major_drawdowns,
                'drawdown_series': drawdown_series
            }
            
        except Exception as e:
            warnings.warn(f"Could not analyze drawdowns: {e}")
            return {
                'max_drawdown': 0.0,
                'avg_drawdown': 0.0,
                'avg_drawdown_length': 0,
                'major_drawdowns': [],
                'drawdown_series': np.array([])
            }
        
    def analyze_trade_distribution(self) -> Dict[str, float]:
        """
        Statistical analysis of individual trade outcomes
        
        Returns:
            Dictionary containing trade distribution statistics
        """
        try:
            trades = self.result.get_trades()
            
            if not trades or len(trades) == 0:
                return {
                    'win_rate': 0.0,
                    'avg_win': 0.0,
                    'avg_loss': 0.0,
                    'profit_factor': 0.0,
                    'trade_count': 0
                }
            
            # Extract PnL from trades
            pnls = []
            for trade in trades:
                if hasattr(trade, 'get_pnl'):
                    pnls.append(trade.get_pnl())
                elif hasattr(trade, 'pnl'):
                    pnls.append(trade.pnl)
                else:
                    # Try to calculate from entry/exit if available
                    if hasattr(trade, 'exit_price') and hasattr(trade, 'entry_price'):
                        pnl = (trade.exit_price - trade.entry_price) * trade.quantity
                        pnls.append(pnl)
            
            if not pnls:
                return {
                    'win_rate': 0.0,
                    'avg_win': 0.0,
                    'avg_loss': 0.0,
                    'profit_factor': 0.0,
                    'trade_count': len(trades)
                }
            
            pnls = np.array(pnls)
            
            # Separate winning and losing trades
            winning_trades = pnls[pnls > 0]
            losing_trades = pnls[pnls < 0]
            
            # Calculate statistics
            win_rate = len(winning_trades) / len(pnls)
            avg_win = np.mean(winning_trades) if len(winning_trades) > 0 else 0.0
            avg_loss = np.mean(losing_trades) if len(losing_trades) > 0 else 0.0
            
            # Profit factor
            total_wins = np.sum(winning_trades) if len(winning_trades) > 0 else 0.0
            total_losses = abs(np.sum(losing_trades)) if len(losing_trades) > 0 else 0.0
            profit_factor = total_wins / total_losses if total_losses > 0 else float('inf')
            
            return {
                'win_rate': win_rate,
                'avg_win': avg_win,
                'avg_loss': avg_loss,
                'profit_factor': profit_factor,
                'trade_count': len(trades)
            }
            
        except Exception as e:
            warnings.warn(f"Could not analyze trade distribution: {e}")
            return {
                'win_rate': 0.0,
                'avg_win': 0.0,
                'avg_loss': 0.0,
                'profit_factor': 0.0,
                'trade_count': 0
            }
    
    def _calculate_skewness(self, returns: np.ndarray) -> float:
        """Calculate skewness of returns"""
        if len(returns) < 3:
            return 0.0
        mean_ret = np.mean(returns)
        std_ret = np.std(returns)
        if std_ret == 0:
            return 0.0
        return np.mean(((returns - mean_ret) / std_ret) ** 3)
    
    def _calculate_kurtosis(self, returns: np.ndarray) -> float:
        """Calculate kurtosis of returns"""
        if len(returns) < 4:
            return 0.0
        mean_ret = np.mean(returns)
        std_ret = np.std(returns)
        if std_ret == 0:
            return 0.0
        return np.mean(((returns - mean_ret) / std_ret) ** 4) - 3.0
    
    def generate_report(self) -> str:
        """
        Generate a comprehensive text report
        
        Returns:
            Formatted string report
        """
        metrics = self.calculate_performance_metrics()
        drawdown_analysis = self.analyze_drawdowns()
        trade_analysis = self.analyze_trade_distribution()
        
        report = []
        report.append("=" * 60)
        report.append("VOLATILITY ARBITRAGE STRATEGY ANALYSIS REPORT")
        report.append("=" * 60)
        report.append("")
        
        # Performance Metrics Section
        report.append("PERFORMANCE METRICS")
        report.append("-" * 30)
        report.append(f"Total Return:        {metrics.get('total_return', 0.0):8.2%}")
        report.append(f"Sharpe Ratio:        {metrics.get('sharpe_ratio', 0.0):8.2f}")
        report.append(f"Sortino Ratio:       {metrics.get('sortino_ratio', 0.0):8.2f}")
        report.append(f"Calmar Ratio:        {metrics.get('calmar_ratio', 0.0):8.2f}")
        report.append(f"Volatility:          {metrics.get('volatility', 0.0):8.2%}")
        report.append(f"Skewness:            {metrics.get('skewness', 0.0):8.2f}")
        report.append(f"Kurtosis:            {metrics.get('kurtosis', 0.0):8.2f}")
        report.append("")
        
        # Risk Metrics Section
        report.append("RISK METRICS")
        report.append("-" * 30)
        report.append(f"Max Drawdown:        {drawdown_analysis.get('max_drawdown', 0.0):8.2%}")
        report.append(f"Avg Drawdown:        {drawdown_analysis.get('avg_drawdown', 0.0):8.2%}")
        report.append(f"Avg DD Length:       {drawdown_analysis.get('avg_drawdown_length', 0):8.0f} periods")
        report.append(f"VaR (95%):           {metrics.get('var_95', 0.0):8.2%}")
        report.append(f"CVaR (95%):          {metrics.get('cvar_95', 0.0):8.2%}")
        report.append("")
        
        # Trade Analysis Section
        report.append("TRADE ANALYSIS")
        report.append("-" * 30)
        report.append(f"Total Trades:        {trade_analysis.get('trade_count', 0):8.0f}")
        report.append(f"Win Rate:            {trade_analysis.get('win_rate', 0.0):8.2%}")
        report.append(f"Avg Win:             {trade_analysis.get('avg_win', 0.0):8.2f}")
        report.append(f"Avg Loss:            {trade_analysis.get('avg_loss', 0.0):8.2f}")
        report.append(f"Profit Factor:       {trade_analysis.get('profit_factor', 0.0):8.2f}")
        report.append("")
        
        # Major Drawdowns Section
        major_dds = drawdown_analysis.get('major_drawdowns', [])
        if major_dds:
            report.append("MAJOR DRAWDOWNS (>5%)")
            report.append("-" * 30)
            for i, dd in enumerate(major_dds[:5]):  # Show top 5
                report.append(f"Drawdown {i+1}:       {dd['peak_drawdown']:8.2%} ({dd['duration']} periods)")
        
        report.append("=" * 60)
        
        return "\n".join(report)
