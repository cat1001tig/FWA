// satellite_scheduler_multiobjective.cpp
#include "satellite_scheduler_multiobjective.h"
#include <algorithm>
#include <limits>
#include <iostream>

SatelliteSchedulerMultiObjective::SatelliteSchedulerMultiObjective() {
    // ���캯��
}

std::unordered_map<int, std::vector<std::vector<std::vector<int>>>>
SatelliteSchedulerMultiObjective::fastNonDominatedSort(
    const std::vector<std::vector<std::vector<int>>>& solutions) {

    std::unordered_map<int, std::vector<std::vector<std::vector<int>>>> fronts;
    fronts[1] = {};

    std::vector<int> domination_count(solutions.size(), 0);
    std::vector<std::vector<int>> dominated_set(solutions.size());

    // �������н��Ŀ��ֵ
    std::vector<std::vector<double>> objectives;
    for (const auto& sol : solutions) {
        auto eval_result = evaluate(sol, false);
        objectives.push_back({
            eval_result.satellite_count,    // f1: ��С��
            -eval_result.coverage,          // f2: ��� -> ȡ����С��
            eval_result.load_variance       // f3: ��С��
            });
    }

    // ����֧���ϵ
    for (size_t i = 0; i < solutions.size(); ++i) {
        for (size_t j = 0; j < solutions.size(); ++j) {
            if (i == j) continue;

            if (dominates(objectives[i], objectives[j])) {
                dominated_set[i].push_back(j);
            }
            else if (dominates(objectives[j], objectives[i])) {
                domination_count[i]++;
            }
        }

        if (domination_count[i] == 0) {
            fronts[1].push_back(solutions[i]);
        }
    }

    // ��������ǰ��
    int current_rank = 1;
    while (!fronts[current_rank].empty()) {
        std::vector<std::vector<std::vector<int>>> next_front;

        for (const auto& sol : fronts[current_rank]) {
            // �ҵ���ǰ����ԭʼ�б��е�����
            auto it = std::find(solutions.begin(), solutions.end(), sol);
            if (it != solutions.end()) {
                int i = std::distance(solutions.begin(), it);

                for (int j : dominated_set[i]) {
                    domination_count[j]--;
                    if (domination_count[j] == 0) {
                        next_front.push_back(solutions[j]);
                    }
                }
            }
        }

        current_rank++;
        if (!next_front.empty()) {
            fronts[current_rank] = next_front;
        }
        else {
            break;
        }
    }

    return fronts;
}

std::vector<std::vector<std::vector<int>>>
SatelliteSchedulerMultiObjective::crowdingSelection(
    const std::vector<std::vector<std::vector<int>>>& solutions,
    int select_num) {

    if (solutions.size() <= select_num) {
        return solutions;
    }

    // �������н��Ŀ��ֵ
    std::vector<SolutionWithObjectives> sols_with_obj;
    for (const auto& sol : solutions) {
        SolutionWithObjectives sol_obj;
        sol_obj.solution = sol;
        sol_obj.objectives = solutionToObjectives(sol);
        sols_with_obj.push_back(sol_obj);
    }

    // ����ӵ������
    for (int m = 0; m < 3; ++m) {
        // ��Ŀ��m����
        std::sort(sols_with_obj.begin(), sols_with_obj.end(),
            [m](const SolutionWithObjectives& a, const SolutionWithObjectives& b) {
                return a.objectives[m] < b.objectives[m];
            });

        // ���ñ߽���ӵ������Ϊ�����
        sols_with_obj.front().crowding_distance = std::numeric_limits<double>::infinity();
        sols_with_obj.back().crowding_distance = std::numeric_limits<double>::infinity();

        // �����м���ӵ������
        double min_obj = sols_with_obj.front().objectives[m];
        double max_obj = sols_with_obj.back().objectives[m];
        double range = max_obj - min_obj;

        if (range > 0) {
            for (size_t i = 1; i < sols_with_obj.size() - 1; ++i) {
                double distance = (sols_with_obj[i + 1].objectives[m] - sols_with_obj[i - 1].objectives[m]) / range;
                sols_with_obj[i].crowding_distance += distance;
            }
        }
    }

    // ��ӵ���������򣨽���
    std::sort(sols_with_obj.begin(), sols_with_obj.end(),
        [](const SolutionWithObjectives& a, const SolutionWithObjectives& b) {
            return a.crowding_distance > b.crowding_distance;
        });

    // ѡ��ǰselect_num����
    std::vector<std::vector<std::vector<int>>> selected;
    for (int i = 0; i < select_num && i < sols_with_obj.size(); ++i) {
        selected.push_back(sols_with_obj[i].solution);
    }

    return selected;
}

bool SatelliteSchedulerMultiObjective::dominates(
    const std::vector<double>& obj_a, const std::vector<double>& obj_b) {

    // ����Ŀ�궼������b������Ŀ�궼����С����
    bool cond1 = (obj_a[0] <= obj_b[0]) && (obj_a[1] <= obj_b[1]) && (obj_a[2] <= obj_b[2]);

    // ������һ��Ŀ���ϸ�����b
    bool cond2 = (obj_a[0] < obj_b[0]) || (obj_a[1] < obj_b[1]) || (obj_a[2] < obj_b[2]);

    return cond1 && cond2;
}

