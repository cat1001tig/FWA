// coverage_loader.cpp
#include "coverage_loader.h"
#include <iostream>
#include <filesystem>
#include "csv_reader_optimized.h"

CoverageDataLoader::CoverageDataLoader(const std::string& data_dir)
    : data_dir_(data_dir), gen_(rd_()), dist_(0.5) {}

void CoverageDataLoader::preloadAllData(const std::vector<int>& satellites,
    const std::vector<int>& special_times, const std::string& data_dir) {
    std::cout << "��ʼԤ������������..." << std::endl;

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

            // ����ļ��Ƿ����
            if (std::filesystem::exists(filepath)) {
                try {
                    auto mesh_data = loadMeshFromCSV(filepath);
                    mesh_cache_[key] = mesh_data;
                    loaded_count++;
                }
                catch (const std::exception& e) {
                    std::cerr << "�����ļ� " << filename << " ����: " << e.what() << std::endl;
                    mesh_cache_[key] = generateRandomMesh(q_);
                    error_count++;
                }
            }
            else {
                // �ļ������ڣ������������
                mesh_cache_[key] = generateRandomMesh(q_);
                std::cout << "�ļ������ڣ������������: " << filename << std::endl;
            }
        }
    }

    std::cout << "Ԥ������ɣ������� " << mesh_cache_.size() << " ����������" << std::endl;
    std::cout << "�ɹ�����: " << loaded_count << ", ����/ȱʧ: " << error_count << std::endl;
}

std::vector<bool> CoverageDataLoader::getMeshData(int sat, int time, int q) {
    CacheKey key{ sat, time };
    auto it = mesh_cache_.find(key);

    if (it != mesh_cache_.end()) {
        return it->second;
    }

    // ���������û�У������������
    return generateRandomMesh(q);
}

std::vector<bool> CoverageDataLoader::loadMeshFromCSV(const std::string& filepath) {
    try {
        // ʹ���Ż���CSV��ȡ��
        auto mesh_data = OptimizedCSVReader::readSingleColumnCSV(filepath, q_);

        // ȷ����������ȷ
        if (mesh_data.size() < q_) {
            std::cout << "����: �ļ� " << filepath << " ֻ�� " << mesh_data.size()
                << " �����ݣ���Ҫ " << q_ << " �У�������������" << std::endl;

            while (mesh_data.size() < q_) {
                mesh_data.push_back(dist_(gen_));
            }
        }
        else if (mesh_data.size() > q_) {
            mesh_data.resize(q_);
            std::cout << "����: �ļ� " << filepath << " �� " << mesh_data.size()
                << " �����ݣ�������Ҫ�� " << q_ << " �У��ѽض�" << std::endl;
        }

        return mesh_data;

    }
    catch (const std::exception& e) {
        std::cerr << "����CSV�ļ�����: " << filepath << " - " << e.what() << std::endl;
        throw; // �����׳��쳣
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