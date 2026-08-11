#pragma once

#include <vector>
#include <span>

#include <eigen3/Eigen/Dense>

/*
    РџРѕСЃС‚СЂРѕРµРЅРёРµ РІС‹РїСѓРєР»РѕР№ РѕР±РѕР»РѕС‡РєРё РІ 2D. Р’РѕР·РІСЂР°С‰Р°РµС‚ РїСѓСЃС‚РѕР№ РјР°СЃСЃРёРІ,
    РµСЃР»Рё РѕР±РѕР»РѕС‡РєР° РІС‹СЂРѕР¶РґР°РµС‚СЃСЏ РІ С‚РѕС‡РєСѓ РёР»Рё РїСЂСЏРјСѓСЋ.
*/
std::vector<Eigen::Vector2d> convexHull(std::span<const Eigen::Vector2d> points);
