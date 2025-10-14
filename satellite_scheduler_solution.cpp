// satellite_scheduler_solution.cpp
#include "satellite_scheduler_solution.h"
#include <numeric>
#include <algorithm>
#include <cmath>
#include <iostream>

SatelliteSchedulerSolution::SatelliteSchedulerSolution(const AlgorithmParams& params)
    : SatelliteSchedulerBase(params) {
    // 初始化边界
    f1_bounds_ = { std::numeric_limits<double>::max(), std::numeric_limits<double>::lowest() };
    f2_bounds_ = { std::numeric_limits<double>::max(), std::numeric_limits<double>::lowest() };
    f3_bounds_ = { std::numeric_limits<double>::max(), std::numeric_limits<double>::lowest() };
}

std::vector<std::vector<int>> SatelliteSchedulerSolution::initializeSolution() {
    std::vector<std::vector<int>> solution;
    std::uniform_real_distribution<double> dist(0.0, 1.0);

    for (const auto& sat_schedule : compressed_) {
        std::vector<int> new_schedule = sat_schedule;

        for (size_t j = 0; j < new_schedule.size(); ++j) {
            if (new_schedule[j] == 1 && dist(gen_) < 0.3) {
                new_schedule[j] = 0; // 随机关闭一些时间窗口
            }
        }

        solution.push_back(new_schedule);
    }

    std::cout << "初始化解生成完成: " << solution.size() << " 颗卫星" << std::endl;
    return solution;
}

std::vector<std::vector<int>> SatelliteSchedulerSolution::copySolution(
    const std::vector<std::vector<int>>& solution) {

    std::vector<std::vector<int>> copy;
    for (const auto& sat_schedule : solution) {
        copy.push_back(sat_schedule);
    }
    return copy;
}

SatelliteSchedulerSolution::EvaluationResult SatelliteSchedulerSolution::evaluate(
    const std::vector<std::vector<int>>& solution, bool return_norm) {

    auto raw_result = evaluateRaw(solution);

    if (params_.update_bounds) {
        updateBounds(raw_result);
    }

    if (return_norm) {
        return normalizeResult(raw_result);
    }

    return raw_result;
}

SatelliteSchedulerSolution::EvaluationResult SatelliteSchedulerSolution::evaluateRaw(
    const std::vector<std::vector<int>>& solution) {

    EvaluationResult result;

    // 计算活跃卫星数量
    result.satellite_count = 0.0;
    for (const auto& sat_sched : solution) {
        bool has_active = std::any_of(sat_sched.begin(), sat_sched.end(),
            [](int val) { return val == 1; });
        if (has_active) {
            result.satellite_count += 1.0;
        }
    }

    // 计算总覆盖率
    result.coverage = 0.0;
    for (int j_original : bounds_) {
        result.coverage += calculateCoverage(solution, j_original);
    }

    // 计算负载方差
    std::vector<double> active_times;
    for (const auto& sat_sched : solution) {
        double active_count = std::count(sat_sched.begin(), sat_sched.end(), 1);
        active_times.push_back(active_count);
    }
    result.load_variance = calculateVariance(active_times);

    return result;
}


void SatelliteSchedulerSolution::setCoverageData(const std::map<int, std::vector<double>>& coverage_data) {
    coverage_data_ = coverage_data;
    std::cout << "设置覆盖率数据: " << coverage_data_.size() << " 颗卫星" << std::endl;
}

void SatelliteSchedulerSolution::setTimeIndices(const std::vector<std::vector<int>>& time_indices) {
    idx_ = time_indices;
    std::cout << "设置时间索引: " << idx_.size() << " 颗卫星" << std::endl;
}

