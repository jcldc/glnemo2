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
#include "glcubeobject2.h"
#include <qcolor.h>

namespace glnemo {

using namespace std;
// ============================================================================
void GLCubeObject2::setColor(const QColor& color) { 
  setVec4FromQColor(m_color, color);
}
// -- rebuild

void GLCubeObject2::rebuild(float square_size)
{
  m_squareSize = square_size;
  destroy();   // libère VAO + VBO existants
  build();     // en recrée de nouveaux
}
// ── Construction du VAO/VBO ───────────────────────────────────────────────
void GLCubeObject2::build()
{
  QOpenGLExtraFunctions *f = QOpenGLContext::currentContext()->extraFunctions();
  std::vector<float> vertices;
  
  float hs = m_squareSize/2.; // half square
  
  // bottom square
  // A
  vertices.push_back(-hs);
  vertices.push_back(-hs);
  vertices.push_back(-hs);
  // B
  vertices.push_back(hs);
  vertices.push_back(-hs);
  vertices.push_back(-hs);
  // B
  vertices.push_back(hs);
  vertices.push_back(-hs);
  vertices.push_back(-hs);
  // C
  vertices.push_back(hs);
  vertices.push_back(hs);
  vertices.push_back(-hs);
  // C
  vertices.push_back(hs);
  vertices.push_back(hs);
  vertices.push_back(-hs);
  // D
  vertices.push_back(-hs);
  vertices.push_back(hs);
  vertices.push_back(-hs);
  // D
  vertices.push_back(-hs);
  vertices.push_back(hs);
  vertices.push_back(-hs);
  // A
  vertices.push_back(-hs);
  vertices.push_back(-hs);
  vertices.push_back(-hs);
  // top square
  // E
  vertices.push_back(-hs);
  vertices.push_back(-hs);
  vertices.push_back(hs);
  // F
  vertices.push_back(hs);
  vertices.push_back(-hs);
  vertices.push_back(hs);
  // F
  vertices.push_back(hs);
  vertices.push_back(-hs);
  vertices.push_back(hs);
  // G
  vertices.push_back(hs);
  vertices.push_back(hs);
  vertices.push_back(hs);
  // G
  vertices.push_back(hs);
  vertices.push_back(hs);
  vertices.push_back(hs);
  // H
  vertices.push_back(-hs);
  vertices.push_back(hs);
  vertices.push_back(hs);
  // H
  vertices.push_back(-hs);
  vertices.push_back(hs);
  vertices.push_back(hs);
  // E
  vertices.push_back(-hs);
  vertices.push_back(-hs);
  vertices.push_back(hs);

  // We connect plans
  // A
  vertices.push_back(-hs);
  vertices.push_back(-hs);
  vertices.push_back(-hs);
  // E
  vertices.push_back(-hs);
  vertices.push_back(-hs);
  vertices.push_back(hs);
  // B
  vertices.push_back(hs);
  vertices.push_back(-hs);
  vertices.push_back(-hs);
  // F
  vertices.push_back(hs);
  vertices.push_back(-hs);
  vertices.push_back(hs);
  // D
  vertices.push_back(-hs);
  vertices.push_back(hs);
  vertices.push_back(-hs);
  // H
  vertices.push_back(-hs);
  vertices.push_back(hs);
  vertices.push_back(hs);
  // C
  vertices.push_back(hs);
  vertices.push_back(hs);
  vertices.push_back(-hs);
  // G
  vertices.push_back(hs);
  vertices.push_back(hs);
  vertices.push_back(hs);

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
void GLCubeObject2::draw(GLuint shaderProgram,
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
  void GLCubeObject2::destroy()
  {
    QOpenGLContext* ctx = QOpenGLContext::currentContext();
    if (!ctx) {
        // contexte absent : fuite mémoire GPU mais au moins pas de crash
        // et on remet à 0 pour forcer une réallocation dans build()
        qWarning("GLCubeObject2::destroy() appelé sans contexte OpenGL actif !");
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

  GLCubeObject2::~GLCubeObject2() { destroy(); }


}
