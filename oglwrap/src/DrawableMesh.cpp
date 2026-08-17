#include "oglwrap/DrawableMesh.hpp"

DrawablePolyline3D::DrawablePolyline3D(const Polyline3D& polyline3d)
    : DrawableObject()
{
    VertexBuffer::Ptr vb = std::make_shared<VertexBuffer>(
        polyline3d.m_points.data(),
        static_cast<GLsizeiptr>(polyline3d.m_points.size() * sizeof(glm::vec3))
    );
    vb->addLayout(Layout{ 0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)(0) });
    m_vao.addVertexBuffer(vb);
    m_count = polyline3d.m_points.size();
    m_uniforms.insert(std::make_pair("u_color", polyline3d.m_color));
}

void DrawablePolyline3D::draw(const ShaderProgram& shaderProgram) const
{
    shaderProgram.setUniforms(m_uniforms);
    m_vao.draw(GL_LINE_LOOP, m_count);
}

DrawableMesh::DrawableMesh(const Mesh& mesh)
    : DrawableObject()
{
    m_count = mesh.indexes.size();

    {
        VertexBuffer::Ptr vb = std::make_shared<VertexBuffer>(
            mesh.positions.data(),
            static_cast<GLsizeiptr>(mesh.positions.size() * sizeof(glm::vec3))
        );
        vb->addLayout(Layout{ 0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)(0) });
        m_vao.addVertexBuffer(vb);
    }
    {
        VertexBuffer::Ptr vb = std::make_shared<VertexBuffer>(
            mesh.normals.data(),
            static_cast<GLsizeiptr>(mesh.normals.size() * sizeof(glm::vec3))
        );
        vb->addLayout(Layout{ 1, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)(0) });
        m_vao.addVertexBuffer(vb);
    }
    {
        auto lock = m_vao.bind(); // Р”Рѕ СЃРѕР·РґР°РЅРёСЏ ElementBuffer Рё Р·Р°РіСЂСѓР·РєРё РµРіРѕ РґР°РЅРЅС‹С…
        ElementBuffer::Ptr eb = std::make_shared<ElementBuffer>(mesh.indexes.data(), static_cast<GLsizeiptr>(mesh.indexes.size()));
        m_vao.setElementBuffer(eb);
    }
}

void DrawableMesh::draw(const ShaderProgram& shaderProgram) const
{
    shaderProgram.setUniforms(m_uniforms);
    m_vao.draw(GL_TRIANGLES, m_count);
}

DrawableColoredMesh::DrawableColoredMesh(const ColoredMesh& coloredMesh)
    : DrawableMesh(coloredMesh)
    , m_ssboColors(0 /*bindingPoint*/)
{
    m_ssboColors.bind();
    m_ssboColors.uploadData(coloredMesh.colors.data(), coloredMesh.colors.size() * sizeof(glm::vec4));
}

void DrawableColoredMesh::draw(const ShaderProgram& shaderProgram) const
{
    m_ssboColors.bind();
    DrawableMesh::draw(shaderProgram);
}

DrawableSmoothMesh::DrawableSmoothMesh(const ColoredMesh& coloredMesh)
    : DrawableMesh(coloredMesh)
{
    VertexBuffer::Ptr vb = std::make_shared<VertexBuffer>(
        coloredMesh.colors.data(),
        static_cast<GLsizeiptr>(coloredMesh.colors.size() * sizeof(glm::vec4))
    );
    vb->addLayout(Layout{ 2, 3, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(0) });
    m_vao.addVertexBuffer(vb);
}

std::unique_ptr<IDrawableObject> createDrawableMesh(std::shared_ptr<IObject> object)
{
    if (Polyline3D* polyline = dynamic_cast<Polyline3D*>(object.get())) {
        return std::make_unique<DrawablePolyline3D>(*polyline);
    } else if (ColoredMesh* coloredMesh = dynamic_cast<ColoredMesh*>(object.get())) {
        if (coloredMesh->m_colorType == ColoredMesh::ColorType::byTriangle) {
            return std::make_unique<DrawableColoredMesh>(*coloredMesh);
        } else if (coloredMesh->m_colorType == ColoredMesh::ColorType::byVertex) {
            return std::make_unique<DrawableSmoothMesh>(*coloredMesh);
        }
    }

    assert(false);
    return nullptr;
}
