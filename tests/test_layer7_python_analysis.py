"""
Comprehensive Test Suite for Layer 7: Python Analysis Framework
Tests StrategyAnalyzer, StrategyVisualizer, and ParameterOptimizer
"""

import sys
import os
import numpy as np
import pandas as pd
import unittest
from unittest.mock import Mock, MagicMock
import warnings

# Add the project root to Python path
sys.path.insert(0, '/Users/y0d046w/volatility-arbitrage-trading-system')

# Import analysis framework
try:
    from python.analysis import StrategyAnalyzer, StrategyVisualizer, ParameterOptimizer
    from python.analysis.optimizer import OptimizationUtils, create_volatility_arbitrage_factory
except ImportError as e:
    print(f"Import error: {e}")
    sys.exit(1)

class MockBacktestResult:
    """Mock backtest result for testing"""
    
    def __init__(self, initial_value=100000, daily_returns=None, trade_count=50):
        self.initial_value = initial_value
        
        # Generate realistic equity curve
        if daily_returns is None:
            np.random.seed(42)  # For reproducible tests
            daily_returns = np.random.normal(0.0005, 0.015, 252)  # Daily returns
        
        self.daily_returns = daily_returns
        self.equity_values = self._generate_equity_curve()
        self.trade_count = trade_count
        
    def _generate_equity_curve(self):
        """Generate equity curve from returns"""
        equity = [self.initial_value]
        for ret in self.daily_returns:
            equity.append(equity[-1] * (1 + ret))
        return np.array(equity)
    
    def get_equity_curve(self):
        """Return equity curve"""
        return MockSeries(self.equity_values)
    
    def get_performance_metrics(self):
        """Return basic performance metrics"""
        total_return = (self.equity_values[-1] / self.equity_values[0]) - 1
        returns = self.daily_returns
        
        # Calculate Sharpe ratio
        mean_return = np.mean(returns)
        std_return = np.std(returns)
        sharpe_ratio = (mean_return * 252) / (std_return * np.sqrt(252)) if std_return > 0 else 0
        
        # Calculate max drawdown
        running_max = np.maximum.accumulate(self.equity_values)
        drawdown = (self.equity_values - running_max) / running_max
        max_drawdown = np.min(drawdown)
        
        # Sortino ratio (simplified)
        downside_returns = returns[returns < 0]
        downside_std = np.std(downside_returns) if len(downside_returns) > 0 else std_return
        sortino_ratio = (mean_return * 252) / (downside_std * np.sqrt(252)) if downside_std > 0 else 0
        
        return {
            'total_return': total_return,
            'sharpe_ratio': sharpe_ratio,
            'sortino_ratio': sortino_ratio,
            'max_drawdown': max_drawdown,
            'volatility': std_return * np.sqrt(252)
        }
    
    def get_trades(self):
        """Return mock trades"""
        trades = []
        np.random.seed(42)
        
        for i in range(self.trade_count):
            pnl = np.random.normal(50, 200)  # Random P&L
            trade = MockTrade(pnl)
            trades.append(trade)
            
        return trades

class MockSeries:
    """Mock pandas Series for testing"""
    
    def __init__(self, values):
        self.values = np.array(values)
        self.index = range(len(values))
    
    def pct_change(self):
        """Calculate percentage change"""
        changes = np.diff(self.values) / self.values[:-1]
        return MockSeries(changes)
    
    def dropna(self):
        """Drop NaN values"""
        clean_values = self.values[np.isfinite(self.values)]
        return MockSeries(clean_values)

class MockTrade:
    """Mock trade for testing"""
    
    def __init__(self, pnl):
        self.pnl = pnl
        
    def get_pnl(self):
        return self.pnl

