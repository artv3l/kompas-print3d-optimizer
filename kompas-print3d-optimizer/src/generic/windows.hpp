#pragma once

#include <functional>
#include <span>

#include <comutil.h>

using Lock = std::function<void()>;

template <typename T>
std::pair<std::span<T>, Lock> getSafeArrayData(const _variant_t& variant)
{
	if (!(variant.vt & VT_ARRAY) || !variant.parray) {
		assert(false);
		return {};
	}

	size_t count = variant.parray->rgsabound[0].cElements - variant.parray->rgsabound[0].lLbound;
	if (count <= 0 || variant.parray->cDims != 1) {
		assert(false);
		return {};
	}

	T HUGEP* data = nullptr;
	SafeArrayAccessData(variant.parray, (void HUGEP * FAR*) & data);

	const UINT elemSize = SafeArrayGetElemsize(variant.parray);
	return { std::span<T>(data, count / (sizeof(T) / elemSize)), [&variant]() { SafeArrayUnaccessData(variant.parray); } };
}
