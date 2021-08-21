// ============================================================================
// Copyright Jean-Charles LAMBERT - 2007-2018                                  
// e-mail:   Jean-Charles.Lambert@lam.fr                                      
// address:  Centre de donneeS Astrophysique de Marseille (CeSAM)              
//           Laboratoire d'Astrophysique de Marseille                          
//           P�le de l'Etoile, site de Ch�teau-Gombert                         
//           38, rue Fr�d�ric Joliot-Curie                                     
//           13388 Marseille cedex 13 France                                   
//           CNRS U.M.R 7326                                                   
// ============================================================================
// See the complete license in LICENSE and/or "http://www.cecill.info".        
// ============================================================================
#include <iostream>
#include "glgridobject.h"
#include <GL/glew.h>
#include <glm/gtx/string_cast.hpp>
namespace glnemo {

int   GLGridObject::nsquare = 50;        // default #nsquare
float GLGridObject::square_size = 1.0;   // default square size

using namespace std;

// ============================================================================
// Constructor
GLGridObject::GLGridObject(int axe_parm, const QColor &c, bool activated) : GLObject() {
  if (axe_parm < 0 || axe_parm > 2)
    axe_parm = 0;
  dplist_index = glGenLists(1);
  axe = axe_parm;
  buildDisplayList();
  setColor(c);
  is_activated = activated;

}

// ============================================================================
// Destructor
// Delete display list
GLGridObject::~GLGridObject() {
  glDeleteLists(dplist_index, 1);
}

// ============================================================================
// GLGridObject::buildDisplayList()
// Build Display List
void GLGridObject::buildDisplayList() {
  float x = 0., y = 0., z = 0., x1 = 0., y1 = 0., z1 = 0.;
  GLfloat
          inf = ((GLfloat) (-nsquare / 2) * square_size),
          sup = ((GLfloat) (nsquare / 2) * square_size);

  // display list
  glNewList(dplist_index, GL_COMPILE);
  glBegin(GL_LINES);
  // draw all the lines
  for (int i = -nsquare / 2; i <= nsquare / 2; i++) {
    switch (axe) {
      case 0:                  // X/Y plane
        x = x1 = square_size * (GLfloat) i;
        y = inf;
        y1 = sup;
        z = 0.0;
        z1 = 0.0;
        break;
      case 1:                  // Y/Z plane
        y = y1 = square_size * (GLfloat) i;
        z = inf;
        z1 = sup;
        x = 0.0;
        x1 = 0.0;
        break;
      case 2:                 // X/Z plane
        z = z1 = square_size * (GLfloat) i;
        x = inf;
        x1 = sup;
        y = 0.0;
        y1 = 0.0;
        break;
    }
    // one line
    glVertex3f(x, y, z);
    glVertex3f(x1, y1, z1);

  }
  // perpendicular line
  for (int i = -nsquare / 2; i <= nsquare / 2; i++) {
    switch (axe) {
      case 0:                  // X/Y plane
        y = y1 = square_size * (GLfloat) i;
        x = inf;
        x1 = sup;
        z = 0.0;
        z1 = 0.0;
        break;
      case 1:                  // Y/Z plane
        z = z1 = square_size * (GLfloat) i;
        y = inf;
        y1 = sup;
        x = 0.0;
        x1 = 0.0;
        break;
      case 2:                 // X/Z plane
        x = x1 = square_size * (GLfloat) i;
        z = inf;
        z1 = sup;
        y = 0.0;
        y1 = 0.0;
        break;
    }
    // one line
    glVertex3f(x, y, z);
    glVertex3f(x1, y1, z1);
  }
  glEnd();
  glEndList();
}


GLNewGridObject::GLNewGridObject(glm::mat4 *proj, glm::mat4 *model) : m_proj(proj), m_model(model)
{
  m_shader = new CShader(
          GlobalOptions::RESPATH.toStdString() + "/shaders/grid.vert",
          GlobalOptions::RESPATH.toStdString() + "/shaders/grid.frag"
          );
  if (!m_shader->init()) {
    delete m_shader;
    std::cerr << "Failed to initialize grid shader\n";
    exit(1);
  }

	glGenVertexArrays(1, &m_vertexarray);
	glBindVertexArray(m_vertexarray);

	glGenBuffers(1, &m_vertexbuffer);
  genVertexBufferData();

  glBindVertexArray(0);

}


void GLNewGridObject::sendShaderData() {
  auto mvp = (*m_proj)*(*m_model);
  m_shader->sendUniformXfv("MVP",16,1,&mvp[0][0]);
}

void GLNewGridObject::display() {
//  glLineWidth(0.5);
  glEnable(GL_LINE_SMOOTH);
  m_shader->start();
  glBindVertexArray(m_vertexarray);
	glBindBufferARB(GL_ARRAY_BUFFER, m_vertexbuffer);

  sendShaderData();

  // 1rst attribute buffer : vertices
  glEnableVertexAttribArrayARB(0);
  glVertexAttribPointer(
    0,                  // attribute. No particular reason for 0, but must match the layout in the shader.
    3,                  // size
    GL_FLOAT,           // type
    GL_FALSE,           // normalized?
    0,                  // stride
    (void*)0            // array buffer offset
  );

  glDrawArrays(GL_LINES, 0, m_nb_lines*2*2); // 3 indices starting at 0 -> 1 triangle

  glDisableVertexAttribArray(0);
  glBindVertexArray(0);

}

void GLNewGridObject::genVertexBufferData() {

	vector<float> vertex_position;

  for(int i = 0; i < m_nb_lines; i++) {
    vertex_position.push_back(-(m_line_gap*m_nb_lines-m_line_gap)/2);
    vertex_position.push_back(m_line_gap * (float) i-(m_line_gap*m_nb_lines-m_line_gap)/2);
    vertex_position.push_back(0.0);
    vertex_position.push_back((m_line_gap*m_nb_lines-m_line_gap)/2);
    vertex_position.push_back(m_line_gap * (float) i-(m_line_gap*m_nb_lines-m_line_gap)/2);
    vertex_position.push_back(0.0);
  }

  for(int i = 0; i <= m_nb_lines; i++) {
    vertex_position.push_back(m_line_gap * (float) i-(m_line_gap*m_nb_lines-m_line_gap)/2);
    vertex_position.push_back(-(m_line_gap*m_nb_lines-m_line_gap)/2);
    vertex_position.push_back(0.0);
    vertex_position.push_back(m_line_gap * (float) i-(m_line_gap*m_nb_lines-m_line_gap)/2);
    vertex_position.push_back((m_line_gap*m_nb_lines-m_line_gap)/2);
    vertex_position.push_back(0.0);
  }
	glBindBufferARB(GL_ARRAY_BUFFER, m_vertexbuffer);
	glBufferDataARB(GL_ARRAY_BUFFER, sizeof(float)*vertex_position.size(), vertex_position.data(), GL_STATIC_DRAW);
  glBindBufferARB(GL_ARRAY_BUFFER, 0);

}

GLNewGridObject::~GLNewGridObject() {
	// Cleanup VBO and shader
	glDeleteBuffers(1, &m_vertexbuffer);
	glDeleteProgram(m_shader->getProgramId());
	glDeleteVertexArrays(1, &m_vertexarray);

}
} // namespace glnemo