class TestStrategyAnalyzer(unittest.TestCase):
    """Test cases for StrategyAnalyzer"""
    
    def setUp(self):
        """Set up test fixtures"""
        self.mock_result = MockBacktestResult()
        self.analyzer = StrategyAnalyzer(self.mock_result)
    
    def test_initialization(self):
        """Test analyzer initialization"""
        self.assertIsNotNone(self.analyzer)
        self.assertEqual(self.analyzer.result, self.mock_result)
        self.assertIsNone(self.analyzer._cached_metrics)
    
    def test_calculate_performance_metrics(self):
        """Test performance metrics calculation"""
        metrics = self.analyzer.calculate_performance_metrics()
        
        # Check required metrics exist
        required_metrics = [
            'total_return', 'sharpe_ratio', 'sortino_ratio', 'max_drawdown',
            'calmar_ratio', 'volatility', 'skewness', 'kurtosis',
            'win_rate', 'profit_factor'
        ]
        
        for metric in required_metrics:
            self.assertIn(metric, metrics)
            self.assertIsInstance(metrics[metric], (int, float))
            self.assertFalse(np.isnan(metrics[metric]))
        
        # Test caching
        metrics2 = self.analyzer.calculate_performance_metrics()
        self.assertEqual(metrics, metrics2)
        
        print(f"✓ Performance metrics: {len(metrics)} calculated")
    
    def test_analyze_drawdowns(self):
        """Test drawdown analysis"""
        dd_analysis = self.analyzer.analyze_drawdowns()
        
        # Check required fields
        required_fields = [
            'max_drawdown', 'avg_drawdown', 'avg_drawdown_length',
            'major_drawdowns', 'drawdown_series'
        ]
        
        for field in required_fields:
            self.assertIn(field, dd_analysis)
        
        # Validate data types
        self.assertIsInstance(dd_analysis['max_drawdown'], (int, float))
        self.assertIsInstance(dd_analysis['avg_drawdown'], (int, float))
        self.assertIsInstance(dd_analysis['major_drawdowns'], list)
        
        print(f"✓ Drawdown analysis: {len(dd_analysis['major_drawdowns'])} major drawdowns found")
    
    def test_analyze_trade_distribution(self):
        """Test trade distribution analysis"""
        trade_analysis = self.analyzer.analyze_trade_distribution()
        
        # Check required fields
        required_fields = [
            'win_rate', 'avg_win', 'avg_loss', 'profit_factor', 'trade_count'
        ]
        
        for field in required_fields:
            self.assertIn(field, trade_analysis)
            self.assertIsInstance(trade_analysis[field], (int, float))
        
        # Validate ranges
        self.assertGreaterEqual(trade_analysis['win_rate'], 0.0)
        self.assertLessEqual(trade_analysis['win_rate'], 1.0)
        self.assertEqual(trade_analysis['trade_count'], self.mock_result.trade_count)
        
        print(f"✓ Trade analysis: {trade_analysis['trade_count']} trades, "
              f"{trade_analysis['win_rate']:.1%} win rate")
    
    def test_generate_report(self):
        """Test report generation"""
        report = self.analyzer.generate_report()
        
        self.assertIsInstance(report, str)
        self.assertGreater(len(report), 500)  # Should be substantial
        
        # Check for key sections
        self.assertIn("PERFORMANCE METRICS", report)
        self.assertIn("RISK METRICS", report)
        self.assertIn("TRADE ANALYSIS", report)
        
        print(f"✓ Report generated: {len(report)} characters")

class TestStrategyVisualizer(unittest.TestCase):
    """Test cases for StrategyVisualizer"""
    
    def setUp(self):
        """Set up test fixtures"""
        self.mock_result = MockBacktestResult()
        self.analyzer = StrategyAnalyzer(self.mock_result)
        self.visualizer = StrategyVisualizer(self.analyzer)
    
    def test_initialization(self):
        """Test visualizer initialization"""
        self.assertIsNotNone(self.visualizer)
        self.assertEqual(self.visualizer.analyzer, self.analyzer)
        self.assertEqual(self.visualizer.result, self.mock_result)
    
    def test_plot_equity_curve(self):
        """Test equity curve plotting"""
        try:
            import matplotlib.pyplot as plt
            
            fig = self.visualizer.plot_equity_curve()
            self.assertIsNotNone(fig)
            self.assertIsInstance(fig, plt.Figure)
            
            # Test with drawdown subplot
            fig_with_dd = self.visualizer.plot_equity_curve(show_drawdown=True)
            self.assertIsNotNone(fig_with_dd)
            
            plt.close('all')  # Clean up
            print("✓ Equity curve plots created successfully")
            
        except ImportError:
            print("⚠ Matplotlib not available - skipping plot tests")
    
    def test_plot_drawdowns(self):
        """Test drawdown plotting"""
        try:
            import matplotlib.pyplot as plt
            
            fig = self.visualizer.plot_drawdowns()
            self.assertIsNotNone(fig)
            self.assertIsInstance(fig, plt.Figure)
            
            plt.close('all')
            print("✓ Drawdown plot created successfully")
            
        except ImportError:
            print("⚠ Matplotlib not available - skipping plot tests")
    
    def test_plot_returns_distribution(self):
        """Test returns distribution plotting"""
        try:
            import matplotlib.pyplot as plt
            
            fig = self.visualizer.plot_returns_distribution()
            self.assertIsNotNone(fig)
            self.assertIsInstance(fig, plt.Figure)
            
            plt.close('all')
            print("✓ Returns distribution plot created successfully")
            
        except ImportError:
            print("⚠ Matplotlib not available - skipping plot tests")
    
    def test_plot_performance_dashboard(self):
        """Test performance dashboard"""
        try:
            import matplotlib.pyplot as plt
            
            fig = self.visualizer.plot_performance_dashboard()
            self.assertIsNotNone(fig)
            self.assertIsInstance(fig, plt.Figure)
            
            plt.close('all')
            print("✓ Performance dashboard created successfully")
            
        except ImportError:
            print("⚠ Matplotlib not available - skipping plot tests")

