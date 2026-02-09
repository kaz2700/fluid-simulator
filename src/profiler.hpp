#ifndef PROFILER_HPP
#define PROFILER_HPP

#include <chrono>
#include <string>
#include <unordered_map>
#include <vector>
#include <iostream>
#include <iomanip>

// Simple high-resolution timer for profiling
class Timer {
private:
    std::chrono::high_resolution_clock::time_point startTime;
    std::string name;
    bool running;

public:
    Timer() : running(false) {}
    Timer(const std::string& n) : name(n), running(false) {}
    
    void begin() {
        startTime = std::chrono::high_resolution_clock::now();
        running = true;
    }
    
    float elapsed() {
        if (!running) return 0.0f;
        auto endTime = std::chrono::high_resolution_clock::now();
        return std::chrono::duration<float, std::milli>(endTime - startTime).count();
    }
    
    float end() {
        float time = elapsed();
        running = false;
        return time;
    }
    
    const std::string& getName() const { return name; }
};

// Performance profiler that tracks timing across multiple sections
class Profiler {
private:
    struct TimingData {
        float current = 0.0f;
        float total = 0.0f;
        int count = 0;
        float average = 0.0f;
        float minTime = 999999.0f;
        float maxTime = 0.0f;
    };
    
    std::unordered_map<std::string, TimingData> timings;
    std::vector<std::string> sectionOrder;
    
public:
    void startSection(const std::string& name) {
        if (timings.find(name) == timings.end()) {
            sectionOrder.push_back(name);
            timings[name] = TimingData();
        }
    }
    
    void recordTime(const std::string& name, float milliseconds) {
        auto& data = timings[name];
        data.current = milliseconds;
        data.total += milliseconds;
        data.count++;
        data.average = data.total / data.count;
        data.minTime = std::min(data.minTime, milliseconds);
        data.maxTime = std::max(data.maxTime, milliseconds);
    }
    
    void printReport() {
        std::cout << "\n=== Performance Report ===\n";
        float totalTime = 0.0f;
        for (const auto& name : sectionOrder) {
            const auto& data = timings[name];
            std::cout << name << ": " 
                      << std::fixed << std::setprecision(3)
                      << data.current << "ms (avg: " << data.average 
                      << "ms, min: " << data.minTime << "ms, max: " << data.maxTime << "ms)\n";
            totalTime += data.current;
        }
        std::cout << "Total: " << totalTime << "ms\n";
    }
    
    const std::unordered_map<std::string, TimingData>& getTimings() const {
        return timings;
    }
    
    void reset() {
        for (auto& pair : timings) {
            pair.second = TimingData();
        }
    }
    
    void resetAverages() {
        for (auto& pair : timings) {
            pair.second.total = 0.0f;
            pair.second.count = 0;
            pair.second.average = 0.0f;
        }
    }
};

// RAII-style timing scope
class ScopedTimer {
private:
    Timer timer;
    Profiler* profiler;
    std::string sectionName;
    
public:
    ScopedTimer(Profiler& prof, const std::string& name) 
        : profiler(&prof), sectionName(name) {
        profiler->startSection(name);
        timer.begin();
    }
    
    ~ScopedTimer() {
        float elapsed = timer.end();
        profiler->recordTime(sectionName, elapsed);
    }
};

#endif // PROFILER_HPP
