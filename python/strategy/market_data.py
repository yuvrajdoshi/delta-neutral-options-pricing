"""
Real-World Market Data Manager - Layer 8 Component 1
Downloads and manages real market data for volatility arbitrage strategy
"""

import pandas as pd
import numpy as np
import yfinance as yf
import requests
import warnings
from datetime import datetime, timedelta
from typing import Dict, List, Optional, Tuple
import json
import os
from pathlib import Path

class MarketDataManager:
    """
    Manages real market data for volatility arbitrage trading
    Handles equity data, options data, and VIX data
    """
    
    def __init__(self, cache_dir: str = "data_cache"):
        """
        Initialize market data manager
        
        Args:
            cache_dir: Directory for caching downloaded data
        """
        self.cache_dir = Path(cache_dir)
        self.cache_dir.mkdir(exist_ok=True)
        
        # Data sources configuration
        self.equity_symbols = ['SPY', 'QQQ', 'IWM']  # Major ETFs
        self.vix_symbol = '^VIX'
        self.treasury_symbol = '^TNX'  # 10-year treasury
        
        print(f"📊 MarketDataManager initialized with cache: {self.cache_dir}")
    
    def download_equity_data(self, symbol: str, start_date: str, end_date: str, 
                           force_refresh: bool = False) -> pd.DataFrame:
        """
        Download equity price data with returns calculation
        
        Args:
            symbol: Stock/ETF symbol (e.g., 'SPY')
            start_date: Start date in YYYY-MM-DD format
            end_date: End date in YYYY-MM-DD format
            force_refresh: Force download even if cached
            
        Returns:
            DataFrame with OHLCV data and calculated returns
        """
        cache_file = self.cache_dir / f"{symbol}_{start_date}_{end_date}_equity.csv"
        
        # Try to load from cache first
        if cache_file.exists() and not force_refresh:
            print(f"📁 Loading {symbol} equity data from cache...")
            try:
                df = pd.read_csv(cache_file, index_col=0, parse_dates=True)
                # Ensure timezone-naive index
                if hasattr(df.index, 'tz') and df.index.tz is not None:
                    df.index = df.index.tz_localize(None)
                return df
            except Exception as e:
                print(f"⚠️  Cache read failed: {e}, downloading fresh data...")
        
        print(f"🌐 Downloading {symbol} equity data from {start_date} to {end_date}...")
        
        try:
            # Download data using yfinance
            ticker = yf.Ticker(symbol)
            df = ticker.history(start=start_date, end=end_date, auto_adjust=True)
            
            if df.empty:
                raise ValueError(f"No data downloaded for {symbol}")
            
            # Ensure timezone-naive index for consistency
            if df.index.tz is not None:
                df.index = df.index.tz_localize(None)
            
            # Calculate returns and additional features
            df = self._calculate_equity_features(df)
            
            # Cache the data
            df.to_csv(cache_file)
            print(f"✅ Downloaded {len(df)} days of {symbol} data")
            
            return df
            
        except Exception as e:
            print(f"❌ Failed to download {symbol} data: {e}")
            # Return empty DataFrame with expected columns
            return pd.DataFrame(columns=['Open', 'High', 'Low', 'Close', 'Volume', 
                                       'Returns', 'LogReturns', 'RealizedVol'])
    
    def _calculate_equity_features(self, df: pd.DataFrame) -> pd.DataFrame:
        """Calculate additional features from price data"""
        # Simple returns
        df['Returns'] = df['Close'].pct_change()
        
        # Log returns
        df['LogReturns'] = np.log(df['Close'] / df['Close'].shift(1))
        
        # Realized volatility (20-day rolling)
        df['RealizedVol'] = df['Returns'].rolling(window=20).std() * np.sqrt(252)
        
        # High-low volatility estimator (Parkinson)
        df['ParkinsonVol'] = np.sqrt(
            (1 / (4 * np.log(2))) * 
            (np.log(df['High'] / df['Low']) ** 2).rolling(window=20).mean() * 252
        )
        
        # Garman-Klass volatility estimator
        hl = np.log(df['High'] / df['Low'])
        co = np.log(df['Close'] / df['Open'])
        df['GarmanKlassVol'] = np.sqrt(
            (0.5 * hl**2 - (2*np.log(2) - 1) * co**2).rolling(window=20).mean() * 252
        )
        
        # Price momentum indicators
        df['SMA_20'] = df['Close'].rolling(window=20).mean()
        df['SMA_50'] = df['Close'].rolling(window=50).mean()
        df['RSI'] = self._calculate_rsi(df['Close'])
        
        return df
    
    def _calculate_rsi(self, prices: pd.Series, window: int = 14) -> pd.Series:
        """Calculate Relative Strength Index"""
        delta = prices.diff()
        gain = (delta.where(delta > 0, 0)).rolling(window=window).mean()
        loss = (-delta.where(delta < 0, 0)).rolling(window=window).mean()
        rs = gain / loss
        return 100 - (100 / (1 + rs))
    
    def download_vix_data(self, start_date: str, end_date: str, 
                         force_refresh: bool = False) -> pd.DataFrame:
        """
        Download VIX (volatility index) data
        
        Args:
            start_date: Start date in YYYY-MM-DD format
            end_date: End date in YYYY-MM-DD format
            force_refresh: Force download even if cached
            
        Returns:
            DataFrame with VIX data and features
        """
        cache_file = self.cache_dir / f"VIX_{start_date}_{end_date}.csv"
        
        if cache_file.exists() and not force_refresh:
            print(f"📁 Loading VIX data from cache...")
            try:
                df = pd.read_csv(cache_file, index_col=0, parse_dates=True)
                # Ensure timezone-naive index
                if hasattr(df.index, 'tz') and df.index.tz is not None:
                    df.index = df.index.tz_localize(None)
                return df
            except Exception as e:
                print(f"⚠️  VIX cache read failed: {e}")
        
        print(f"🌐 Downloading VIX data from {start_date} to {end_date}...")
        
        try:
            ticker = yf.Ticker(self.vix_symbol)
            df = ticker.history(start=start_date, end=end_date)
            
            if df.empty:
                raise ValueError("No VIX data downloaded")
            
            # Ensure timezone-naive index for consistency
            if df.index.tz is not None:
                df.index = df.index.tz_localize(None)
            
            # Calculate VIX features
            df['VIX'] = df['Close']
            df['VIX_SMA_10'] = df['VIX'].rolling(window=10).mean()
            df['VIX_SMA_30'] = df['VIX'].rolling(window=30).mean()
            df['VIX_Change'] = df['VIX'].pct_change()
            
            # VIX regime indicators
            df['VIX_Low'] = df['VIX'] < 15  # Low volatility regime
            df['VIX_High'] = df['VIX'] > 25  # High volatility regime
            df['VIX_Spike'] = df['VIX_Change'] > 0.1  # VIX spike (>10% increase)
            
            # Cache the data
            df.to_csv(cache_file)
            print(f"✅ Downloaded {len(df)} days of VIX data")
            
            return df
            
        except Exception as e:
            print(f"❌ Failed to download VIX data: {e}")
            return pd.DataFrame(columns=['Open', 'High', 'Low', 'Close', 'Volume', 
                                       'VIX', 'VIX_Change', 'VIX_Low', 'VIX_High'])
    
    def download_treasury_rates(self, start_date: str, end_date: str,
                               force_refresh: bool = False) -> pd.DataFrame:
        """
        Download 10-year treasury rates for risk-free rate
        """
        cache_file = self.cache_dir / f"TNX_{start_date}_{end_date}.csv"
        
        if cache_file.exists() and not force_refresh:
            print(f"📁 Loading Treasury rates from cache...")
            try:
                df = pd.read_csv(cache_file, index_col=0, parse_dates=True)
                # Ensure timezone-naive index
                if hasattr(df.index, 'tz') and df.index.tz is not None:
                    df.index = df.index.tz_localize(None)
                return df
            except Exception as e:
                print(f"⚠️  Treasury cache read failed: {e}")
        
        print(f"🌐 Downloading Treasury rates from {start_date} to {end_date}...")
        
        try:
            ticker = yf.Ticker(self.treasury_symbol)
            df = ticker.history(start=start_date, end=end_date)
            
            if df.empty:
                print("⚠️  No Treasury data, using default 2% risk-free rate")
                # Create default data
                date_range = pd.date_range(start=start_date, end=end_date, freq='D')
                df = pd.DataFrame(index=date_range)
                df['Close'] = 2.0  # Default 2% rate
            
            # Ensure timezone-naive index for consistency
            if df.index.tz is not None:
                df.index = df.index.tz_localize(None)
            
            # Convert percentage to decimal for calculations
            df['RiskFreeRate'] = df['Close'] / 100
            df['RiskFreeRateDaily'] = df['RiskFreeRate'] / 365
            
            # Cache the data
            df.to_csv(cache_file)
            print(f"✅ Downloaded {len(df)} days of Treasury data")
            
            return df
            
        except Exception as e:
            print(f"❌ Failed to download Treasury data: {e}")
            # Return default data
            date_range = pd.date_range(start=start_date, end=end_date, freq='D')
            df = pd.DataFrame(index=date_range)
            df['RiskFreeRate'] = 0.02
            df['RiskFreeRateDaily'] = 0.02 / 365
            return df
    
    def get_comprehensive_dataset(self, symbol: str = 'SPY', 
                                start_date: str = '2020-01-01',
                                end_date: str = '2024-08-01',
                                force_refresh: bool = False) -> pd.DataFrame:
        """
        Download comprehensive dataset combining equity, VIX, and Treasury data
        
        Args:
            symbol: Primary equity symbol
            start_date: Start date
            end_date: End date
            force_refresh: Force refresh all data
            
        Returns:
            Combined DataFrame with all market data
        """
        print(f"\n🚀 Downloading comprehensive dataset for {symbol}...")
        print(f"📅 Date range: {start_date} to {end_date}")
        
        # Download all data sources
        equity_data = self.download_equity_data(symbol, start_date, end_date, force_refresh)
        vix_data = self.download_vix_data(start_date, end_date, force_refresh)
        treasury_data = self.download_treasury_rates(start_date, end_date, force_refresh)
        
        if equity_data.empty:
            print(f"❌ No equity data available for {symbol}")
            return pd.DataFrame()
        
        # Combine all data sources
        print("🔗 Combining data sources...")
        
        # Start with equity data as base
        combined = equity_data.copy()
        
        # Add VIX data
        if not vix_data.empty:
            vix_cols = ['VIX', 'VIX_Change', 'VIX_Low', 'VIX_High', 'VIX_Spike']
            for col in vix_cols:
                if col in vix_data.columns:
                    combined[col] = vix_data[col]
                else:
                    combined[col] = np.nan
        else:
            combined['VIX'] = 20.0  # Default VIX level
            combined['VIX_Change'] = 0.0
            combined['VIX_Low'] = False
            combined['VIX_High'] = False
            combined['VIX_Spike'] = False
        
        # Add Treasury data
        if not treasury_data.empty:
            treasury_cols = ['RiskFreeRate', 'RiskFreeRateDaily']
            for col in treasury_cols:
                if col in treasury_data.columns:
                    combined[col] = treasury_data[col]
                else:
                    combined[col] = 0.02  # Default 2%
        else:
            combined['RiskFreeRate'] = 0.02
            combined['RiskFreeRateDaily'] = 0.02 / 365
        
        # Fill missing values with forward fill then backward fill
        combined = combined.ffill().bfill()
        
        # Remove weekends and holidays (keep only valid trading days)
        combined = combined[combined['Volume'] > 0]
        
        # Add additional market regime indicators
        combined = self._add_market_regime_features(combined)
        
        print(f"✅ Combined dataset ready: {len(combined)} trading days")
        print(f"📊 Columns: {list(combined.columns)}")
        
        return combined
    
    def _add_market_regime_features(self, df: pd.DataFrame) -> pd.DataFrame:
        """Add market regime and feature engineering"""
        # Volatility regimes based on realized vol
        vol_20 = df['RealizedVol'].rolling(window=60).quantile(0.2)
        vol_80 = df['RealizedVol'].rolling(window=60).quantile(0.8)
        
        df['LowVolRegime'] = df['RealizedVol'] < vol_20
        df['HighVolRegime'] = df['RealizedVol'] > vol_80
        
        # Market trend indicators
        df['Bullish'] = (df['Close'] > df['SMA_20']) & (df['SMA_20'] > df['SMA_50'])
        df['Bearish'] = (df['Close'] < df['SMA_20']) & (df['SMA_20'] < df['SMA_50'])
        
        # Volatility spread (realized vs VIX)
        if 'VIX' in df.columns:
            df['VolSpread'] = df['RealizedVol'] - (df['VIX'] / 100)
            df['VolSpreadPercentile'] = df['VolSpread'].rolling(window=252).rank(pct=True)
        
        # Mean reversion indicators
        df['PriceZScore'] = (
            (df['Close'] - df['Close'].rolling(window=252).mean()) / 
            df['Close'].rolling(window=252).std()
        )
        
        # Earnings season indicator (simplified - first month of each quarter)
        try:
            # Ensure index is datetime and handle timezone issues
            if not isinstance(df.index, pd.DatetimeIndex):
                df.index = pd.to_datetime(df.index)
            
            # Convert to timezone-naive if timezone-aware
            if df.index.tz is not None:
                df.index = df.index.tz_localize(None)
            
            df['EarningsSeason'] = df.index.month.isin([1, 4, 7, 10])
        except (AttributeError, TypeError, ValueError) as e:
            # Fallback: create a simple quarterly pattern
            print(f"⚠️  Could not calculate earnings season: {e}, using fallback")
            df['EarningsSeason'] = False  # Default to False
        
        return df
    
    def create_training_test_split(self, df: pd.DataFrame, 
                                 test_start_date: str = '2023-01-01') -> Tuple[pd.DataFrame, pd.DataFrame]:
        """
        Split data into training and test sets
        
        Args:
            df: Combined market data
            test_start_date: Start date for test set
            
        Returns:
            Tuple of (training_data, test_data)
        """
        test_start = pd.to_datetime(test_start_date)
        
        train_data = df[df.index < test_start].copy()
        test_data = df[df.index >= test_start].copy()
        
        print(f"📊 Data split:")
        print(f"   Training: {train_data.index[0].date()} to {train_data.index[-1].date()} ({len(train_data)} days)")
        print(f"   Test:     {test_data.index[0].date()} to {test_data.index[-1].date()} ({len(test_data)} days)")
        
        return train_data, test_data
    
    def export_for_cpp(self, df: pd.DataFrame, filename: str = 'market_data.csv'):
        """
        Export data in format suitable for C++ backtesting engine
        
        Args:
            df: Market data DataFrame  
            filename: Output filename
        """
        export_path = self.cache_dir / filename
        
        # Select essential columns for C++ engine
        cpp_columns = [
            'Open', 'High', 'Low', 'Close', 'Volume',
            'Returns', 'LogReturns', 'RealizedVol',
            'VIX', 'RiskFreeRate'
        ]
        
        # Ensure all required columns exist
        for col in cpp_columns:
            if col not in df.columns:
                if col == 'VIX':
                    df[col] = 20.0  # Default VIX
                elif col == 'RiskFreeRate':
                    df[col] = 0.02  # Default risk-free rate
                else:
                    df[col] = 0.0
        
        # Export with proper formatting
        export_df = df[cpp_columns].copy()
        export_df.to_csv(export_path, float_format='%.6f')
        
        print(f"💾 Exported data for C++ engine: {export_path}")
        print(f"📊 Shape: {export_df.shape}")
        
        return export_path

