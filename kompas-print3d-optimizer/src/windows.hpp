#pragma once

#include <functional>
#include <span>

#include <comutil.h>

#include <oglwrap/ActionLock.hpp>

template <typename T>
std::pair<std::span<T>, ActionLock> getSafeArrayData(const _variant_t& variant)
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
	std::span<T> span(data, count / (sizeof(T) / elemSize));
	ActionLock lock([&variant]() { SafeArrayUnaccessData(variant.parray); });
	return std::make_pair(span, std::move(lock));
}

inline _variant_t toVariant(std::span<double> span)
{
    SAFEARRAYBOUND bound;
    bound.lLbound = 0;
    bound.cElements = span.size();

    SAFEARRAY* sa = SafeArrayCreate(VT_R8, 1, &bound);

    double* data = nullptr;
    SafeArrayAccessData(sa, (void**)&data);
    std::ranges::copy(span, data);
    SafeArrayUnaccessData(sa);

    _variant_t var;
    var.vt = VT_ARRAY | VT_R8;
    var.parray = sa;

    return var;
}
