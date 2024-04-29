#include <stdexcept>
#include <iostream>
#include <filesystem>
#include <string>
#include <fstream>
#include <list>

#include "glm/glm.hpp"
#include "nlohmann/json.hpp"
using njson = nlohmann::json;

#include "kapiwrap/connection.hpp"

#include "Test.hpp"

bool doubleEqual(double a, double b, double epsilon = 0.0001);
std::string read_file(std::string_view path);
std::list<Test> readTests(std::string testPath);
kapi::ksFaceDefinitionPtr findPrintFace(kapi::ksPartPtr part, double area);
kapi::IProceduresLibraryPtr findLibrary(kapi::KompasObjectPtr kompas, _bstr_t name);

int main() {
    CoInitialize(nullptr);

    kapi::KompasObjectPtr kompas = nullptr;
    try {
        kompas = kompasInit();
    } catch (const std::runtime_error& e) {
        std::cout << e.what() << "\n";
        return 0;
    }

    kapi::IProceduresLibraryPtr library = findLibrary(kompas, "Подготовка к FDM 3D печати");
    if (!library) {
        std::cerr << "Library not found" << "\n";
        return 1;
    }
    library->Attach = true;

    std::list<Test> tests = readTests("F:\\code\\kompas-print3d-optimizer\\tests");

    for (const Test& test : tests) {
        kapi::ksDocument3DPtr document3d = kompas->Document3D();
        if (!document3d->Open(test.modelFilename.c_str(), false)) {
            std::cerr << "Document not found" << "\n";
            continue;
        }
        kapi::ksPartPtr part = document3d->GetPart(kapi::pTop_Part);
        kapi::ksSelectionMngPtr selectionMng = document3d->GetSelectionMng();

        kapi::ksFaceDefinitionPtr printFace = findPrintFace(part, test.printFaceArea);
        if (!printFace) {
            std::cerr << "PrintFace not found" << "\n";
            continue;
        }
        selectionMng->Select(printFace);
        library->Execute(2, nullptr, true);
        while (library->Executable);

        for (int command : test.scenario) {
            library->Execute(command, nullptr, true);
            while (library->Executable);
        }

        kapi::ksMassInertiaParamPtr massInertiaParam(part->CalcMassInertiaProperties(0x1 | 0x10)); // mm kg
        bool testOk = doubleEqual(massInertiaParam->v, test.expected.volume);
        std::cout << test.label << " : " << (testOk ? "Ok" : "Error") << "\n";

        document3d->close();
    }
    
    return 0;
}


bool doubleEqual(double a, double b, double epsilon) {
    return (abs(a - b) < epsilon);
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

kapi::ksFaceDefinitionPtr findPrintFace(kapi::ksPartPtr part, double area) {
    kapi::ksFaceDefinitionPtr result = nullptr;

    kapi::ksBodyPtr body = part->GetMainBody();
    kapi::ksFaceCollectionPtr faces = body->FaceCollection();
    for (int iFace = 0, count = faces->GetCount(); iFace < count; iFace++) {
        kapi::ksFaceDefinitionPtr face = faces->GetByIndex(iFace);
        if (doubleEqual(face->GetArea(0x01), area)) {
            result = face;
            break;
        }
    }
    
    return result;
}

kapi::IProceduresLibraryPtr findLibrary(kapi::KompasObjectPtr kompas, _bstr_t name) {
    kapi::IProceduresLibraryPtr result = nullptr;

    kapi::IApplicationPtr app = kompas->ksGetApplication7();
    kapi::ILibraryManagerPtr libManager = app->LibraryManager;
    kapi::IProceduresLibrariesPtr procLibs = libManager->ProceduresLibraries;

    for (int i = 0, count = procLibs->Count; i < count; i++) {
        kapi::IProceduresLibraryPtr procLib = procLibs->GetItem(i);
        if (procLib->LibraryName == name) {
            result = procLib;
            break;
        }
    }

    return result;
}
