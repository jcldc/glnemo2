//
// Created by kalterkrieg on 25/11/2019.
//

#include "glcharacteristicpoint.h"
#include "globaloptions.h"

namespace glnemo {

    GLObjectCharacteristicPoint::GLObjectCharacteristicPoint(std::array<float, 3> coords, Shape shape, float radius,
                                                             float fill_ratio)
            : m_coords(coords), m_shape(shape), m_fill_ratio(fill_ratio), m_radius(radius) {
    }


    Shape GLObjectCharacteristicPoint::getShape() const {
        return m_shape;
    }

    float GLObjectCharacteristicPoint::getRadius() const {
        return m_radius;
    }

    float GLObjectCharacteristicPoint::getFillRatio() const {
        return m_fill_ratio;
    }

    const std::array<float, 3> &GLObjectCharacteristicPoint::getCoords() const {
        return m_coords;
    }

    /******* GLObjectCharacteristicPointList ********/

    GLObjectCharacteristicPointList::GLObjectCharacteristicPointList(json characteristic_points) {

//        for (json::iterator it = characteristic_points.begin(); it != characteristic_points.end(); ++it) {
//            std::cout << *it << '\n';
//        }
        glGenVertexArrays(1, &m_vao);
        glBindVertexArray(m_vao);

        glGenBuffers(1, &m_vbo);
        this->m_charac_shader = new CShader(GlobalOptions::RESPATH.toStdString() + "/shaders/characteristic.vert",
                                            GlobalOptions::RESPATH.toStdString() + "/shaders/characteristic.frag");
        if (!m_charac_shader->init()) {
            delete m_charac_shader;
            m_charac_shader = nullptr;
        }
        std::array<float, 3> coords = {1, 1, 1};
        m_characteristic_points.push_back(new GLObjectCharacteristicPoint(coords, Shape::annulus, 1.5, 0.5));
        m_indices.resize(100, 1);
        m_vertices.resize(3, 1);

        glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
        glBufferData(GL_ARRAY_BUFFER, sizeof(float) * m_vertices.size(), m_vertices.data(), GL_STATIC_DRAW);

        glGenBuffers(1, &m_ebo);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ebo);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int) * m_indices.size(), m_indices.data(), GL_STATIC_DRAW);

        GLuint vpositions = glGetAttribLocation(m_charac_shader->getProgramId(), "point_center");
        int stride = 3 * sizeof(float);
        glVertexAttribPointerARB(vpositions, 3, GL_FLOAT, GL_FALSE, stride, (void *) 0);
        glEnableVertexAttribArrayARB(vpositions);

    }

    GLObjectCharacteristicPointList::~GLObjectCharacteristicPointList() {
        for (auto charac_point : m_characteristic_points) {
            delete charac_point;
        }
        m_characteristic_points.clear();
    }

    bool GLObjectCharacteristicPointList::ready() {
        return m_charac_shader != nullptr;
    }

    void GLObjectCharacteristicPointList::display_all() {

        if (!this->ready())
            return;

        m_charac_shader->start();

        // bind buffer
        // buffer data

        glBindVertexArray(m_vao);

        // send matrix
        GLfloat proj[16];
        glGetFloatv(GL_PROJECTION_MATRIX, proj);
        m_charac_shader->sendUniformXfv("projMatrix", 16, 1, &proj[0]);

        GLfloat mview[16];
        glGetFloatv(GL_MODELVIEW_MATRIX, mview);
        m_charac_shader->sendUniformXfv("modelviewMatrix", 16, 1, &mview[0]);
        m_charac_shader->sendUniformXfv("point_center", 3, 1, m_characteristic_points[0]->getCoords().data());

        m_charac_shader->sendUniformi("nb_vertices", 100);
        m_charac_shader->sendUniformf("radius", m_characteristic_points[0]->getRadius());
        m_charac_shader->sendUniformi("is_annulus", false);



        glDrawArrays(GL_TRIANGLE_FAN, 0, 100);
        glBindVertexArray(0);

//        #define BUFFER_OFFSET(offset) (static_cast<char*>(0) + (offset))
//        GLsizei count[1] = {100};
//        const GLvoid* indices[1] = {
//            BUFFER_OFFSET(0 * sizeof(GLushort)),
//        };
//        glMultiDrawElements(GL_TRIANGLE_STRIP, count, GL_UNSIGNED_SHORT, indices, 1);

        m_charac_shader->stop();
    }
}
