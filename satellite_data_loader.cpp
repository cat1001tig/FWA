// satellite_data_loader.cpp
#include "satellite_data_loader.h"
#include <iostream>
#include <regex>
#include <stdexcept>
#include "csv_parser.h"

SatelliteDataLoader::SatelliteDataLoader() {
    // ��ʼ�����ھ���
    window_.resize(num_satellites_, std::vector<int>(total_minutes_, -1));
    idx_.resize(num_satellites_);
}

int SatelliteDataLoader::parseTimeToMinutes(const std::string& timeStr) {
    auto timePoint = safeParseTime(timeStr);
    return timePoint.toMinutesSince8AM();
}

SatelliteDataLoader::TimePoint SatelliteDataLoader::safeParseTime(const std::string& timeStr) {
    if (timeStr.empty() || timeStr == "NaN" || timeStr == "nan") {
        return TimePoint(-1, -1, -1); // ��Чʱ����
    }

    try {
        // �µ�CSV��ʽ��"1 May 2025,08:23:00.000,6.57,6.57"
        // ������Ҫ��ȡʱ�䲿�� "08:23:00.000"

        // �����ŷָ��ַ���
        std::vector<std::string> parts;
        std::stringstream ss(timeStr);
        std::string part;

        while (std::getline(ss, part, ',')) {
            parts.push_back(part);
        }

        /*// ����Ƿ����㹻�Ĳ���
        if (parts.size() < 2) {
            std::cerr << "ʱ���ʽ�����ֶβ���: " << timeStr << std::endl;
            return TimePoint(-1, -1, -1);
        }*/

        // �ڶ�������Ӧ����ʱ�� "08:23:00.000"
        std::string timePart = parts[0];

        // ������ܵĺ����ʽ
        std::string cleanTimeStr = timePart;
        size_t dotPos = timePart.find('.');
        if (dotPos != std::string::npos) {
            cleanTimeStr = timePart.substr(0, dotPos);
        }

        // ������ʽƥ��ʱ���ʽ HH:MM:SS
        std::regex pattern(R"((\d{1,2}):(\d{2}):(\d{2}))");
        std::smatch matches;

        if (std::regex_match(cleanTimeStr, matches, pattern)) {
            int hour = std::stoi(matches[1]);
            int minute = std::stoi(matches[2]);
            int second = std::stoi(matches[3]);

            return TimePoint(hour, minute, second);
        }
        else {
            std::cerr << "�޷�����ʱ���ʽ: " << cleanTimeStr << std::endl;
        }

    }
    catch (const std::exception& e) {
        std::cerr << "ʱ���������: " << timeStr << " - " << e.what() << std::endl;
    }

    return TimePoint(-1, -1, -1); // ����ʧ��
}

bool SatelliteDataLoader::loadDataFromExcel(const std::string& excelPath) {
    std::cout << "��ʼ������������..." << std::endl;

    for (int i = 1; i <= 10; ++i) {
        std::string filename = "satellite_" + std::to_string(i) + ".csv";

        try {
            auto csv_data = CSVParser::parseCSV(filename);

            // ���������У�����У�
            size_t start_row = 0;

            std::vector<double> coverage_values;
            std::vector<int> time_indices;
            int satellite_index = i - 1;

            for (size_t row = start_row; row < csv_data.size(); ++row) {
                const auto& fields = csv_data[row];

                if (fields.size() >= 3) {
                    // ��ȡʱ�䲿�֣��ڶ����ֶΣ�
                    std::string time_str = fields[1];

                    // ����ʱ��
                    int minutes = parseTimeToMinutes(time_str);
                    if (minutes >= 0) {
                        time_indices.push_back(minutes);
                    }

                    // ��ȡ�����ʣ��������ֶΣ�
                    std::string coverage_str = fields[2];
                    try {
                        double coverage = std::stod(coverage_str);
                        coverage_values.push_back(coverage);
                    }
                    catch (const std::exception& e) {
                        std::cerr << "�����ʽ�������: " << coverage_str << " - " << e.what() << std::endl;
                    }
                }
            }

            // �洢����
            idx_[satellite_index] = time_indices;
            coverage_data_[satellite_index] = coverage_values;

            // ��䴰�ھ���
            for (int time_index : time_indices) {
                if (time_index >= 0 && time_index < total_minutes_) {
                    window_[satellite_index][time_index] = 1;
                }
            }

            std::cout << "���� " << i << " ���ݼ�����ɣ�ʱ�������: " << time_indices.size()
                << ", ��������������: " << coverage_values.size() << std::endl;

        }
        catch (const std::exception& e) {
            std::cerr << "�����ļ� " << filename << " ����: " << e.what() << std::endl;
            continue;
        }
    }

    // ѹ��ʱ�䴰��
    compressTimeWindows();

    std::cout << "�����������ݼ������" << std::endl;
    return true;
}

void SatelliteDataLoader::compressTimeWindows() {
    std::cout << "��ʼѹ��ʱ�䴰��..." << std::endl;

    // �ҵ�����Ϊ-1����
    std::vector<bool> all_minus_one_columns(total_minutes_, true);

    for (int col = 0; col < total_minutes_; ++col) {
        for (int row = 0; row < num_satellites_; ++row) {
            if (window_[row][col] != -1) {
                all_minus_one_columns[col] = false;
                break;
            }
        }
    }

    // �ռ���ȫ-1�е�����
    for (int col = 0; col < total_minutes_; ++col) {
        if (!all_minus_one_columns[col]) {
            bounds_.push_back(col);
        }
    }

    // ����ѹ������
    compressed_.resize(num_satellites_);
    for (int row = 0; row < num_satellites_; ++row) {
        compressed_[row].reserve(bounds_.size());
        for (int bound : bounds_) {
            compressed_[row].push_back(window_[row][bound]);
        }
    }

    std::cout << "ʱ�䴰��ѹ����ɣ�ԭʼ����: " << total_minutes_
        << ", ѹ��������: " << bounds_.size() << std::endl;
}

void SatelliteDataLoader::saveCompressedData(const std::string& filename) {
    std::ofstream file(filename);

    if (!file.is_open()) {
        std::cerr << "�޷������ļ�: " << filename << std::endl;
        return;
    }

    // ����bounds����
    file << "# bounds\n";
    for (size_t i = 0; i < bounds_.size(); ++i) {
        file << bounds_[i];
        if (i < bounds_.size() - 1) file << ",";
    }
    file << "\n";

    // ����compressed����
    file << "# compressed\n";
    for (const auto& sat_schedule : compressed_) {
        for (size_t i = 0; i < sat_schedule.size(); ++i) {
            file << sat_schedule[i];
            if (i < sat_schedule.size() - 1) file << ",";
        }
        file << "\n";
    }

    file.close();
    std::cout << "ѹ�������ѱ��浽: " << filename << std::endl;
}