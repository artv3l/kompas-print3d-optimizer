if (visualizer == Visualizer::meshHighlight3dp) {
    if (!m_settings->isPrintSurfaceSelected())
        continue;

    /*
        Р’ СЃРїСЂР°РІРєРµ РЅР°РїРёСЃР°РЅРѕ, С‡С‚Рѕ РјРµС‚РѕРґ GetZoomScale СЂР°Р±РѕС‚Р°РµС‚ С‚РѕР»СЊРєРѕ РґР»СЏ РіСЂР°С„РёС‡РµСЃРєРёС… РґРѕРєСѓРјРµРЅС‚РѕРІ, РЅРѕ РґР»СЏ РјРѕРґРµР»Рё scale СЃС‡РёС‚Р°РµС‚СЃСЏ РєРѕСЂСЂРµРєС‚РЅРѕ.
        РџРѕСЌС‚РѕРјСѓ РµРіРѕ Рё Р±СѓРґРµРј РёСЃРїРѕР»СЊР·РѕРІР°С‚СЊ РґР»СЏ СЂР°СЃС‡РµС‚Р° С‚РѕР»С‰РёРЅС‹ Р»РёРЅРёРё РІ С€РµР№РґРµСЂРµ (РєР°РєР°СЏ РѕР±Р»Р°СЃС‚СЊ РІРѕРєСЂСѓРі С‚РѕС‡РЅРѕР№ РіСЂР°РЅРёС†С‹ СЃР»РѕРµРІ Р±СѓРґРµС‚ РѕС‚СЂРёСЃРѕРІС‹РІР°С‚СЊСЃСЏ).
        Р§РµРј Р±Р»РёР¶Рµ РјРѕРґРµР»СЊРєР°, С‚РµРј РјРµРЅСЊС€Рµ С‚РѕР»С‰РёРЅР° СЂРёСЃСѓРµРјРѕР№ Р»РёРЅРёРё.

        (scale, lineWidth): (410.2, 0.001), (137.4, 0.003), (31.9, 0.0085), (8.9, 0.025), (4.3, 0.033)
        Р°РїРїСЂРѕРєСЃРёРјРёСЂСѓРµРј СЃС‚РµРїРµРЅРЅРѕР№ С„СѓРЅРєС†РёРµР№: lineWidth = 0.1192 * scale^(-0.7731)

        РўР°РєР¶Рµ СЂР°СЃСЃС‡РёС‚Р°РµРј СЂР°РґРёСѓСЃ РѕРєСЂСѓР¶РЅРѕСЃС‚Рё, РІ РїСЂРµРґРµР»Р°С… РєРѕС‚РѕСЂРѕР№ Р±СѓРґСѓС‚ РѕС‚СЂРёСЃРѕРІС‹РІР°С‚СЊСЃСЏ СЃР»РѕРё РІ СЂРµР¶РёРјРµ РѕС‚СЂРёСЃРѕРІРєРё Сѓ РєСѓСЂСЃРѕСЂР°

        (scale, mouseRadius): (341.8, 450), (137.4, 220), (46.6, 110), (26.6, 90), (15.4, 60), (10.7, 70), (5.2, 40), (3.0, 20)
        Р°РїРїСЂРѕРєСЃРёРјРёСЂСѓРµРј СЃС‚РµРїРµРЅРЅРѕР№ С„СѓРЅРєС†РёРµР№: mouseRadius = 12.8668 * scale^(0.5943)
    */
    double unused, scale; m_documentFrame->GetZoomScale(&unused, &unused, &scale);
    float lineWidth = static_cast<float>(0.1192 * std::pow(scale, -0.7731));
    int mouseRadius = static_cast<int>(12.8668 * std::pow(scale, 0.5943));

    PrintSurface printSurface = *m_settings->getPrintSurface();
    glm::vec3 printSurfaceNormal(printSurface.eq.a, printSurface.eq.b, printSurface.eq.c);
    const double overhangThreshold = m_settings->getDoubleSetting(si::overhangThreshold.name)->getValue();

    kapi::ksPartPtr part = m_document3d->GetPart(kapi::Part_Type::pTop_Part);

    shaderProgram.setUniform("u_printSurfaceNormal", printSurfaceNormal);
    shaderProgram.setUniform("u_printSurfaceD", static_cast<float>(printSurface.eq.d));
    shaderProgram.setUniform("u_layerHeight", static_cast<float>(m_settings->getDoubleSetting(si::layerHeight.name)->getValue()));
    shaderProgram.setUniform("u_lineWidth", lineWidth);
    shaderProgram.setUniform("u_mode", m_mode);
    shaderProgram.setUniform("u_overhangThreshold", static_cast<float>(math::toRadians(overhangThreshold)));
    shaderProgram.setUniform("u_mouseCoord", m_mouseCoord);
    shaderProgram.setUniform("u_mouseRadius", mouseRadius);
}
