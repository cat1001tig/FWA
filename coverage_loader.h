#pragma once
// coverage_loader.h
#pragma once
#include <vector>
#include <string>
#include <unordered_map>
#include <memory>
#include <fstream>
#include <sstream>
#include <random>

class CoverageDataLoader {
private:
    struct CacheKey {
        int satellite;
        int time_point;

        bool operator==(const CacheKey& other) const {
            return satellite == other.satellite && time_point == other.time_point;
        }
    };

    struct CacheKeyHash {
        std::size_t operator()(const CacheKey& key) const {
            return std::hash<int>()(key.satellite) ^ (std::hash<int>()(key.time_point) << 1);
        }
    };

public:
    CoverageDataLoader(const std::string& data_dir = "mesh_data");
    void preloadAllData(const std::vector<int>& satellites, const std::vector<int>& special_times);
    std::vector<bool> getMeshData(int sat, int time, int q);

    int getQ() const { return q_; }

private:
    std::vector<bool> loadMeshFromCSV(const std::string& filepath);
    std::vector<bool> generateRandomMesh(int q);
    std::string makeFilename(int sat, int time) const;

    std::string data_dir_;
    std::unordered_map<CacheKey, std::vector<bool>, CacheKeyHash> mesh_cache_;
    const int q_ = 6665;

    // 随机数生成器
    std::random_device rd_;
    std::mt19937 gen_;
    std::bernoulli_distribution dist_;
};