#pragma once
// satellite_scheduler_multiobjective.h
#pragma once
#include "satellite_scheduler_fireworks.h"

class SatelliteSchedulerMultiObjective : public SatelliteSchedulerFireworks {
public:
    SatelliteSchedulerMultiObjective();

    // 多目标优化
    std::unordered_map<int, std::vector<std::vector<std::vector<int>>>>
        fastNonDominatedSort(const std::vector<std::vector<std::vector<int>>>& solutions);

    std::vector<std::vector<std::vector<int>>>
        crowdingSelection(const std::vector<std::vector<std::vector<int>>>& solutions,
            int select_num);

    // 支配关系判断
    bool dominates(const std::vector<double>& obj_a, const std::vector<double>& obj_b);

    // 完整的优化流程
    std::pair<std::vector<std::vector<std::vector<int>>>,
        std::vector<std::vector<std::vector<int>>>>
        optimize(int max_iter = 100, int num_fireworks = 20, int max_changes = 10);

private:
    struct SolutionWithObjectives {
        std::vector<std::vector<int>> solution;
        std::vector<double> objectives;
        double crowding_distance = 0.0;
    };

    std::vector<double> solutionToObjectives(const std::vector<std::vector<int>>& solution);
    double calculateCrowdingDistance(const std::vector<SolutionWithObjectives>& solutions,
        int solution_idx, int objective_idx);
};