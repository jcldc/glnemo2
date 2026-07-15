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
#include "glcolorbar.h"
#include "glwindow.h"
#include <GL/gl.h>
#include <GL/glu.h>

namespace glnemo {

// ============================================================================
// constructor    
GLColorbar::GLColorbar(const GlobalOptions  * _go,bool _enable ):GLObject()
        , m_vao(0), m_vbo(0)
        , m_texColormap(0)
        , m_shader(0)
        , m_screenW(800), m_screenH(600)
        , m_alpha(1.f)
        , m_direction(Direction::Horizontal)
{
  go     = _go;
  is_activated = _enable;

  #if 0 // diable core330
  legend = new GLTextObject(); // new object for text display
  updateFont();
  #endif
}

// ============================================================================
// destructor                                                                  
GLColorbar::~GLColorbar()
{
  #if 0 // diable core330
  delete  legend;
  #endif
}
// ============================================================================
// void init 
bool GLColorbar::init(GLuint shader_program)
{
  initializeOpenGLFunctions();

  // -- VAO / VBO ----------------------------------------------------------
  // The square is 2 triangles = 6 vertices.
  // Each vertex: x(1) + y(1) + texU(1) = 3 floats.
  // Total: 6 × 3 = 18 floats. Updated every draw() via glBufferSubData.
  glGenVertexArrays(1, &m_vao);
  glGenBuffers(1, &m_vbo);

  glBindVertexArray(m_vao);
  glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
  glBufferData(GL_ARRAY_BUFFER, 18 * sizeof(float), nullptr, GL_DYNAMIC_DRAW);

  // location 0 : vec2 aPos  (x, y)
  glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
  glEnableVertexAttribArray(0);

  // location 1 : float aTexU  (colormap coordinate)
  glVertexAttribPointer(1, 1, GL_FLOAT, GL_FALSE, 3 * sizeof(float),
                        (void*)(2 * sizeof(float)));
  glEnableVertexAttribArray(1);

  glBindBuffer(GL_ARRAY_BUFFER, 0);
  glBindVertexArray(0);

  // -- Colormap texture (placeholder: greyscale ramp until setColormap()) -
  // std::vector<float> grey(256);
  // std::iota(grey.begin(), grey.end(), 0.f);
  // for (auto& v : grey) v /= 255.f;
  //setColormap(grey, grey, grey);   // initialise with a greyscale ramp
  setColormap((*go->R),(*go->G),(*go->B));   // initialise with a greyscale ramp

  // -- Shaders -----------------------------------------------------------
  m_shader = shader_program;
  return (m_shader != 0);
}
// -- Update the colormap texture from three equal-length vectors -----------
//  R, G, B : float values in [0, 1].  Vectors must have the same length.
//  Can be called at any time (even before init(), but the texture upload
//  requires an active GL context).
void GLColorbar::setColormap(const std::vector<float>& R,
                const std::vector<float>& G,
                const std::vector<float>& B)
{
  if (R.empty() || R.size() != G.size() || R.size() != B.size()) {
      fprintf(stderr, "[GLColormapSquare] R/G/B vectors must be non-empty "
                      "and have the same length.\n");
      return;
  }

  // Interleave into a packed RGB array
  const int n = static_cast<int>(R.size());
  std::vector<float> rgb;
  rgb.reserve(n * 3);
  for (int i = 0; i < n; ++i) {
      rgb.push_back(std::clamp(R[i], 0.f, 1.f));
      rgb.push_back(std::clamp(G[i], 0.f, 1.f));
      rgb.push_back(std::clamp(B[i], 0.f, 1.f));
  }

  // Delete previous texture if any
  if (m_texColormap) { 
    glDeleteTextures(1, &m_texColormap); m_texColormap = 0; 
  }

  glGenTextures(1, &m_texColormap);
  glBindTexture(GL_TEXTURE_1D, m_texColormap);

  // GL_LINEAR interpolation gives smooth colour transitions between entries.
  glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);

  glTexImage1D(GL_TEXTURE_1D, 0, GL_RGB32F,
                n, 0, GL_RGB, GL_FLOAT, rgb.data());

  glBindTexture(GL_TEXTURE_1D, 0);
}

