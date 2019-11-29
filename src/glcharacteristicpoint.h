//
// Created by kalterkrieg on 25/11/2019.
//

#ifndef GLNEMO2_GLCHARACTERISTICPOINT_H
#define GLNEMO2_GLCHARACTERISTICPOINT_H

#include <iostream>
#include "cshader.h"

namespace glnemo {

    class GLObjectCharacteristicPoint {
    public:
        GLObjectCharacteristicPoint(CShader *_charac_shader);

        void display(double *mScreen);

        bool ready();

    private:
        CShader *charac_shader;
        GLuint vbo_pos;
        float point_center[3] = {
                1.0f, 1.f, 0.0f,
        };
        float vertices[9] = {
                -0.2f, -0.2f, 0.0f,
                0.2f, -0.2f, 0.0f,
                -0.2f, 0.2f, 0.0f,
//                0.2f, 0.2f, 0.0f,
        };
        int vpos;
    };

}
#endif //GLNEMO2_GLCHARACTERISTICPOINT_H
