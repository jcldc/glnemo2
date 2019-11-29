//
// Created by kalterkrieg on 25/11/2019.
//

#include "glcharacteristicpoint.h"

namespace glnemo {

    GLObjectCharacteristicPoint::GLObjectCharacteristicPoint(CShader *_charac_shader) {
        charac_shader = _charac_shader;
    }

    void GLObjectCharacteristicPoint::display() {

        charac_shader->start();

        // send matrix
        GLfloat proj[16];
        glGetFloatv(GL_PROJECTION_MATRIX, proj);
        charac_shader->sendUniformXfv("projMatrix", 16, 1, &proj[0]);

        GLfloat mview[16];
        glGetFloatv(GL_MODELVIEW_MATRIX, mview);
        charac_shader->sendUniformXfv("modelviewMatrix", 16, 1, &mview[0]);

        charac_shader->sendUniformXfv("point_center", 3, 1, &point_center[0]);
        charac_shader->sendUniformi("nb_vertices", nb_points);
        charac_shader->sendUniformf("radius", radius);
        charac_shader->sendUniformi("is_annulus", is_annulus);

        if(is_annulus)
            glDrawArrays(GL_TRIANGLE_STRIP, 0, nb_points*2);
        else
            glDrawArrays(GL_TRIANGLE_FAN, 0, nb_points);

        charac_shader->stop();

    }

    bool GLObjectCharacteristicPoint::ready() {
        return charac_shader != NULL;
    }
}
