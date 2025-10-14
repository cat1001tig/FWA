// main.cpp
#include "satellite_data_loader.h"
#include <iostream>
#include "coverage_loader.h"
#include "satellite_scheduler_base.h"
#include "satellite_scheduler_solution.h"
#include "satellite_scheduler_multiobjective.h"

int main() {
    
    try {
        // 0. 设置算法参数
        AlgorithmParams params;
        params.max_switches = 8;
        params.q = 6665;
        params.weights = { 0.34, 0.33, 0.33 };
        params.max_sparks = 30;
        params.max_length = 10;
        params.update_bounds = true;
        params.max_variation = 3;
        params.special_times = { 263, 264, 265, 266, 267, 268, 269, 270, 271, 272, 449 };
        int starthour = 8; //观测时间的起始小时
        int startminute = 0; //观测时间的起始分钟
        int startsecond = 0; //观测时间的起始秒
        int endhour = 20; //观测时间的结束小时
        int endminute = 0; //观测时间的结束分钟
        int endsecond = 0; //观测时间的结束秒

        int endSeconds = endhour * 3600 + endminute * 60 + endsecond;
        int startSeconds = starthour * 3600 + startminute * 60 + startsecond;
        int diffSeconds = endSeconds - startSeconds;
        // 假设不会跨天，(如果需要，则加24*3600来处理跨天)
        int diffMinutes = diffSeconds / 60; // 向下取整

        params.num_satellites_ = 10;
        params.total_minutes_ = diffMinutes + 1; // 12*60 + 1


        ////////////////////////1.数据加载
        SatelliteDataLoader dloader;

        // 加载数据（假设CSV文件已准备好）
        if (dloader.loadDataFromExcel("dummy_path")) {
            // 保存压缩数据
            dloader.saveCompressedData("compressed_example_3.0.txt");

            // 获取数据用于后续处理
            const auto& compressed = dloader.getCompressedMatrix();
            const auto& bounds = dloader.getBounds();
            const auto& coverage_data = dloader.getCoverageData();
            

            std::cout << "压缩矩阵大小: " << compressed.size() << " x "
                << (compressed.empty() ? 0 : compressed[0].size()) << std::endl;
            std::cout << "边界数量: " << bounds.size() << std::endl;
            std::cout << "覆盖率数据卫星数量: " << coverage_data.size() << std::endl;

            // 验证数据完整性
            if (compressed.size() == 10 && !bounds.empty()) {
                std::cout << "数据加载和压缩成功!" << std::endl;
            }

        }
        /////////////////////2.网格数据加载
        // 创建覆盖率加载器
        CoverageDataLoader cloader("mesh_data");

        // 预加载特定卫星和时间的数据
        std::vector<int> satellites = { 2, 4, 5 };
        std::vector<int> special_times = { 263, 264, 265, 266, 267, 268, 269, 270, 271, 272, 449 };

        cloader.preloadAllData(satellites, special_times);

        // ****测试获取网格数据****
        std::vector<bool> mesh_data = cloader.getMeshData(2, 263, 6665);
        std::cout << "获取的网格数据大小: " << mesh_data.size() << std::endl;

        // 统计覆盖率（true的数量）
        int coverage_count = 0;
        for (bool covered : mesh_data) {
            if (covered) coverage_count++;
        }

        double coverage_rate = static_cast<double>(coverage_count) / mesh_data.size();
        std::cout << "覆盖率: " << (coverage_rate * 100) << "%" << std::endl;

        // 测试获取未预加载的数据（应该返回随机数据）
        std::vector<bool> random_mesh = cloader.getMeshData(1, 100, 6665);
        std::cout << "随机网格数据大小: " << random_mesh.size() << std::endl;

        /////////////////////////3.算法核心
        //3.1声明对象

        //测试基类SatelliteSchedulerBase
        std::cout << "测试基类SatelliteSchedulerBase:" << std::endl;
        SatelliteSchedulerBase s;
        s.loadCompressedData("compressed_example_3.0.txt");
        s.initializeCoverageLoader();
        std::vector<int> ts = s.getBounds();
        std::cout << "bounds:" << std::endl;
        for (auto& t : ts) {
            std::cout << t << " ";
        }

        //测试每个卫星时间窗口的覆盖率数据
        std::cout << "测试每个卫星时间窗口的覆盖率数据:" << std::endl;
        for (auto& pair : dloader.getCoverageData()) {
            int key = pair.first;
            std::vector<double> d = pair.second;
            for (auto& c : d) {
                std::cout << c << " ";
            }
            std::cout << std::endl;
        }

        std::cout << "测试每个卫星的时间窗口数据:" << std::endl;
        for (auto& f : dloader.getTimeIndices()) {
            for (int i : f) {
                std::cout << i << " ";
            }
            std::cout << std::endl;
        }

        //测试具有重叠时间窗口卫星该时间点的网格数据
        std::cout << "测试具有重叠时间窗口卫星该时间点的网格数据:" << std::endl;
        std::vector<bool> md = cloader.getMeshData(2, 263, 6556);
        for (int i = 0; i < md.size();i++) {
            std::cout << md[i] << " ";
        }
        std::cout << std::endl;

        //测试satellite_scheduler_solution（它继承自satellite_scheduler_base）
        SatelliteSchedulerSolution ss;
        ss.setCoverageData(dloader.getCoverageData());//覆盖率数据获取
        ss.setTimeIndices(dloader.getTimeIndices());//压缩后的时间片索引获取

        ///////////////////////////测试结束，正式开始调度//////////////////////////////
        std::cout << std::endl << std::endl << "测试结束，正式开始调度：" << std::endl;
        //创建调度器，SatelliteSchedulerMultiObjective继承自satellite_scheduler_fireworks，satellite_scheduler_fireworks继承自satellite_scheduler_solution
        SatelliteSchedulerMultiObjective scheduler(params);
        if (!scheduler.loadCompressedData("compressed_example_3.0.txt")) {
            std::cerr << "压缩数据加载失败" << std::endl;
            return 1;
        }
        // 设置覆盖率数据
        scheduler.setCoverageData(dloader.getCoverageData());
        scheduler.setTimeIndices(dloader.getTimeIndices());

        // 初始化覆盖率加载器
        scheduler.initializeCoverageLoader();

        //3.2优化过程——迭代
        std::cout << "开始优化..." << std::endl;
        auto [best_solutions, all_solutions] = scheduler.optimize(10, 5, 42);

        int cnt_times = dloader.getBounds().size();
        for (size_t i = 0; i < best_solutions.size(); ++i) {
            auto eval_result = scheduler.evaluate(best_solutions[i], false);
            std::cout << "解 " << i + 1 << ": "
                << "卫星=" << eval_result.satellite_count
                << ", 覆盖率=" << eval_result.coverage / 100 << "%"
                << ", 平均每时间片覆盖率=" << eval_result.coverage / cnt_times
                << ", 负载方差=" << eval_result.load_variance << std::endl;

            std::vector<int> bo = dloader.getBounds();//压缩后的索引对应的原始索引

            std::cout << "解" << i + 1 << "方案：" << std::endl;
            for (int s = 0; s < best_solutions[i].size(); s++) {
                for (int t = 0; t < best_solutions[i][s].size(); t++) {
                    std::cout << best_solutions[i][s][t] << " ";
                }
                std::cout << std::endl;
            }
        }
    }
    catch (const std::exception& e) {
        std::cerr << "错误: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}