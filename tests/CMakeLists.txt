# Tests CMakeLists.txt
cmake_minimum_required(VERSION 3.16)

# Function to create a test executable
function(create_test TEST_NAME TEST_SOURCE)
    add_executable(${TEST_NAME} ${TEST_SOURCE})
    target_link_libraries(${TEST_NAME} volatility_core)
    target_include_directories(${TEST_NAME} PRIVATE ${CMAKE_SOURCE_DIR}/include)
    add_test(NAME ${TEST_NAME} COMMAND ${TEST_NAME})
endfunction()

# Create test executables
create_test(test_datetime test_datetime.cpp)
create_test(test_marketdata test_marketdata.cpp)
create_test(test_math test_math.cpp)
create_test(test_equity test_equity.cpp)
create_test(test_options test_options.cpp)
create_test(test_volatility test_volatility.cpp)
create_test(test_instrument_factory test_instrument_factory.cpp)
create_test(test_garch_model test_garch_model.cpp)
create_test(test_bsm_pricing test_bsm_pricing.cpp)
create_test(test_model_factory test_model_factory.cpp)
create_test(test_strategy test_strategy.cpp)

# Set test properties
set_tests_properties(
    test_datetime test_marketdata test_math test_equity test_options 
    test_volatility test_instrument_factory test_garch_model 
    test_bsm_pricing test_model_factory test_strategy
    PROPERTIES 
    TIMEOUT 30
)
