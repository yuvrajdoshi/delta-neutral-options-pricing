#!/usr/bin/env python3
"""
Volatility Arbitrage Trading System - Main Entry Point

This is the main execution file for the volatility arbitrage trading system.
It provides comprehensive functionality for data management, strategy execution,
and analysis across different market regimes.

Author: Yuvraj Doshi
Email: doshiyuvraj@gmail.com
Version: 1.0.0
"""

import sys
import os
import argparse
import warnings
from datetime import datetime, timedelta
from pathlib import Path
from typing import Dict, List, Optional, Tuple
import yfinance as yf
import pandas as pd
import numpy as np

# Add project root to Python path
project_root = Path(__file__).parent
sys.path.insert(0, str(project_root))
sys.path.insert(0, str(project_root / "python" / "strategy"))

warnings.filterwarnings('ignore')

# Import strategy components
try:
    from python.strategy.market_data import MarketDataManager
    from python.strategy.real_world_strategy import RealWorldVolatilityArbitrageStrategy, create_default_config
    from python.strategy.backtest_engine import RealWorldBacktestEngine, BacktestResult
    from python.strategy.multi_regime_backtester import MultiRegimeBacktester
    from python.strategy.results_summary import create_executive_summary
except ImportError as e:
    print(f"❌ Failed to import strategy components: {e}")
    print("Please ensure you're running from the project root directory")
    sys.exit(1)


