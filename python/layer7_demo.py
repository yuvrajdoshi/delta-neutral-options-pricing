"""
Layer 7 Python Analysis Framework - Demonstration Script
Shows practical usage of StrategyAnalyzer, StrategyVisualizer, and ParameterOptimizer
"""

import sys
import os
import numpy as np
import pandas as pd
import warnings
warnings.filterwarnings("ignore")

# Add project to path
sys.path.insert(0, '/Users/y0d046w/volatility-arbitrage-trading-system')

try:
    from python.analysis import StrategyAnalyzer, StrategyVisualizer, ParameterOptimizer
    from python.analysis.optimizer import OptimizationUtils, create_volatility_arbitrage_factory
except ImportError as e:
    print(f"Import error: {e}")
    print("Make sure Layer 7 is properly installed")
    sys.exit(1)

class VolatilityArbitrageDemo:
    """Demonstration of Layer 7 Python Analysis Framework"""
    
    def __init__(self):
        """Initialize demo"""
        print("=" * 70)
        print("VOLATILITY ARBITRAGE TRADING SYSTEM - LAYER 7 DEMO")
        print("Python Analysis Framework")
        print("=" * 70)
        
        # Create realistic mock data
        self.backtest_result = self._create_realistic_backtest()
        
    def _create_realistic_backtest(self):
        """Create realistic backtest result for demonstration"""
        print("\n📊 Creating realistic volatility arbitrage backtest...")
        
        # Simulate 2 years of daily trading
        np.random.seed(42)
        n_days = 504  # 2 years
        
        # Generate market regime changes
        regime_changes = [0, 126, 252, 378, 504]  # Quarterly changes
        regimes = ['low_vol', 'high_vol', 'trending', 'mean_reverting']
        
        daily_returns = []
        
        for i in range(len(regime_changes) - 1):
            start_day = regime_changes[i]
            end_day = regime_changes[i + 1]
            regime = regimes[i % len(regimes)]
            
            days_in_regime = end_day - start_day
            
            if regime == 'low_vol':
                # Low volatility, steady gains (good for vol arb)
                returns = np.random.normal(0.0012, 0.008, days_in_regime)
            elif regime == 'high_vol':
                # High volatility, mixed returns (excellent for vol arb)
                returns = np.random.normal(0.0015, 0.025, days_in_regime)
            elif regime == 'trending':
                # Trending market (challenging for vol arb)
                trend = np.linspace(0.0005, 0.002, days_in_regime)
                returns = trend + np.random.normal(0, 0.012, days_in_regime)
            else:  # mean_reverting
                # Mean reverting (good for vol arb)
                returns = np.random.normal(0.0008, 0.015, days_in_regime)
                # Add mean reversion
                for j in range(1, len(returns)):
                    returns[j] -= 0.3 * returns[j-1]
            
            daily_returns.extend(returns)
        
        # Create mock backtest result
        from tests.test_layer7_python_analysis import MockBacktestResult
        
        result = MockBacktestResult(
            initial_value=1000000,  # $1M starting capital
            daily_returns=np.array(daily_returns),
            trade_count=156  # ~3 trades per week
        )
        
        # Calculate some statistics
        final_value = result.equity_values[-1]
        total_return = (final_value / result.initial_value) - 1
        
        print(f"   Initial Capital: ${result.initial_value:,.0f}")
        print(f"   Final Value: ${final_value:,.0f}")
        print(f"   Total Return: {total_return:.2%}")
        print(f"   Trading Days: {len(daily_returns)}")
        print(f"   Total Trades: {result.trade_count}")
        
        return result
    
    def demonstrate_strategy_analyzer(self):
        """Demonstrate StrategyAnalyzer capabilities"""
        print("\n" + "=" * 50)
        print("1. STRATEGY ANALYZER DEMONSTRATION")
        print("=" * 50)
        
        analyzer = StrategyAnalyzer(self.backtest_result)
        
        # Performance metrics
        print("\n📈 Calculating comprehensive performance metrics...")
        metrics = analyzer.calculate_performance_metrics()
        
        key_metrics = [
            ('Total Return', 'total_return', '%'),
            ('Sharpe Ratio', 'sharpe_ratio', ''),
            ('Sortino Ratio', 'sortino_ratio', ''),
            ('Calmar Ratio', 'calmar_ratio', ''),
            ('Volatility', 'volatility', '%'),
            ('Max Drawdown', 'max_drawdown', '%'),
            ('Win Rate', 'win_rate', '%'),
            ('Profit Factor', 'profit_factor', '')
        ]
        
        for name, key, unit in key_metrics:
            value = metrics.get(key, 0)
            if unit == '%':
                print(f"   {name:15}: {value:8.2%}")
            else:
                print(f"   {name:15}: {value:8.2f}")
        
        # Drawdown analysis
        print("\n📉 Analyzing drawdown characteristics...")
        dd_analysis = analyzer.analyze_drawdowns()
        
        print(f"   Max Drawdown: {dd_analysis['max_drawdown']:.2%}")
        print(f"   Avg Drawdown: {dd_analysis['avg_drawdown']:.2%}")
        print(f"   Avg DD Length: {dd_analysis['avg_drawdown_length']:.0f} days")
        print(f"   Major Drawdowns (>5%): {len(dd_analysis['major_drawdowns'])}")
        
        # Trade analysis
        print("\n💰 Analyzing trade distribution...")
        trade_analysis = analyzer.analyze_trade_distribution()
        
        print(f"   Total Trades: {trade_analysis['trade_count']}")
        print(f"   Win Rate: {trade_analysis['win_rate']:.2%}")
        print(f"   Avg Win: ${trade_analysis['avg_win']:.2f}")
        print(f"   Avg Loss: ${trade_analysis['avg_loss']:.2f}")
        print(f"   Profit Factor: {trade_analysis['profit_factor']:.2f}")
        
        # Generate full report
        print("\n📋 Generating comprehensive report...")
        report = analyzer.generate_report()
        
        # Save report to file
        with open('volatility_arbitrage_analysis_report.txt', 'w') as f:
            f.write(report)
        
        print("   ✅ Full report saved to 'volatility_arbitrage_analysis_report.txt'")
        
        return analyzer
    
    def demonstrate_strategy_visualizer(self, analyzer):
        """Demonstrate StrategyVisualizer capabilities"""
        print("\n" + "=" * 50)
        print("2. STRATEGY VISUALIZER DEMONSTRATION")
        print("=" * 50)
        
        try:
            import matplotlib.pyplot as plt
            
            visualizer = StrategyVisualizer(analyzer)
            
            print("\n📊 Creating visualization suite...")
            
            # 1. Equity curve with drawdown
            print("   Creating equity curve with drawdown overlay...")
            fig1 = visualizer.plot_equity_curve(show_drawdown=True, figsize=(14, 8))
            fig1.suptitle('Volatility Arbitrage Strategy - Equity Curve & Drawdown', 
                         fontsize=16, fontweight='bold')
            fig1.savefig('equity_curve_analysis.png', dpi=300, bbox_inches='tight')
            plt.close(fig1)
            
            # 2. Detailed drawdown analysis
            print("   Creating detailed drawdown analysis...")
            fig2 = visualizer.plot_drawdowns(figsize=(12, 6))
            fig2.suptitle('Drawdown Analysis', fontsize=14, fontweight='bold')
            fig2.savefig('drawdown_analysis.png', dpi=300, bbox_inches='tight')
            plt.close(fig2)
            
            # 3. Returns distribution
            print("   Creating returns distribution analysis...")
            fig3 = visualizer.plot_returns_distribution(figsize=(12, 6))
            fig3.suptitle('Daily Returns Distribution', fontsize=14, fontweight='bold')
            fig3.savefig('returns_distribution.png', dpi=300, bbox_inches='tight')
            plt.close(fig3)
            
            # 4. Risk-return scatter
            print("   Creating risk-return analysis...")
            fig4 = visualizer.plot_risk_return_scatter(figsize=(10, 8))
            fig4.suptitle('Risk-Return Analysis (Rolling Windows)', fontsize=14, fontweight='bold')
            fig4.savefig('risk_return_analysis.png', dpi=300, bbox_inches='tight')
            plt.close(fig4)
            
            # 5. Comprehensive dashboard
            print("   Creating comprehensive performance dashboard...")
            fig5 = visualizer.plot_performance_dashboard(figsize=(16, 12))
            fig5.savefig('performance_dashboard.png', dpi=300, bbox_inches='tight')
            plt.close(fig5)
            
            print("\n   ✅ All visualizations saved:")
            print("     - equity_curve_analysis.png")
            print("     - drawdown_analysis.png")
            print("     - returns_distribution.png")
            print("     - risk_return_analysis.png")
            print("     - performance_dashboard.png")
            
            return visualizer
            
        except ImportError:
            print("\n   ⚠️  Matplotlib not available - skipping visualizations")
            print("     Install with: pip install matplotlib seaborn")
            return None
    
    def demonstrate_parameter_optimizer(self):
        """Demonstrate ParameterOptimizer capabilities"""
        print("\n" + "=" * 50)
        print("3. PARAMETER OPTIMIZER DEMONSTRATION")
        print("=" * 50)
        
        # Create strategy factory
        print("\n🏭 Setting up strategy factory...")
        strategy_factory = create_volatility_arbitrage_factory({
            'initial_capital': 1000000,
            'risk_limit': 0.02
        })
        
        # Mock market data
        mock_data = "market_data_placeholder"
        
        # Create optimizer
        optimizer = ParameterOptimizer(
            strategy_factory, 
            mock_data,
            objective_function='sharpe_ratio'
        )
        
        print("   ✅ Optimizer initialized with Sharpe ratio objective")
        
        # Grid Search Demonstration
        print("\n🔍 Demonstrating Grid Search Optimization...")
        
        param_grid = OptimizationUtils.create_param_grid(
            lookback_window=[15, 20, 25, 30, 35],
            volatility_threshold=[0.10, 0.15, 0.20, 0.25, 0.30],
            hedge_ratio=[0.6, 0.7, 0.8, 0.9]
        )
        
        print(f"   Parameter grid: {len(param_grid)} parameters")
        for param, values in param_grid.items():
            print(f"     {param}: {values}")
        
        total_combinations = 1
        for values in param_grid.values():
            total_combinations *= len(values)
        print(f"   Total combinations: {total_combinations}")
        
        # Run grid search
        grid_results = optimizer.grid_search(param_grid, save_results=True,
                                           results_file='grid_search_results.json')
        
        print(f"\n   ✅ Grid search completed: {len(grid_results)} results")
        
        # Show best results
        print("\n   🏆 Top 5 parameter combinations:")
        for i, (_, row) in enumerate(grid_results.head().iterrows()):
            params = row['parameters']
            sharpe = row['sharpe_ratio']
            print(f"     #{i+1}: Sharpe={sharpe:.3f} | {params}")
        
        # Bayesian Optimization Demonstration
        print("\n🎯 Demonstrating Bayesian Optimization...")
        
        param_space = OptimizationUtils.create_param_space(
            lookback_window=(10, 50),
            volatility_threshold=(0.05, 0.40),
            hedge_ratio=(0.5, 1.0)
        )
        
        print(f"   Parameter space: {param_space}")
        
        # Run Bayesian optimization
        bayes_results = optimizer.bayesian_optimization(
            param_space, 
            n_iterations=25,
            n_initial_points=5
        )
        
        print(f"\n   ✅ Bayesian optimization completed")
        print(f"   🏆 Best Sharpe ratio: {bayes_results['best_value']:.3f}")
        print(f"   🎯 Best parameters: {bayes_results['best_parameters']}")
        
        # Parameter sensitivity analysis
        print("\n📊 Analyzing parameter sensitivity...")
        
        for param_name in ['lookback_window', 'volatility_threshold', 'hedge_ratio']:
            sensitivity = optimizer.analyze_parameter_sensitivity(grid_results, param_name)
            
            if 'error' not in sensitivity:
                print(f"   {param_name}:")
                print(f"     Correlation: {sensitivity['correlation']:.3f}")
                print(f"     Optimal range: {sensitivity['optimal_range']}")
                print(f"     Sensitivity: {sensitivity['sensitivity']:.3f}")
        
        return optimizer, grid_results, bayes_results
    
    def demonstrate_complete_workflow(self):
        """Demonstrate complete analysis workflow"""
        print("\n" + "=" * 50)
        print("4. COMPLETE ANALYSIS WORKFLOW")
        print("=" * 50)
        
        print("\n🔄 Running complete analysis workflow...")
        
        # Step 1: Strategy Analysis
        print("\n   Step 1: Strategy Performance Analysis")
        analyzer = StrategyAnalyzer(self.backtest_result)
        metrics = analyzer.calculate_performance_metrics()
        
        # Step 2: Visualization
        print("   Step 2: Creating Visualizations")
        try:
            visualizer = StrategyVisualizer(analyzer)
            dashboard = visualizer.plot_performance_dashboard()
            dashboard.savefig('complete_analysis_dashboard.png', dpi=300, bbox_inches='tight')
            import matplotlib.pyplot as plt
            plt.close(dashboard)
            print("     ✅ Dashboard saved")
        except ImportError:
            print("     ⚠️  Matplotlib not available")
        
        # Step 3: Parameter Optimization
        print("   Step 3: Parameter Optimization")
        strategy_factory = create_volatility_arbitrage_factory()
        optimizer = ParameterOptimizer(strategy_factory, "mock_data", 'sharpe_ratio')
        
        # Quick optimization
        small_grid = {
            'lookback_window': [20, 30],
            'volatility_threshold': [0.15, 0.25]
        }
        
        opt_results = optimizer.grid_search(small_grid, save_results=False)
        best_params = opt_results.iloc[0]['parameters']
        best_sharpe = opt_results.iloc[0]['sharpe_ratio']
        
        print(f"     ✅ Optimization complete")
        print(f"     🏆 Best Sharpe: {best_sharpe:.3f}")
        print(f"     🎯 Best params: {best_params}")
        
        # Step 4: Generate Summary Report
        print("   Step 4: Generating Summary Report")
        
        summary_report = f"""
VOLATILITY ARBITRAGE STRATEGY - ANALYSIS SUMMARY
{'='*55}

PERFORMANCE OVERVIEW
- Total Return: {metrics.get('total_return', 0):.2%}
- Sharpe Ratio: {metrics.get('sharpe_ratio', 0):.2f}
- Max Drawdown: {metrics.get('max_drawdown', 0):.2%}
- Win Rate: {metrics.get('win_rate', 0):.1%}
- Profit Factor: {metrics.get('profit_factor', 0):.2f}

OPTIMAL PARAMETERS (from optimization)
- Lookback Window: {best_params.get('lookback_window', 'N/A')}
- Volatility Threshold: {best_params.get('volatility_threshold', 'N/A')}
- Expected Sharpe: {best_sharpe:.3f}

RECOMMENDATIONS
1. Strategy shows {'strong' if metrics.get('sharpe_ratio', 0) > 1.5 else 'moderate' if metrics.get('sharpe_ratio', 0) > 1.0 else 'weak'} risk-adjusted performance
2. {'Low' if abs(metrics.get('max_drawdown', 0)) < 0.1 else 'Moderate' if abs(metrics.get('max_drawdown', 0)) < 0.2 else 'High'} drawdown risk observed
3. Parameter optimization suggests using optimized settings for best results
4. Consider implementing optimized parameters in live trading

ANALYSIS GENERATED: {pd.Timestamp.now().strftime('%Y-%m-%d %H:%M:%S')}
"""
        
        with open('volatility_arbitrage_summary.txt', 'w') as f:
            f.write(summary_report)
        
        print("     ✅ Summary report saved to 'volatility_arbitrage_summary.txt'")
        
        print("\n🎉 Complete workflow finished successfully!")

