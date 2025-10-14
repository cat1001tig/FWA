#pragma once
// csv_parser.h
#pragma once
#include <vector>
#include <string>
#include <sstream>
#include <fstream>

class CSVParser {
public:
    static std::vector<std::vector<std::string>> parseCSV(const std::string& filename, char delimiter = ',') {
        std::vector<std::vector<std::string>> data;
        std::ifstream file(filename);

        if (!file.is_open()) {
            throw std::runtime_error("�޷����ļ�: " + filename);
        }

        std::string line;
        while (std::getline(file, line)) {
            // ������ܵ����Ű�Χ���ֶ�
            std::vector<std::string> row = parseCSVLine(line, delimiter);
            data.push_back(row);
        }

        file.close();
        return data;
    }

    static std::vector<std::string> parseCSVLine(const std::string& line, char delimiter = ',') {
        std::vector<std::string> result;
        std::stringstream ss(line);
        std::string field;
        bool in_quotes = false;
        char prev_char = '\0';

        for (char c : line) {
            if (c == '"' && prev_char != '\\') {
                in_quotes = !in_quotes;
            }
            else if (c == delimiter && !in_quotes) {
                result.push_back(field);
                field.clear();
            }
            else {
                field += c;
            }
            prev_char = c;
        }

        // ������һ���ֶ�
        result.push_back(field);

        // ȥ���ֶ���Χ������
        for (auto& field : result) {
            if (field.size() >= 2 && field.front() == '"' && field.back() == '"') {
                field = field.substr(1, field.size() - 2);
            }
        }

        return result;
    }
};