// -- Draw the coloured square ----------------------------------------------
//  x0, x1 : left  / right  edge in pixels (from the left  of the window)
//  y0, y1 : bottom/ top    edge in pixels (from the bottom of the window)
void GLColorbar::draw(float x0, float x1, float y0, float y1)
{
  if (!m_shader || !m_texColormap) return;
  const float W = static_cast<float>(m_screenW);
  const float H = static_cast<float>(m_screenH);

  glm::mat4 proj = glm::ortho(0.f, W, 0.f, H, -1.f, 1.f);

  glUseProgram(m_shader);
  glUniformMatrix4fv(glGetUniformLocation(m_shader, "uProjection"),
                      1, GL_FALSE, glm::value_ptr(proj));
  glUniform1i(glGetUniformLocation(m_shader, "uColormap"), 0);
  glUniform1f(glGetUniformLocation(m_shader, "uAlpha"), m_alpha);

  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_1D, m_texColormap);

  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  glDisable(GL_DEPTH_TEST);

  // -- Build the quad vertices with UV coordinates ------------------------
  //
  //  Each vertex: { x, y, u }
  //  u = colormap coordinate in [0, 1].
  //
  //  Horizontal mapping (default):
  //    left  column (x0) → u = 0.0
  //    right column (x1) → u = 1.0
  //
  //  Vertical mapping:
  //    bottom row (y0) → u = 0.0
  //    top    row (y1) → u = 1.0
  //
  float u_x0y0, u_x1y0, u_x1y1, u_x0y1;

  if (m_direction == Direction::Horizontal) {
      u_x0y0 = 0.f;  u_x1y0 = 1.f;
      u_x1y1 = 1.f;  u_x0y1 = 0.f;
  } else {                              // Vertical
      u_x0y0 = 0.f;  u_x1y0 = 0.f;
      u_x1y1 = 1.f;  u_x0y1 = 1.f;
  }

  //  Layout (CCW winding):
  //
  //   (x0,y1) u_x0y1 ---- (x1,y1) u_x1y1
  //       │      tri 2         │
  //       │           tri 1   │
  //   (x0,y0) u_x0y0 ---- (x1,y0) u_x1y0
  //
  float verts[18] = {
      // triangle 1
      x0, y0, u_x0y0,
      x1, y0, u_x1y0,
      x1, y1, u_x1y1,
      // triangle 2
      x0, y0, u_x0y0,
      x1, y1, u_x1y1,
      x0, y1, u_x0y1
  };

  glBindVertexArray(m_vao);
  glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
  glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(verts), verts);
  glBindBuffer(GL_ARRAY_BUFFER, 0);

  glDrawArrays(GL_TRIANGLES, 0, 6);
  glBindVertexArray(0);

  glBindTexture(GL_TEXTURE_1D, 0);
  glEnable(GL_DEPTH_TEST);
}



// ============================================================================
// void updateFont
void GLColorbar::updateFont()
{
#if 0 // disable core330
  fntRenderer text;
  if (font) delete font;
  font = new fntTexFont(go->gcb_font_name.toStdString().c_str());  
  text.setFont(font);
  text.setPointSize(go->gcb_font_size);  
  legend->setFont(text);
  legend->setColor(go->gcb_color);
#endif
}

// ============================================================================
// void update
void GLColorbar::update(GLObjectParticlesVector * _gpv, PhysicalData * _phys_select,
                         GlobalOptions   * _go, QRecursiveMutex * _mutex)
{
  // update variables
  gpv           = _gpv;
  go            = _go;
  mutex_data    = _mutex;
  phys_select   = _phys_select;
  
}
// ============================================================================
// void GLSelection::display
void GLColorbar::display(const int _width, const int _height)
{
  height = _height;
  width  = _width;
  setScreenSize(width, height);
  //GLWindow::m_glWidget->makeCurrent(); // 17-apr-2026
  if (go && go->gcb_enable && phys_select && phys_select->isValid()) {
    #if 0
    glDisable( GL_DEPTH_TEST );
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    gluOrtho2D(0.,width,0.,height);
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();
        
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);// No Alpha bending accumulation
    glEnable(GL_BLEND);
    glLineWidth (1.01); // long time bug on ATI/Intel hardware !!!!
                        // On Intel witdh must be > 1 after shaders....crazy bug
    #endif 
    // draw box
    drawBox();
    #if 0
    // draw text
    drawLegend();
    glDisable(GL_BLEND);
    // draw color
    drawColor();
    #endif
    #if 0
    // go back to normal mode
    glMatrixMode( GL_PROJECTION );
    glPopMatrix();
    glMatrixMode( GL_MODELVIEW );
    glPopMatrix();
    glEnable( GL_DEPTH_TEST );
    #endif
  }
  //GLWindow::m_glWidget->doneCurrent();
}

