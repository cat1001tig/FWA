// main.cpp
#include "satellite_data_loader.h"
#include <iostream>
#include "coverage_loader.h"
#include "satellite_scheduler_base.h"
#include "satellite_scheduler_solution.h"
#include "satellite_scheduler_multiobjective.h"

int main() {
    
    try {
        ////////////////////////1.���ݼ���
        SatelliteDataLoader dloader;

        // �������ݣ�����CSV�ļ���׼���ã�
        if (dloader.loadDataFromExcel("dummy_path")) {
            // ����ѹ������
            dloader.saveCompressedData("compressed_example_3.0.txt");

            // ��ȡ�������ں�������
            const auto& compressed = dloader.getCompressedMatrix();
            const auto& bounds = dloader.getBounds();
            const auto& coverage_data = dloader.getCoverageData();
            

            std::cout << "ѹ�������С: " << compressed.size() << " x "
                << (compressed.empty() ? 0 : compressed[0].size()) << std::endl;
            std::cout << "�߽�����: " << bounds.size() << std::endl;
            std::cout << "������������������: " << coverage_data.size() << std::endl;

            // ��֤����������
            if (compressed.size() == 10 && !bounds.empty()) {
                std::cout << "���ݼ��غ�ѹ���ɹ�!" << std::endl;
            }

        }
        /////////////////////2.�������ݼ���
        // ���������ʼ�����
        CoverageDataLoader cloader("mesh_data");

        // Ԥ�����ض����Ǻ�ʱ�������
        std::vector<int> satellites = { 2, 4, 5 };
        std::vector<int> special_times = { 263, 264, 265, 266, 267, 268, 269, 270, 271, 272, 449 };

        cloader.preloadAllData(satellites, special_times);

        // ****���Ի�ȡ��������****
        std::vector<bool> mesh_data = cloader.getMeshData(2, 263, 6665);
        std::cout << "��ȡ���������ݴ�С: " << mesh_data.size() << std::endl;

        // ͳ�Ƹ����ʣ�true��������
        int coverage_count = 0;
        for (bool covered : mesh_data) {
            if (covered) coverage_count++;
        }

        double coverage_rate = static_cast<double>(coverage_count) / mesh_data.size();
        std::cout << "������: " << (coverage_rate * 100) << "%" << std::endl;

        // ���Ի�ȡδԤ���ص����ݣ�Ӧ�÷���������ݣ�
        std::vector<bool> random_mesh = cloader.getMeshData(1, 100, 6665);
        std::cout << "����������ݴ�С: " << random_mesh.size() << std::endl;

        /////////////////////////3.�㷨����
        //3.1��������

        //���Ի���SatelliteSchedulerBase
        std::cout << "���Ի���SatelliteSchedulerBase:" << std::endl;
        SatelliteSchedulerBase s;
        s.loadCompressedData("compressed_example_3.0.txt");
        s.initializeCoverageLoader();
        std::vector<int> ts = s.getBounds();
        std::cout << "bounds:" << std::endl;
        for (auto& t : ts) {
            std::cout << t << " ";
        }

        //����ÿ������ʱ�䴰�ڵĸ���������
        std::cout << "����ÿ������ʱ�䴰�ڵĸ���������:" << std::endl;
        for (auto& pair : dloader.getCoverageData()) {
            int key = pair.first;
            std::vector<double> d = pair.second;
            for (auto& c : d) {
                std::cout << c << " ";
            }
            std::cout << std::endl;
        }

        std::cout << "����ÿ�����ǵ�ʱ�䴰������:" << std::endl;
        for (auto& f : dloader.getTimeIndices()) {
            for (int i : f) {
                std::cout << i << " ";
            }
            std::cout << std::endl;
        }

        //���Ծ����ص�ʱ�䴰�����Ǹ�ʱ������������
        std::cout << "���Ծ����ص�ʱ�䴰�����Ǹ�ʱ������������:" << std::endl;
        std::vector<bool> md = cloader.getMeshData(2, 263, 6556);
        for (int i = 0; i < md.size();i++) {
            std::cout << md[i] << " ";
        }
        std::cout << std::endl;

        //����satellite_scheduler_solution�����̳���satellite_scheduler_base��
        SatelliteSchedulerSolution ss;
        ss.setCoverageData(dloader.getCoverageData());//���������ݻ�ȡ
        ss.setTimeIndices(dloader.getTimeIndices());//ѹ�����ʱ��Ƭ������ȡ

        ///////////////////////////���Խ�������ʽ��ʼ����//////////////////////////////
        std::cout << std::endl << std::endl << "���Խ�������ʽ��ʼ���ȣ�" << std::endl;
        //������������SatelliteSchedulerMultiObjective�̳���satellite_scheduler_fireworks��satellite_scheduler_fireworks�̳���satellite_scheduler_solution
        SatelliteSchedulerMultiObjective scheduler;
        if (!scheduler.loadCompressedData("compressed_example_3.0.txt")) {
            std::cerr << "ѹ�����ݼ���ʧ��" << std::endl;
            return 1;
        }
        // ���ø���������
        scheduler.setCoverageData(dloader.getCoverageData());
        scheduler.setTimeIndices(dloader.getTimeIndices());

        // ��ʼ�������ʼ�����
        scheduler.initializeCoverageLoader();

        //3.2�Ż����̡�������
        std::cout << "��ʼ�Ż�..." << std::endl;
        auto [best_solutions, all_solutions] = scheduler.optimize(10, 5, 42);

        int cnt_times = dloader.getBounds().size();
        for (size_t i = 0; i < best_solutions.size(); ++i) {
            auto eval_result = scheduler.evaluate(best_solutions[i], false);
            std::cout << "�� " << i + 1 << ": "
                << "����=" << eval_result.satellite_count
                << ", ������=" << eval_result.coverage / 100 << "%"
                << ", ƽ��ÿʱ��Ƭ������=" << eval_result.coverage / cnt_times
                << ", ���ط���=" << eval_result.load_variance << std::endl;
        }
    }
    catch (const std::exception& e) {
        std::cerr << "����: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}