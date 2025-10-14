// satellite_data_loader.cpp
#include "satellite_data_loader.h"
#include <iostream>
#include <regex>
#include <stdexcept>
#include "csv_parser.h"

SatelliteDataLoader::SatelliteDataLoader() {
    // 初始化窗口矩阵
    window_.resize(num_satellites_, std::vector<int>(total_minutes_, -1));
    idx_.resize(num_satellites_);
}

int SatelliteDataLoader::parseTimeToMinutes(const std::string& timeStr) {
    auto timePoint = safeParseTime(timeStr);
    return timePoint.toMinutesSince8AM();
}

SatelliteDataLoader::TimePoint SatelliteDataLoader::safeParseTime(const std::string& timeStr) {
    if (timeStr.empty() || timeStr == "NaN" || timeStr == "nan") {
        return TimePoint(-1, -1, -1); // 无效时间标记
    }

    try {
        // 新的CSV格式："1 May 2025,08:23:00.000,6.57,6.57"
        // 我们需要提取时间部分 "08:23:00.000"

        // 按逗号分割字符串
        std::vector<std::string> parts;
        std::stringstream ss(timeStr);
        std::string part;

        while (std::getline(ss, part, ',')) {
            parts.push_back(part);
        }

        /*// 检查是否有足够的部分
        if (parts.size() < 2) {
            std::cerr << "时间格式错误，字段不足: " << timeStr << std::endl;
            return TimePoint(-1, -1, -1);
        }*/

        // 第二个部分应该是时间 "08:23:00.000"
        std::string timePart = parts[0];

        // 处理可能的毫秒格式
        std::string cleanTimeStr = timePart;
        size_t dotPos = timePart.find('.');
        if (dotPos != std::string::npos) {
            cleanTimeStr = timePart.substr(0, dotPos);
        }

        // 正则表达式匹配时间格式 HH:MM:SS
        std::regex pattern(R"((\d{1,2}):(\d{2}):(\d{2}))");
        std::smatch matches;

        if (std::regex_match(cleanTimeStr, matches, pattern)) {
            int hour = std::stoi(matches[1]);
            int minute = std::stoi(matches[2]);
            int second = std::stoi(matches[3]);

            return TimePoint(hour, minute, second);
        }
        else {
            std::cerr << "无法解析时间格式: " << cleanTimeStr << std::endl;
        }

    }
    catch (const std::exception& e) {
        std::cerr << "时间解析错误: " << timeStr << " - " << e.what() << std::endl;
    }

    return TimePoint(-1, -1, -1); // 解析失败
}

bool SatelliteDataLoader::loadDataFromExcel(const std::string& excelPath) {
    std::cout << "开始加载卫星数据..." << std::endl;

    for (int i = 1; i <= 10; ++i) {
        std::string filename = "satellite_" + std::to_string(i) + ".csv";

        try {
            auto csv_data = CSVParser::parseCSV(filename);

            // 跳过标题行（如果有）
            size_t start_row = 0;

            std::vector<double> coverage_values;
            std::vector<int> time_indices;
            int satellite_index = i - 1;

            for (size_t row = start_row; row < csv_data.size(); ++row) {
                const auto& fields = csv_data[row];

                if (fields.size() >= 3) {
                    // 提取时间部分（第二个字段）
                    std::string time_str = fields[1];

                    // 解析时间
                    int minutes = parseTimeToMinutes(time_str);
                    if (minutes >= 0) {
                        time_indices.push_back(minutes);
                    }

                    // 提取覆盖率（第三个字段）
                    std::string coverage_str = fields[2];
                    try {
                        double coverage = std::stod(coverage_str);
                        coverage_values.push_back(coverage);
                    }
                    catch (const std::exception& e) {
                        std::cerr << "覆盖率解析错误: " << coverage_str << " - " << e.what() << std::endl;
                    }
                }
            }

            // 存储数据
            idx_[satellite_index] = time_indices;
            coverage_data_[satellite_index] = coverage_values;

            // 填充窗口矩阵
            for (int time_index : time_indices) {
                if (time_index >= 0 && time_index < total_minutes_) {
                    window_[satellite_index][time_index] = 1;
                }
            }

            std::cout << "卫星 " << i << " 数据加载完成，时间点数量: " << time_indices.size()
                << ", 覆盖率数据数量: " << coverage_values.size() << std::endl;

        }
        catch (const std::exception& e) {
            std::cerr << "加载文件 " << filename << " 错误: " << e.what() << std::endl;
            continue;
        }
    }

    // 压缩时间窗口
    compressTimeWindows();

    std::cout << "所有卫星数据加载完成" << std::endl;
    return true;
}

void SatelliteDataLoader::compressTimeWindows() {
    std::cout << "开始压缩时间窗口..." << std::endl;

    // 找到所有为-1的列
    std::vector<bool> all_minus_one_columns(total_minutes_, true);

    for (int col = 0; col < total_minutes_; ++col) {
        for (int row = 0; row < num_satellites_; ++row) {
            if (window_[row][col] != -1) {
                all_minus_one_columns[col] = false;
                break;
            }
        }
    }

    // 收集非全-1列的索引
    for (int col = 0; col < total_minutes_; ++col) {
        if (!all_minus_one_columns[col]) {
            bounds_.push_back(col);
        }
    }

    // 创建压缩矩阵
    compressed_.resize(num_satellites_);
    for (int row = 0; row < num_satellites_; ++row) {
        compressed_[row].reserve(bounds_.size());
        for (int bound : bounds_) {
            compressed_[row].push_back(window_[row][bound]);
        }
    }

    std::cout << "时间窗口压缩完成，原始列数: " << total_minutes_
        << ", 压缩后列数: " << bounds_.size() << std::endl;
}

void SatelliteDataLoader::saveCompressedData(const std::string& filename) {
    std::ofstream file(filename);

    if (!file.is_open()) {
        std::cerr << "无法创建文件: " << filename << std::endl;
        return;
    }

    // 保存bounds数据
    file << "# bounds\n";
    for (size_t i = 0; i < bounds_.size(); ++i) {
        file << bounds_[i];
        if (i < bounds_.size() - 1) file << ",";
    }
    file << "\n";

    // 保存compressed数据
    file << "# compressed\n";
    for (const auto& sat_schedule : compressed_) {
        for (size_t i = 0; i < sat_schedule.size(); ++i) {
            file << sat_schedule[i];
            if (i < sat_schedule.size() - 1) file << ",";
        }
        file << "\n";
    }

    file.close();
    std::cout << "压缩数据已保存到: " << filename << std::endl;
}