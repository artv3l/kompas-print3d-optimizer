#include <stdexcept>
#include <iostream>
#include <filesystem>
#include <string>
#include <fstream>
#include <list>

#include "nlohmann/json.hpp"
using njson = nlohmann::json;

#include "kapiwrap/connection.hpp"

#include "Test.hpp"

std::string read_file(std::string_view path);
std::list<Test> readTests(std::string testPath);

int main() {
    CoInitialize(nullptr);

    kapi::KompasObjectPtr kompas = nullptr;
    try {
        kompas = kompasInit();
    } catch (const std::runtime_error& e) {
        std::cout << e.what() << "\n";
        return 0;
    }

    std::list<Test> tests = readTests("F:/code/kompas-print3d-optimizer/tests");

    for (const Test& test : tests) {
        kapi::ksDocument3DPtr document3d = kompas->Document3D();
        if (!document3d->Open(test.modelFilename.c_str(), true)) {
            continue;
        }

    }
    
    return 0;
}


std::string read_file(std::string_view path) {
    constexpr auto read_size = std::size_t(4096);
    auto stream = std::ifstream(path.data());
    stream.exceptions(std::ios_base::badbit);

    if (!stream) {
        throw std::ios_base::failure("file does not exist");
    }

    auto out = std::string();
    auto buf = std::string(read_size, '\0');
    while (stream.read(&buf[0], read_size)) {
        out.append(buf, 0, stream.gcount());
    }
    out.append(buf, 0, stream.gcount());
    return out;
}

std::list<Test> readTests(std::string testPath) {
    namespace fs = std::filesystem;

    std::list<Test> tests;
    for (const fs::directory_entry& dirEntry : fs::recursive_directory_iterator(testPath)) {
        if (!dirEntry.is_regular_file() || dirEntry.path().extension().string() != ".json") {
            continue;
        }

        std::string testsString = read_file(dirEntry.path().string());
        njson testsJson = njson::parse(testsString);
        for (const njson& testJson : testsJson) {
            tests.push_back(Test(dirEntry.path().parent_path().string(), testJson));
        }
    }
    return tests;
}