class VolatilityTradingSystem:
    """
    Main class for the Volatility Arbitrage Trading System
    
    This system supports multiple securities and provides comprehensive
    functionality for data management, backtesting, and live analysis.
    """
    
    # Supported securities with their characteristics
    SUPPORTED_SECURITIES = {
        'SPY': {
            'name': 'SPDR S&P 500 ETF',
            'type': 'ETF',
            'description': 'Tracks S&P 500 index - high liquidity, moderate volatility',
            'options_available': True,
            'typical_iv_range': (0.10, 0.40),
            'recommended': True
        },
        'QQQ': {
            'name': 'Invesco QQQ Trust',
            'type': 'ETF', 
            'description': 'Tracks NASDAQ-100 - tech heavy, higher volatility',
            'options_available': True,
            'typical_iv_range': (0.15, 0.50),
            'recommended': True
        },
        'IWM': {
            'name': 'iShares Russell 2000 ETF',
            'type': 'ETF',
            'description': 'Small-cap exposure - higher volatility than SPY',
            'options_available': True,
            'typical_iv_range': (0.15, 0.55),
            'recommended': True
        },
        'AAPL': {
            'name': 'Apple Inc.',
            'type': 'Stock',
            'description': 'Large-cap tech stock - high liquidity, event-driven volatility',
            'options_available': True,
            'typical_iv_range': (0.20, 0.80),
            'recommended': False
        },
        'TSLA': {
            'name': 'Tesla Inc.',
            'type': 'Stock',
            'description': 'High volatility stock - extreme price movements',
            'options_available': True,
            'typical_iv_range': (0.30, 1.20),
            'recommended': False
        },
        'NVDA': {
            'name': 'NVIDIA Corporation',
            'type': 'Stock',
            'description': 'AI/GPU leader - high volatility, strong trends',
            'options_available': True,
            'typical_iv_range': (0.25, 0.90),
            'recommended': False
        }
    }
    
    def __init__(self):
        """Initialize the trading system"""
        self.data_manager = MarketDataManager()
        self.current_security = 'SPY'  # Default security
        self.data_directory = project_root / "data"
        self.results_directory = project_root / "python" / "strategy" / "backtest_results"
        
        # Ensure directories exist
        self.data_directory.mkdir(exist_ok=True)
        self.results_directory.mkdir(exist_ok=True)
        
        print("🚀 Volatility Arbitrage Trading System Initialized")
        print(f"📂 Data directory: {self.data_directory}")
        print(f"📊 Results directory: {self.results_directory}")
    
    def display_security_info(self) -> None:
        """Display information about supported securities"""
        print("\n" + "="*80)
        print("📈 SUPPORTED SECURITIES FOR VOLATILITY ARBITRAGE")
        print("="*80)
        
        print("\n🎯 RECOMMENDED SECURITIES (ETFs - Better for Vol Arbitrage):")
        for symbol, info in self.SUPPORTED_SECURITIES.items():
            if info['recommended']:
                iv_low, iv_high = info['typical_iv_range']
                print(f"  • {symbol:4} - {info['name']}")
                print(f"    {info['description']}")
                print(f"    Typical IV Range: {iv_low:.0%} - {iv_high:.0%}")
                print()
        
        print("⚠️  ALTERNATIVE SECURITIES (Individual Stocks - Higher Risk):")
        for symbol, info in self.SUPPORTED_SECURITIES.items():
            if not info['recommended']:
                iv_low, iv_high = info['typical_iv_range']
                print(f"  • {symbol:4} - {info['name']}")
                print(f"    {info['description']}")
                print(f"    Typical IV Range: {iv_low:.0%} - {iv_high:.0%}")
                print()
        
        print("💡 CURRENT SYSTEM CONFIGURATION:")
        print(f"  • Primary Security: {self.current_security}")
        security_info = self.SUPPORTED_SECURITIES[self.current_security]
        print(f"  • Name: {security_info['name']}")
        print(f"  • Type: {security_info['type']}")
        print(f"  • Description: {security_info['description']}")
        
        if security_info['recommended']:
            print(f"  • Status: ✅ Recommended for volatility arbitrage")
        else:
            print(f"  • Status: ⚠️  Alternative security (higher risk)")
    
    def download_and_save_data(self, 
                               symbol: str, 
                               start_date: str, 
                               end_date: str,
                               save_to_csv: bool = True) -> pd.DataFrame:
        """
        Download data from Yahoo Finance and optionally save to CSV
        
        Args:
            symbol: Security symbol (e.g., 'SPY', 'QQQ', 'AAPL')
            start_date: Start date in 'YYYY-MM-DD' format
            end_date: End date in 'YYYY-MM-DD' format  
            save_to_csv: Whether to save data to CSV file
            
        Returns:
            DataFrame with comprehensive market data
        """
        print(f"\n🌐 Downloading data for {symbol} from Yahoo Finance...")
        print(f"📅 Period: {start_date} to {end_date}")
        
        if symbol not in self.SUPPORTED_SECURITIES:
            print(f"⚠️  Warning: {symbol} is not in our supported securities list")
            print("   This may affect strategy performance and reliability")
        
        try:
            # Get comprehensive dataset using our data manager
            dataset = self.data_manager.get_comprehensive_dataset(
                symbol, start_date, end_date
            )
            
            if dataset.empty:
                raise ValueError(f"No data retrieved for {symbol}")
            
            print(f"✅ Downloaded {len(dataset)} trading days")
            print(f"📊 Data columns: {list(dataset.columns)}")
            print(f"📈 Price range: ${dataset['Close'].min():.2f} - ${dataset['Close'].max():.2f}")
            
            # Display data quality metrics
            self._display_data_quality(dataset, symbol)
            
            if save_to_csv:
                # Save to CSV in data directory
                csv_filename = f"{symbol}_{start_date}_{end_date}_comprehensive.csv"
                csv_path = self.data_directory / csv_filename
                dataset.to_csv(csv_path)
                print(f"💾 Data saved to: {csv_path}")
            
            return dataset
            
        except Exception as e:
            print(f"❌ Failed to download data for {symbol}: {e}")
            return pd.DataFrame()
    
    def _display_data_quality(self, data: pd.DataFrame, symbol: str) -> None:
        """Display data quality metrics"""
        print(f"\n📋 DATA QUALITY REPORT for {symbol}:")
        
        # Basic statistics
        print(f"  • Trading days: {len(data)}")
        print(f"  • Date range: {data.index[0].strftime('%Y-%m-%d')} to {data.index[-1].strftime('%Y-%m-%d')}")
        
        # Price statistics
        if 'Close' in data.columns:
            returns = data['Close'].pct_change().dropna()
            vol = returns.std() * np.sqrt(252)  # Annualized volatility
            print(f"  • Annualized volatility: {vol:.1%}")
            print(f"  • Max daily return: {returns.max():.2%}")
            print(f"  • Min daily return: {returns.min():.2%}")
        
        # VIX statistics if available
        if 'VIX' in data.columns:
            vix_mean = data['VIX'].mean()
            vix_range = (data['VIX'].min(), data['VIX'].max())
            print(f"  • Average VIX: {vix_mean:.1f}")
            print(f"  • VIX range: {vix_range[0]:.1f} - {vix_range[1]:.1f}")
        
        # Missing data check
        missing_data = data.isnull().sum()
        if missing_data.sum() > 0:
            print(f"  • Missing data points: {missing_data.sum()}")
            for col, missing in missing_data.items():
                if missing > 0:
                    print(f"    - {col}: {missing} missing values")
        else:
            print(f"  • Data completeness: ✅ No missing values")
    
    def run_single_backtest(self, 
                           symbol: str,
                           start_date: str, 
                           end_date: str,
                           download_fresh: bool = False) -> None:
        """
        Run a single backtest on specified security and date range
        
        Args:
            symbol: Security symbol
            start_date: Start date for backtest
            end_date: End date for backtest
            download_fresh: Whether to download fresh data
        """
        print(f"\n🎯 SINGLE BACKTEST: {symbol}")
        print("="*50)
        
        # Download/load data
        if download_fresh:
            dataset = self.download_and_save_data(symbol, start_date, end_date)
        else:
            dataset = self.data_manager.get_comprehensive_dataset(symbol, start_date, end_date)
        
        if dataset.empty:
            print("❌ No data available for backtest")
            return
        
        # Run backtest
        print(f"\n📈 Running backtest from {start_date} to {end_date}...")
        
        config = create_default_config()
        engine = RealWorldBacktestEngine(config)
        
        result = engine.run_backtest(
            start_date=start_date,
            end_date=end_date,
            symbol=symbol
        )
        
        # Display results
        self._display_backtest_results(result, symbol, start_date, end_date)
    
    def run_multi_regime_analysis(self, 
                                 symbol: str = None,
                                 generate_images: bool = True) -> None:
        """
        Run comprehensive multi-regime backtesting analysis
        
        Args:
            symbol: Security symbol (defaults to current security)
            generate_images: Whether to generate visualization images
        """
        if symbol is None:
            symbol = self.current_security
            
        print(f"\n🎯 MULTI-REGIME ANALYSIS: {symbol}")
        print("="*60)
        
        try:
            # Initialize multi-regime backtester
            backtester = MultiRegimeBacktester(
                output_dir=str(self.results_directory),
                symbol=symbol
            )
            
            # Run comprehensive analysis
            print(f"🚀 Starting comprehensive multi-regime backtesting...")
            results = backtester.run_comprehensive_backtest()
            
            # Generate visualizations if requested
            if generate_images:
                print(f"\n📊 Generating visualizations...")
                backtester.generate_all_visualizations()
            
            # Generate executive summary
            if generate_images:
                print(f"\n📊 Generating executive summary and visualizations...")
                create_executive_summary()
            
            print(f"\n✅ Multi-regime analysis complete!")
            print(f"📂 Results saved in: {self.results_directory}")
            
            # Display key findings
            self._display_regime_summary()
            
        except Exception as e:
            print(f"❌ Multi-regime analysis failed: {e}")
            import traceback
            traceback.print_exc()
    
    def _display_backtest_results(self, result, symbol: str, start_date: str, end_date: str) -> None:
        """Display single backtest results"""
        if not hasattr(result, 'final_portfolio_value'):
            print("❌ Backtest failed to generate results")
            return
            
        print(f"\n📊 BACKTEST RESULTS for {symbol}")
        print("="*40)
        
        total_return = (result.final_portfolio_value - 100000) / 100000 * 100
        print(f"💰 Total Return: {total_return:.2f}%")
        print(f"📈 Final Portfolio Value: ${result.final_portfolio_value:,.2f}")
        
        if hasattr(result, 'trades') and result.trades:
            print(f"🔄 Total Trades: {len(result.trades)}")
            
        if hasattr(result, 'sharpe_ratio'):
            print(f"📊 Sharpe Ratio: {result.sharpe_ratio:.2f}")
            
        if hasattr(result, 'max_drawdown'):
            print(f"📉 Max Drawdown: {result.max_drawdown:.2%}")
    
    def _display_regime_summary(self) -> None:
        """Display summary of multi-regime analysis"""
        try:
            # Read the analysis report
            report_path = self.results_directory / "MULTI_REGIME_ANALYSIS_REPORT.md"
            if report_path.exists():
                print(f"\n📋 MULTI-REGIME ANALYSIS SUMMARY")
                print("="*50)
                print(f"📄 Full report available at: {report_path}")
                print(f"🖼️  Visualizations available in: {self.results_directory / 'images'}")
                
                # Try to extract key metrics from report
                with open(report_path, 'r') as f:
                    content = f.read()
                    if "profitable regimes" in content.lower():
                        lines = content.split('\n')
                        for line in lines:
                            if any(keyword in line.lower() for keyword in 
                                  ['profitable', 'sharpe', 'drawdown', 'return']):
                                if line.strip() and not line.startswith('#'):
                                    print(f"  • {line.strip()}")
        except Exception as e:
            print(f"⚠️  Could not load summary: {e}")
    
    def set_security(self, symbol: str) -> None:
        """
        Change the current security for analysis
        
        Args:
            symbol: New security symbol
        """
        if symbol not in self.SUPPORTED_SECURITIES:
            print(f"⚠️  Warning: {symbol} is not in our supported securities list")
            response = input(f"Continue with {symbol}? (y/n): ")
            if response.lower() != 'y':
                return
        
        self.current_security = symbol
        print(f"✅ Current security changed to: {symbol}")
        
        if symbol in self.SUPPORTED_SECURITIES:
            info = self.SUPPORTED_SECURITIES[symbol]
            print(f"📋 {info['name']} - {info['description']}")
    
    def interactive_menu(self) -> None:
        """Interactive menu for system operations"""
        while True:
            print(f"\n" + "="*80)
            print("🎯 VOLATILITY ARBITRAGE TRADING SYSTEM - MAIN MENU")
            print("="*80)
            print(f"Current Security: {self.current_security} ({self.SUPPORTED_SECURITIES.get(self.current_security, {}).get('name', 'Unknown')})")
            print("\nOptions:")
            print("1. 📈 Display Supported Securities")
            print("2. 🌐 Download Fresh Data")
            print("3. 🎯 Run Single Backtest")
            print("4. 📊 Run Multi-Regime Analysis")
            print("5. 🔧 Change Security")
            print("6. 📋 View Recent Results")
            print("7. ❌ Exit")
            
            choice = input("\nEnter your choice (1-7): ").strip()
            
            if choice == '1':
                self.display_security_info()
            
            elif choice == '2':
                symbol = input(f"Enter symbol (default: {self.current_security}): ").strip().upper()
                if not symbol:
                    symbol = self.current_security
                
                # Default to last 2 years
                end_date = datetime.now()
                start_date = end_date - timedelta(days=730)
                
                start_str = input(f"Start date (YYYY-MM-DD, default: {start_date.strftime('%Y-%m-%d')}): ").strip()
                if start_str:
                    start_date = datetime.strptime(start_str, '%Y-%m-%d')
                
                end_str = input(f"End date (YYYY-MM-DD, default: {end_date.strftime('%Y-%m-%d')}): ").strip()
                if end_str:
                    end_date = datetime.strptime(end_str, '%Y-%m-%d')
                
                self.download_and_save_data(
                    symbol, 
                    start_date.strftime('%Y-%m-%d'), 
                    end_date.strftime('%Y-%m-%d')
                )
            
            elif choice == '3':
                symbol = input(f"Enter symbol (default: {self.current_security}): ").strip().upper()
                if not symbol:
                    symbol = self.current_security
                
                start_str = input("Start date (YYYY-MM-DD): ").strip()
                end_str = input("End date (YYYY-MM-DD): ").strip()
                
                if start_str and end_str:
                    download_fresh = input("Download fresh data? (y/n, default: n): ").strip().lower() == 'y'
                    self.run_single_backtest(symbol, start_str, end_str, download_fresh)
                else:
                    print("❌ Please provide both start and end dates")
            
            elif choice == '4':
                symbol = input(f"Enter symbol (default: {self.current_security}): ").strip().upper()
                if not symbol:
                    symbol = self.current_security
                
                generate_imgs = input("Generate visualization images? (y/n, default: y): ").strip().lower()
                generate_imgs = generate_imgs != 'n'
                
                self.run_multi_regime_analysis(symbol, generate_imgs)
            
            elif choice == '5':
                new_symbol = input("Enter new security symbol: ").strip().upper()
                if new_symbol:
                    self.set_security(new_symbol)
            
            elif choice == '6':
                self._view_recent_results()
            
            elif choice == '7':
                print("👋 Goodbye! Thank you for using the Volatility Arbitrage Trading System")
                break
            
            else:
                print("❌ Invalid choice. Please enter 1-7.")
    
    def _view_recent_results(self) -> None:
        """Display recent analysis results"""
        print(f"\n📂 RECENT RESULTS")
        print("="*40)
        
        # Check for recent files
        if self.results_directory.exists():
            files = list(self.results_directory.glob("*.md"))
            files.extend(list(self.results_directory.glob("*.csv")))
            
            if files:
                print("Recent analysis files:")
                for file in sorted(files, key=lambda x: x.stat().st_mtime, reverse=True)[:10]:
                    mod_time = datetime.fromtimestamp(file.stat().st_mtime)
                    print(f"  📄 {file.name} (Modified: {mod_time.strftime('%Y-%m-%d %H:%M')})")
                
                # Check for images
                images_dir = self.results_directory / "images"
                if images_dir.exists():
                    image_files = list(images_dir.glob("*.png"))
                    if image_files:
                        print(f"\n🖼️  Visualization images ({len(image_files)} files):")
                        for img in sorted(image_files)[:5]:
                            print(f"  🖼️  {img.name}")
                        if len(image_files) > 5:
                            print(f"  ... and {len(image_files) - 5} more images")
            else:
                print("No recent results found. Run an analysis to generate results.")
        else:
            print("Results directory not found.")


