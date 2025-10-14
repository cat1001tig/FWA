#pragma once
// satellite_scheduler_fireworks.h
#pragma once
#include "satellite_scheduler_solution.h"
#include <unordered_map>

class SatelliteSchedulerFireworks : public SatelliteSchedulerSolution {
public:
    SatelliteSchedulerFireworks();

    // 爆炸和变异操作
    std::vector<std::vector<std::vector<int>>> explode(
        const std::vector<std::vector<int>>& solution,
        int num_sparks,
        int max_changes,
        double fmax,
        double fsum,
        double size,
        double epsilon);

    // 辅助函数
    std::vector<std::pair<int, int>> findValidIntervals(const std::vector<int>& schedule);
    int findLongestSequence(const std::vector<int>& schedule);

    // 变异操作
    void applyRandomMutation(std::vector<std::vector<int>>& solution, int mutation_type);
    void applyDirectedSleepMutation(std::vector<std::vector<int>>& solution, int variation_sat, int max_length);

protected:
    // 随机选择辅助函数
    int randomSatellite();
    int randomInt(int min, int max);
    double randomDouble(double min = 0.0, double max = 1.0);
    bool randomBool(double probability = 0.5);
};