// satellite_scheduler_fireworks.cpp
#include "satellite_scheduler_fireworks.h"
#include <algorithm>
#include <iostream>

SatelliteSchedulerFireworks::SatelliteSchedulerFireworks(const AlgorithmParams& params)
    : SatelliteSchedulerSolution(params) {
}

std::vector<std::vector<std::vector<int>>> SatelliteSchedulerFireworks::explode(
    const std::vector<std::vector<int>>& solution,
    int num_sparks,
    int max_changes,
    double fmax,
    double fsum,
    double size,
    double epsilon) {

    std::vector<std::vector<std::vector<int>>> sparks;

    for (int spark_idx = 0; spark_idx < num_sparks; ++spark_idx) {
        // 复制原解
        auto new_sol = copySolution(solution);

        // 评估当前解
        auto eval_result = evaluate(new_sol, true);
        double value = params_.weights[0] * -eval_result.satellite_count +
            params_.weights[1] * eval_result.coverage +
            params_.weights[2] * -eval_result.load_variance;

        // 计算概率p
        double p = (fmax - value + epsilon) / (fmax * size - fsum + epsilon);

        int max_length = 0;

        // 爆炸算子：随机选择卫星进行修改
        std::vector<bool> if_sat(m_, false);

        for (int change = 0; change < max_changes; ++change) {
            // 选择卫星
            int sat;
            if (std::all_of(if_sat.begin(), if_sat.end(), [](bool v) { return v; })) {
                sat = randomSatellite();
            }
            else {
                // 选择尚未修改的卫星
                std::vector<int> available;
                for (int i = 0; i < m_; ++i) {
                    if (!if_sat[i]) {
                        available.push_back(i);
                    }
                }
                sat = available[randomInt(0, available.size() - 1)];
            }
            if_sat[sat] = true;

            // 寻找最长连续序列
            int longest_seq = findLongestSequence(new_sol[sat]);
            max_length = std::min(longest_seq, params_.max_length);

            // 选择时间窗口进行修改
            if (max_length >= 2) {
                int n = randomInt(2, max_length); // 修改的连续窗口长度

                // 查找连续序列的结束位置
                auto intervals = findValidIntervals(new_sol[sat]);
                if (!intervals.empty()) {
                    // 选择最长的区间
                    auto& longest_interval = intervals[0];
                    for (const auto& interval : intervals) {
                        if (interval.second - interval.first + 1 > longest_interval.second - longest_interval.first + 1) {
                            longest_interval = interval;
                        }
                    }

                    int start = longest_interval.second - n + 1;
                    int end = longest_interval.second;

                    // 确保范围有效
                    start = std::max(start, longest_interval.first);
                    end = std::min(end, longest_interval.second);

                    // 根据概率p决定设置的值
                    int set_value;
                    if (randomDouble() >= p) {
                        set_value = randomInt(0, 1);
                    }
                    else {
                        set_value = randomBool() ? 0 : 1;
                    }

                    // 修改选中的时间窗口
                    for (int k = start; k <= end; ++k) {
                        new_sol[sat][k] = set_value;
                    }
                }
            }
        }

        // 变异算子：三种变异方式
        int rand_choice = randomInt(0, 2);
        applyRandomMutation(new_sol, rand_choice);

        // 定向休眠变异算子
        int variation_sat = randomSatellite();
        applyDirectedSleepMutation(new_sol, variation_sat, max_length);

        sparks.push_back(new_sol);
    }

    return sparks;
}

std::vector<std::pair<int, int>> SatelliteSchedulerFireworks::findValidIntervals(
    const std::vector<int>& schedule) {

    std::vector<std::pair<int, int>> intervals;
    int start = -1;

    for (size_t j = 0; j < schedule.size(); ++j) {
        if (schedule[j] != -1) {
            if (start == -1) {
                start = j;
            }
        }
        else {
            if (start != -1) {
                intervals.emplace_back(start, j - 1);
                start = -1;
            }
        }
    }

    if (start != -1) {
        intervals.emplace_back(start, schedule.size() - 1);
    }

    return intervals;
}

int SatelliteSchedulerFireworks::findLongestSequence(const std::vector<int>& schedule) {
    int longest_seq = 0;
    int current_seq = 0;

    for (int val : schedule) {
        if (val != -1) {
            current_seq++;
            longest_seq = std::max(longest_seq, current_seq);
        }
        else {
            current_seq = 0;
        }
    }

    return longest_seq;
}

void SatelliteSchedulerFireworks::applyRandomMutation(
    std::vector<std::vector<int>>& solution, int mutation_type) {

    for (int sat = 0; sat < m_; ++sat) {
        auto intervals = findValidIntervals(solution[sat]);

        for (const auto& interval : intervals) {
            int start_col = interval.first;
            int end_col = interval.second;

            switch (mutation_type) {
            case 0: // 随机选点置1
                if (randomBool(0.5)) {
                    int selected = randomInt(start_col, end_col);
                    solution[sat][selected] = 1;
                }
                break;

            case 1: // 随机选点置0
                if (randomBool(0.5)) {
                    int selected = randomInt(start_col, end_col);
                    solution[sat][selected] = 0;
                }
                break;

            case 2: // 整个区间置1
                if (randomBool(0.1)) {
                    for (int k = start_col; k <= end_col; ++k) {
                        solution[sat][k] = 1;
                    }
                }
                break;
            }
        }
    }
}

void SatelliteSchedulerFireworks::applyDirectedSleepMutation(
    std::vector<std::vector<int>>& solution, int variation_sat, int max_length) {

    int actual_length = std::min(max_length, static_cast<int>(solution[variation_sat].size()));

    for (int wd = 0; wd < actual_length; ++wd) {
        if (solution[variation_sat][wd] == 1) {
            solution[variation_sat][wd] = 0; // 关闭时间窗口
        }
    }
}

// 随机数辅助函数
int SatelliteSchedulerFireworks::randomSatellite() {
    std::uniform_int_distribution<int> dist(0, m_ - 1);
    return dist(gen_);
}

int SatelliteSchedulerFireworks::randomInt(int min, int max) {
    std::uniform_int_distribution<int> dist(min, max);
    return dist(gen_);
}

double SatelliteSchedulerFireworks::randomDouble(double min, double max) {
    std::uniform_real_distribution<double> dist(min, max);
    return dist(gen_);
}

bool SatelliteSchedulerFireworks::randomBool(double probability) {
    std::bernoulli_distribution dist(probability);
    return dist(gen_);
}