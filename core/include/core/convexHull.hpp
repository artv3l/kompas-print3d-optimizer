#pragma once

#include <vector>
#include <span>

#include <eigen3/Eigen/Dense>

// РџРѕСЃС‚СЂРѕРµРЅРёРµ РІС‹РїСѓРєР»РѕР№ РѕР±РѕР»РѕС‡РєРё РІ 2D. РњРѕР¶РµС‚ РІРµСЂРЅСѓС‚СЊ РїСѓСЃС‚РѕР№ РјР°СЃСЃРёРІ, РѕРґРЅСѓ РёР»Рё РґРІРµ С‚РѕС‡РєРё, РµСЃР»Рё Р±С‹Р»Рё РїРµСЂРµРґР°РЅС‹ С‚Р°РєРёРµ РІС…РѕРґРЅС‹Рµ РґР°РЅРЅС‹Рµ
std::vector<Eigen::Vector2d> convexHull(std::span<Eigen::Vector2d> points);