std::pair<std::vector<std::vector<std::vector<int>>>,
    std::vector<std::vector<std::vector<int>>>>
    SatelliteSchedulerMultiObjective::optimize(int max_iter, int num_fireworks, int max_changes) {

    // ��ʼ���̻���Ⱥ
    std::vector<std::vector<std::vector<int>>> fireworks;
    for (int i = 0; i < num_fireworks; ++i) {
        fireworks.push_back(initializeSolution());
    }

    std::vector<std::vector<std::vector<int>>> all_candidates;

    std::cout << "��ʼ�̻��㷨�Ż�����������: " << max_iter
        << ", �̻�����: " << num_fireworks << std::endl;

    for (int iter = 0; iter < max_iter; ++iter) {
        std::vector<std::vector<std::vector<int>>> sparks;

        // ������Ӧ��ͳ����Ϣ
        double value_sum = 0.0;
        double value_min = std::numeric_limits<double>::max();
        double value_max = std::numeric_limits<double>::lowest();
        int size = fireworks.size();

        for (const auto& fw : fireworks) {
            auto eval_result = evaluate(fw, true);
            double value = weights_[0] * -eval_result.satellite_count +
                weights_[1] * eval_result.coverage +
                weights_[2] * -eval_result.load_variance;

            value_min = std::min(value_min, value);
            value_max = std::max(value_max, value);
            value_sum += value;
        }

        // Ϊÿ���̻�������
        for (const auto& fw : fireworks) {
            auto eval_result = evaluate(fw, true);
            double value = weights_[0] * -eval_result.satellite_count +
                weights_[1] * eval_result.coverage +
                weights_[2] * -eval_result.load_variance;

            // ��������Ļ�����
            int num = static_cast<int>(std::round(
                max_sparks_ * (value - value_min + std::numeric_limits<double>::epsilon()) /
                (value_sum - size * value_min + std::numeric_limits<double>::epsilon())));

            // ȷ�����ٲ���һ����
            num = std::max(1, num);

            // ���ѡ���޸ĵ���������
            int max_changes_for_firework = randomInt(1, max_changes);

            // ������
            auto new_sparks = explode(fw, num, max_changes_for_firework,
                value_max, value_sum, size,
                std::numeric_limits<double>::epsilon());

            sparks.insert(sparks.end(), new_sparks.begin(), new_sparks.end());
        }

        // �ϲ���ѡ��
        auto candidates = fireworks;
        candidates.insert(candidates.end(), sparks.begin(), sparks.end());
        all_candidates = candidates; // �������к�ѡ��

        // ����л�Ƶ��Լ��
        std::vector<std::vector<std::vector<int>>> valid_candidates;
        for (const auto& candidate : candidates) {
            if (checkSwitches(candidate)) {
                valid_candidates.push_back(candidate);
            }
        }

        // ���ٷ�֧������
        auto ranked = fastNonDominatedSort(valid_candidates);

        // ѡ����һ��
        std::vector<std::vector<std::vector<int>>> selected;
        int current_rank = 1;

        while (selected.size() < num_fireworks + max_sparks_ &&
            ranked.find(current_rank) != ranked.end()) {

            int needed = num_fireworks + max_sparks_ - selected.size();
            auto& available = ranked[current_rank];

            if (available.size() > needed) {
                auto selected_from_rank = crowdingSelection(available, needed);
                selected.insert(selected.end(), selected_from_rank.begin(), selected_from_rank.end());
            }
            else {
                selected.insert(selected.end(), available.begin(), available.end());
            }

            current_rank++;
        }

        fireworks = selected;

        // ���������Ϣ
        if (iter % 1 == 0) {
            std::cout << "���� " << iter << "/" << max_iter;
            if (ranked.find(1) != ranked.end() && !ranked[1].empty()) {
                auto best = ranked[1][0];
                auto best_eval = evaluate(best, false);
                std::cout << " - ��ѽ�: ����=" << best_eval.satellite_count
                    << ", ������=" << best_eval.coverage / 100 << "%"
                    << ", ƽ��ÿʱ��Ƭ������=" << best_eval.coverage / bounds_.size()
                    << ", ����=" << best_eval.load_variance;
            }
            std::cout << std::endl;
        }
    }

    // ��������
    auto final_ranking = fastNonDominatedSort(fireworks);
    auto all_ranking = fastNonDominatedSort(all_candidates);

    std::vector<std::vector<std::vector<int>>> best_solutions;
    if (final_ranking.find(1) != final_ranking.end()) {
        best_solutions = final_ranking[1];
    }

    std::vector<std::vector<std::vector<int>>> all_solutions;
    if (all_ranking.find(1) != all_ranking.end()) {
        all_solutions = all_ranking[1];
    }

    std::cout << "�Ż���ɣ��ҵ� " << best_solutions.size() << " �����Ž�" << std::endl;

    return { best_solutions, all_solutions };
}

std::vector<double> SatelliteSchedulerMultiObjective::solutionToObjectives(
    const std::vector<std::vector<int>>& solution) {

    auto eval_result = evaluate(solution, false);
    return {
        eval_result.satellite_count,    // f1: ��С��
        -eval_result.coverage,          // f2: ��� -> ȡ����С��  
        eval_result.load_variance       // f3: ��С��
    };
}

double SatelliteSchedulerMultiObjective::calculateCrowdingDistance(
    const std::vector<SolutionWithObjectives>& solutions,
    int solution_idx, int objective_idx) {

    if (solution_idx == 0 || solution_idx == solutions.size() - 1) {
        return std::numeric_limits<double>::infinity();
    }

    double prev_obj = solutions[solution_idx - 1].objectives[objective_idx];
    double next_obj = solutions[solution_idx + 1].objectives[objective_idx];

    return next_obj - prev_obj;
}