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
    QFile infile(filename.c_str());
    if (infile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QTextStream in(&infile);
        QString line;
        bool valid=false;
        int cpt=0;
        int start=0;
        int n=0;
        int len_segment=0;
        line = in.readLine();
        std::vector<GLfloat> vertex;
        float u,v,w;
        u=v=w=0.0;
        bool first=true;
        while (!line.isNull()) {
            valid=true;
            cpt++;
            if (cpt > 3) {
                std::istringstream ss(line.toStdString());
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
                        n=cpt-4-start+1;
                        seg_properties sp(start,n);
                        segment.push_back(sp);
                        start+=n;
                        // we add u,v,w vertex which is the end of segment
                        vertex.push_back(u);
                        vertex.push_back(v);
                        vertex.push_back(w);
                      }
                    ss >> u;
                    ss >> v;
                    ss >> w;

                }


            }
            line=in.readLine();
        }
     }
  }

  // ============================================================================
  // display
  // display segments
  void GLFilaments::display(const int _win_height)
  {
    win_height = _win_height;

  }
}
