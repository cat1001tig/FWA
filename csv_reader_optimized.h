#pragma once
// csv_reader_optimized.h
#pragma once
#include <vector>
#include <string>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <cctype>

class OptimizedCSVReader {
public:
    static std::vector<bool> readSingleColumnCSV(const std::string& filepath, int expected_rows = -1) {
        std::vector<bool> data;
        std::ifstream file(filepath);

        if (!file.is_open()) {
            throw std::runtime_error("无法打开文件: " + filepath);
        }

        if (expected_rows > 0) {
            data.reserve(expected_rows);
        }

        std::string line;
        int row_count = 0;

        while (std::getline(file, line)) {
            // 清理行
            line = trim(line);

            if (line.empty()) {
                continue; // 跳过空行
            }

            // 处理可能的引号
            if (line.front() == '"' && line.back() == '"') {
                line = line.substr(1, line.size() - 2);
            }

            // 尝试解析为各种格式
            bool value = parseBoolValue(line);
            data.push_back(value);
            row_count++;

            // 如果指定了期望行数且已达到，提前退出
            if (expected_rows > 0 && row_count >= expected_rows) {
                break;
            }
        }

        file.close();
        return data;
    }

private:
    static std::string trim(const std::string& str) {
        size_t start = str.find_first_not_of(" \t\r\n");
        if (start == std::string::npos) {
            return "";
        }

        size_t end = str.find_last_not_of(" \t\r\n");
        return str.substr(start, end - start + 1);
    }

    static bool parseBoolValue(const std::string& str) {
        // 转换为小写以便比较
        std::string lower_str = str;
        std::transform(lower_str.begin(), lower_str.end(), lower_str.begin(), ::tolower);

        if (lower_str == "1" || lower_str == "true" || lower_str == "t" || lower_str == "yes" || lower_str == "y") {
            return true;
        }
        else if (lower_str == "0" || lower_str == "false" || lower_str == "f" || lower_str == "no" || lower_str == "n") {
            return false;
        }

        // 尝试解析为数字
        try {
            int value = std::stoi(str);
            return value != 0;
        }
        catch (const std::exception&) {
            // 如果所有解析都失败，使用启发式规则
            return !str.empty() && str != "0" && str != "false";
        }
    }
};