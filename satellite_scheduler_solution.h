#pragma once
// satellite_scheduler_solution.h
#include "satellite_scheduler_base.h"
#include <map>
#include <vector>

class SatelliteSchedulerSolution : public SatelliteSchedulerBase {
public:
    SatelliteSchedulerSolution();

    // �������ݷ���
    void setCoverageData(const std::map<int, std::vector<double>>& coverage_data);
    void setTimeIndices(const std::vector<std::vector<int>>& time_indices);

    // �����������
    std::vector<std::vector<int>> initializeSolution();
    std::vector<std::vector<int>> copySolution(const std::vector<std::vector<int>>& solution);

    // ��������
    struct EvaluationResult {
        double satellite_count;
        double coverage;
        double load_variance;
    };

    EvaluationResult evaluate(const std::vector<std::vector<int>>& solution, bool return_norm = true);

    // Լ�����
    bool checkSwitches(const std::vector<std::vector<int>>& solution);

protected:
    // �����ʼ��� - �޸�Ϊ�޲����汾
    double calculateCoverage(const std::vector<std::vector<int>>& solution, int j_original);

    // ������������
    EvaluationResult evaluateRaw(const std::vector<std::vector<int>>& solution);
    double calculateVariance(const std::vector<double>& data);

    // ���ݳ�Ա
    std::map<int, std::vector<double>> coverage_data_;  // ÿ�����ǵĸ���������
    std::vector<std::vector<int>> idx_;                 // ʱ������

    // ��һ���߽�
    struct NormBounds {
        double min_val = 0.0;
        double max_val = 1.0;
    };

    NormBounds f1_bounds_, f2_bounds_, f3_bounds_;

    void updateBounds(const EvaluationResult& result);
    EvaluationResult normalizeResult(const EvaluationResult& result);
};