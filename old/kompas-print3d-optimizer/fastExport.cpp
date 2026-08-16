void fastExportStl(kapi::ksDocument3DPtr document3d, Settings& settings)
{
    auto splitFileNameAndRemoveExtension = [](std::string fileName) -> std::pair<std::string, std::string>
        {
            size_t lastSlashIndex = fileName.find_last_of('\\');
            size_t lastDotIndex = fileName.find_last_of('.');
            return std::make_pair(fileName.substr(0, lastSlashIndex), fileName.substr(lastSlashIndex + 1, lastDotIndex - lastSlashIndex - 1));
        };

    kapi::ksAdditionFormatParamPtr param = document3d->AdditionFormatParam();
    param->Init();
    param->format = kapi::D3FormatConvType::format_STL;
    param->formatBinary = true;
    param->angle = 2 * M_PI / 180.0;
    param->stepType = kapi::ksStepTypeEnum::ksDeviationStep;

    std::pair<std::string, std::string> pair = splitFileNameAndRemoveExtension(std::string(document3d->fileName));
    std::string stlFolder = std::string(settings.getStringSetting(si::exportStlFolder.name)->getValue());

    std::string resultFolder = pair.first + "\\" + stlFolder;
    CreateDirectoryA(resultFolder.c_str(), nullptr);

    std::string result = resultFolder + "\\" + pair.second + ".stl";
    document3d->SaveAsToAdditionFormat(result.c_str(), param);
}
