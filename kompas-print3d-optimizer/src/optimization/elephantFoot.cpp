#include "elephantFoot.hpp"

#include <list>
#include <sstream>

#include <KsAPI.h>

#include "kapiwrap/Macro.hpp"
#include "kapiwrap/3d/part.hpp"

#include "settings/PrintSurface.hpp"
#include "settings/Settings.hpp"
#include "settings/Setting.hpp"
#include "settings/SettingInitializer.hpp"

constexpr std::wstring_view c_macroNameElephantFoot = L"Фаски слоновьей ноги";

std::list<ksapi::ILoopPtr> getElephantFootTargets(ksapi::IPartPtr part, PrintSurface printSurface)
{
	std::list<ksapi::ILoopPtr> elephantFootTargets;

	auto faces = getFaces(part);

	for (ksapi::IFacePtr face : faces) {
		if (!face->IsPlanar()) {
			continue;
		}
		if (PlaneEq(face) != printSurface.eq) {
			continue;
		}

		for (ksapi::ILoopPtr loop : face->GetLoops()) {
			elephantFootTargets.push_back(loop);
		}
	}
	return elephantFootTargets;
}

ksapi::IModelObjectPtr createElephantFootChamfers(ksapi::IPartPtr part, std::list<ksapi::ILoopPtr> elephantFootTargets, Settings& settings)
{
	Macro macro(part, c_macroNameElephantFoot, true);

	ksapi::IModelContainerPtr modelCont = part;

	for (ksapi::ILoopPtr loopTarget : elephantFootTargets) {
		const double width = settings.getDoubleSetting(si::bridgeHoleFillLayersCount.name)->getValue() * settings.getDoubleSetting(si::layerHeight.name)->getValue();

		ksapi::IChamferPtr chamfer = modelCont->GetChamfers()->Add();
		
		chamfer->SetBuildingType(ksChamferBuildingTypeEnum::ksChamferTwoSides);
		chamfer->SetDirection(true);
		chamfer->SetDistance1(width);
		chamfer->SetDistance2(width);

		std::vector<ksapi::IModelObjectPtr> array;
		for (auto edge : loopTarget->GetEdges()) {
			array.push_back(edge);
		}
		chamfer->SetBaseObjects(array);

		if (chamfer->Update()) {
			// Привязываем размеры к переменным
			ksapi::IFeaturePtr feature = chamfer;
			ksapi::IVariablePtr variable2 = feature->GetVariable(false, true, 2); // Индекс=2 - "Длина 1"
			ksapi::IVariablePtr variable3 = feature->GetVariable(false, true, 3); // Индекс=3 - "Длина 2"

			std::wostringstream oss;
			oss << settings.getDoubleSetting(si::bridgeHoleFillLayersCount.name)->getExpression().c_str()
				<< " * "
				<< settings.getDoubleSetting(si::layerHeight.name)->getExpression().c_str();
			variable2->SetExpression(oss.str().c_str());
			variable3->SetExpression(oss.str().c_str());

			macro.add(chamfer);
		}
	}
	return macro.getModelObject();
}

ksapi::IModelObjectPtr optimizeElephantFoot(ksapi::IPartPtr part, Settings& settings)
{
	std::list<ksapi::ILoopPtr> targets = getElephantFootTargets(part, *settings.getPrintSurface());
	if (targets.empty()) {
		return nullptr;
	}
	return createElephantFootChamfers(part, targets, settings);
}
