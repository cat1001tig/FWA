// satellite_scheduler_base.cpp
#include "satellite_scheduler_base.h"
#include <fstream>
#include <sstream>
#include <iostream>
#include <algorithm>

SatelliteSchedulerBase::SatelliteSchedulerBase(const AlgorithmParams& params)
    : params_(params), gen_(rd_()) {
    // 构造函数初始化
}

bool SatelliteSchedulerBase::loadCompressedData(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "无法打开压缩数据文件: " << filename << std::endl;
        return false;
    }

    std::vector<std::string> lines;
    std::string line;

    while (std::getline(file, line)) {
        if (!line.empty() && line[0] != '#') { // 跳过空行和注释
            lines.push_back(line);
        }
    }
    file.close();

    if (lines.size() < 2) {
        std::cerr << "压缩数据文件格式错误" << std::endl;
        return false;
    }

    // 第一行是bounds，其余是compressed数据
    std::string bounds_line = lines[0];
    std::vector<std::string> compressed_lines(lines.begin() + 1, lines.end());

    bounds_ = parseBoundsData(bounds_line);
    compressed_ = parseCompressedData(compressed_lines);
    m_ = compressed_.size();

    std::cout << "加载压缩数据完成: " << m_ << " 颗卫星, "
        << (compressed_.empty() ? 0 : compressed_[0].size()) << " 个时间窗口" << std::endl;

    return true;
}

void SatelliteSchedulerBase::initializeCoverageLoader() {
    coverage_loader_ = std::make_unique<CoverageDataLoader>();

    // 预加载特殊卫星在特殊时间点的数据
    std::vector<int> satellites = params_.satellites; // 卫星编号
    coverage_loader_->preloadAllData(satellites, params_.special_times);

    std::cout << "覆盖率加载器初始化完成" << std::endl;
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
            std::cerr << "解析bounds数据错误: " << token << " - " << e.what() << std::endl;
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
                std::cerr << "解析压缩数据错误: " << token << " - " << e.what() << std::endl;
                satellite_schedule.push_back(-1); // 错误时设为无效值
            }
        }

        if (!satellite_schedule.empty()) {
            compressed.push_back(satellite_schedule);
        }
    }

    return compressed;
}