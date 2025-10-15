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
#include "satellite_scheduler_base.h"

class SatelliteDataLoader {
private:
    struct TimePoint {
        int hour;
        int minute;
        int second;

        TimePoint(int h = 0, int m = 0, int s = 0) : hour(h), minute(m), second(s) {}

        int toMinutesSincestartAM(int startHour, int startMinute, int startSecond) {
            int currentSeconds = hour * 3600 + minute * 60 + second;
            int startSeconds = startHour * 3600 + startMinute * 60 + startSecond;
            int diffSeconds = currentSeconds - startSeconds;
            // 假设不会跨天，(如果需要，则加24*3600来处理跨天)
            int diffMinutes = diffSeconds / 60; // 向下取整
            return diffMinutes;
        }
    };

public:
    SatelliteDataLoader(const AlgorithmParams& params = AlgorithmParams());
    bool loadDataFromExcel(const std::string& directoryPath);
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

    // 算法参数 - 现在通过构造函数设置
    AlgorithmParams params_;

};