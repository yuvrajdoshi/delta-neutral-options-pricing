"""Time series analysis and data handling module."""

import pandas as pd
import numpy as np
from typing import List, Optional, Union, Dict, Any
from datetime import datetime
import volatility_arbitrage as va

class TimeSeriesWrapper:
    """Enhanced Python wrapper for C++ TimeSeries with pandas integration."""
    
    def __init__(self, data: Optional[Union[va.TimeSeries, pd.Series, Dict]] = None, name: str = ""):
        """Initialize TimeSeries wrapper.
        
        Args:
            data: TimeSeries data from various sources
            name: Series name
        """
        if isinstance(data, va.TimeSeries):
            self._ts = data
        elif isinstance(data, pd.Series):
            self._ts = self._from_pandas(data, name)
        elif isinstance(data, dict):
            self._ts = self._from_dict(data, name)
        else:
            self._ts = va.TimeSeries(name)
    
    def _from_pandas(self, series: pd.Series, name: str) -> va.TimeSeries:
        """Convert pandas Series to C++ TimeSeries."""
        ts = va.TimeSeries(name or series.name or "")
        
        for timestamp, value in series.items():
            if pd.isna(value):
                continue
            
            # Convert timestamp to DateTime
            if isinstance(timestamp, pd.Timestamp):
                dt = va.DateTime(timestamp.year, timestamp.month, timestamp.day,
                               timestamp.hour, timestamp.minute, timestamp.second,
                               timestamp.microsecond // 1000)
            else:
                dt = va.DateTime.fromString(str(timestamp))
            
            ts.addDataPoint(dt, float(value))
        
        return ts
    
    def _from_dict(self, data: Dict, name: str) -> va.TimeSeries:
        """Convert dictionary to C++ TimeSeries."""
        ts = va.TimeSeries(name)
        
        timestamps = data.get('timestamps', data.get('dates', []))
        values = data.get('values', data.get('prices', []))
        
        for timestamp, value in zip(timestamps, values):
            if isinstance(timestamp, str):
                dt = va.DateTime.fromString(timestamp)
            else:
                dt = timestamp
            ts.addDataPoint(dt, float(value))
        
        return ts
    
    def to_pandas(self) -> pd.Series:
        """Convert to pandas Series."""
        timestamps = self._ts.getTimestamps()
        values = self._ts.getValues()
        
        # Convert DateTime objects to pandas timestamps
        pd_timestamps = [
            pd.Timestamp(dt.year(), dt.month(), dt.day(), 
                        dt.hour(), dt.minute(), dt.second(),
                        dt.millisecond() * 1000)
            for dt in timestamps
        ]
        
        return pd.Series(values, index=pd_timestamps, name=self.name)
    
    def to_numpy(self) -> np.ndarray:
        """Convert values to numpy array."""
        return np.array(self._ts.getValues())
    
    @property
    def name(self) -> str:
        """Get series name."""
        return getattr(self._ts, 'name', '')
    
    @property
    def size(self) -> int:
        """Get series size."""
        return self._ts.size()
    
    def __len__(self) -> int:
        return self._ts.size()
    
    def __getitem__(self, key: Union[int, slice]):
        if isinstance(key, int):
            return self._ts.getValue(key)
        elif isinstance(key, slice):
            start = key.start or 0
            stop = key.stop or self._ts.size()
            return TimeSeriesWrapper(self._ts.getSubseries(start, stop))
    
    # Statistical methods
    def mean(self) -> float:
        """Calculate mean."""
        return self._ts.mean()
    
    def std(self) -> float:
        """Calculate standard deviation."""
        return self._ts.standardDeviation()
    
    def var(self) -> float:
        """Calculate variance."""
        return self._ts.variance()
    
    def skew(self) -> float:
        """Calculate skewness."""
        return self._ts.skewness()
    
    def kurt(self) -> float:
        """Calculate kurtosis."""
        return self._ts.kurtosis()
    
    def autocorr(self, lag: int = 1) -> float:
        """Calculate autocorrelation."""
        return self._ts.autocorrelation(lag)
    
    # Transformation methods
    def diff(self, periods: int = 1) -> 'TimeSeriesWrapper':
        """Calculate differences."""
        return TimeSeriesWrapper(self._ts.diff(periods))
    
    def pct_change(self, periods: int = 1) -> 'TimeSeriesWrapper':
        """Calculate percentage changes."""
        return TimeSeriesWrapper(self._ts.pctChange(periods))
    
    def log_returns(self) -> 'TimeSeriesWrapper':
        """Calculate log returns."""
        return TimeSeriesWrapper(self._ts.logReturn())
    
    def rolling_mean(self, window: int) -> 'TimeSeriesWrapper':
        """Calculate rolling mean."""
        return TimeSeriesWrapper(self._ts.rollingMean(window))
    
    def rolling_std(self, window: int) -> 'TimeSeriesWrapper':
        """Calculate rolling standard deviation."""
        return TimeSeriesWrapper(self._ts.rollingStd(window))
    
    # I/O methods
    def to_csv(self, filename: str):
        """Save to CSV."""
        self._ts.saveToCSV(filename)
    
    @classmethod
    def from_csv(cls, filename: str) -> 'TimeSeriesWrapper':
        """Load from CSV."""
        ts = va.TimeSeries.loadFromCSV(filename)
        return cls(ts)
    
    def __repr__(self) -> str:
        return f"TimeSeriesWrapper(size={self.size}, name='{self.name}')"


class MarketDataWrapper:
    """Enhanced Python wrapper for market data handling."""
    
    def __init__(self, symbol: str = "", data: Optional[Union[List[va.MarketData], pd.DataFrame]] = None):
        """Initialize market data wrapper.
        
        Args:
            symbol: Instrument symbol
            data: Market data from various sources
        """
        self.symbol = symbol
        self._data = []
        
        if isinstance(data, list):
            self._data = data
        elif isinstance(data, pd.DataFrame):
            self._data = self._from_dataframe(data, symbol)
    
    def _from_dataframe(self, df: pd.DataFrame, symbol: str) -> List[va.MarketData]:
        """Convert pandas DataFrame to MarketData list."""
        data = []
        
        # Standard column mappings
        col_mapping = {
            'open': ['open', 'Open', 'OPEN'],
            'high': ['high', 'High', 'HIGH'],
            'low': ['low', 'Low', 'LOW'],
            'close': ['close', 'Close', 'CLOSE', 'price', 'Price'],
            'volume': ['volume', 'Volume', 'VOLUME', 'vol', 'Vol']
        }
        
        # Find actual column names
        cols = {}
        for field, candidates in col_mapping.items():
            for candidate in candidates:
                if candidate in df.columns:
                    cols[field] = candidate
                    break
        
        for timestamp, row in df.iterrows():
            # Convert timestamp
            if isinstance(timestamp, pd.Timestamp):
                dt = va.DateTime(timestamp.year, timestamp.month, timestamp.day,
                               timestamp.hour, timestamp.minute, timestamp.second,
                               timestamp.microsecond // 1000)
            else:
                dt = va.DateTime.fromString(str(timestamp))
            
            # Extract OHLCV data
            open_price = float(row.get(cols.get('open', 'close'), row[cols['close']]))
            high_price = float(row.get(cols.get('high', 'close'), row[cols['close']]))
            low_price = float(row.get(cols.get('low', 'close'), row[cols['close']]))
            close_price = float(row[cols['close']])
            volume = float(row.get(cols.get('volume', 'volume'), 0.0))
            
            md = va.MarketData(symbol, dt, open_price, high_price, low_price, close_price, volume)
            
            # Add additional data
            for col in df.columns:
                if col not in cols.values() and col not in ['timestamp', 'date']:
                    md.setAdditionalData(col, str(row[col]))
            
            data.append(md)
        
        return data
    
    def to_dataframe(self) -> pd.DataFrame:
        """Convert to pandas DataFrame."""
        if not self._data:
            return pd.DataFrame()
        
        records = []
        for md in self._data:
            dt = md.getTimestamp()
            timestamp = pd.Timestamp(dt.year(), dt.month(), dt.day(),
                                   dt.hour(), dt.minute(), dt.second(),
                                   dt.millisecond() * 1000)
            
            record = {
                'timestamp': timestamp,
                'open': md.getOpen(),
                'high': md.getHigh(),
                'low': md.getLow(),
                'close': md.getClose(),
                'volume': md.getVolume()
            }
            records.append(record)
        
        df = pd.DataFrame(records)
        df.set_index('timestamp', inplace=True)
        return df
    
    def get_price_series(self, field: str = 'close') -> TimeSeriesWrapper:
        """Extract price time series."""
        ts_data = va.MarketData.extractTimeSeries(self._data, field)
        return TimeSeriesWrapper(ts_data)
    
    def add_data(self, market_data: va.MarketData):
        """Add market data point."""
        self._data.append(market_data)
    
    def __len__(self) -> int:
        return len(self._data)
    
    def __getitem__(self, key: int) -> va.MarketData:
        return self._data[key]
    
    def __repr__(self) -> str:
        return f"MarketDataWrapper(symbol='{self.symbol}', size={len(self._data)})"
