#pragma once
// satellite_data_loader.h
#pragma once
#include <vector>
#include <string>
#include <map>
#include <chrono>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <algorithm>

class SatelliteDataLoader {
private:
    struct TimePoint {
        int hour;
        int minute;
        int second;

        TimePoint(int h = 0, int m = 0, int s = 0) : hour(h), minute(m), second(s) {}

        int toMinutesSince8AM() const {
            return (hour - 8) * 60 + minute + second / 60;
        }
    };

public:
    SatelliteDataLoader();
    bool loadDataFromExcel(const std::string& excelPath);
    void saveCompressedData(const std::string& filename = "compressed_example_3.0.txt");

    const std::vector<std::vector<int>>& getWindowMatrix() const { return window_; }
    const std::vector<std::vector<int>>& getCompressedMatrix() const { return compressed_; }
    const std::vector<int>& getBounds() const { return bounds_; }
    const std::map<int, std::vector<double>>& getCoverageData() const { return coverage_data_; }
    const std::vector<std::vector<int>>& getTimeIndices() const { return idx_; }

private:
    TimePoint safeParseTime(const std::string& timeStr);
    int parseTimeToMinutes(const std::string& timeStr);
    void compressTimeWindows();

    // 数据成员
    std::vector<std::vector<int>> window_;           // 时间窗口矩阵
    std::vector<std::vector<int>> compressed_;       // 压缩后的矩阵
    std::vector<int> bounds_;                        // 原始时间索引
    std::map<int, std::vector<double>> coverage_data_; // 覆盖率数据
    std::vector<std::vector<int>> idx_;              // 时间索引

    const int num_satellites_ = 10;
    const int total_minutes_ = 721; // 12*60 + 1
};