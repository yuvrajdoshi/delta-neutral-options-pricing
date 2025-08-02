"""
Parameter Optimization Module - Layer 7 Component 3
Grid search and Bayesian optimization for strategy parameters
"""

import numpy as np
import pandas as pd
import itertools
from typing import Dict, List, Any, Callable, Optional, Union, Tuple
import warnings
from concurrent.futures import ProcessPoolExecutor, as_completed
import time
import json

class ParameterOptimizer:
    """
    Advanced parameter optimization with grid search and Bayesian optimization
    """
    
    def __init__(self, strategy_factory: Callable, market_data: Any, 
                 objective_function: str = 'sharpe_ratio'):
        """
        Initialize optimizer
        
        Args:
            strategy_factory: Function that creates strategy with given parameters
            market_data: Market data for backtesting
            objective_function: Metric to optimize ('sharpe_ratio', 'sortino_ratio', 'calmar_ratio', etc.)
        """
        self.strategy_factory = strategy_factory
        self.data = market_data
        self.objective_function = objective_function
        self.results_cache = {}
        
    def grid_search(self, param_grid: Dict[str, List], 
                   max_workers: int = 4,
                   save_results: bool = True,
                   results_file: str = "optimization_results.json") -> pd.DataFrame:
        """
        Exhaustive grid search optimization
        
        Args:
            param_grid: Dictionary with parameter names as keys and lists of values
            max_workers: Number of parallel workers
            save_results: Whether to save results to file
            results_file: File to save results
            
        Returns:
            DataFrame with optimization results
        """
        print(f"Starting grid search optimization...")
        print(f"Parameter grid: {param_grid}")
        
        # Generate all parameter combinations
        param_names = list(param_grid.keys())
        param_values = list(param_grid.values())
        param_combinations = list(itertools.product(*param_values))
        
        total_combinations = len(param_combinations)
        print(f"Total parameter combinations: {total_combinations}")
        
        results = []
        start_time = time.time()
        
        # Sequential execution for now (can be parallelized later)
        for i, params in enumerate(param_combinations):
            param_dict = dict(zip(param_names, params))
            
            try:
                # Check cache first
                cache_key = str(sorted(param_dict.items()))
                if cache_key in self.results_cache:
                    result_metrics = self.results_cache[cache_key]
                else:
                    # Run single backtest
                    result_metrics = self._run_single_backtest(param_dict)
                    self.results_cache[cache_key] = result_metrics
                
                # Store results
                result_row = {
                    'parameters': param_dict.copy(),
                    'param_string': cache_key,
                    **result_metrics
                }
                results.append(result_row)
                
                # Progress reporting
                if (i + 1) % max(1, total_combinations // 10) == 0:
                    elapsed = time.time() - start_time
                    progress = (i + 1) / total_combinations
                    eta = elapsed / progress - elapsed if progress > 0 else 0
                    print(f"Progress: {i+1}/{total_combinations} ({progress:.1%}) - "
                          f"ETA: {eta:.1f}s")
                
            except Exception as e:
                warnings.warn(f"Error with parameters {param_dict}: {e}")
                # Add failed result
                result_row = {
                    'parameters': param_dict.copy(),
                    'param_string': str(sorted(param_dict.items())),
                    'error': str(e),
                    self.objective_function: -np.inf  # Mark as failed
                }
                results.append(result_row)
        
        # Convert to DataFrame
        df_results = pd.DataFrame(results)
        
        # Sort by objective function (descending)
        if self.objective_function in df_results.columns:
            df_results = df_results.sort_values(self.objective_function, ascending=False)
        
        # Save results if requested
        if save_results and results_file:
            self._save_results(df_results, results_file)
        
        elapsed_total = time.time() - start_time
        print(f"Grid search completed in {elapsed_total:.1f} seconds")
        print(f"Best {self.objective_function}: {df_results.iloc[0][self.objective_function]:.4f}")
        
        return df_results
    
    def bayesian_optimization(self, param_space: Dict[str, Tuple], 
                            n_iterations: int = 50,
                            n_initial_points: int = 10,
                            random_state: int = 42) -> Dict[str, Any]:
        """
        Bayesian optimization for parameter tuning
        
        Args:
            param_space: Dictionary with parameter bounds (name: (min, max))
            n_iterations: Number of optimization iterations
            n_initial_points: Number of initial random points
            random_state: Random seed
            
        Returns:
            Dictionary with optimization results
        """
        try:
            from skopt import gp_minimize
            from skopt.space import Real
            from skopt.utils import use_named_args
        except ImportError:
            warnings.warn("scikit-optimize not available. Install with: pip install scikit-optimize")
            return self._fallback_random_search(param_space, n_iterations)
        
        print(f"Starting Bayesian optimization...")
        print(f"Parameter space: {param_space}")
        print(f"Iterations: {n_iterations}, Initial points: {n_initial_points}")
        
        # Define search space
        dimensions = []
        param_names = list(param_space.keys())
        
        for param_name, (min_val, max_val) in param_space.items():
            dimensions.append(Real(min_val, max_val, name=param_name))
        
        # Define objective function
        @use_named_args(dimensions)
        def objective(**params):
            try:
                result_metrics = self._run_single_backtest(params)
                # Return negative value for minimization
                return -result_metrics.get(self.objective_function, -np.inf)
            except Exception as e:
                warnings.warn(f"Error in objective function: {e}")
                return np.inf  # Return high value for failed runs
        
        # Run optimization
        start_time = time.time()
        
        result = gp_minimize(
            func=objective,
            dimensions=dimensions,
            n_calls=n_iterations,
            n_initial_points=n_initial_points,
            random_state=random_state,
            n_jobs=1  # Sequential for stability
        )
        
        elapsed = time.time() - start_time
        
        # Extract best parameters
        best_params = dict(zip(param_names, result.x))
        best_value = -result.fun  # Convert back from minimization
        
        print(f"Bayesian optimization completed in {elapsed:.1f} seconds")
        print(f"Best {self.objective_function}: {best_value:.4f}")
        print(f"Best parameters: {best_params}")
        
        # Prepare detailed results
        all_results = []
        for i, (params, value) in enumerate(zip(result.x_iters, result.func_vals)):
            param_dict = dict(zip(param_names, params))
            all_results.append({
                'iteration': i,
                'parameters': param_dict,
                self.objective_function: -value  # Convert back
            })
        
        return {
            'best_parameters': best_params,
            'best_value': best_value,
            'all_results': all_results,
            'optimization_result': result,
            'convergence_curve': [-val for val in result.func_vals]
        }
    
    def _fallback_random_search(self, param_space: Dict[str, Tuple], 
                               n_iterations: int) -> Dict[str, Any]:
        """
        Fallback random search when Bayesian optimization is not available
        """
        print("Using random search fallback...")
        
        np.random.seed(42)
        results = []
        best_value = -np.inf
        best_params = None
        
        for i in range(n_iterations):
            # Generate random parameters
            params = {}
            for param_name, (min_val, max_val) in param_space.items():
                params[param_name] = np.random.uniform(min_val, max_val)
            
            try:
                result_metrics = self._run_single_backtest(params)
                value = result_metrics.get(self.objective_function, -np.inf)
                
                if value > best_value:
                    best_value = value
                    best_params = params.copy()
                
                results.append({
                    'iteration': i,
                    'parameters': params,
                    self.objective_function: value
                })
                
            except Exception as e:
                warnings.warn(f"Error in iteration {i}: {e}")
        
        return {
            'best_parameters': best_params or {},
            'best_value': best_value,
            'all_results': results
        }
    
    def _run_single_backtest(self, param_dict: Dict[str, Any]) -> Dict[str, float]:
        """
        Run single backtest with given parameters
        
        Args:
            param_dict: Parameter dictionary
            
        Returns:
            Dictionary with performance metrics
        """
        try:
            # Create strategy with parameters
            strategy = self.strategy_factory(**param_dict)
            
            # Import BacktestEngine (assuming it's available from C++ layer)
            try:
                import volatility_core as vc
                engine = vc.BacktestEngine()
                result = engine.run(strategy, self.data)
                
                # Extract metrics
                metrics = {
                    'total_return': result.get_total_return() if hasattr(result, 'get_total_return') else 0.0,
                    'sharpe_ratio': result.get_sharpe_ratio() if hasattr(result, 'get_sharpe_ratio') else 0.0,
                    'sortino_ratio': result.get_sortino_ratio() if hasattr(result, 'get_sortino_ratio') else 0.0,
                    'max_drawdown': abs(result.get_max_drawdown()) if hasattr(result, 'get_max_drawdown') else 0.0,
                    'volatility': result.get_volatility() if hasattr(result, 'get_volatility') else 0.0
                }
                
                # Calculate Calmar ratio
                if metrics['max_drawdown'] > 0:
                    metrics['calmar_ratio'] = metrics['total_return'] / metrics['max_drawdown']
                else:
                    metrics['calmar_ratio'] = 0.0
                
                return metrics
                
            except ImportError:
                # Fallback to mock results for testing
                warnings.warn("volatility_core not available, using mock results")
                return self._generate_mock_metrics(param_dict)
                
        except Exception as e:
            warnings.warn(f"Error running backtest: {e}")
            return {
                'total_return': -np.inf,
                'sharpe_ratio': -np.inf,
                'sortino_ratio': -np.inf,
                'max_drawdown': np.inf,
                'calmar_ratio': -np.inf,
                'volatility': np.inf
            }
    
    def _generate_mock_metrics(self, param_dict: Dict[str, Any]) -> Dict[str, float]:
        """
        Generate mock performance metrics for testing
        """
        # Create deterministic but realistic metrics based on parameters
        np.random.seed(hash(str(sorted(param_dict.items()))) % 2**31)
        
        # Simulate realistic ranges
        total_return = np.random.normal(0.08, 0.15)  # 8% ± 15%
        volatility = np.random.uniform(0.1, 0.3)     # 10-30% volatility
        max_drawdown = np.random.uniform(0.05, 0.25) # 5-25% drawdown
        
        sharpe_ratio = total_return / volatility if volatility > 0 else 0.0
        sortino_ratio = sharpe_ratio * 1.2  # Typically higher than Sharpe
        calmar_ratio = total_return / max_drawdown if max_drawdown > 0 else 0.0
        
        return {
            'total_return': total_return,
            'sharpe_ratio': sharpe_ratio,
            'sortino_ratio': sortino_ratio,
            'max_drawdown': max_drawdown,
            'calmar_ratio': calmar_ratio,
            'volatility': volatility
        }
    
    def _save_results(self, results_df: pd.DataFrame, filename: str):
        """
        Save optimization results to file
        """
        try:
            # Convert DataFrame to JSON-serializable format
            results_dict = {
                'metadata': {
                    'timestamp': time.strftime('%Y-%m-%d %H:%M:%S'),
                    'objective_function': self.objective_function,
                    'total_combinations': len(results_df)
                },
                'results': []
            }
            
            for _, row in results_df.iterrows():
                result_dict = {}
                for col, val in row.items():
                    if col == 'parameters':
                        result_dict[col] = val
                    elif isinstance(val, (np.integer, np.floating)):
                        result_dict[col] = float(val)
                    elif pd.isna(val):
                        result_dict[col] = None
                    else:
                        result_dict[col] = val
                results_dict['results'].append(result_dict)
            
            with open(filename, 'w') as f:
                json.dump(results_dict, f, indent=2)
                
            print(f"Results saved to {filename}")
            
        except Exception as e:
            warnings.warn(f"Could not save results: {e}")
    
    def analyze_parameter_sensitivity(self, results_df: pd.DataFrame, 
                                    param_name: str) -> Dict[str, Any]:
        """
        Analyze sensitivity of objective function to specific parameter
        
        Args:
            results_df: Results from grid search
            param_name: Parameter to analyze
            
        Returns:
            Dictionary with sensitivity analysis
        """
        try:
            # Extract parameter values
            param_values = []
            objective_values = []
            
            for _, row in results_df.iterrows():
                if param_name in row['parameters']:
                    param_values.append(row['parameters'][param_name])
                    objective_values.append(row[self.objective_function])
            
            if not param_values:
                return {'error': f'Parameter {param_name} not found in results'}
            
            # Calculate correlation
            correlation = np.corrcoef(param_values, objective_values)[0, 1]
            
            # Find optimal range
            sorted_indices = np.argsort(objective_values)
            top_10_percent = int(len(sorted_indices) * 0.9)
            top_param_values = [param_values[i] for i in sorted_indices[top_10_percent:]]
            
            return {
                'parameter': param_name,
                'correlation': correlation,
                'optimal_range': (min(top_param_values), max(top_param_values)),
                'optimal_mean': np.mean(top_param_values),
                'sensitivity': abs(correlation)
            }
            
        except Exception as e:
            return {'error': f'Could not analyze sensitivity: {e}'}
    
    def plot_optimization_results(self, results_df: pd.DataFrame, 
                                param_name: str, figsize: Tuple[int, int] = (10, 6)):
        """
        Plot optimization results for a specific parameter
        
        Args:
            results_df: Results DataFrame
            param_name: Parameter to plot
            figsize: Figure size
        """
        try:
            import matplotlib.pyplot as plt
            
            # Extract data
            param_values = []
            objective_values = []
            
            for _, row in results_df.iterrows():
                if param_name in row['parameters']:
                    param_values.append(row['parameters'][param_name])
                    objective_values.append(row[self.objective_function])
            
            if not param_values:
                print(f"Parameter {param_name} not found in results")
                return
            
            # Create scatter plot
            fig, ax = plt.subplots(figsize=figsize)
            scatter = ax.scatter(param_values, objective_values, alpha=0.6, c=objective_values, 
                               cmap='viridis')
            
            ax.set_xlabel(param_name)
            ax.set_ylabel(self.objective_function)
            ax.set_title(f'Parameter Optimization: {param_name} vs {self.objective_function}')
            ax.grid(True, alpha=0.3)
            
            # Add colorbar
            plt.colorbar(scatter, ax=ax, label=self.objective_function)
            
            # Mark best point
            best_idx = np.argmax(objective_values)
            ax.scatter(param_values[best_idx], objective_values[best_idx], 
                      color='red', s=100, marker='*', label='Best')
            ax.legend()
            
            plt.tight_layout()
            plt.show()
            
        except ImportError:
            print("Matplotlib not available for plotting")
        except Exception as e:
            print(f"Could not create plot: {e}")

class OptimizationUtils:
    """
    Utility functions for parameter optimization
    """
    
    @staticmethod
    def create_param_grid(**kwargs) -> Dict[str, List]:
        """
        Create parameter grid from keyword arguments
        
        Example:
            grid = OptimizationUtils.create_param_grid(
                lookback_window=range(10, 50, 5),
                volatility_threshold=[0.1, 0.2, 0.3]
            )
        """
        return {k: list(v) for k, v in kwargs.items()}
    
    @staticmethod
    def create_param_space(**kwargs) -> Dict[str, Tuple]:
        """
        Create parameter space for Bayesian optimization
        
        Example:
            space = OptimizationUtils.create_param_space(
                lookback_window=(10, 50),
                volatility_threshold=(0.1, 0.5)
            )
        """
        return kwargs
    
    @staticmethod
    def load_optimization_results(filename: str) -> pd.DataFrame:
        """
        Load optimization results from JSON file
        """
        try:
            with open(filename, 'r') as f:
                data = json.load(f)
            
            return pd.DataFrame(data['results'])
            
        except Exception as e:
            warnings.warn(f"Could not load results: {e}")
            return pd.DataFrame()

# Example usage and factory functions
def create_volatility_arbitrage_factory(base_params: Dict[str, Any] = None):
    """
    Create a strategy factory function for volatility arbitrage
    
    Args:
        base_params: Base parameters to use for all strategies
        
    Returns:
        Factory function that creates strategies with given parameters
    """
    base_params = base_params or {}
    
    def factory(**params):
        """Strategy factory function"""
        try:
            import volatility_core as vc
            
            # Merge base params with optimization params
            full_params = {**base_params, **params}
            
            # Create strategy components
            vol_estimator = vc.VolatilityEstimator()
            signal_generator = vc.SignalGenerator()
            
            # Create strategy with parameters
            strategy = vc.VolatilityArbitrageStrategy(
                vol_estimator, signal_generator,
                full_params.get('lookback_window', 30),
                full_params.get('volatility_threshold', 0.2),
                full_params.get('hedge_ratio', 0.8)
            )
            
            return strategy
            
        except ImportError:
            # Mock strategy for testing
            class MockStrategy:
                def __init__(self, **params):
                    self.params = params
                    
            return MockStrategy(**params)
    
    return factory
