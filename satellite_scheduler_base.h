#pragma once
// satellite_scheduler_base.h
#pragma once
#include <vector>
#include <string>
#include <memory>
#include <random>
#include "coverage_loader.h"

// �㷨�����ṹ��
struct AlgorithmParams {
    int max_switches = 5;
    int q = 6665;
    std::vector<double> weights = { 0.34, 0.33, 0.33 };
    int max_sparks = 30;
    int max_length = 10;
    bool update_bounds = true;
    int max_variation = 3;
    std::vector<int> special_times = { 263, 264, 265, 266, 267, 268, 269, 270, 271, 272, 449 };
    std::vector<int> satellites = { 2, 4, 5 }; // ���Ǳ��
    int num_satellites_ = 10;
    int total_minutes_ = 721; // 12*60 + 1
    int starthour = 8; //�۲�ʱ�����ʼСʱ
    int startminute = 0; //�۲�ʱ�����ʼ����
    int startsecond = 0; //�۲�ʱ�����ʼ��
    int endhour = 20; //�۲�ʱ��Ľ���Сʱ
    int endminute = 0; //�۲�ʱ��Ľ�������
    int endsecond = 0; //�۲�ʱ��Ľ�����
    std::string directoryPath = "";//ʱ�䴰���ļ�·��
    std::string data_dir_ = "";//�ص��۲�ʱ��������������ļ�·��

    // Ĭ�Ϲ��캯��
    AlgorithmParams() = default;

    // �������Ĺ��캯��
    AlgorithmParams(int max_sw, int q_val, const std::vector<double>& w,
        int max_sp, int max_len, bool update_b, int max_var,
        const std::vector<int>& special_t)
        : max_switches(max_sw), q(q_val), weights(w), max_sparks(max_sp),
        max_length(max_len), update_bounds(update_b), max_variation(max_var),
        special_times(special_t) {}
};

class SatelliteSchedulerBase {
protected:
    std::vector<std::vector<int>> compressed_;
    std::vector<int> bounds_;
    int m_;

    // �㷨���� - ����ͨ�����캯������
    AlgorithmParams params_;

    std::unique_ptr<CoverageDataLoader> coverage_loader_;
    std::random_device rd_;
    std::mt19937 gen_;

public:
    // �޸Ĺ��캯���Խ��ܲ���
    SatelliteSchedulerBase(const AlgorithmParams& params = AlgorithmParams());
    virtual ~SatelliteSchedulerBase() = default;

    bool loadCompressedData(const std::string& filename);
    void initializeCoverageLoader();

    const std::vector<std::vector<int>>& getCompressed() const { return compressed_; }
    const std::vector<int>& getBounds() const { return bounds_; }
    int getSatelliteCount() const { return m_; }

    // ��ȡ�����ò����ĺ���
    const AlgorithmParams& getParams() const { return params_; }
    void setParams(const AlgorithmParams& params) { params_ = params; }

protected:
    std::vector<std::vector<int>> parseCompressedData(const std::vector<std::string>& lines);
    std::vector<int> parseBoundsData(const std::string& bounds_line);
};
