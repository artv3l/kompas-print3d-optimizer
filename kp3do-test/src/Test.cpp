#include "Test.hpp"

TestExpected::TestExpected(const njson& json):
    volume(json["volume"])
{}

Test::Test(const std::filesystem::path& path_, const njson& json):
    label(json["label"]),
    modelFilename(path_.parent_path().string() + "\\" + static_cast<std::string>(json["modelFilename"])),
    printFaceArea(json["printFaceArea"]),
    scenario(),
    expected(json["expected"]),
    path(path_.string())
{
    for (int step : json["scenario"]) {
        scenario.push_back(step);
    }
}
