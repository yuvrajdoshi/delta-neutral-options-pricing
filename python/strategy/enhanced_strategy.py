"""
Enhanced Volatility Arbitrage Strategy - Layer 8 Component 5
Optimized strategy with advanced risk management and dynamic parameter adjustment
"""

import pandas as pd
import numpy as np
from datetime import datetime, timedelta
from typing import Dict, List, Optional, Tuple, NamedTuple
import sys
import os

# Add paths for existing components
sys.path.append(os.path.dirname(__file__))

from real_world_strategy import RealWorldVolatilityArbitrageStrategy, Signal, Position, create_default_config
from optimization_engine import EnhancedRiskManager
from market_data import MarketDataManager

class EnhancedVolatilityArbitrageStrategy(RealWorldVolatilityArbitrageStrategy):
    """
    Enhanced volatility arbitrage strategy with:
    1. Advanced risk management
    2. Dynamic position sizing  
    3. Regime-aware parameters
    4. Multiple exit strategies
    """
    
    def __init__(self, config: Dict):
        """Initialize enhanced strategy"""
        super().__init__(config)
        
        # Initialize enhanced risk manager
        self.risk_manager = EnhancedRiskManager(config)
        
        # Enhanced parameters
        self.regime_adjustment = config.get('regime_adjustment', True)
        self.dynamic_thresholds = config.get('dynamic_thresholds', True)
        self.volatility_scaling = config.get('volatility_scaling', True)
        
        # Track performance for adaptive adjustments
        self.recent_trades = []
        self.performance_window = 20  # Last 20 trades for performance tracking
        
        print(f"🚀 Enhanced Volatility Arbitrage Strategy initialized")
        print(f"   Regime adjustment: {self.regime_adjustment}")
        print(f"   Dynamic thresholds: {self.dynamic_thresholds}")
        print(f"   Volatility scaling: {self.volatility_scaling}")
    
    def adjust_entry_threshold_dynamically(self, market_data: pd.DataFrame) -> float:
        """
        Dynamically adjust entry threshold based on market conditions
        
        Args:
            market_data: Current market data
            
        Returns:
            Adjusted entry threshold
        """
        base_threshold = self.entry_threshold
        
        if not self.dynamic_thresholds:
            return base_threshold
        
        try:
            # Adjust based on VIX level
            if 'VIX' in market_data.columns:
                current_vix = market_data['VIX'].iloc[-1]
                vix_20d_avg = market_data['VIX'].tail(20).mean()
                
                if current_vix > vix_20d_avg * 1.3:  # High vol regime
                    # Lower threshold in high vol (more opportunities)
                    base_threshold *= 0.8
                elif current_vix < vix_20d_avg * 0.7:  # Low vol regime  
                    # Higher threshold in low vol (be more selective)
                    base_threshold *= 1.3
            
            # Adjust based on recent performance
            if len(self.recent_trades) >= 5:
                recent_win_rate = sum(1 for trade in self.recent_trades[-10:] if trade['pnl'] > 0) / min(10, len(self.recent_trades))
                
                if recent_win_rate < 0.3:  # Poor recent performance
                    base_threshold *= 1.4  # Be more selective
                elif recent_win_rate > 0.7:  # Good recent performance
                    base_threshold *= 0.9  # Be more aggressive
            
            # Adjust based on realized volatility vs implied volatility correlation
            if 'RealizedVol' in market_data.columns and 'VIX' in market_data.columns:
                try:
                    realized_vol = market_data['RealizedVol'].tail(30).dropna()
                    vix_vol = market_data['VIX'].tail(30).dropna() / 100
                    
                    # Ensure both series have the same length
                    min_length = min(len(realized_vol), len(vix_vol))
                    if min_length >= 15:  # Minimum data for correlation
                        correlation = np.corrcoef(
                            realized_vol.tail(min_length), 
                            vix_vol.tail(min_length)
                        )[0, 1]
                        
                        if not np.isnan(correlation):
                            if correlation > 0.8:  # High correlation - less arbitrage opportunity
                                base_threshold *= 1.2
                            elif correlation < 0.4:  # Low correlation - more arbitrage opportunity
                                base_threshold *= 0.85
                except Exception as e:
                    print(f"⚠️  Warning in volatility correlation calculation: {e}")
                    pass  # Continue with base threshold
            
            # Ensure threshold stays within reasonable bounds
            min_threshold = self.entry_threshold * 0.5
            max_threshold = self.entry_threshold * 2.0
            
            return max(min(base_threshold, max_threshold), min_threshold)
            
        except Exception as e:
            print(f"⚠️  Error adjusting threshold: {e}")
            return self.entry_threshold
    
    def calculate_enhanced_garch_volatility(self, returns: pd.Series, market_data: pd.DataFrame) -> float:
        """
        Enhanced GARCH volatility calculation with regime awareness
        
        Args:
            returns: Historical return series
            market_data: Market data for regime detection
            
        Returns:
            Enhanced volatility forecast
        """
        base_vol = super().calculate_garch_volatility(returns)
        
        if not self.volatility_scaling:
            return base_vol
        
        try:
            # Adjust based on market regime
            if 'HighVolRegime' in market_data.columns:
                if market_data['HighVolRegime'].iloc[-1]:
                    # In high vol regime, increase forecast
                    base_vol *= 1.15
                else:
                    # In low vol regime, slightly decrease forecast  
                    base_vol *= 0.95
            
            # Adjust based on term structure
            if 'VIX' in market_data.columns and len(market_data) >= 30:
                short_term_vol = market_data['RealizedVol'].tail(10).mean()
                medium_term_vol = market_data['RealizedVol'].tail(30).mean()
                
                # If short-term vol is trending up relative to medium-term
                vol_trend = short_term_vol / medium_term_vol
                if vol_trend > 1.2:
                    base_vol *= 1.1  # Expect continued high vol
                elif vol_trend < 0.8:
                    base_vol *= 0.9  # Expect continued low vol
            
            return base_vol
            
        except Exception as e:
            print(f"⚠️  Error in enhanced GARCH: {e}")
            return base_vol
    
    def generate_enhanced_signals(self, market_data: pd.DataFrame) -> Optional[Signal]:
        """
        Generate signals with enhanced logic and dynamic thresholds
        """
        try:
            # Ensure we have enough data
            if len(market_data) < self.lookback_period:
                return None
            
            # Get dynamic entry threshold
            current_threshold = self.adjust_entry_threshold_dynamically(market_data)
            
            # Calculate enhanced volatility forecast
            returns = market_data['Returns'].dropna()
            forecasted_vol = self.calculate_enhanced_garch_volatility(returns, market_data)
            
            # Get market implied volatility
            implied_vol = self.calculate_implied_volatility(market_data)
            
            # Calculate volatility spread
            vol_spread = forecasted_vol - implied_vol
            
            # Check if spread exceeds dynamic threshold
            if abs(vol_spread) < current_threshold:
                return None
            
            # Enhanced signal type determination
            signal_type = None
            confidence_multiplier = 1.0
            
            if vol_spread > current_threshold:
                signal_type = 'BUY_STRADDLE'
                # Higher confidence if we're in a trending vol environment
                if 'RealizedVol' in market_data.columns:
                    vol_trend = market_data['RealizedVol'].tail(5).mean() / market_data['RealizedVol'].tail(20).mean()
                    if vol_trend > 1.1:  # Vol trending up
                        confidence_multiplier = 1.2
            elif vol_spread < -current_threshold:
                signal_type = 'SELL_STRADDLE'
                # Higher confidence in low vol regime for selling vol
                if 'VIX' in market_data.columns:
                    current_vix = market_data['VIX'].iloc[-1]
                    if current_vix < 20:  # Low VIX
                        confidence_multiplier = 1.15
            else:
                return None
            
            # Select strike and expiry with enhanced logic
            strike, expiry_days = self.select_enhanced_strike_and_expiry(market_data, vol_spread)
            
            # Calculate enhanced parameters
            hedge_ratio = self.calculate_hedge_ratio(market_data, signal_type)
            expected_return = self.calculate_enhanced_expected_return(vol_spread, signal_type, market_data)
            risk_estimate = self.calculate_enhanced_risk_estimate(vol_spread, market_data)
            
            # Enhanced signal strength calculation
            spread_strength = min(abs(vol_spread) / (current_threshold * 2.5), 1.0)
            
            # Adjust strength based on market conditions
            if 'VIX' in market_data.columns:
                vix_percentile = self.calculate_vix_percentile(market_data)
                if signal_type == 'SELL_STRADDLE' and vix_percentile > 0.8:
                    spread_strength *= 1.3  # Strong sell signal in high VIX
                elif signal_type == 'BUY_STRADDLE' and vix_percentile < 0.2:
                    spread_strength *= 1.2  # Strong buy signal in low VIX
            
            strength = min(spread_strength, 1.0)
            
            # Enhanced confidence calculation
            base_confidence = min(1 / (1 + market_data['RealizedVol'].tail(20).std()), 0.95)
            confidence = min(base_confidence * confidence_multiplier, 0.95)
            
            signal = Signal(
                type=signal_type,
                strength=strength,
                timestamp=market_data.index[-1],
                strike=strike,
                expiry_days=expiry_days,
                vol_spread=vol_spread,
                confidence=confidence,
                hedge_ratio=hedge_ratio,
                expected_return=expected_return,
                risk_estimate=risk_estimate
            )
            
            print(f"🎯 Enhanced {signal_type} signal:")
            print(f"   Dynamic threshold: {current_threshold:.2%}")
            print(f"   Forecasted vol: {forecasted_vol:.2%}")
            print(f"   Implied vol: {implied_vol:.2%}")
            print(f"   Vol spread: {vol_spread:.2%}")
            print(f"   Enhanced strength: {strength:.2f}")
            print(f"   Enhanced confidence: {confidence:.2f}")
            
            return signal
            
        except Exception as e:
            print(f"❌ Error generating enhanced signal: {str(e)}")
            return None
    
    def select_enhanced_strike_and_expiry(self, market_data: pd.DataFrame, vol_spread: float) -> Tuple[float, int]:
        """
        Enhanced strike and expiry selection based on volatility spread and market conditions
        """
        current_price = market_data['Close'].iloc[-1]
        
        # Dynamic strike selection based on vol spread magnitude
        if abs(vol_spread) > 0.06:  # Large spread
            # Go slightly OTM for better risk/reward
            strike_offset = 0.02 if vol_spread > 0 else 0.015
            strike = current_price * (1 + strike_offset)
        else:
            # Stay closer to ATM for smaller spreads
            strike = current_price * 1.005
        
        # Round to nearest $1 for liquid strikes
        strike = round(strike)
        
        # Dynamic expiry selection
        base_expiry = 30
        
        # Adjust expiry based on volatility level
        if 'VIX' in market_data.columns:
            current_vix = market_data['VIX'].iloc[-1]
            if current_vix > 30:  # High vol - shorter expiry
                base_expiry = 25
            elif current_vix < 15:  # Low vol - longer expiry
                base_expiry = 35
        
        # Adjust based on vol spread magnitude
        if abs(vol_spread) > 0.05:
            base_expiry = 25  # Shorter expiry for large spreads
        
        return strike, base_expiry
    
    def calculate_vix_percentile(self, market_data: pd.DataFrame, lookback: int = 252) -> float:
        """Calculate VIX percentile rank over lookback period"""
        if 'VIX' not in market_data.columns or len(market_data) < lookback:
            return 0.5  # Default to median if no data
        
        vix_data = market_data['VIX'].tail(lookback)
        current_vix = vix_data.iloc[-1]
        
        percentile = (vix_data < current_vix).sum() / len(vix_data)
        return percentile
    
    def calculate_enhanced_expected_return(self, vol_spread: float, signal_type: str, market_data: pd.DataFrame) -> float:
        """Enhanced expected return calculation with market regime consideration"""
        base_return = super().calculate_expected_return(vol_spread, signal_type)
        
        # Adjust based on VIX percentile
        if 'VIX' in market_data.columns:
            vix_percentile = self.calculate_vix_percentile(market_data)
            
            if signal_type == 'SELL_STRADDLE':
                # Selling vol is more profitable when VIX is high
                if vix_percentile > 0.8:
                    base_return *= 1.4
                elif vix_percentile < 0.3:
                    base_return *= 0.7
            else:  # BUY_STRADDLE
                # Buying vol is more profitable when VIX is low but realized vol picks up
                if vix_percentile < 0.3:
                    realized_vol_trend = market_data['RealizedVol'].tail(5).mean() / market_data['RealizedVol'].tail(20).mean()
                    if realized_vol_trend > 1.1:
                        base_return *= 1.3
        
        return base_return
    
    def calculate_enhanced_risk_estimate(self, vol_spread: float, market_data: pd.DataFrame) -> float:
        """Enhanced risk estimation with regime awareness"""
        base_risk = super().calculate_risk_estimate(vol_spread, market_data)
        
        # Adjust risk based on market conditions
        if 'VIX' in market_data.columns:
            current_vix = market_data['VIX'].iloc[-1]
            vix_20d_avg = market_data['VIX'].tail(20).mean()
            
            # Higher risk in volatile markets
            vix_multiplier = current_vix / vix_20d_avg
            base_risk *= vix_multiplier
        
        # Adjust for correlation regime
        if len(market_data) >= 60:
            try:
                returns_60 = market_data['Returns'].tail(60).dropna()
                vix_changes_60 = market_data['VIX'].tail(60).diff().dropna()
                
                # Ensure both series have the same length
                min_length = min(len(returns_60), len(vix_changes_60))
                if min_length >= 30:  # Minimum data for correlation
                    spy_vix_corr = np.corrcoef(
                        returns_60.tail(min_length),
                        vix_changes_60.tail(min_length)
                    )[0, 1]
                    
                    # Higher risk when correlation breaks down
                    if not np.isnan(spy_vix_corr) and abs(spy_vix_corr) < 0.3:
                        base_risk *= 1.3
            except Exception as e:
                print(f"⚠️  Warning in correlation calculation: {e}")
                pass  # Use base risk if correlation calculation fails
        
        return base_risk
    
    def validate_enhanced_signal(self, signal: Signal, portfolio: Dict, market_data: pd.DataFrame) -> Tuple[bool, float]:
        """Use enhanced risk manager for signal validation"""
        return self.risk_manager.validate_signal_enhanced(signal, portfolio, market_data)
    
    def execute_enhanced_signal(self, signal: Signal, position_size: float) -> Optional[Position]:
        """Execute signal with dynamic position sizing"""
        try:
            # Calculate position value using dynamic size
            position_value = self.portfolio_value * position_size
            
            # Estimate option premium with enhanced model
            premium_multiplier = 0.08 + abs(signal.vol_spread) * 0.5  # More sophisticated pricing
            entry_price = signal.strike * premium_multiplier
            
            if signal.type == 'SELL_STRADDLE':
                quantity = -int(position_value / (entry_price * 100))
            else:
                quantity = int(position_value / (entry_price * 100))
            
            # Create position
            position = Position(signal, entry_price, quantity)
            
            # Store entry VIX for regime change detection
            if hasattr(signal, 'timestamp') and 'VIX' in signal.__dict__:
                position.entry_vix = signal.vol_spread  # Store for later use
            
            self.position_id += 1
            self.positions[self.position_id] = position
            
            print(f"✅ Enhanced execution of {signal.type}:")
            print(f"   Position size: {position_size:.1%} of portfolio")
            print(f"   Quantity: {quantity}")
            print(f"   Entry price: ${entry_price:.2f}")
            print(f"   Expected return: {signal.expected_return:.2%}")
            
            return position
            
        except Exception as e:
            print(f"❌ Error executing enhanced signal: {str(e)}")
            return None
    
    def update_enhanced_positions(self, market_data: pd.DataFrame):
        """Update positions with enhanced exit logic"""
        current_date = market_data.index[-1]
        current_price = market_data['Close'].iloc[-1]
        
        positions_to_close = []
        
        for pos_id, position in self.positions.items():
            if not position.is_active:
                continue
            
            # Calculate days to expiry
            days_held = (current_date - position.entry_time).days
            days_to_expiry = position.signal.expiry_days - days_held
            
            # Update P&L with more sophisticated model
            volatility_factor = market_data['RealizedVol'].iloc[-1] / 0.20  # Normalize to 20% vol
            option_price = position.entry_price * (1 + np.random.normal(0, 0.03) * volatility_factor)
            position.update_pnl(option_price, current_price)
            
            # Enhanced exit decision
            should_close, reason = self.risk_manager.should_close_position_enhanced(
                position, market_data, days_to_expiry
            )
            
            if should_close:
                positions_to_close.append((pos_id, reason))
        
        # Close positions with reasons
        for pos_id, reason in positions_to_close:
            self.close_enhanced_position(pos_id, market_data, reason)
    
    def close_enhanced_position(self, position_id: int, market_data: pd.DataFrame, reason: str = ""):
        """Close position with enhanced tracking"""
        if position_id not in self.positions:
            return
        
        position = self.positions[position_id]
        position.is_active = False
        
        # Update portfolio value
        self.portfolio_value += position.pnl
        
        # Track trade for performance analysis
        trade_record = {
            'entry_time': position.entry_time,
            'exit_time': market_data.index[-1],
            'signal_type': position.signal.type,
            'pnl': position.pnl,
            'return_pct': position.pnl / (abs(position.quantity) * position.entry_price * 100),
            'vol_spread': position.signal.vol_spread,
            'reason': reason
        }
        
        self.recent_trades.append(trade_record)
        
        # Keep only recent trades for performance tracking
        if len(self.recent_trades) > self.performance_window:
            self.recent_trades = self.recent_trades[-self.performance_window:]
        
        print(f"🔄 Enhanced close position {position_id} ({reason}):")
        print(f"   P&L: ${position.pnl:.2f}")
        print(f"   Return: {trade_record['return_pct']:.1%}")
        print(f"   Portfolio value: ${self.portfolio_value:.2f}")

