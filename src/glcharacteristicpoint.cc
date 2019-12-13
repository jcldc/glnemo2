//
// Created by kalterkrieg on 25/11/2019.
//

#include "glcharacteristicpoint.h"
#include "globaloptions.h"

namespace glnemo {

    GLObjectCharacteristicPoint::GLObjectCharacteristicPoint(std::array<float, 3> coords, Shape shape, float radius,
                                                             float fill_ratio = 0)
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

        for (json::iterator it = characteristic_points.begin(); it != characteristic_points.end(); ++it) {
            if ((*it)["shape"] == "disk")
                m_disks.push_back(new GLObjectCharacteristicPoint((*it)["coords"], Shape::disk, (*it)["radius"]));
            else if ((*it)["shape"] == "annulus")
                m_annuli.push_back(new GLObjectCharacteristicPoint((*it)["coords"], Shape::annulus, (*it)["radius"],
                                                                   (*it)["fill_ratio"]));

        }

        // SHADER INIT
        this->m_disk_shader = new CShader(
                GlobalOptions::RESPATH.toStdString() + "/shaders/characteristic_points/disk.vert",
                GlobalOptions::RESPATH.toStdString() + "/shaders/characteristic_points/characteristic.frag");
        if (!m_disk_shader->init()) {
            delete m_disk_shader;
            m_disk_shader = nullptr;
            std::cerr << "Failed to initialize disk shader\n";
            exit(1);
        }

        // SHADER INIT
        this->m_annulus_shader = new CShader(
                GlobalOptions::RESPATH.toStdString() + "/shaders/characteristic_points/annulus.vert",
                GlobalOptions::RESPATH.toStdString() + "/shaders/characteristic_points/characteristic.frag");
        if (!m_annulus_shader->init()) {
            delete m_annulus_shader;
            m_annulus_shader = nullptr;
            std::cerr << "Failed to initialize annulus shader\n";
            exit(1);
        }

        // DATA INIT
//
        for (auto point : m_disks) {
            m_disk_data.push_back(point->getCoords().data()[0]);
            m_disk_data.push_back(point->getCoords().data()[1]);
            m_disk_data.push_back(point->getCoords().data()[2]);
            m_disk_data.push_back(point->getRadius());
        }

        for (auto point : m_annuli) {
            m_annulus_data.push_back(point->getCoords().data()[0]);
            m_annulus_data.push_back(point->getCoords().data()[1]);
            m_annulus_data.push_back(point->getCoords().data()[2]);
            m_annulus_data.push_back(point->getRadius());
            m_annulus_data.push_back(point->getFillRatio());
        }
        // BUFFER INIT
        GLuint disk_vbo;
        GLuint annulus_vbo;

        glGenBuffersARB(1, &disk_vbo);
        glGenBuffersARB(1, &annulus_vbo);
        glGenVertexArrays(1, &m_disk_vao);
        glGenVertexArrays(1, &m_annulus_vao);

        // SEND DATA
        glBindVertexArray(m_disk_vao);
        glBindBufferARB(GL_ARRAY_BUFFER_ARB, disk_vbo);
        glBufferData(GL_ARRAY_BUFFER_ARB, sizeof(float) * m_disk_data.size(), m_disk_data.data(), GL_STATIC_DRAW);

