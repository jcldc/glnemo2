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
#include "glgridobject.h"
#include <qcolor.h>

namespace glnemo {

using namespace std;
// ============================================================================
void GLGridObject::setColor(const QColor& color) { 
  setVec4FromQColor(m_color, color);
}
// -- rebuild

void GLGridObject::rebuild(int nsquare, float square_size)
{
  m_nsquare    = nsquare;
  m_squareSize = square_size;
  destroy();   // libère VAO + VBO existants
  build();     // en recrée de nouveaux
}
// ── Construction du VAO/VBO ───────────────────────────────────────────────
void GLGridObject::build()
{
  QOpenGLExtraFunctions *f = QOpenGLContext::currentContext()->extraFunctions();
  std::vector<float> vertices;
  //vertices.reserve(6 * (m_nsquare + 1) * 2); // 2 points × 3 floats × 2 boucles
  //vertices.reserve((m_nsquare + 1) * 2 * 2 * 3);

  const float inf = static_cast<float>(-m_nsquare / 2) * m_squareSize;
  const float sup = static_cast<float>( m_nsquare / 2) * m_squareSize;

  // ── Lignes parallèles puis perpendiculaires ───────────────────────────
  int cpt=0;
  for (int pass = 0; pass < 2; ++pass) {
      for (int i = -m_nsquare / 2; i <= m_nsquare / 2; ++i) {
          float fi = static_cast<float>(i) * m_squareSize;
          float x, y, z, x1, y1, z1;

          switch (m_axe) {
              case 0: // Plan XY
                  if (pass == 0) { x = x1 = fi; y = inf; y1 = sup; z = z1 = 0.f; }
                  else           { y = y1 = fi; x = inf; x1 = sup; z = z1 = 0.f; }
                  break;
              case 1: // Plan YZ
                  if (pass == 0) { y = y1 = fi; z = inf; z1 = sup; x = x1 = 0.f; }
                  else           { z = z1 = fi; y = inf; y1 = sup; x = x1 = 0.f; }
                  break;
              default: // Plan XZ  (axe == 2)
                  if (pass == 0) { z = z1 = fi; x = inf; x1 = sup; y = y1 = 0.f; }
                  else           { x = x1 = fi; z = inf; z1 = sup; y = y1 = 0.f; }
                  break;
          }

          // Premier sommet
          vertices.push_back(x);  vertices.push_back(y);  vertices.push_back(z);
          // Deuxième sommet
          vertices.push_back(x1); vertices.push_back(y1); vertices.push_back(z1);
          cpt+=6;
      }
  }

  m_vertexCount = static_cast<GLsizei>(vertices.size() / 3);
  // ── Upload GPU ────────────────────────────────────────────────────────
  f->glGenVertexArrays(1, &m_vao);
  f->glGenBuffers(1, &m_vbo);

  f->glBindVertexArray(m_vao);

  f->glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
  f->glBufferData(GL_ARRAY_BUFFER,
                static_cast<GLsizeiptr>(vertices.size() * sizeof(float)),
                vertices.data(),
                GL_STATIC_DRAW);

  // location 0 : vec3 aPos
  f->glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
  f->glEnableVertexAttribArray(0);

  f->glBindBuffer(GL_ARRAY_BUFFER, 0);
  f->glBindVertexArray(0);
}

// ── Rendu ─────────────────────────────────────────────────────────────────
//  shaderProgram : ID du programme (vertex + fragment shaders compilés)
//  model / view / projection : matrices GLM issues de votre propre gestion
void GLGridObject::draw(GLuint shaderProgram,
          const glm::mat4& model,
          const glm::mat4& view,
          const glm::mat4& projection) const
{
  if (is_activated) {
    QOpenGLExtraFunctions *f = QOpenGLContext::currentContext()->extraFunctions();
    f->glUseProgram(shaderProgram);
    // Matrices
    f->glUniformMatrix4fv(f->glGetUniformLocation(shaderProgram, "uModel"),
                        1, GL_FALSE, &model[0][0]);
    f->glUniformMatrix4fv(f->glGetUniformLocation(shaderProgram, "uView"),
                        1, GL_FALSE, &view[0][0]);
    f->glUniformMatrix4fv(f->glGetUniformLocation(shaderProgram, "uProjection"),
                        1, GL_FALSE, &projection[0][0]);

    // Couleur
    f->glUniform4fv(f->glGetUniformLocation(shaderProgram, "uColor"),
                  1, &m_color[0]);

    // Dessin
    f->glBindVertexArray(m_vao);
    f->glDrawArrays(GL_LINES, 0, m_vertexCount);
    f->glBindVertexArray(0);
  }
}

  // ── Libération des ressources GPU ─────────────────────────────────────────
  void GLGridObject::destroy()
  {
    QOpenGLContext* ctx = QOpenGLContext::currentContext();
    if (!ctx) {
        // contexte absent : fuite mémoire GPU mais au moins pas de crash
        // et on remet à 0 pour forcer une réallocation dans build()
        qWarning("GLGridObject::destroy() appelé sans contexte OpenGL actif !");
        m_vao = 0;
        m_vbo = 0;
        m_vertexCount = 0;
        return;
    }
    QOpenGLExtraFunctions *f = ctx->extraFunctions();
      
    if (m_vao) { f->glDeleteVertexArrays(1, &m_vao); m_vao = 0; }
    if (m_vbo) { f->glDeleteBuffers(1, &m_vbo);       m_vbo = 0; }
    m_vertexCount = 0;
  }

  GLGridObject::~GLGridObject() { destroy(); }


}
