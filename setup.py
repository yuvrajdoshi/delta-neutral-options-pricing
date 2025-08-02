#!/usr/bin/env python3
"""
Volatility Arbitrage Trading System - Python Package Setup
Layer 6: Python Integration Layer

This setup.py provides Python package installation for the volatility arbitrage
trading system with pybind11 C++ bindings.
"""

from pybind11.setup_helpers import Pybind11Extension, build_ext
from pybind11 import get_cmake_dir
import pybind11
from setuptools import setup, Extension
import os
import glob

# Package information
PACKAGE_NAME = "volatility_arbitrage"
VERSION = "1.0.0"
DESCRIPTION = "High-performance volatility arbitrage trading system with C++ core and Python bindings"
AUTHOR = "Volatility Arbitrage System"
AUTHOR_EMAIL = "dev@volatility-arbitrage.com"
URL = "https://github.com/volatility-arbitrage/trading-system"

# Get all source files
def get_source_files():
    """Collect all C++ source files for compilation."""
    source_files = []
    
    # Core source files
    source_files.extend(glob.glob("src/core/*.cpp"))
    source_files.extend(glob.glob("src/instruments/*.cpp"))
    source_files.extend(glob.glob("src/models/*.cpp"))
    source_files.extend(glob.glob("src/strategy/*.cpp"))
    
    # Python bindings
    source_files.append("src/bindings/binding.cpp")
    
    return source_files

# Define the extension module
ext_modules = [
    Pybind11Extension(
        name=PACKAGE_NAME,
        sources=get_source_files(),
        include_dirs=[
            # Path to pybind11 headers
            pybind11.get_include(),
            # Local include directories
            "include",
            "include/core",
            "include/instruments", 
            "include/models",
            "include/strategy",
        ],
        language='c++',
        cxx_std=17,
        define_macros=[
            ("VERSION_INFO", f'"{VERSION}"'),
            ("PYBIND11_DETAILED_ERROR_MESSAGES", None),
        ],
    ),
]

# Custom build_ext class for additional configuration
class CustomBuildExt(build_ext):
    """Custom build extension to handle C++ compilation flags."""
    
    def build_extensions(self):
        # C++ standard and optimization flags
        for ext in self.extensions:
            ext.extra_compile_args.extend([
                "-std=c++17",
                "-O3",
                "-Wall",
                "-Wextra",
                "-fPIC",
            ])
            
            # Platform-specific flags
            if self.compiler.compiler_type == "unix":
                ext.extra_compile_args.extend([
                    "-O2",  # Optimize for performance without processor-specific flags
                ])
        
        super().build_extensions()

# Long description from README
def get_long_description():
    """Read the README file for the long description."""
    try:
        with open("README.md", "r", encoding="utf-8") as fh:
            return fh.read()
    except FileNotFoundError:
        return DESCRIPTION

# Package setup
setup(
    name=PACKAGE_NAME,
    version=VERSION,
    author=AUTHOR,
    author_email=AUTHOR_EMAIL,
    url=URL,
    description=DESCRIPTION,
    long_description=get_long_description(),
    long_description_content_type="text/markdown",
    ext_modules=ext_modules,
    cmdclass={"build_ext": CustomBuildExt},
    zip_safe=False,
    python_requires=">=3.7",
    install_requires=[
        "numpy>=1.19.0",
        "pandas>=1.3.0",
        "matplotlib>=3.3.0",
        "scipy>=1.7.0",
        "pybind11>=2.6.0",
    ],
    extras_require={
        "dev": [
            "pytest>=6.0.0",
            "pytest-cov>=2.10.0",
            "black>=21.0.0",
            "flake8>=3.8.0",
            "mypy>=0.812",
            "jupyter>=1.0.0",
            "notebook>=6.1.0",
        ],
        "analysis": [
            "seaborn>=0.11.0",
            "plotly>=5.0.0",
            "dash>=2.0.0",
            "streamlit>=1.0.0",
            "yfinance>=0.1.60",
            "alpha_vantage>=2.3.0",
        ],
    },
    package_dir={"": "src"},
    packages=["bindings"],
    package_data={
        "bindings": ["*.py"],
    },
    classifiers=[
        "Development Status :: 4 - Beta",
        "Intended Audience :: Financial and Insurance Industry",
        "Intended Audience :: Developers",
        "License :: OSI Approved :: MIT License",
        "Operating System :: OS Independent",
        "Programming Language :: Python :: 3",
        "Programming Language :: Python :: 3.7",
        "Programming Language :: Python :: 3.8",
        "Programming Language :: Python :: 3.9",
        "Programming Language :: Python :: 3.10",
        "Programming Language :: Python :: 3.11",
        "Programming Language :: C++",
        "Topic :: Office/Business :: Financial :: Investment",
        "Topic :: Scientific/Engineering :: Mathematics",
        "Topic :: Software Development :: Libraries :: Python Modules",
    ],
    keywords="finance trading volatility arbitrage options derivatives quantitative",
    project_urls={
        "Bug Reports": f"{URL}/issues",
        "Documentation": f"{URL}/docs",
        "Source": URL,
    },
)