def create_sample_dataset():
    """Create a sample dataset for testing"""
    print("🚀 Creating sample market dataset...")
    
    manager = MarketDataManager()
    
    # Download comprehensive SPY data
    dataset = manager.get_comprehensive_dataset(
        symbol='SPY',
        start_date='2020-01-01', 
        end_date='2024-08-01',
        force_refresh=False
    )
    
    if dataset.empty:
        print("❌ Failed to create dataset")
        return None
    
    # Create train/test split
    train_data, test_data = manager.create_training_test_split(dataset)
    
    # Export for C++ engine
    manager.export_for_cpp(dataset, 'spy_market_data.csv')
    
    # Show some statistics
    print(f"\n📈 Dataset Statistics:")
    print(f"Date range: {dataset.index[0].date()} to {dataset.index[-1].date()}")
    print(f"Total trading days: {len(dataset)}")
    print(f"Average daily return: {dataset['Returns'].mean():.4f}")
    print(f"Daily return volatility: {dataset['Returns'].std():.4f}")
    print(f"Annualized return: {dataset['Returns'].mean() * 252:.2%}")
    print(f"Annualized volatility: {dataset['Returns'].std() * np.sqrt(252):.2%}")
    print(f"Average VIX level: {dataset['VIX'].mean():.2f}")
    print(f"VIX range: {dataset['VIX'].min():.2f} - {dataset['VIX'].max():.2f}")
    
    return dataset

if __name__ == '__main__':
    # Create sample dataset for strategy testing
    dataset = create_sample_dataset()
