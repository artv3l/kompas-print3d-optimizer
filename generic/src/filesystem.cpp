#include "filesystem.hpp"

#include <random>

namespace filesystem
{
std::filesystem::path getTempFile()
{
    constexpr size_t c_filenameLength = 20;
    constexpr std::wstring_view chars = L"0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";

    std::mt19937_64 rng{std::random_device{}()};
    std::uniform_int_distribution<std::size_t> dist(0, chars.size() - 1);

    std::wstring name;
    for (size_t i = 0; i < c_filenameLength; ++i)
        name += chars[dist(rng)];

    return std::filesystem::temp_directory_path() / name;
}
} // namespace filesystem