def create_enhanced_config() -> Dict:
    """Create enhanced strategy configuration"""
    config = create_default_config()
    
    # Enhanced risk management parameters
    config.update({
        # Dynamic position sizing
        'kelly_fraction': 0.25,
        'min_position_size': 0.03,
        'max_position_size': 0.12,
        
        # Enhanced risk controls
        'max_portfolio_var': 0.025,
        'max_position_risk': 0.06,
        'vol_regime_multiplier': 1.4,
        
        # Dynamic exits
        'stop_loss_pct': -0.30,
        'profit_target_pct': 0.50,
        'trailing_stop_pct': 0.15,
        
        # Strategy enhancements
        'regime_adjustment': True,
        'dynamic_thresholds': True,
        'volatility_scaling': True,
        
        # Entry parameters (will be adjusted dynamically)
        'entry_threshold': 0.025,  # 2.5% base threshold
        'lookback_period': 50,
        'max_positions': 6,
    })
    
    return config

def main():
    """Test the enhanced strategy"""
    print("🚀 Testing Enhanced Volatility Arbitrage Strategy...")
    
    # Create enhanced strategy
    config = create_enhanced_config()
    strategy = EnhancedVolatilityArbitrageStrategy(config)
    
    # Initialize market data manager
    data_manager = MarketDataManager()
    
    # Get test data
    end_date = datetime.now()
    start_date = end_date - timedelta(days=120)
    
    print(f"\n📊 Getting market data from {start_date.date()} to {end_date.date()}...")
    
    dataset = data_manager.get_comprehensive_dataset(
        'SPY',
        start_date.strftime('%Y-%m-%d'),
        end_date.strftime('%Y-%m-%d')
    )
    
    if dataset.empty:
        print("❌ No data available for testing")
        return
    
    print(f"✅ Loaded {len(dataset)} days of market data")
    
    # Test enhanced signal generation
    print(f"\n🎯 Testing enhanced signal generation...")
    
    signal = strategy.generate_enhanced_signals(dataset)
    
    if signal:
        print(f"✅ Enhanced signal generated!")
        
        # Test enhanced validation
        portfolio_state = {'positions': strategy.positions}
        is_valid, position_size = strategy.validate_enhanced_signal(signal, portfolio_state, dataset)
        
        if is_valid:
            print(f"✅ Signal passed enhanced validation")
            print(f"   Optimal position size: {position_size:.1%}")
            
            # Test enhanced execution
            position = strategy.execute_enhanced_signal(signal, position_size)
            if position:
                print(f"✅ Enhanced signal executed")
                
                # Test enhanced position management
                strategy.update_enhanced_positions(dataset)
                
                # Show enhanced portfolio summary
                summary = strategy.get_portfolio_summary()
                print(f"\n📈 Enhanced Portfolio Summary:")
                for key, value in summary.items():
                    print(f"   {key}: {value}")
        else:
            print(f"⚠️  Signal failed enhanced validation")
    else:
        print(f"ℹ️  No enhanced signal generated")
    
    print(f"\n✅ Enhanced strategy test completed!")

if __name__ == "__main__":
    main()
