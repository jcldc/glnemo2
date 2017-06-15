// ============================================================================
// Copyright Jean-Charles LAMBERT - 2007-2017
// e-mail:   Jean-Charles.Lambert@lam.fr
// address:  Centre de donneeS Astrophysique de Marseille (CeSAM)
//           Laboratoire d'Astrophysique de Marseille
//           Pôle de l'Etoile, site de Château-Gombert
//           38, rue Frédéric Joliot-Curie
//           13388 Marseille cedex 13 France
//           CNRS U.M.R 7326
// ============================================================================
// See the complete license in LICENSE and/or "http://www.cecill.info".
// ============================================================================
#include <QtGlobal>
//#if QT_VERSION >= QT_VERSION_CHECK(5, 0, 0)
#include <GL/glew.h>
#include <QtGui>
//#endif
#include <QtOpenGL>
#include <QFile>
#include <QString>

#include <sstream>
#include "camera.h"
#include "glwindow.h"
#include <GL/glu.h>
#include "glfilaments.h"

namespace glnemo {

// ============================================================================
// constructor
GLFilaments::GLFilaments(GlobalOptions *so)
{
  shader = NULL;
  store_options = so;
  texture=NULL;
}
// ============================================================================
// destructor
GLFilaments::~GLFilaments()
{

}
// ============================================================================
// loadShader
void GLFilaments::loadShader()
{
  if (GLWindow::GLSL_support) {
    glGenBuffersARB(1,&vbo_points);
    glGenBuffersARB(1,&vbo_color);
    // particles shader
    shader = new CShader(GlobalOptions::RESPATH.toStdString()+"/shaders/camera.vert.cc",
                         GlobalOptions::RESPATH.toStdString()+"/shaders/camera.frag.cc");
    shader->init();
  }
  if (! texture) texture = new GLTexture();
  if (texture->load(GlobalOptions::RESPATH+"gaussian",NULL)) {
  } else {
    std::cerr << "\n\nUnable to load TEXTURE.....\n\n";
  }

}
// ============================================================================
// load (filename)
void GLFilaments::load(const std::string filename)
{
  bool valid=false;
  QFile infile(filename.c_str());
  vertex.clear();

  if (infile.open(QIODevice::ReadOnly | QIODevice::Text)) {
    QTextStream in(&infile);
    QString line;
    int cpt=0;
    int start=0;
    int n=0;
    int len_segment=0;
    line = in.readLine();

    float u,v,w;
    u=v=w=0.0;
    bool first=true;
    while (!line.isNull()) {
      valid=true;
      cpt++;
      if (cpt > 3) { // skip first 3 lines (header)
            std::istringstream ss(line.toStdString()); // parse string
        float x,y,z;
        ss >> x;
        ss >> y;
        ss >> z;
        vertex.push_back(x);
        vertex.push_back(y);
        vertex.push_back(z);
        if (first) {
          first=false;
          ss >> u;
          ss >> v;
          ss >> w;
        } else {
          if (u!=x && v!=y && z!=w) { // end of segment
            // new segment
            n=cpt-4-start;
            seg_properties sp(start,n);
            segment.push_back(sp);
            start+=n;
            // we add latest u,v,w vertex as end of segment
            //vertex.push_back(u);
            //vertex.push_back(v);
            //vertex.push_back(w);
          }


          // read new vertex
          ss >> u;
          ss >> v;
          ss >> w;

        }
      }
      line=in.readLine();
    } // end of while
    for (int i=0; i<segment.size();i++) {
      std::cerr << i << " -- " << segment[i].start << " -- " << segment[i].n << "\n";
    }

  }
  if (valid) {
    updateVbo();
  }
  //
}
// ============================================================================
// updateVbo
void GLFilaments::updateVbo()
{
  // bind VBO buffer for sending data
  glBindBufferARB(GL_ARRAY_BUFFER_ARB, vbo_points);
  // upload vertex to VBO
  glBufferDataARB(GL_ARRAY_BUFFER_ARB,vertex.size()*sizeof(GLfloat),&vertex[0], GL_STATIC_DRAW_ARB);
  // unbind
  glBindBufferARB(GL_ARRAY_BUFFER_ARB, 0);

  vertex.clear(); // free memory
}
// ============================================================================
// display
// display segments
void GLFilaments::display(const int _win_height)
{
  win_height = _win_height;

  // color
  mycolor = Qt::yellow;
  glColor4ub(mycolor.red(), mycolor.green(), mycolor.blue(),mycolor.alpha());

  glTexEnvi(GL_POINT_SPRITE,GL_COORD_REPLACE,GL_TRUE);

  // ------------------------------
  // start shader
  // ------------------------------

  shader->start();

  // texture
  glActiveTextureARB(GL_TEXTURE0_ARB);
  texture->glBindTexture();  // bind texture

  // send matrix
  GLfloat proj[16];
  glGetFloatv( GL_PROJECTION_MATRIX,proj);
  shader->sendUniformXfv("projMatrix",16,1,&proj[0]);
  GLfloat mview[16];
  glGetFloatv( GL_MODELVIEW_MATRIX,mview);
  shader->sendUniformXfv("modelviewMatrix",16,1,&mview[0]);

  // Send data to Pixel Shader
  shader->sendUniformi("splatTexture",0);

  // get attribute location for sprite size
  int a_sprite_size = glGetAttribLocationARB(shader->getProgramId(), "a_sprite_size");
  glVertexAttrib1fARB(a_sprite_size,1.0);
  if ( a_sprite_size == -1) {
    std::cerr << "Error occured when getting \"a_sprite_size\" attribute\n";
    exit(1);
  }
  // setup lines
  glDisable(GL_DEPTH_TEST);
  glEnable (GL_LINE_SMOOTH);
  glEnable (GL_BLEND);
  glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); // default


  glHint (GL_LINE_SMOOTH_HINT, GL_DONT_CARE);
  glLineWidth (4.5);

  glBindBufferARB(GL_ARRAY_BUFFER_ARB, vbo_points);
  glEnableClientState(GL_VERTEX_ARRAY);
  glVertexPointer((GLint) 3, GL_FLOAT, (GLsizei) 0, (void *) 0);
  for (int i=0; i<segment.size(); i+=1) {
    if (segment[i].n > 1) {
      glDrawArrays(GL_LINE_STRIP, segment[i].start, segment[i].n);
    }
  }
  glBindBufferARB(GL_ARRAY_BUFFER_ARB, 0);

  shader->stop();

  glDisableClientState(GL_VERTEX_ARRAY);
  glDisable(GL_POINT_SPRITE_ARB);
  glDisable(GL_BLEND);
  glDepthMask(GL_TRUE);
  glEnable(GL_DEPTH_TEST);

}
}