double SatelliteSchedulerSolution::calculateCoverage(
    const std::vector<std::vector<int>>& solution, int j_original) {

    std::vector<bool> combined(params_.q, false);
    std::vector<int> active_sats;

    // 查找当前时间点活跃的卫星
    auto bounds_it = std::find(bounds_.begin(), bounds_.end(), j_original);
    if (bounds_it != bounds_.end()) {
        int comp_idx = std::distance(bounds_.begin(), bounds_it);

        for (int sat = 0; sat < m_; ++sat) {
            if (solution[sat][comp_idx] == 1) {
                active_sats.push_back(sat);
            }
        }
    }

    // 当前时间点没有卫星调度
    if (active_sats.empty()) {
        return 0.0;
    }

    // 当前时间点有1个卫星调度
    else if (active_sats.size() == 1) {
        int sat = active_sats[0];

        // 查找时间点在idx_中的位置
        if (sat < idx_.size()) {
            const auto& sat_indices = idx_[sat];
            auto time_it = std::find(sat_indices.begin(), sat_indices.end(), j_original);

            if (time_it != sat_indices.end()) {
                int index = std::distance(sat_indices.begin(), time_it);

                // 从coverage_data_中获取覆盖率
                auto coverage_it = coverage_data_.find(sat);
                if (coverage_it != coverage_data_.end() &&
                    index < coverage_it->second.size()) {
                    return coverage_it->second[index];
                }
            }
        }

        // 如果找不到对应的覆盖率数据，返回0
        return 0.0;
    }

    // 当前时间点有2个及以上卫星调度
    else {
        for (int sat : active_sats) {
            // 使用覆盖率加载器获取网格数据
            auto mesh_array = coverage_loader_->getMeshData(
                sat + 1,  // 卫星编号转为1-based
                j_original,
                params_.q);

            // 将网格值取或运算
            for (int i = 0; i < params_.q; ++i) {
                combined[i] = combined[i] || mesh_array[i];
            }
        }

        // 计算覆盖率（true的数量占总数的比例）
        int coverage_count = 0;
        for (bool covered : combined) {
            if (covered) coverage_count++;
        }

        return static_cast<double>(coverage_count) / params_.q;
    }
}

double SatelliteSchedulerSolution::calculateVariance(const std::vector<double>& data) {
    if (data.empty()) return 0.0;

    double mean = std::accumulate(data.begin(), data.end(), 0.0) / data.size();

    double variance = 0.0;
    for (double val : data) {
        variance += (val - mean) * (val - mean);
    }

    return variance / data.size();
}

bool SatelliteSchedulerSolution::checkSwitches(const std::vector<std::vector<int>>& solution) {
    for (int sat = 0; sat < m_; ++sat) {
        int switches = 0;
        const auto& sat_schedule = solution[sat];

        for (size_t j = 1; j < sat_schedule.size(); ++j) {
            // 跳过无效时间窗口
            if (sat_schedule[j] == -1 || sat_schedule[j - 1] == -1) {
                switches = 0;
                continue;
            }

            // 检查状态切换
            if (sat_schedule[j] != sat_schedule[j - 1]) {
                switches++;
                if (switches > params_.max_switches) {
                    return false;
                }
            }
        }
    }

    return true;
}

void SatelliteSchedulerSolution::updateBounds(const EvaluationResult& result) {
    f1_bounds_.min_val = std::min(f1_bounds_.min_val, result.satellite_count);
    f1_bounds_.max_val = std::max(f1_bounds_.max_val, result.satellite_count);

    f2_bounds_.min_val = std::min(f2_bounds_.min_val, result.coverage);
    f2_bounds_.max_val = std::max(f2_bounds_.max_val, result.coverage);

    f3_bounds_.min_val = std::min(f3_bounds_.min_val, result.load_variance);
    f3_bounds_.max_val = std::max(f3_bounds_.max_val, result.load_variance);
}

SatelliteSchedulerSolution::EvaluationResult SatelliteSchedulerSolution::normalizeResult(
    const EvaluationResult& result) {

    EvaluationResult norm_result;

    const double epsilon = 1e-10;

    // 归一化卫星数量（最小化）
    double f1_range = f1_bounds_.max_val - f1_bounds_.min_val + epsilon;
    norm_result.satellite_count = (result.satellite_count - f1_bounds_.min_val) / f1_range;

    // 归一化覆盖率（最大化）
    double f2_range = f2_bounds_.max_val - f2_bounds_.min_val + epsilon;
    norm_result.coverage = (result.coverage - f2_bounds_.min_val) / f2_range;

    // 归一化负载方差（最小化）
    double f3_range = f3_bounds_.max_val - f3_bounds_.min_val + epsilon;
    norm_result.load_variance = (result.load_variance - f3_bounds_.min_val) / f3_range;

    return norm_result;
}