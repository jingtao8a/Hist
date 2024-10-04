#include <iostream>
#include <random>
#include <chrono>
#include <set>
#include <vector>
#include "HistogramTree.h"

#define keyType int32_t
#define DATA_SIZE 199

std::vector<keyType> data_init() {
    std::random_device rd;
    std::mt19937 engine(rd());
    unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();

    std::uniform_int_distribution<keyType> dist(0, DATA_SIZE * 100);
    std::set<keyType> key_set;
    std::vector<keyType> keys;

    auto _n = DATA_SIZE;
    while (key_set.size() < _n) {
        keyType key = dist(engine);
        key_set.insert(key);
    }
    std::copy(key_set.begin(), key_set.end(), std::back_inserter(keys));
    return keys;
}

// 测试 HistogramTree 的功能
int main() {
    std::vector<keyType> data = data_init();
    keyType testKey = 999999;
    data.push_back(testKey);
    std::sort(data.begin(), data.end());
    HistogramTree<keyType> histogramTree(data);
//    histogramTree.print();

    std::cout << "real pos: " << (std::lower_bound(data.begin(), data.end(), testKey) - data.begin()) << std::endl;

    std::cout << "predict pos: " << histogramTree.lookup(testKey) << std::endl;

    return 0;
}
