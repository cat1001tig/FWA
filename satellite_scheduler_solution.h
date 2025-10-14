#pragma once
// satellite_scheduler_solution.h
#include "satellite_scheduler_base.h"
#include <map>
#include <vector>

class SatelliteSchedulerSolution : public SatelliteSchedulerBase {
public:
    SatelliteSchedulerSolution();

    // 设置数据方法
    void setCoverageData(const std::map<int, std::vector<double>>& coverage_data);
    void setTimeIndices(const std::vector<std::vector<int>>& time_indices);

    // 解决方案操作
    std::vector<std::vector<int>> initializeSolution();
    std::vector<std::vector<int>> copySolution(const std::vector<std::vector<int>>& solution);

    // 评估函数
    struct EvaluationResult {
        double satellite_count;
        double coverage;
        double load_variance;
    };

    EvaluationResult evaluate(const std::vector<std::vector<int>>& solution, bool return_norm = true);

    // 约束检查
    bool checkSwitches(const std::vector<std::vector<int>>& solution);

protected:
    // 覆盖率计算 - 修改为无参数版本
    double calculateCoverage(const std::vector<std::vector<int>>& solution, int j_original);

    // 评估辅助函数
    EvaluationResult evaluateRaw(const std::vector<std::vector<int>>& solution);
    double calculateVariance(const std::vector<double>& data);

    // 数据成员
    std::map<int, std::vector<double>> coverage_data_;  // 每个卫星的覆盖率数据
    std::vector<std::vector<int>> idx_;                 // 时间索引

    // 归一化边界
    struct NormBounds {
        double min_val = 0.0;
        double max_val = 1.0;
    };

    NormBounds f1_bounds_, f2_bounds_, f3_bounds_;

    void updateBounds(const EvaluationResult& result);
    EvaluationResult normalizeResult(const EvaluationResult& result);
};