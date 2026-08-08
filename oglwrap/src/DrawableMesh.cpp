#include "oglwrap/DrawableMesh.hpp"

DrawableMesh::DrawableMesh(std::shared_ptr<IObject> object, Uniforms uniforms) :
    m_vao(),
    m_uniforms(uniforms)
{
    if (Mesh* mesh = dynamic_cast<Mesh*>(object.get())) {
        m_vao = VertexArray(*mesh);
        m_count = mesh->indexes.size();
        m_mode = GL_TRIANGLES;

        if (ColoredMesh* coloredMesh = dynamic_cast<ColoredMesh*>(object.get())) {
            VertexBuffer::Ptr vb = std::make_shared<VertexBuffer>(
                coloredMesh->colors.data(),
                static_cast<GLsizeiptr>(coloredMesh->colors.size() * sizeof(glm::vec3))
            );
            vb->addLayout(Layout{ 2, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)(0) });
            m_vao.addVertexBuffer(vb);
        }
    }
    else if (Polyline3D* polyline = dynamic_cast<Polyline3D*>(object.get())) {
        VertexBuffer::Ptr vb = std::make_shared<VertexBuffer>(
            polyline->m_points.data(),
            static_cast<GLsizeiptr>(polyline->m_points.size() * sizeof(glm::vec3))
        );
        vb->addLayout(Layout{ 0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)(0) });
        m_vao.addVertexBuffer(vb);
        m_count = polyline->m_points.size();
        m_mode = GL_LINE_LOOP;
        m_uniforms.insert(std::make_pair("u_color", polyline->m_color));
    }
}

void DrawableMesh::draw(const ShaderProgram& shaderProgram) const
{
    shaderProgram.setUniforms(m_uniforms);
    m_vao.draw(m_mode, m_count);
}
