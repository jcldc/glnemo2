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

        void display();
        bool ready();

    private:
        CShader *charac_shader;
        float point_center[3] = {
                1.0f, 1.f, 0.0f,
        };
        int nb_points = 100;
        float radius = 0.3;
        bool is_annulus = false;
    };

}
#endif //GLNEMO2_GLCHARACTERISTICPOINT_H
