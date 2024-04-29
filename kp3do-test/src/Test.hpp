#ifndef TEST_HPP
#define TEST_HPP

#include <string>
#include <list>

#include "nlohmann/json.hpp"
using njson = nlohmann::json;

struct TestExpected {
    TestExpected(const njson& json);

    double volume;
};

struct Test {
    Test(const std::string& path, const njson& json);

    std::string label;
    std::string modelFilename;
    double printFaceArea;
    std::list<int> scenario;
    TestExpected expected;
};

#endif /* TEST_HPP */