        GLuint point_center_disk_attrib = glGetAttribLocation(m_disk_shader->getProgramId(), "point_center");
        GLuint radius_disk_attrib = glGetAttribLocation(m_disk_shader->getProgramId(), "radius");
        if (point_center_disk_attrib == -1) {
            std::cerr << "Error occured when getting \"point_center\" attribute (disk shader)\n";
            exit(1);
        }
        if (radius_disk_attrib == -1) {
            std::cerr << "Error occured when getting \"radius\" attribute (disk shader)\n";
            exit(1);
        }
        glEnableVertexAttribArrayARB(point_center_disk_attrib);
        glVertexAttribPointerARB(point_center_disk_attrib, 3, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void *) 0);
        glVertexBindingDivisor(point_center_disk_attrib, 1);

        glEnableVertexAttribArrayARB(radius_disk_attrib);
        glVertexAttribPointerARB(radius_disk_attrib, 1, GL_FLOAT, GL_FALSE, 4 * sizeof(float),
                                 (void *) (3 * sizeof(float)));
        glVertexBindingDivisor(radius_disk_attrib, 1);

        glBindVertexArray(0);


        glBindVertexArray(m_annulus_vao);
        glBindBufferARB(GL_ARRAY_BUFFER_ARB, annulus_vbo);
        glBufferData(GL_ARRAY_BUFFER_ARB, sizeof(float) * m_annulus_data.size(), m_annulus_data.data(), GL_STATIC_DRAW);
        GLuint point_center_annulus_attrib = glGetAttribLocation(m_annulus_shader->getProgramId(), "point_center");
        GLuint radius_annulus_attrib = glGetAttribLocation(m_annulus_shader->getProgramId(), "radius");
        GLuint fill_ratio_annulus_attrib = glGetAttribLocation(m_annulus_shader->getProgramId(), "fill_ratio");
        if (point_center_annulus_attrib == -1) {
            std::cerr << "Error occured when getting \"point_center\" attribute (annulus shader)\n";
            exit(1);
        }
        if (radius_annulus_attrib == -1) {
            std::cerr << "Error occured when getting \"radius\" attribute (annulus shader)\n";
            exit(1);
        }
        if (fill_ratio_annulus_attrib == -1) {
            std::cerr << "Error occured when getting \"fill_ratio\" attribute (annulus shader)\n";
            exit(1);
        }
        glEnableVertexAttribArrayARB(point_center_annulus_attrib);
        glVertexAttribPointerARB(point_center_annulus_attrib, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void *) 0);
        glVertexBindingDivisor(point_center_annulus_attrib, 1);

        glEnableVertexAttribArrayARB(radius_annulus_attrib);
        glVertexAttribPointerARB(radius_annulus_attrib, 1, GL_FLOAT, GL_FALSE, 5 * sizeof(float),
                                 (void *) (3 * sizeof(float)));
        glVertexBindingDivisor(radius_annulus_attrib, 1);

        glEnableVertexAttribArrayARB(fill_ratio_annulus_attrib);
        glVertexAttribPointerARB(fill_ratio_annulus_attrib, 1, GL_FLOAT, GL_FALSE, 5 * sizeof(float),
                                 (void *) (4 * sizeof(float)));
        glVertexBindingDivisor(fill_ratio_annulus_attrib, 1);

        glBindVertexArray(0);
    }

    GLObjectCharacteristicPointList::~GLObjectCharacteristicPointList() {
        for (auto charac_point : m_disks) {
            delete charac_point;
        }
        for (auto charac_point : m_annuli) {
            delete charac_point;
        }
        m_disks.clear();
        m_annuli.clear();
        glDeleteVertexArrays(1, &m_annulus_vao);
        glDeleteVertexArrays(1, &m_disk_vao);
    }

    void GLObjectCharacteristicPointList::display_all() {
        if (!this->ready())
            return;

        GLfloat proj[16];
        glGetFloatv(GL_PROJECTION_MATRIX, proj);
        GLfloat mview[16];
        glGetFloatv(GL_MODELVIEW_MATRIX, mview);

        m_disk_shader->start();
        glBindVertexArray(m_disk_vao);
        m_disk_shader->sendUniformXfv("projMatrix", 16, 1, &proj[0]);
        m_disk_shader->sendUniformXfv("modelviewMatrix", 16, 1, &mview[0]);
        m_disk_shader->sendUniformi("nb_vertices", 100);
        glDrawArraysInstancedARB(GL_TRIANGLE_FAN, 0, 100, m_disks.size());
        glBindVertexArray(0);
        m_disk_shader->stop();


        m_annulus_shader->start();
        glBindVertexArray(m_annulus_vao);
        m_annulus_shader->sendUniformXfv("projMatrix", 16, 1, &proj[0]);
        m_annulus_shader->sendUniformXfv("modelviewMatrix", 16, 1, &mview[0]);
        m_annulus_shader->sendUniformi("nb_vertices", 100);
        glDrawArraysInstancedARB(GL_TRIANGLE_STRIP, 0, 100, m_annuli.size());
        glBindVertexArray(0);
        m_annulus_shader->stop();

        glBindVertexArray(0);
    }

    bool GLObjectCharacteristicPointList::ready() {
        return m_disk_shader != nullptr;
    }
}
