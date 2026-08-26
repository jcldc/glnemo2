// ============================================================================
// Copyright Jean-Charles LAMBERT - 2007-2026                                  
// e-mail:   Jean-Charles.Lambert@lam.fr                                      
// address:  Centre de donneeS Astrophysique de Marseille (CeSAM)              
//           Laboratoire d'Astrophysique de Marseille                          
//           Pole de l'Etoile, site de Ch�teau-Gombert                         
//           38, rue Fr�d�ric Joliot-Curie                                     
//           13388 Marseille cedex 13 France                                   
//           CNRS U.M.R 7326                                                   
// ============================================================================
// See the complete license in LICENSE and/or "http://www.cecill.info".        
// ============================================================================

#include "glaxesobject.h"
#include "glwindow.h"
#include <cmath>

namespace glnemo {

using namespace std;

void GLAxesObject::buildArrowGeometry(std::vector<GizmoVertex> &verts,
                                    float shaftRadius, float shaftLength,
                                    float headRadius, float headLength,
                                    int segments)
{
    // Arrow is built along +Z: cylinder shaft from 0 to shaftLength,
    // cone head from shaftLength to shaftLength + headLength.
    const float PI = 3.14159265359f;

    auto addTri = [&](glm::vec3 a, glm::vec3 b, glm::vec3 c) {
        glm::vec3 n = glm::normalize(glm::cross(b - a, c - a));
        verts.push_back({a, n});
        verts.push_back({b, n});
        verts.push_back({c, n});
    };

    // --- Cylinder (arrow shaft) ---
    for (int i = 0; i < segments; ++i) {
        float a0 = 2.0f * PI * i / segments;
        float a1 = 2.0f * PI * (i + 1) / segments;

        glm::vec3 p0(shaftRadius * cosf(a0), shaftRadius * sinf(a0), 0.0f);
        glm::vec3 p1(shaftRadius * cosf(a1), shaftRadius * sinf(a1), 0.0f);
        glm::vec3 p0top = p0 + glm::vec3(0.0f, 0.0f, shaftLength);
        glm::vec3 p1top = p1 + glm::vec3(0.0f, 0.0f, shaftLength);

        // Two triangles per segment (cylinder side quad)
        addTri(p0, p1, p1top);
        addTri(p0, p1top, p0top);
    }

    // --- Cone (arrow head) ---
    glm::vec3 apex(0.0f, 0.0f, shaftLength + headLength);
    for (int i = 0; i < segments; ++i) {
        float a0 = 2.0f * PI * i / segments;
        float a1 = 2.0f * PI * (i + 1) / segments;

        glm::vec3 p0(headRadius * cosf(a0), headRadius * sinf(a0), shaftLength);
        glm::vec3 p1(headRadius * cosf(a1), headRadius * sinf(a1), shaftLength);

        addTri(p0, p1, apex);

        // Cone base cap (closed disk)
        glm::vec3 center(0.0f, 0.0f, shaftLength);
        addTri(center, p1, p0); // reversed winding so the normal faces downward
    }
}

void GLAxesObject::init(GLuint shader_program)
{
    initializeOpenGLFunctions();

    std::vector<GizmoVertex> verts;
    buildArrowGeometry(verts,
                        0.04f,   // shaftRadius
                        0.6f,    // shaftLength
                        0.15f,   // headRadius
                        0.3f,    // headLength
                        32);     // segments

    vertexCount = (int)verts.size();

    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, verts.size() * sizeof(GizmoVertex), verts.data(), GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(GizmoVertex), (void*)offsetof(GizmoVertex, position));
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(GizmoVertex), (void*)offsetof(GizmoVertex, normal));
    glEnableVertexAttribArray(1);

    glBindVertexArray(0);

    shader = shader_program;
    // Load shader program (see .vert/.frag below)
    // shader = new QOpenGLShaderProgram();
    // shader->addShaderFromSourceFile(QOpenGLShader::Vertex, ":/shaders/gizmo.vert");
    // shader->addShaderFromSourceFile(QOpenGLShader::Fragment, ":/shaders/gizmo.frag");
    // shader->link();
}

void GLAxesObject::render(const glm::mat4 &viewRotationOnly, int viewportW, int viewportH)
{
    // Small viewport in the bottom-right corner of the window
    const int margin = 2;
    int size=go->axes_psize*viewportW;
    
    int pwidth,pheight;
    switch (go->axes_loc) {
    case 0: // bottom right
      pwidth = viewportW-size-margin;
      pheight= 0;
      break;
    case 1: // center
      pwidth = viewportW/2-size/2;
      pheight= viewportH/2-size/2;
      break;
    }
    glViewport(pwidth,pheight, size, size);
  
    // Clear depth only, so the gizmo is never occluded by the main scene
    glClear(GL_DEPTH_BUFFER_BIT);

    glm::mat4 proj = glm::perspective(glm::radians(30.0f), 1.0f, 0.1f, 10.0f);

    // Push the gizmo back a bit so it's fully inside the frustum,
    // then apply only the camera's rotation (no translation/zoom)
    glm::mat4 view = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, -4.5f)) * viewRotationOnly;

    // Rotations that align the base arrow (built along +Z) onto each axis
    glm::mat4 modelX = glm::rotate(glm::mat4(1.0f), glm::radians(90.0f), glm::vec3(0, 1, 0));  // Z -> X
    glm::mat4 modelY = glm::rotate(glm::mat4(1.0f), glm::radians(-90.0f), glm::vec3(1, 0, 0)); // Z -> Y
    glm::mat4 modelZ = glm::mat4(1.0f); // identity, already aligned along Z

    glm::mat4 models[3] = { modelX, modelY, modelZ };
    glm::vec3 colors[3] = { glm::vec3(1, 0, 0), glm::vec3(0, 1, 0), glm::vec3(0, 0, 1) };

    glUseProgram(shader);
    glBindVertexArray(vao);

    // These two are the same for all three axes, set them once
    glUniformMatrix4fv(glGetUniformLocation(shader, "viewMatrix"), 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(glGetUniformLocation(shader, "projMatrix"), 1, GL_FALSE, glm::value_ptr(proj));

    for (int i = 0; i < 3; ++i) {
        // Normal matrix: inverse-transpose of the upper 3x3 of the model matrix,
        // needed because non-uniform scaling would otherwise distort normals
        // (here we only rotate, so this is mostly a formality, but keeps it correct)
        glm::mat3 normalMatrix = glm::transpose(glm::inverse(glm::mat3(models[i])));

        glUniformMatrix4fv(glGetUniformLocation(shader, "modelMatrix"), 1, GL_FALSE, glm::value_ptr(models[i]));
        glUniformMatrix3fv(glGetUniformLocation(shader, "normalMatrix"), 1, GL_FALSE, glm::value_ptr(normalMatrix));
        glUniform3fv(glGetUniformLocation(shader, "axisColor"), 1, glm::value_ptr(colors[i]));

        glDrawArrays(GL_TRIANGLES, 0, vertexCount);
    }

    glBindVertexArray(0);

    // Restore full viewport for the rest of the frame
    glViewport(0, 0, viewportW, viewportH);
}

void GLAxesObject::cleanup()
{
  if (!initialized) return;

    if (vbo) { glDeleteBuffers(1, &vbo); vbo = 0; }
    if (vao) { glDeleteVertexArrays(1, &vao); vao = 0; }
    initialized = false;
}
}
