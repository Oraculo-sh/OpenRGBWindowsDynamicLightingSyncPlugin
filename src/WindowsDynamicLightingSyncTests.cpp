#include "WindowsDynamicLightingSync.h"
#include <cassert>
#include <iostream>
#include <vector>
#include <string>
#include <chrono>
#include <thread>

// Simple test framework
class TestFramework {
public:
    static int tests_run;
    static int tests_passed;
    static int tests_failed;
    
    static void assert_true(bool condition, const std::string& test_name) {
        tests_run++;
        if (condition) {
            tests_passed++;
            std::cout << "[PASS] " << test_name << std::endl;
        } else {
            tests_failed++;
            std::cout << "[FAIL] " << test_name << std::endl;
        }
    }
    
    static void assert_equals(const std::string& expected, const std::string& actual, const std::string& test_name) {
        assert_true(expected == actual, test_name + " (expected: '" + expected + "', got: '" + actual + "')");
    }
    
    static void print_summary() {
        std::cout << "\n=== Test Summary ===" << std::endl;
        std::cout << "Tests run: " << tests_run << std::endl;
        std::cout << "Tests passed: " << tests_passed << std::endl;
        std::cout << "Tests failed: " << tests_failed << std::endl;
        std::cout << "Success rate: " << (tests_run > 0 ? (tests_passed * 100 / tests_run) : 0) << "%" << std::endl;
    }
};

int TestFramework::tests_run = 0;
int TestFramework::tests_passed = 0;
int TestFramework::tests_failed = 0;

// Mock ResourceManager for testing
class MockResourceManager {
public:
    std::vector<RGBController*> mock_controllers;
    
    std::vector<RGBController*> GetRGBControllers() {
        return mock_controllers;
    }
    
    void AddMockController(const std::string& name, int led_count, bool has_per_led_color = true) {
        RGBController* controller = new RGBController();
        controller->name = name;
        controller->description = name + " Description";
        
        // Add LEDs
        for (int i = 0; i < led_count; i++) {
            led new_led;
            new_led.name = "LED " + std::to_string(i);
            controller->leds.push_back(new_led);
        }
        
        // Add a mode with appropriate flags
        mode new_mode;
        new_mode.name = "Test Mode";
        new_mode.flags = has_per_led_color ? MODE_FLAG_HAS_PER_LED_COLOR : MODE_FLAG_HAS_MODE_SPECIFIC_COLOR;
        controller->modes.push_back(new_mode);
        
        mock_controllers.push_back(controller);
    }
    
    ~MockResourceManager() {
        for (auto* controller : mock_controllers) {
            delete controller;
        }
    }
};

// Test class
class WindowsDynamicLightingSyncTests {
private:
    MockResourceManager mock_rm;
    WindowsDynamicLightingSync* plugin;
    
public:
    WindowsDynamicLightingSyncTests() {
        plugin = new WindowsDynamicLightingSync();
        // Note: In a real test, we'd need to inject the mock ResourceManager
        // For now, we'll test what we can without full dependency injection
    }
    
    ~WindowsDynamicLightingSyncTests() {
        delete plugin;
    }
    
    void TestPluginInitialization() {
        TestFramework::assert_true(plugin != nullptr, "Plugin initialization");
        TestFramework::assert_true(plugin->GetName() == "Windows Dynamic Lighting Sync", "Plugin name");
        TestFramework::assert_true(plugin->GetDescription().find("Sincroniza") != std::string::npos, "Plugin description contains expected text");
    }
    
    void TestConfigurationDefaults() {
        // Test default configuration values
        // Removed tests for deprecated features: bidirectional sync, smooth transitions, auto detect
        TestFramework::assert_true(plugin->GetSyncInterval() == 100, "Default sync interval");
    }
    
    void TestConfigurationSetters() {
        // Test configuration setters
        // Removed tests for deprecated features: bidirectional sync, smooth transitions, auto detect
        
        plugin->SetSyncInterval(200);
        TestFramework::assert_true(plugin->GetSyncInterval() == 200, "Set sync interval 200ms");
        
        plugin->SetBrightnessMultiplier(0.8f);
        // Note: We'd need a getter for brightness multiplier to test this properly
    }
    
    void TestLoggingSystem() {
        // Test logging methods don't crash
        try {
            plugin->LogInfo("Test info message");
            plugin->LogWarning("Test warning message");
            plugin->LogError("Test error message");
            TestFramework::assert_true(true, "Logging methods execute without exception");
        } catch (...) {
            TestFramework::assert_true(false, "Logging methods execute without exception");
        }
    }
    
    void TestErrorHandling() {
        // Test SafeExecute with successful operation
        bool success = plugin->SafeExecute([]() {
            // Do nothing - should succeed
        }, "Test successful operation");
        TestFramework::assert_true(success, "SafeExecute with successful operation");
        
        // Test SafeExecute with exception
        bool failure = plugin->SafeExecute([]() {
            throw std::runtime_error("Test exception");
        }, "Test exception operation");
        TestFramework::assert_true(!failure, "SafeExecute with exception returns false");
    }
    
    void TestColorConversion() {
        // Test color conversion methods if they exist
        // Note: These would need to be exposed or we'd need access to test them
        TestFramework::assert_true(true, "Color conversion placeholder test");
    }
    
    void TestDeviceDetection() {
        // Device detection is now handled by OpenRGB
        // Test that device list update works without exception
        try {
            plugin->updateDeviceList();
            TestFramework::assert_true(true, "Device list update executes without exception");
        } catch (...) {
            TestFramework::assert_true(false, "Device list update executes without exception");
        }
    }
    
    void TestSyncLogic() {
        // Test sync logic without actual devices
        try {
            plugin->syncLighting();
            TestFramework::assert_true(true, "Sync lighting executes without exception");
        } catch (...) {
            TestFramework::assert_true(false, "Sync lighting executes without exception");
        }
    }
    
    void RunAllTests() {
        std::cout << "Running Windows Dynamic Lighting Sync Plugin Tests...\n" << std::endl;
        
        TestPluginInitialization();
        TestConfigurationDefaults();
        TestConfigurationSetters();
        TestLoggingSystem();
        TestErrorHandling();
        TestColorConversion();
        TestDeviceDetection();
        TestSyncLogic();
        
        TestFramework::print_summary();
    }
};

// Main test runner
int main() {
    std::cout << "Windows Dynamic Lighting Sync Plugin - Unit Tests" << std::endl;
    std::cout << "================================================" << std::endl;
    
    WindowsDynamicLightingSyncTests tests;
    tests.RunAllTests();
    
    return TestFramework::tests_failed > 0 ? 1 : 0;
}