def main():
    """Main entry point with command line argument support"""
    parser = argparse.ArgumentParser(
        description="Volatility Arbitrage Trading System",
        formatter_class=argparse.RawDescriptionHelpFormatter,
        epilog="""
Examples:
  python main.py                                    # Interactive menu
  python main.py --info                            # Show supported securities
  python main.py --download SPY --start 2023-01-01 # Download SPY data
  python main.py --backtest SPY --start 2023-01-01 --end 2024-01-01
  python main.py --multi-regime SPY               # Full multi-regime analysis
        """
    )
    
    parser.add_argument('--info', action='store_true', 
                       help='Display supported securities information')
    parser.add_argument('--download', metavar='SYMBOL',
                       help='Download data for specified symbol')
    parser.add_argument('--backtest', metavar='SYMBOL',
                       help='Run single backtest for specified symbol')
    parser.add_argument('--multi-regime', metavar='SYMBOL',
                       help='Run multi-regime analysis for specified symbol')
    parser.add_argument('--start', metavar='YYYY-MM-DD',
                       help='Start date for analysis')
    parser.add_argument('--end', metavar='YYYY-MM-DD',
                       help='End date for analysis')
    parser.add_argument('--no-images', action='store_true',
                       help='Skip generating visualization images')
    
    args = parser.parse_args()
    
    # Initialize system
    system = VolatilityTradingSystem()
    
    # Handle command line arguments
    if args.info:
        system.display_security_info()
        return
    
    if args.download:
        start_date = args.start or (datetime.now() - timedelta(days=730)).strftime('%Y-%m-%d')
        end_date = args.end or datetime.now().strftime('%Y-%m-%d')
        system.download_and_save_data(args.download, start_date, end_date)
        return
    
    if args.backtest:
        if not args.start or not args.end:
            print("❌ Both --start and --end dates required for backtesting")
            return
        system.run_single_backtest(args.backtest, args.start, args.end, download_fresh=True)
        return
    
    if args.multi_regime:
        generate_images = not args.no_images
        system.run_multi_regime_analysis(args.multi_regime, generate_images)
        return
    
    # If no command line arguments, start interactive menu
    if len(sys.argv) == 1:
        system.interactive_menu()
    else:
        parser.print_help()


if __name__ == "__main__":
    main()
