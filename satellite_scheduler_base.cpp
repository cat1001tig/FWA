// satellite_scheduler_base.cpp
#include "satellite_scheduler_base.h"
#include <fstream>
#include <sstream>
#include <iostream>
#include <algorithm>

SatelliteSchedulerBase::SatelliteSchedulerBase(const AlgorithmParams& params)
    : params_(params), gen_(rd_()) {
    // ���캯����ʼ��
}

bool SatelliteSchedulerBase::loadCompressedData(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "�޷���ѹ�������ļ�: " << filename << std::endl;
        return false;
    }

    std::vector<std::string> lines;
    std::string line;

    while (std::getline(file, line)) {
        if (!line.empty() && line[0] != '#') { // �������к�ע��
            lines.push_back(line);
        }
    }
    file.close();

    if (lines.size() < 2) {
        std::cerr << "ѹ�������ļ���ʽ����" << std::endl;
        return false;
    }

    // ��һ����bounds��������compressed����
    std::string bounds_line = lines[0];
    std::vector<std::string> compressed_lines(lines.begin() + 1, lines.end());

    bounds_ = parseBoundsData(bounds_line);
    compressed_ = parseCompressedData(compressed_lines);
    m_ = compressed_.size();

    std::cout << "����ѹ���������: " << m_ << " ������, "
        << (compressed_.empty() ? 0 : compressed_[0].size()) << " ��ʱ�䴰��" << std::endl;

    return true;
}

void SatelliteSchedulerBase::initializeCoverageLoader() {
    coverage_loader_ = std::make_unique<CoverageDataLoader>();

    // Ԥ������������������ʱ��������
    std::vector<int> satellites = params_.satellites; // ���Ǳ��
    coverage_loader_->preloadAllData(satellites, params_.special_times);

    std::cout << "�����ʼ�������ʼ�����" << std::endl;
}

std::vector<int> SatelliteSchedulerBase::parseBoundsData(const std::string& bounds_line) {
    std::vector<int> bounds;
    std::stringstream ss(bounds_line);
    std::string token;

    while (std::getline(ss, token, ',')) {
        try {
            bounds.push_back(std::stoi(token));
        }
        catch (const std::exception& e) {
            std::cerr << "����bounds���ݴ���: " << token << " - " << e.what() << std::endl;
        }
    }

    return bounds;
}

std::vector<std::vector<int>> SatelliteSchedulerBase::parseCompressedData(
    const std::vector<std::string>& lines) {

    std::vector<std::vector<int>> compressed;

    for (const auto& line : lines) {
        std::vector<int> satellite_schedule;
        std::stringstream ss(line);
        std::string token;

        while (std::getline(ss, token, ',')) {
            try {
                satellite_schedule.push_back(std::stoi(token));
            }
            catch (const std::exception& e) {
                std::cerr << "����ѹ�����ݴ���: " << token << " - " << e.what() << std::endl;
                satellite_schedule.push_back(-1); // ����ʱ��Ϊ��Чֵ
            }
        }

        if (!satellite_schedule.empty()) {
            compressed.push_back(satellite_schedule);
        }
    }

    return compressed;
}