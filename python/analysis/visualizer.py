"""
Strategy Visualization Module - Layer 7 Component 2
Advanced visualization capabilities for volatility arbitrage strategies
"""

import numpy as np
import pandas as pd
import matplotlib.pyplot as plt
import seaborn as sns
from typing import Dict, List, Any, Optional, Tuple
import warnings
from datetime import datetime, timedelta

# Set style for professional plots
plt.style.use('seaborn-v0_8-darkgrid')
sns.set_palette("husl")

class StrategyVisualizer:
    """
    Advanced strategy visualization with comprehensive plotting capabilities
    """
    
    def __init__(self, analyzer):
        """
        Initialize visualizer with strategy analyzer
        
        Args:
            analyzer: StrategyAnalyzer instance
        """
        self.analyzer = analyzer
        self.result = analyzer.result
        
    def plot_equity_curve(self, figsize: Tuple[int, int] = (12, 6), 
                         show_drawdown: bool = True) -> plt.Figure:
        """
        Plot advanced equity curve with optional drawdown overlay
        
        Args:
            figsize: Figure size tuple
            show_drawdown: Whether to show drawdown as subplot
            
        Returns:
            matplotlib Figure object
        """
        try:
            equity_curve = self.result.get_equity_curve()
            if hasattr(equity_curve, 'values'):
                equity_values = equity_curve.values
                if hasattr(equity_curve, 'index'):
                    dates = equity_curve.index
                else:
                    dates = range(len(equity_values))
            else:
                equity_values = np.array(equity_curve)
                dates = range(len(equity_values))
            
            if show_drawdown:
                fig, (ax1, ax2) = plt.subplots(2, 1, figsize=figsize, 
                                             height_ratios=[3, 1], sharex=True)
            else:
                fig, ax1 = plt.subplots(1, 1, figsize=figsize)
            
            # Main equity curve
            ax1.plot(dates, equity_values, linewidth=2, color='steelblue', 
                    label='Portfolio Value')
            ax1.set_title('Strategy Equity Curve', fontsize=14, fontweight='bold')
            ax1.set_ylabel('Portfolio Value ($)', fontsize=12)
            ax1.grid(True, alpha=0.3)
            ax1.legend()
            
            # Format y-axis
            ax1.yaxis.set_major_formatter(plt.FuncFormatter(
                lambda x, p: f'${x:,.0f}'))
            
            if show_drawdown:
                # Calculate and plot drawdown
                running_max = np.maximum.accumulate(equity_values)
                drawdown = (equity_values - running_max) / running_max * 100
                
                ax2.fill_between(dates, 0, drawdown, color='red', alpha=0.3)
                ax2.plot(dates, drawdown, color='darkred', linewidth=1)
                ax2.set_title('Drawdown', fontsize=12)
                ax2.set_ylabel('Drawdown (%)', fontsize=10)
                ax2.set_xlabel('Time', fontsize=12)
                ax2.grid(True, alpha=0.3)
                
                # Format drawdown axis
                ax2.yaxis.set_major_formatter(plt.FuncFormatter(
                    lambda x, p: f'{x:.1f}%'))
            else:
                ax1.set_xlabel('Time', fontsize=12)
            
            plt.tight_layout()
            return fig
            
        except Exception as e:
            warnings.warn(f"Could not plot equity curve: {e}")
            fig, ax = plt.subplots(figsize=figsize)
            ax.text(0.5, 0.5, f'Error plotting equity curve:\n{str(e)}', 
                   ha='center', va='center', transform=ax.transAxes)
            return fig
    
    def plot_drawdowns(self, figsize: Tuple[int, int] = (12, 6)) -> plt.Figure:
        """
        Plot detailed drawdown analysis
        
        Args:
            figsize: Figure size tuple
            
        Returns:
            matplotlib Figure object
        """
        try:
            drawdown_analysis = self.analyzer.analyze_drawdowns()
            drawdown_series = drawdown_analysis.get('drawdown_series', np.array([]))
            
            if len(drawdown_series) == 0:
                fig, ax = plt.subplots(figsize=figsize)
                ax.text(0.5, 0.5, 'No drawdown data available', 
                       ha='center', va='center', transform=ax.transAxes)
                return fig
            
            fig, ax = plt.subplots(figsize=figsize)
            
            # Plot drawdown series
            dates = range(len(drawdown_series))
            drawdown_pct = drawdown_series * 100
            
            ax.fill_between(dates, 0, drawdown_pct, color='red', alpha=0.3, 
                           label='Drawdown')
            ax.plot(dates, drawdown_pct, color='darkred', linewidth=1)
            
            # Mark major drawdowns
            major_drawdowns = drawdown_analysis.get('major_drawdowns', [])
            for i, dd in enumerate(major_drawdowns):
                peak_idx = dd.get('peak_idx', 0)
                peak_dd = dd.get('peak_drawdown', 0) * 100
                ax.scatter(peak_idx, peak_dd, color='black', s=50, zorder=5)
                ax.annotate(f'{peak_dd:.1f}%', 
                           xy=(peak_idx, peak_dd), 
                           xytext=(5, 5), textcoords='offset points',
                           fontsize=9, ha='left')
            
            ax.set_title('Strategy Drawdowns', fontsize=14, fontweight='bold')
            ax.set_xlabel('Time', fontsize=12)
            ax.set_ylabel('Drawdown (%)', fontsize=12)
            ax.grid(True, alpha=0.3)
            ax.legend()
            
            # Format y-axis
            ax.yaxis.set_major_formatter(plt.FuncFormatter(
                lambda x, p: f'{x:.1f}%'))
            
            plt.tight_layout()
            return fig
            
        except Exception as e:
            warnings.warn(f"Could not plot drawdowns: {e}")
            fig, ax = plt.subplots(figsize=figsize)
            ax.text(0.5, 0.5, f'Error plotting drawdowns:\n{str(e)}', 
                   ha='center', va='center', transform=ax.transAxes)
            return fig
    
    def plot_returns_distribution(self, figsize: Tuple[int, int] = (12, 6), 
                                bins: int = 50) -> plt.Figure:
        """
        Plot returns distribution with statistical overlays
        
        Args:
            figsize: Figure size tuple  
            bins: Number of histogram bins
            
        Returns:
            matplotlib Figure object
        """
        try:
            equity_curve = self.result.get_equity_curve()
            if hasattr(equity_curve, 'values'):
                equity_values = equity_curve.values
            else:
                equity_values = np.array(equity_curve)
            
            if len(equity_values) < 2:
                fig, ax = plt.subplots(figsize=figsize)
                ax.text(0.5, 0.5, 'Insufficient data for returns analysis', 
                       ha='center', va='center', transform=ax.transAxes)
                return fig
            
            # Calculate returns
            returns = np.diff(equity_values) / equity_values[:-1]
            returns = returns[np.isfinite(returns)] * 100  # Convert to percentage
            
            fig, ax = plt.subplots(figsize=figsize)
            
            # Plot histogram with KDE
            ax.hist(returns, bins=bins, density=True, alpha=0.7, 
                   color='steelblue', label='Returns')
            
            # Add KDE overlay
            try:
                from scipy import stats
                kde = stats.gaussian_kde(returns)
                x_range = np.linspace(returns.min(), returns.max(), 200)
                ax.plot(x_range, kde(x_range), 'r-', linewidth=2, 
                       label='KDE', alpha=0.8)
            except ImportError:
                pass
            
            # Add normal distribution overlay for comparison
            mean_ret = np.mean(returns)
            std_ret = np.std(returns)
            x_norm = np.linspace(returns.min(), returns.max(), 200)
            y_norm = stats.norm.pdf(x_norm, mean_ret, std_ret)
            ax.plot(x_norm, y_norm, 'g--', linewidth=2, 
                   label='Normal Dist', alpha=0.8)
            
            # Add vertical lines for statistics
            ax.axvline(mean_ret, color='orange', linestyle='-', linewidth=2, 
                      label=f'Mean: {mean_ret:.2f}%')
            ax.axvline(np.percentile(returns, 5), color='red', linestyle='--', 
                      linewidth=1, label='5% VaR')
            ax.axvline(np.percentile(returns, 95), color='red', linestyle='--', 
                      linewidth=1, label='95% VaR')
            
            ax.set_title('Returns Distribution Analysis', fontsize=14, fontweight='bold')
            ax.set_xlabel('Daily Return (%)', fontsize=12)
            ax.set_ylabel('Density', fontsize=12)
            ax.grid(True, alpha=0.3)
            ax.legend()
            
            # Add statistics text box
            metrics = self.analyzer.calculate_performance_metrics()
            stats_text = f"""Statistics:
Mean: {mean_ret:.2f}%
Std: {std_ret:.2f}%
Skewness: {metrics.get('skewness', 0.0):.2f}
Kurtosis: {metrics.get('kurtosis', 0.0):.2f}"""
            
            ax.text(0.02, 0.98, stats_text, transform=ax.transAxes, 
                   verticalalignment='top', bbox=dict(boxstyle='round', 
                   facecolor='white', alpha=0.8), fontsize=10)
            
            plt.tight_layout()
            return fig
            
        except Exception as e:
            warnings.warn(f"Could not plot returns distribution: {e}")
            fig, ax = plt.subplots(figsize=figsize)
            ax.text(0.5, 0.5, f'Error plotting returns distribution:\n{str(e)}', 
                   ha='center', va='center', transform=ax.transAxes)
            return fig
    
    def plot_risk_return_scatter(self, figsize: Tuple[int, int] = (10, 8)) -> plt.Figure:
        """
        Plot risk-return scatter with different time periods
        
        Args:
            figsize: Figure size tuple
            
        Returns:
            matplotlib Figure object
        """
        try:
            equity_curve = self.result.get_equity_curve()
            if hasattr(equity_curve, 'values'):
                equity_values = equity_curve.values
            else:
                equity_values = np.array(equity_curve)
            
            if len(equity_values) < 252:  # Need at least 1 year of data
                fig, ax = plt.subplots(figsize=figsize)
                ax.text(0.5, 0.5, 'Insufficient data for risk-return analysis', 
                       ha='center', va='center', transform=ax.transAxes)
                return fig
            
            fig, ax = plt.subplots(figsize=figsize)
            
            # Calculate rolling windows
            windows = [30, 60, 120, 252]  # 1M, 2M, 4M, 1Y
            colors = ['red', 'orange', 'green', 'blue']
            
            for window, color in zip(windows, colors):
                if len(equity_values) < window:
                    continue
                    
                returns_list = []
                volatilities = []
                
                for i in range(window, len(equity_values)):
                    period_values = equity_values[i-window:i+1]
                    period_returns = np.diff(period_values) / period_values[:-1]
                    
                    # Annualized return and volatility
                    total_return = (period_values[-1] / period_values[0]) ** (252/window) - 1
                    volatility = np.std(period_returns) * np.sqrt(252)
                    
                    returns_list.append(total_return * 100)
                    volatilities.append(volatility * 100)
                
                ax.scatter(volatilities, returns_list, alpha=0.6, 
                          color=color, s=30, label=f'{window}D Rolling')
            
            ax.set_title('Risk-Return Analysis (Rolling Windows)', 
                        fontsize=14, fontweight='bold')
            ax.set_xlabel('Volatility (% Annualized)', fontsize=12)
            ax.set_ylabel('Return (% Annualized)', fontsize=12)
            ax.grid(True, alpha=0.3)
            ax.legend()
            
            # Add reference lines
            ax.axhline(y=0, color='black', linestyle='-', alpha=0.3)
            ax.axvline(x=0, color='black', linestyle='-', alpha=0.3)
            
            plt.tight_layout()
            return fig
            
        except Exception as e:
            warnings.warn(f"Could not plot risk-return scatter: {e}")
            fig, ax = plt.subplots(figsize=figsize)
            ax.text(0.5, 0.5, f'Error plotting risk-return scatter:\n{str(e)}', 
                   ha='center', va='center', transform=ax.transAxes)
            return fig
    
    def plot_performance_dashboard(self, figsize: Tuple[int, int] = (16, 12)) -> plt.Figure:
        """
        Create comprehensive performance dashboard
        
        Args:
            figsize: Figure size tuple
            
        Returns:
            matplotlib Figure object with multiple subplots
        """
        try:
            fig = plt.figure(figsize=figsize)
            
            # Create grid layout
            gs = fig.add_gridspec(3, 3, height_ratios=[2, 1, 1], 
                                 width_ratios=[2, 1, 1])
            
            # Equity curve (top left - spans 2 columns)
            ax1 = fig.add_subplot(gs[0, :2])
            equity_curve = self.result.get_equity_curve()
            if hasattr(equity_curve, 'values'):
                equity_values = equity_curve.values
                dates = range(len(equity_values))
            else:
                equity_values = np.array(equity_curve)
                dates = range(len(equity_values))
            
            ax1.plot(dates, equity_values, linewidth=2, color='steelblue')
            ax1.set_title('Equity Curve', fontweight='bold')
            ax1.grid(True, alpha=0.3)
            
            # Performance metrics table (top right)
            ax2 = fig.add_subplot(gs[0, 2])
            ax2.axis('off')
            
            metrics = self.analyzer.calculate_performance_metrics()
            metrics_text = f"""PERFORMANCE METRICS
            
Total Return: {metrics.get('total_return', 0.0):.2%}
Sharpe Ratio: {metrics.get('sharpe_ratio', 0.0):.2f}
Sortino Ratio: {metrics.get('sortino_ratio', 0.0):.2f}
Max Drawdown: {metrics.get('max_drawdown', 0.0):.2%}
Volatility: {metrics.get('volatility', 0.0):.2%}
Win Rate: {metrics.get('win_rate', 0.0):.2%}
Profit Factor: {metrics.get('profit_factor', 0.0):.2f}"""
            
            ax2.text(0.05, 0.95, metrics_text, transform=ax2.transAxes,
                    verticalalignment='top', fontsize=10, fontfamily='monospace',
                    bbox=dict(boxstyle='round', facecolor='lightgray', alpha=0.8))
            
            # Drawdown (middle left)
            ax3 = fig.add_subplot(gs[1, :2])
            drawdown_analysis = self.analyzer.analyze_drawdowns()
            drawdown_series = drawdown_analysis.get('drawdown_series', np.array([]))
            
            if len(drawdown_series) > 0:
                ax3.fill_between(dates, 0, drawdown_series * 100, 
                               color='red', alpha=0.3)
                ax3.plot(dates, drawdown_series * 100, color='darkred')
            
            ax3.set_title('Drawdown', fontweight='bold')
            ax3.set_ylabel('Drawdown (%)')
            ax3.grid(True, alpha=0.3)
            
            # Returns distribution (middle right)
            ax4 = fig.add_subplot(gs[1, 2])
            if len(equity_values) > 1:
                returns = np.diff(equity_values) / equity_values[:-1] * 100
                returns = returns[np.isfinite(returns)]
                ax4.hist(returns, bins=20, density=True, alpha=0.7, color='green')
                ax4.set_title('Returns Dist.', fontweight='bold')
                ax4.set_xlabel('Return (%)')
            
            # Monthly returns heatmap (bottom)
            ax5 = fig.add_subplot(gs[2, :])
            
            # Create mock monthly returns for visualization
            if len(equity_values) > 30:
                monthly_data = []
                for i in range(0, len(equity_values), 21):  # Approx monthly
                    if i + 21 < len(equity_values):
                        month_return = (equity_values[i+21] / equity_values[i] - 1) * 100
                        monthly_data.append(month_return)
                
                if len(monthly_data) >= 12:
                    # Reshape to year x month grid
                    years = len(monthly_data) // 12
                    monthly_matrix = np.array(monthly_data[:years*12]).reshape(years, 12)
                    
                    im = ax5.imshow(monthly_matrix, cmap='RdYlGn', aspect='auto')
                    ax5.set_title('Monthly Returns Heatmap (%)', fontweight='bold')
                    ax5.set_xlabel('Month')
                    ax5.set_ylabel('Year')
                    
                    # Add colorbar
                    cbar = plt.colorbar(im, ax=ax5, orientation='horizontal', pad=0.1)
                    cbar.set_label('Return (%)')
                else:
                    ax5.text(0.5, 0.5, 'Insufficient data for monthly heatmap', 
                           ha='center', va='center', transform=ax5.transAxes)
            else:
                ax5.text(0.5, 0.5, 'Insufficient data for monthly heatmap', 
                       ha='center', va='center', transform=ax5.transAxes)
            
            plt.suptitle('Volatility Arbitrage Strategy Dashboard', 
                        fontsize=16, fontweight='bold')
            plt.tight_layout()
            
            return fig
            
        except Exception as e:
            warnings.warn(f"Could not create performance dashboard: {e}")
            fig, ax = plt.subplots(figsize=figsize)
            ax.text(0.5, 0.5, f'Error creating dashboard:\n{str(e)}', 
                   ha='center', va='center', transform=ax.transAxes)
            return fig
    
    def save_all_plots(self, output_dir: str = "analysis_plots"):
        """
        Save all visualization plots to directory
        
        Args:
            output_dir: Directory to save plots
        """
        import os
        os.makedirs(output_dir, exist_ok=True)
        
        # Generate and save all plots
        plots = {
            'equity_curve.png': self.plot_equity_curve(),
            'drawdowns.png': self.plot_drawdowns(),
            'returns_distribution.png': self.plot_returns_distribution(),
            'risk_return_scatter.png': self.plot_risk_return_scatter(),
            'performance_dashboard.png': self.plot_performance_dashboard()
        }
        
        for filename, fig in plots.items():
            filepath = os.path.join(output_dir, filename)
            fig.savefig(filepath, dpi=300, bbox_inches='tight')
            plt.close(fig)
            
        print(f"All plots saved to {output_dir}/")
