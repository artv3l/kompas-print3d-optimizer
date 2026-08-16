#include "concaveAngle.hpp"

#include <stdexcept>

const double CHAMFER_WIDTH = 0.1;

bool isConcaveAngle(kapi::ksDocument3DPtr document3d, kapi::ksEdgeDefinitionPtr edge) {
    kapi::ksPartPtr part(document3d->GetPart(kapi::pTop_Part));

    kapi::ksMassInertiaParamPtr massInertiaParam(part->CalcMassInertiaProperties(0x1 | 0x10)); // mm kg
    double startVolume = massInertiaParam->v;

    kapi::ksEntityPtr chamferEntity(part->NewEntity(kapi::Obj3dType::o3d_chamfer));
    kapi::ksChamferDefinitionPtr chamfer(chamferEntity->GetDefinition());
    chamfer->SetChamferParam(true, CHAMFER_WIDTH, CHAMFER_WIDTH);
    kapi::ksEntityCollectionPtr array(chamfer->array());
    array->Add(edge);
    chamferEntity->hidden = true;

    if (!chamferEntity->Create()) {
        throw std::runtime_error("Ошибка создания фаски");
    }

    massInertiaParam = part->CalcMassInertiaProperties(0x1 | 0x10); // mm kg
    double endVolume = massInertiaParam->v;
    document3d->DeleteObject(chamferEntity);
    return endVolume > startVolume;
}
