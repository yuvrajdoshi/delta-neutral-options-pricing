#include "core/DateTime.h"
#include <iostream>

int main() {
    try {
        // Test DateTime creation according to LLD specification
        core::DateTime now = core::DateTime::now();
        std::cout << "Current time: " << now.toString() << std::endl;
        
        // Test DateTime with specific values
        core::DateTime specificDate(2025, 7, 25, 10, 30, 45);
        std::cout << "Specific date: " << specificDate.toString() << std::endl;
        
        // Test different format
        std::cout << "Date only: " << specificDate.toString("%Y-%m-%d") << std::endl;
        
        // Test isValid method
        std::cout << "Is valid: " << (specificDate.isValid() ? "true" : "false") << std::endl;
        
        // Test TimeDelta
        core::TimeDelta delta(1, 2, 30, 15); // 1 day, 2 hours, 30 minutes, 15 seconds
        std::cout << "TimeDelta: " << delta.toString() << std::endl;
        
        // Test arithmetic
        core::DateTime future = specificDate + delta;
        std::cout << "Future date: " << future.toString() << std::endl;
        
        // Test comparison
        if (future > specificDate) {
            std::cout << "Arithmetic and comparison working correctly!" << std::endl;
        }
        
        // Test accessors according to LLD
        std::cout << "Year: " << specificDate.year() << ", Month: " << specificDate.month() 
                  << ", Day: " << specificDate.day() << std::endl;
        std::cout << "Hour: " << specificDate.hour() << ", Minute: " << specificDate.minute() 
                  << ", Second: " << specificDate.second() << std::endl;
        
        // Test fromString static method
        core::DateTime parsed = core::DateTime::fromString("2025-12-31 23:59:59", "%Y-%m-%d %H:%M:%S");
        std::cout << "Parsed date: " << parsed.toString() << std::endl;
        
        // Test modifiers
        core::DateTime modifiable = specificDate;
        modifiable.setYear(2026);
        modifiable.setMonth(12);
        std::cout << "Modified date: " << modifiable.toString() << std::endl;
        
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
}