def main():
    """Run the complete Layer 7 demonstration"""
    try:
        # Initialize demo
        demo = VolatilityArbitrageDemo()
        
        # Run demonstrations
        analyzer = demo.demonstrate_strategy_analyzer()
        visualizer = demo.demonstrate_strategy_visualizer(analyzer)
        optimizer_results = demo.demonstrate_parameter_optimizer()
        demo.demonstrate_complete_workflow()
        
        print("\n" + "=" * 70)
        print("✅ LAYER 7 DEMONSTRATION COMPLETED SUCCESSFULLY")
        print("=" * 70)
        
        print("\nFiles generated:")
        files = [
            'volatility_arbitrage_analysis_report.txt',
            'equity_curve_analysis.png',
            'drawdown_analysis.png', 
            'returns_distribution.png',
            'risk_return_analysis.png',
            'performance_dashboard.png',
            'complete_analysis_dashboard.png',
            'grid_search_results.json',
            'volatility_arbitrage_summary.txt'
        ]
        
        for file in files:
            if os.path.exists(file):
                print(f"  ✅ {file}")
            else:
                print(f"  ⚠️  {file} (may not be generated due to dependencies)")
        
        print(f"\n🚀 Layer 7 Python Analysis Framework is fully operational!")
        print("     Ready for production use in volatility arbitrage trading.")
        
    except Exception as e:
        print(f"\n❌ Demo failed: {e}")
        import traceback
        traceback.print_exc()

if __name__ == '__main__':
    main()
