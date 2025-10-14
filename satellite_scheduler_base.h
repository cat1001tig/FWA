#pragma once
// satellite_scheduler_base.h
#pragma once
#include <vector>
#include <string>
#include <memory>
#include <random>
#include "coverage_loader.h"

class SatelliteSchedulerBase {
protected:
    // 基础数据结构
    std::vector<std::vector<int>> compressed_;  // 压缩时间窗口矩阵
    std::vector<int> bounds_;                   // 原始时间索引
    int m_;                                     // 卫星数量

    // 算法参数
    int max_switches_ = 7;
    int q_ = 6665;
    std::vector<double> weights_ = { 0.34, 0.33, 0.33 };

    int max_sparks_ = 30;
    int max_length_ = 10;
    bool update_bounds_ = true;
    int max_variation_ = 3;

    std::vector<int> special_times_ = { 263, 264, 265, 266, 267, 268, 269, 270, 271, 272, 449 };

    // 组件
    std::unique_ptr<CoverageDataLoader> coverage_loader_;

    // 随机数生成
    std::random_device rd_;
    std::mt19937 gen_;

public:
    SatelliteSchedulerBase();
    virtual ~SatelliteSchedulerBase() = default;

    // 基础功能
    bool loadCompressedData(const std::string& filename);
    void initializeCoverageLoader();

    // 获取器
    const std::vector<std::vector<int>>& getCompressed() const { return compressed_; }
    const std::vector<int>& getBounds() const { return bounds_; }
    int getSatelliteCount() const { return m_; }
    int getQ() const { return q_; }

protected:
    std::vector<std::vector<int>> parseCompressedData(const std::vector<std::string>& lines);
    std::vector<int> parseBoundsData(const std::string& bounds_line);
};