class TestParameterOptimizer(unittest.TestCase):
    """Test cases for ParameterOptimizer"""
    
    def setUp(self):
        """Set up test fixtures"""
        self.mock_data = Mock()
        self.strategy_factory = create_volatility_arbitrage_factory()
        self.optimizer = ParameterOptimizer(
            self.strategy_factory, 
            self.mock_data,
            objective_function='sharpe_ratio'
        )
    
    def test_initialization(self):
        """Test optimizer initialization"""
        self.assertIsNotNone(self.optimizer)
        self.assertEqual(self.optimizer.strategy_factory, self.strategy_factory)
        self.assertEqual(self.optimizer.data, self.mock_data)
        self.assertEqual(self.optimizer.objective_function, 'sharpe_ratio')
    
    def test_grid_search_small(self):
        """Test grid search with small parameter space"""
        param_grid = {
            'lookback_window': [20, 30],
            'volatility_threshold': [0.1, 0.2]
        }
        
        results = self.optimizer.grid_search(param_grid, save_results=False)
        
        self.assertIsInstance(results, pd.DataFrame)
        self.assertEqual(len(results), 4)  # 2 x 2 combinations
        
        # Check columns
        required_columns = ['parameters', 'sharpe_ratio']
        for col in required_columns:
            self.assertIn(col, results.columns)
        
        print(f"✓ Grid search completed: {len(results)} combinations tested")
    
    def test_mock_backtest(self):
        """Test single backtest execution"""
        params = {'lookback_window': 30, 'volatility_threshold': 0.2}
        
        metrics = self.optimizer._run_single_backtest(params)
        
        self.assertIsInstance(metrics, dict)
        
        required_metrics = [
            'total_return', 'sharpe_ratio', 'sortino_ratio', 
            'max_drawdown', 'calmar_ratio', 'volatility'
        ]
        
        for metric in required_metrics:
            self.assertIn(metric, metrics)
            self.assertIsInstance(metrics[metric], (int, float))
        
        print(f"✓ Single backtest executed: Sharpe = {metrics['sharpe_ratio']:.3f}")
    
    def test_bayesian_optimization_fallback(self):
        """Test Bayesian optimization fallback"""
        param_space = {
            'lookback_window': (10, 50),
            'volatility_threshold': (0.1, 0.5)
        }
        
        # This should use fallback since scikit-optimize may not be available
        results = self.optimizer.bayesian_optimization(param_space, n_iterations=5)
        
        self.assertIsInstance(results, dict)
        self.assertIn('best_parameters', results)
        self.assertIn('best_value', results)
        self.assertIn('all_results', results)
        
        print(f"✓ Bayesian optimization: Best value = {results['best_value']:.3f}")

class TestOptimizationUtils(unittest.TestCase):
    """Test cases for OptimizationUtils"""
    
    def test_create_param_grid(self):
        """Test parameter grid creation"""
        grid = OptimizationUtils.create_param_grid(
            param1=[1, 2, 3],
            param2=range(10, 20, 5)
        )
        
        self.assertIsInstance(grid, dict)
        self.assertEqual(grid['param1'], [1, 2, 3])
        self.assertEqual(grid['param2'], [10, 15])
        
        print(f"✓ Parameter grid created: {len(grid)} parameters")
    
    def test_create_param_space(self):
        """Test parameter space creation"""
        space = OptimizationUtils.create_param_space(
            param1=(0, 10),
            param2=(0.1, 1.0)
        )
        
        self.assertIsInstance(space, dict)
        self.assertEqual(space['param1'], (0, 10))
        self.assertEqual(space['param2'], (0.1, 1.0))
        
        print(f"✓ Parameter space created: {len(space)} parameters")

