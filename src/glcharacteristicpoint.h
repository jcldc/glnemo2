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
        GLObjectCharacteristicPoint(std::array<float,3> coords , Shape shape, float radius, float fill_ratio);
        Shape getShape() const;
        float getRadius() const;
        float getFillRatio() const;
        const std::array<float, 3> &getCoords() const;
    private:
        std::array<float,3> m_coords;
        Shape m_shape;
        float m_radius;
        float m_fill_ratio;
    };


    class GLObjectCharacteristicPointList {
    public:
        GLObjectCharacteristicPointList(json characteristic_points);
        ~GLObjectCharacteristicPointList();
        bool ready();
        void display_all();

    private:
        CShader *m_charac_shader = nullptr;
        std::vector<GLObjectCharacteristicPoint*> m_characteristic_points;
        std::vector<float> m_point_center_vertices;
        GLuint m_vbo;
        GLuint m_ebo;
        GLuint m_vao;
        std::vector<unsigned int> m_indices;
        std::vector<float> m_vertices;
    };

}
#endif //GLNEMO2_GLCHARACTERISTICPOINT_H
