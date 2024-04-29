#include "Test.hpp"

TestExpected::TestExpected(const njson& json):
    volume(json["volume"])
{}

Test::Test(const std::string& path, const njson& json):
    label(json["label"]),
    modelFilename(path + "/" + static_cast<std::string>(json["modelFilename"])),
    printFaceArea(json["printFaceArea"]),
    scenario(),
    expected(json["expected"])
{
    for (int step : json["scenario"]) {
        scenario.push_back(step);
    }
}