// ============================================================================
// void GLColorbar::drawBox
// ***************************************************
// Est and West
// 
//       x=x[0][0]           x=x[1][0]
//       y=x[0][1]           y=x[1][1]
//                +-------+
//                |       |
//                |       |
//                |       |
//                |       |
//                |       |
//                |       |
//                |       |
//                |       |
//                +-------+
//       x=x[3][0]           x=x[2][0]
//       y=x[3][1]           y=x[2][1]
// ***************************************************
// North and South
// 
//       x=x[1][0]                      x=x[2][0]
//       y=x[1][1]                      y=x[2][1]
//                +----------------------+
//                |                      |
//                |                      |
//                |                      |
//                +----------------------+
//       x=x[0][0]                      x=x[3][0]
//       y=x[0][1]                      y=x[3][1]
//
// ***************************************************
//        (0,0) OpenGL   (screen)           (width,height) glText
//          +-------------------------------------+
//          |                                     |
//          |                                     |
//          |                                     |
//          |                                     |
//          |                                     |
//          |                                     |
//          |                                     |
//          |                                     |
//          +-------------------------------------+
//        (0,0) glText                         (width,height) OpenGL
void GLColorbar::drawBox()
{
  switch (go->gcb_orientation) { 
  case 0:// NORTH
    x[0][0] = width/2-go->gcb_pheight*width/2;
    x[0][1] = height-go->gcb_offset-go->gcb_pwidth*height;
    
    x[1][0] = x[0][0];
    x[1][1] = height-go->gcb_offset;
    
    x[2][0] = width/2+go->gcb_pheight*width/2;
    x[2][1] = x[1][1];
    
    x[3][0] = x[2][0];
    x[3][1] = x[0][1];
    m_direction = Direction::Horizontal;
    break;

  case 1:// EST
    x[0][0] = width-go->gcb_offset-go->gcb_pwidth*width;
    x[0][1] = height/2-go->gcb_pheight*height/2;
    
    x[1][0] = width-go->gcb_offset;
    x[1][1] = x[0][1];
    
    x[2][0] = x[1][0];
    x[2][1] = height/2+go->gcb_pheight*height/2;
    
    x[3][0] = x[0][0];
    x[3][1] = x[2][1];
    m_direction = Direction::Vertical;
    break;
  case 2:// SOUTH
    x[0][0] = width/2-go->gcb_pheight*width/2;
    x[0][1] = go->gcb_offset;
    
    x[1][0] = x[0][0];
    x[1][1] = go->gcb_offset+go->gcb_pwidth*height;
    
    x[2][0] = width/2+go->gcb_pheight*width/2;
    x[2][1] = x[1][1];
    
    x[3][0] = x[2][0];
    x[3][1] = x[0][1];
    m_direction = Direction::Horizontal;
    break;

  case 3:// WEST
    x[0][0] = go->gcb_offset;
    x[0][1] = height/2-go->gcb_pheight*height/2;
    
    x[1][0] = go->gcb_offset+go->gcb_pwidth*width;
    x[1][1] = x[0][1];
    
    x[2][0] = x[1][0];
    x[2][1] = height/2+go->gcb_pheight*height/2;
    
    x[3][0] = x[0][0];
    x[3][1] = x[2][1];
    m_direction = Direction::Vertical;
    break;
  default: break;  
  }
  draw(x[0][0], x[2][0], x[0][1], x[2][1]);
  #if 0
  // draw box
  glColor4f( 1.0f, 0.f, 0.f,1.f );
  glBegin(GL_LINE_STRIP);
  glVertex2i(x[0][0],x[0][1]);
  glVertex2i(x[1][0],x[1][1]);
  glVertex2i(x[2][0],x[2][1]);
  glVertex2i(x[3][0],x[3][1]);
  glVertex2i(x[0][0],x[0][1]);
  glEnd();
  #endif
}
// ============================================================================
// void GLColorbar::drawColor
void GLColorbar::drawColor() {
  if (go && phys_select && phys_select->isValid()) {
    // int large_box,long_box;
    int long_box;
    if (go->gcb_orientation == 1 || go->gcb_orientation == 3) { // Est or West
      long_box = x[3][1] - x[0][1];                             // -2 pixels
      // large_box=x[2][0]-x[3][0];
    } else {                        // South or North
      long_box = x[3][0] - x[0][0]; // -2 pixels
      // large_box=x[0][1]-x[1][1];
    }
    int ncolors = go->R->size();
    // ncolors = (go->gcb_max-percmin)*ncolors;

    int R, G, B;
    int cpt = 0;
    // for (int i=0; i<=vbox;i++) {
    for (int i = 0; i < long_box - 2; i++) {
      int index;
      if (go->dynamic_cmap) {  // dynamic cmap
        if (!go->reverse_cmap) //    normal cmap
          index = i * ncolors / (long_box - 2);
        else //    reverse cmap
          index = (long_box - i - 2) * ncolors / (long_box - 2);
      } else { // constant cmap
        int ncolors2 = (go->gcb_max - go->gcb_min) / 100. * ncolors;
        if (!go->reverse_cmap) //    normal cmap
          index =
              (go->gcb_min * ncolors) / 100. + i * ncolors2 / (long_box - 2);
        else //    reverse cmap
          index = ncolors - (go->gcb_min * ncolors) / 100. -
                  i * ncolors2 / (long_box - 2);

        // std::cerr << index << " " << percmin << " " << go->gcb_max << " " <<
        // ncolors2 << " "<< ncolors<< "\n";
      }

      if (index >= 0 && index < ncolors) {
        cpt++;
        R = pow((*go->R)[index], go->powercolor) * 255;
        G = pow((*go->G)[index], go->powercolor) * 255;
        B = pow((*go->B)[index], go->powercolor) * 255;
        // draw color line
        glColor3ub(R, G, B);
        glBegin(GL_LINES);
        if (go->gcb_orientation == 1 ||
            go->gcb_orientation == 3) { // Est or West
          glVertex2i(x[0][0] + 1, x[0][1] + i + 1);
          glVertex2i(x[1][0] - 1, x[1][1] + i + 1);
        } else { // South or North
          float fac = -1;
          // if (place==0) fac=1;
          glVertex2i(x[0][0] + i + 1, x[0][1] - fac);
          glVertex2i(x[1][0] + i + 1, x[1][1] + fac);
        }
        glEnd();
      } else {
      }
    }
    // std::cerr << "cpt =  " << cpt << " " << long_box << " +++ " <<  x[3][0] -
    // x[0][0] << "\n";
  }
}
// ============================================================================
// void GLColorbar::drawLegend
void GLColorbar::drawLegend()
{
  if (go && phys_select && phys_select->isValid()) {
    double value;
    double diff_rho=(log(phys_select->getMax()*go->gcb_factor)-log(phys_select->getMin()*go->gcb_factor))/100.;
    //max
    value=log(phys_select->getMin()*go->gcb_factor)+go->gcb_max*diff_rho;
    drawText(value,0);
    //max - 1/3 (max-min)
    value=log(phys_select->getMin()*go->gcb_factor)+(go->gcb_max-(go->gcb_max-go->gcb_min)/3.)*diff_rho;
    drawText(value,1);
    // max - 2/3 (max-min)
    value=log(phys_select->getMin()*go->gcb_factor)+(go->gcb_max-2.*(go->gcb_max-go->gcb_min)/3.)*diff_rho;
    drawText(value,2);
    // max - 3/3 (max-min) 
    value = log(phys_select->getMin()*go->gcb_factor)+go->gcb_min*diff_rho;
    drawText(value,3);
  }
}
// ============================================================================
// void GLColorbar::drawText
void GLColorbar::drawText(float value, int fac)
{
  #if 0  // disblae core330
  QString text1,text0="";
  int xx=0,yy=0,tw=0,th=0;
  // max
  if (!go->gcb_logmode) value=exp(value);
  if (fac ==0) {
    // add legend name for the first line
    text1=QString("%1 %2").arg(value,0,'E',go->gcb_ndigits).arg(go->gcb_legend_name);
  } else {
    text1=QString("%1").arg(value,0,'E',go->gcb_ndigits);
  }
  legend->setText(text0,text1);
  switch (go->gcb_orientation) {
  case 0: // North
    tw=legend->getTextWidth();    
    th=legend->getHeight();    
    xx=x[3][0]-tw/2-fac*(x[3][0]-x[0][0])/3.;
    yy=height-(-5+x[0][1]-th);
    break;
  case 1: // Est
    tw=legend->getTextWidth();
    xx=x[0][0]-tw-5;
    yy=5+x[0][1]+fac*(x[3][1]-x[0][1])/3.;
    break;
  case 2: // South
    tw=legend->getTextWidth();    
    xx=x[2][0]-tw/2-fac*(x[2][0]-x[1][0])/3.;
    yy=height-(5+x[1][1]);
    break;
  case 3: // West
    xx=5+x[1][0];
    yy=5+x[1][1]+fac*(x[2][1]-x[1][1])/3.;
    break;
  }
  legend->setPos(xx,yy,xx);
  font->begin(); // mandatory !!
  legend->display(width,height);
  font->end();   // mandatory !!
#endif
}
} // namespace glnemo
