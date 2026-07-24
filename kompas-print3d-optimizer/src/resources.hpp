#pragma once

#include <string_view>

namespace resources
{
inline constexpr std::wstring_view c_libraryName = L"kompas-print3d-optimizer";

inline constexpr std::wstring_view c_macroNameElephantFoot = L"Фаски слоновьей ноги";
inline constexpr std::wstring_view c_macroNameRoundingEdgesOnPrintFace = L"Скругленные ребра на плоскости печати";
inline constexpr std::wstring_view c_macroNameRoundingEdgesOnPrintFaceElement = L"Контур";
inline constexpr std::wstring_view c_macroNameRoundingEdgesOnPrintFaceElementWithRework = L"Контур - ДОРАБОТКА";
}
