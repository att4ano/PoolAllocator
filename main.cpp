#include "PoolAllocator.h"
#include <vector>
#include <chrono>
#include <fstream>

int main() {
    std::ofstream out("C:\\Users\\Home\\Documents\\labs\\labwork-9-dubstephf\\output.txt");
    for (int i = 10000; i < 500000; i += 10000) {
        std::vector<int> vec1;
        auto start = std::chrono::system_clock::now();
        for (int j = 0; j < i; ++j) {
            vec1.emplace_back(j);
        }
        vec1.clear();
        auto stop = std::chrono::system_clock::now();
        auto dur1 = std::chrono::duration_cast<std::chrono::microseconds>(stop - start);

        std::vector<int, PoolAllocator<int>> vec2(PoolAllocator<int>{{512, 1000000}});
        start = std::chrono::system_clock::now();
        for (int j = 0; j < i; ++j) {
            vec2.emplace_back(j);
        }
        vec2.clear();
        stop = std::chrono::system_clock::now();
        auto dur2 = std::chrono::duration_cast<std::chrono::microseconds>(stop - start);
        out << i << " " << dur1.count() << " " << dur2.count() << "\n";
    }
    return 0;
}