def run_layer7_integration_test():
    """Run comprehensive integration test"""
    print("\n" + "="*60)
    print("LAYER 7 PYTHON ANALYSIS FRAMEWORK - INTEGRATION TEST")
    print("="*60)
    
    try:
        # Create test data
        print("\n1. Creating test backtest result...")
        mock_result = MockBacktestResult(
            initial_value=100000,
            daily_returns=np.random.normal(0.0008, 0.012, 252),  # Positive drift
            trade_count=75
        )
        
        # Test StrategyAnalyzer
        print("\n2. Testing StrategyAnalyzer...")
        analyzer = StrategyAnalyzer(mock_result)
        
        # Performance metrics
        metrics = analyzer.calculate_performance_metrics()
        print(f"   - Performance metrics: {len(metrics)} calculated")
        print(f"   - Sharpe Ratio: {metrics['sharpe_ratio']:.3f}")
        print(f"   - Total Return: {metrics['total_return']:.2%}")
        print(f"   - Max Drawdown: {metrics['max_drawdown']:.2%}")
        
        # Drawdown analysis
        dd_analysis = analyzer.analyze_drawdowns()
        print(f"   - Drawdown analysis: {len(dd_analysis['major_drawdowns'])} major drawdowns")
        
        # Trade analysis
        trade_analysis = analyzer.analyze_trade_distribution()
        print(f"   - Trade analysis: {trade_analysis['trade_count']} trades, "
              f"{trade_analysis['win_rate']:.1%} win rate")
        
        # Test StrategyVisualizer
        print("\n3. Testing StrategyVisualizer...")
        visualizer = StrategyVisualizer(analyzer)
        
        try:
            import matplotlib.pyplot as plt
            
            # Create plots
            equity_fig = visualizer.plot_equity_curve()
            drawdown_fig = visualizer.plot_drawdowns()
            returns_fig = visualizer.plot_returns_distribution()
            dashboard_fig = visualizer.plot_performance_dashboard()
            
            print("   - All visualization plots created successfully")
            plt.close('all')  # Clean up
            
        except ImportError:
            print("   - Matplotlib not available, skipping plots")
        
        # Test ParameterOptimizer
        print("\n4. Testing ParameterOptimizer...")
        strategy_factory = create_volatility_arbitrage_factory()
        mock_data = Mock()
        
        optimizer = ParameterOptimizer(strategy_factory, mock_data, 'sharpe_ratio')
        
        # Small grid search
        param_grid = {
            'lookback_window': [20, 30, 40],
            'volatility_threshold': [0.15, 0.20, 0.25]
        }
        
        results = optimizer.grid_search(param_grid, save_results=False)
        print(f"   - Grid search: {len(results)} combinations tested")
        
        best_result = results.iloc[0]
        print(f"   - Best Sharpe ratio: {best_result['sharpe_ratio']:.3f}")
        print(f"   - Best parameters: {best_result['parameters']}")
        
        # Bayesian optimization
        param_space = {
            'lookback_window': (15, 45),
            'volatility_threshold': (0.1, 0.3)
        }
        
        bayes_results = optimizer.bayesian_optimization(param_space, n_iterations=10)
        print(f"   - Bayesian optimization: Best value = {bayes_results['best_value']:.3f}")
        
        # Test report generation
        print("\n5. Testing Report Generation...")
        report = analyzer.generate_report()
        print(f"   - Report generated: {len(report)} characters")
        
        # Print sample of report
        lines = report.split('\n')
        print("   - Report preview:")
        for line in lines[:10]:
            print(f"     {line}")
        print("     ...")
        
        print("\n" + "="*60)
        print("✅ LAYER 7 INTEGRATION TEST COMPLETED SUCCESSFULLY")
        print("="*60)
        
        return True
        
    except Exception as e:
        print(f"\n❌ Integration test failed: {e}")
        import traceback
        traceback.print_exc()
        return False

if __name__ == '__main__':
    # Suppress warnings for cleaner output
    warnings.filterwarnings("ignore")
    
    print("Starting Layer 7 Python Analysis Framework Tests...")
    
    # Run unit tests
    unittest.main(argv=[''], exit=False, verbosity=2)
    
    # Run integration test
    run_layer7_integration_test()
