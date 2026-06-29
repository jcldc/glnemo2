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
#include "gltextobject2.h"
#include <qcolor.h>

namespace glnemo {

using namespace std;

// ── Initialise FreeType, rasterise ASCII 32-127, build VAO ───────────────
//  fontPath  : absolute path to a .ttf font file
//  pixelSize : glyph height in pixels at scale=1.0
bool GLTextObject2::init(const std::string& fontPath, unsigned int pixelSize) {
// ── FreeType ──────────────────────────────────────────────────────────
  FT_Library ft;
  if (FT_Init_FreeType(&ft)) {
      fprintf(stderr, "[GLTextObject2] Failed to init FreeType\n");
      return false;
  }

  FT_Face face;
  if (FT_New_Face(ft, fontPath.c_str(), 0, &face)) {
      fprintf(stderr, "[GLTextObject2] Failed to load font: %s\n", fontPath.c_str());
      FT_Done_FreeType(ft);
      return false;
  }

  // Width=0 lets FreeType derive it from height automatically.
  FT_Set_Pixel_Sizes(face, 0, pixelSize);
  
  QOpenGLExtraFunctions *f = QOpenGLContext::currentContext()->extraFunctions();

  // ── Rasterise printable ASCII into individual GL textures ─────────────
  // Disable default 4-byte row alignment; FreeType bitmaps are tightly packed.
  f->glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

  for (unsigned char c = 32; c < 128; ++c) {
      if (FT_Load_Char(face, c, FT_LOAD_RENDER)) {
          fprintf(stderr, "[GLTextRenderer] Failed to load glyph '%c'\n", c);
          continue;
      }

      GLuint tex;
      f->glGenTextures(1, &tex);
      f->glBindTexture(GL_TEXTURE_2D, tex);

      // Single red channel – the fragment shader uses it as alpha.
      f->glTexImage2D(
          GL_TEXTURE_2D, 0, GL_RED,
          static_cast<GLsizei>(face->glyph->bitmap.width),
          static_cast<GLsizei>(face->glyph->bitmap.rows),
          0, GL_RED, GL_UNSIGNED_BYTE,
          face->glyph->bitmap.buffer
      );

      f->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
      f->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
      f->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
      f->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

      Glyph glyph = {
          tex,
          glm::ivec2(face->glyph->bitmap.width, face->glyph->bitmap.rows),
          glm::ivec2(face->glyph->bitmap_left,  face->glyph->bitmap_top),
          static_cast<GLuint>(face->glyph->advance.x)   // 26.6 fixed-point
      };
      m_glyphs[c] = glyph;
  }

  f->glBindTexture(GL_TEXTURE_2D, 0);
  FT_Done_Face(face);
  FT_Done_FreeType(ft);

  // ── VAO / VBO ─────────────────────────────────────────────────────────
  // The VBO is updated every frame (GL_DYNAMIC_DRAW): one 6-vertex quad
  // per character, each vertex = vec4(x, y, u, v).
  f->glGenVertexArrays(1, &m_vao);
  f->glGenBuffers(1, &m_vbo);

  f->glBindVertexArray(m_vao);
  f->glBindBuffer(GL_ARRAY_BUFFER, m_vbo);

  // Allocate enough room for one quad (6 vertices × 4 floats).
  // We'll overwrite it with glBufferSubData for each character.
  f->glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 6 * 4, nullptr, GL_DYNAMIC_DRAW);

  f->glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
  f->glEnableVertexAttribArray(0);

  f->glBindBuffer(GL_ARRAY_BUFFER, 0);
  f->glBindVertexArray(0);

  return true;

}
// ── Render a string ───────────────────────────────────────────────────────
//  text  : UTF-8 string (ASCII subset only in this implementation)
//  x, y  : position in pixels, origin = bottom-left of the window
//  scale : multiplier applied to the glyph size (1.0 = original pixel size)
//  color : RGBA, components in [0, 1]
void GLTextObject2::draw(const std::string& text,
          float x, float y,
          float scale,
          const glm::vec4& color) {
  // Build an orthographic projection: pixel (0,0) → NDC bottom-left.
  glm::mat4 proj = glm::ortho(
      0.f, static_cast<float>(m_screenW),
      0.f, static_cast<float>(m_screenH)
  );

  QOpenGLExtraFunctions *f = QOpenGLContext::currentContext()->extraFunctions();
  f->glUseProgram(m_shader);
  f->glUniformMatrix4fv(f->glGetUniformLocation(m_shader, "uProjection"),
                      1, GL_FALSE, glm::value_ptr(proj));
  f->glUniform4fv(f->glGetUniformLocation(m_shader, "uTextColor"),
                1, glm::value_ptr(color));

  // Bind glyph texture unit 0
  f->glActiveTexture(GL_TEXTURE0);
  f->glUniform1i(f->glGetUniformLocation(m_shader, "uGlyphTexture"), 0);

  // Enable blending so glyph alpha works correctly.
  // Save previous blend state if you need to restore it.
  f->glEnable(GL_BLEND);
  f->glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

  f->glBindVertexArray(m_vao);

  float cursorX = x;  // advances rightward as characters are drawn

  for (char c : text) {
      auto it = m_glyphs.find(static_cast<unsigned char>(c));
      if (it == m_glyphs.end()) continue;
      const Glyph& g = it->second;

      // Top-left pixel position of this glyph's bitmap on screen
      float xpos = cursorX + g.bearing.x * scale;
      float ypos = y       - (g.size.y - g.bearing.y) * scale;

      float w = g.size.x * scale;
      float h = g.size.y * scale;

      // Two triangles forming a quad (CCW winding):
      //   (xpos,ypos+h) ──── (xpos+w,ypos+h)
      //        │                    │
      //   (xpos,ypos  ) ──── (xpos+w,ypos  )
      //
      // Each row: x, y, texU, texV
      float quad[6][4] = {
          { xpos,     ypos + h,  0.f, 0.f },
          { xpos,     ypos,      0.f, 1.f },
          { xpos + w, ypos,      1.f, 1.f },

          { xpos,     ypos + h,  0.f, 0.f },
          { xpos + w, ypos,      1.f, 1.f },
          { xpos + w, ypos + h,  1.f, 0.f }
      };

      f->glBindTexture(GL_TEXTURE_2D, g.textureID);

      // Update VBO content for this character's quad
      f->glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
      f->glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(quad), quad);
      f->glBindBuffer(GL_ARRAY_BUFFER, 0);

      f->glDrawArrays(GL_TRIANGLES, 0, 6);

      // Advance cursor: advance is stored in 1/64 pixels (26.6 fixed-point)
      cursorX += (g.advance >> 6) * scale;
  }

  f->glBindVertexArray(0);
  f->glBindTexture(GL_TEXTURE_2D, 0);

}

}


