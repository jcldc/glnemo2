// ============================================================================
// Copyright Jean-Charles LAMBERT - 2007-2020                                  
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
#include "globaloptions.h"
#include <GL/glew.h>

namespace glnemo {

using namespace std;


Grid::Grid(int axis, glm::mat4 *proj, glm::mat4 *view, glm::mat4 *model, bool activated, QColor color = QColor(Qt::yellow)) : m_axis(axis), m_proj(proj), m_view(view), m_model(model), m_display(activated)
{
  m_color = color;
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


void Grid::sendShaderData() {
  auto mvp = (*m_proj)*(*m_view)*(*m_model);
  m_shader->sendUniformXfv("MVP",16,1,&mvp[0][0]);
  auto rgb_color = m_color.toRgb();
  m_shader->sendUniformXfv("grid_color",3,1,&glm::vec3(rgb_color.redF(),
                                                       rgb_color.greenF(),
                                                       rgb_color.blueF())[0]);
}

void Grid::display() {
  if(!m_display)
    return
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

void Grid::genVertexBufferData() {

	vector<glm::vec3> vertices;
	vector<float> position_data;

  for(int i = 0; i < m_nb_lines; i++) {
    // horizontal lines
    //left vertex
    float x = -(m_line_gap*m_nb_lines-m_line_gap)/2;
    float y = m_line_gap * (float) i-(m_line_gap*m_nb_lines-m_line_gap)/2;
    float z = 0.0;
    auto vertex = glm::vec3(x, y, z);
    vertices.push_back(vertex);
    //right vertex
    x = (m_line_gap*m_nb_lines-m_line_gap)/2;
    y = m_line_gap * (float) i-(m_line_gap*m_nb_lines-m_line_gap)/2;
    z = 0.0;
    vertex = glm::vec3(x, y, z);
    vertices.push_back(vertex);
    //vertical lines
    //bottom vertex
    x = (m_line_gap * (float) i-(m_line_gap*m_nb_lines-m_line_gap)/2);
    y = (-(m_line_gap*m_nb_lines-m_line_gap)/2);
    z = 0.0;
    vertex = glm::vec3(x, y, z);
    vertices.push_back(vertex);
    //top vertex
    x = (m_line_gap * (float) i-(m_line_gap*m_nb_lines-m_line_gap)/2);
    y = ((m_line_gap*m_nb_lines-m_line_gap)/2);
    z = 0;
    vertex = glm::vec3(x, y, z);
    vertices.push_back(vertex);
  }

  // X/Y plane
  auto rotation_matrix = glm::identity<glm::mat3>();

  // Y/Z plane
  if(m_axis == 1)
    rotation_matrix = glm::mat3(0,0,1,
                                0,1,0,
                                1,0,0);
  // X/Z plane
  else if(m_axis == 2)
    rotation_matrix = glm::mat3(1, 0, 0,
                                0, 0, 1,
                                0, 1, 0);

  for(auto vertex : vertices) {
    vertex = rotation_matrix * vertex;
    position_data.push_back(vertex.x);
    position_data.push_back(vertex.y);
    position_data.push_back(vertex.z);
  }


	glBindBufferARB(GL_ARRAY_BUFFER, m_vertexbuffer);
	glBufferDataARB(GL_ARRAY_BUFFER, sizeof(float)*position_data.size(), position_data.data(), GL_STATIC_DRAW);
  glBindBufferARB(GL_ARRAY_BUFFER, 0);

}

Grid::~Grid() {
	// Cleanup VBO and shader
	glDeleteBuffers(1, &m_vertexbuffer);
	glDeleteProgram(m_shader->getProgramId());
	glDeleteVertexArrays(1, &m_vertexarray);

}


bool Grid::isDisplay() const {
  return m_display;
}

void Grid::setDisplay(bool display) {
  m_display = display;
}

const QColor &Grid::getColor() const {
  return m_color;
}

void Grid::setColor(const QColor &color) {
  m_color = color;
}

int Grid::getNbLines() const {
  return m_nb_lines;
}

void Grid::setNbLines(int nb_lines) {
  if(nb_lines > 0)
    m_nb_lines = nb_lines;
}

float Grid::getLineGap() const {
  return m_line_gap;
}

void Grid::setLineGap(float line_gap) {
  if(line_gap > 0)
    m_line_gap = line_gap;
}
} // namespace glnemo
