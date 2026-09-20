#pragma once

#include <filesystem>

namespace filesystem
{
// Сгенерировать случайное имя файла (без расширения) в temp директории
std::filesystem::path getTempFile();
} // namespace filesystem
