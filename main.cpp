#include "vector.hpp"
#include <iostream>
#include <vector>

void test_basic() {
    sjtu::vector<int> v;
    v.push_back(1);
    v.push_back(2);
    v.push_back(3);
    if (v.size() != 3) throw std::runtime_error("size error");
    if (v[0] != 1 || v[1] != 2 || v[2] != 3) throw std::runtime_error("value error");
}

int main() {
    try {
        test_basic();
        std::cout << "Tests passed!" << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "Test failed: " << e.what() << std::endl;
        return 1;
    }
    return 0;
}
