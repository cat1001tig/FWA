// coverage_loader.cpp
#include "coverage_loader.h"
#include <iostream>
#include <filesystem>
#include "csv_reader_optimized.h"

CoverageDataLoader::CoverageDataLoader(const std::string& data_dir)
    : data_dir_(data_dir), gen_(rd_()), dist_(0.5) {}

void CoverageDataLoader::preloadAllData(const std::vector<int>& satellites,
    const std::vector<int>& special_times, const std::string& data_dir) {
    std::cout << "开始预加载网格数据..." << std::endl;

    int loaded_count = 0;
    int error_count = 0;
    std::string fileroute;

    for (int sat : satellites) {
        for (int time : special_times) {
            std::string filename = makeFilename(sat, time);
            if (data_dir_ != "") {
                std::filesystem::path dir(data_dir_);
                std::filesystem::path file(filename);
                std::filesystem::path fullPath = dir / file;
                fileroute = fullPath.string();
            }

            fileroute = filename;
            std::string filepath = filename;

            CacheKey key{ sat, time };

            // 检查文件是否存在
            if (std::filesystem::exists(filepath)) {
                try {
                    auto mesh_data = loadMeshFromCSV(filepath);
                    mesh_cache_[key] = mesh_data;
                    loaded_count++;
                }
                catch (const std::exception& e) {
                    std::cerr << "加载文件 " << filename << " 出错: " << e.what() << std::endl;
                    mesh_cache_[key] = generateRandomMesh(q_);
                    error_count++;
                }
            }
            else {
                // 文件不存在，生成随机数据
                mesh_cache_[key] = generateRandomMesh(q_);
                std::cout << "文件不存在，生成随机数据: " << filename << std::endl;
            }
        }
    }

    std::cout << "预加载完成，共缓存 " << mesh_cache_.size() << " 个网格数据" << std::endl;
    std::cout << "成功加载: " << loaded_count << ", 错误/缺失: " << error_count << std::endl;
}

std::vector<bool> CoverageDataLoader::getMeshData(int sat, int time, int q) {
    CacheKey key{ sat, time };
    auto it = mesh_cache_.find(key);

    if (it != mesh_cache_.end()) {
        return it->second;
    }

    // 如果缓存中没有，生成随机数据
    return generateRandomMesh(q);
}

std::vector<bool> CoverageDataLoader::loadMeshFromCSV(const std::string& filepath) {
    try {
        // 使用优化的CSV读取器
        auto mesh_data = OptimizedCSVReader::readSingleColumnCSV(filepath, q_);

        // 确保数据量正确
        if (mesh_data.size() < q_) {
            std::cout << "警告: 文件 " << filepath << " 只有 " << mesh_data.size()
                << " 行数据，需要 " << q_ << " 行，用随机数据填充" << std::endl;

            while (mesh_data.size() < q_) {
                mesh_data.push_back(dist_(gen_));
            }
        }
        else if (mesh_data.size() > q_) {
            mesh_data.resize(q_);
            std::cout << "警告: 文件 " << filepath << " 有 " << mesh_data.size()
                << " 行数据，超过需要的 " << q_ << " 行，已截断" << std::endl;
        }

        return mesh_data;

    }
    catch (const std::exception& e) {
        std::cerr << "加载CSV文件错误: " << filepath << " - " << e.what() << std::endl;
        throw; // 重新抛出异常
    }
}
std::vector<bool> CoverageDataLoader::generateRandomMesh(int q) {
    std::vector<bool> random_data;
    random_data.reserve(q);

    for (int i = 0; i < q; ++i) {
        random_data.push_back(dist_(gen_));
    }

    return random_data;
}

std::string CoverageDataLoader::makeFilename(int sat, int time) const {
    return "s" + std::to_string(sat) + "_" + std::to_string(time) + ".csv";
}