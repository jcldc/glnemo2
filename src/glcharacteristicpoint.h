//
// Created by kalterkrieg on 25/11/2019.
//

#ifndef GLNEMO2_GLCHARACTERISTICPOINT_H
#define GLNEMO2_GLCHARACTERISTICPOINT_H

#include <iostream>
#include "cshader.h"
#include <QtCore/QString>
#include <json.hpp>
#include "vec3d.h"

using json = nlohmann::json;

#define NB_POINTS 100

namespace glnemo {
    enum Shape {
        disk, annulus
    };

    class GLObjectCharacteristicPoint {
    public:
        GLObjectCharacteristicPoint(std::array<float, 3> coords, Shape shape, float radius, float fill_ratio);
        Shape getShape() const;
        float getRadius() const;
        float getFillRatio() const;
        const std::array<float, 3> &getCoords() const;
    private:
        std::array<float, 3> m_coords;
        Shape m_shape;
        float m_radius;
        float m_fill_ratio;
        bool m_display;
    };


    class GLObjectCharacteristicPointList {
    public:
        GLObjectCharacteristicPointList();
        GLObjectCharacteristicPointList(json characteristic_points);
        ~GLObjectCharacteristicPointList();

        void initFromJson(json characteristic_points);
        void initVboData();
        bool ready();
        void display_all();
        void addPoint(GLObjectCharacteristicPoint *point);

    private:
        CShader *m_disk_shader = nullptr;
        CShader *m_annulus_shader = nullptr;
        std::vector<GLObjectCharacteristicPoint *> m_disks;
        std::vector<GLObjectCharacteristicPoint *> m_annuli;
        GLuint m_disk_vao, m_annulus_vao,
                m_disk_vbo, m_annulus_vbo;
        std::vector<float> m_disk_data;
        std::vector<float> m_annulus_data;
    };

}
#endif //GLNEMO2_GLCHARACTERISTICPOINT_H
