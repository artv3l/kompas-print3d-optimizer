#ifndef TEST_HPP
#define TEST_HPP

#include <string>
#include <list>
#include <filesystem>

#include "nlohmann/json.hpp"
using njson = nlohmann::json;

struct TestExpected {
    TestExpected(const njson& json);

    double volume;
};

struct Test {
    Test(const std::filesystem::path& path_, const njson& json);

    std::string label;
    std::string modelFilename;
    double printFaceArea;
    std::list<int> scenario;
    TestExpected expected;

    std::string path;
};

#endif /* TEST_HPP */
