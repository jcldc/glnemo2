//
// Created by kalterkrieg on 25/11/2019.
//

#include "glcharacteristicpoint.h"

namespace glnemo {

    GLObjectCharacteristicPoint::GLObjectCharacteristicPoint(CShader *_charac_shader) {
        charac_shader = _charac_shader;
    }

    void GLObjectCharacteristicPoint::display(double *mScreen) {

        charac_shader->start();

        // send matrix
        GLfloat proj[16];
        glGetFloatv(GL_PROJECTION_MATRIX, proj);
        charac_shader->sendUniformXfv("projMatrix", 16, 1, &proj[0]);

        GLfloat mview[16];
        glGetFloatv(GL_MODELVIEW_MATRIX, mview);
        charac_shader->sendUniformXfv("modelviewMatrix", 16, 1, &mview[0]);

        GLfloat CameraRight_worldspace[3] = {mScreen[0], mScreen[1], mScreen[2]};
        GLfloat CameraUp_worldspace[3] = {mScreen[4], mScreen[5], mScreen[6]};

        charac_shader->sendUniformXfv("CameraRight_worldspace", 3, 1, &CameraRight_worldspace[0]);
        charac_shader->sendUniformXfv("CameraUp_worldspace", 3, 1, &CameraUp_worldspace[0]);
        charac_shader->sendUniformXfv("point_center", 3, 1, &point_center[0]);
        charac_shader->sendUniformi("nb_vertices", nb_points);
        charac_shader->sendUniformf("radius", radius);

        glDrawArrays(GL_TRIANGLE_FAN, 0, nb_points);

        charac_shader->stop();

    }

    bool GLObjectCharacteristicPoint::ready() {
        return charac_shader != NULL;
    }
}
