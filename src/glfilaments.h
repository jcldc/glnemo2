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

#ifndef GLFILAMENTS_H
#define GLFILAMENTS_H
#include <QObject>
#include <QColor>
#include <QTimer>
#include <fstream>
#include "cshader.h"
#include "catmull_rom_spline.h"
#include "globject.h"
#include "globaloptions.h"
#include "gltexture.h"

namespace glnemo {
class seg_properties {
public:
  seg_properties(const int _s, const int _n) {
    start=_s; n=_n;
  }

  int start;
  int n;
};

class GLFilaments : public GLObject {
    Q_OBJECT
public:
    GLFilaments(GlobalOptions * so);
    ~GLFilaments();
signals:

private:
    std::vector<seg_properties> segment;
    GlobalOptions * store_options;
    GLTexture * texture;
    // VBO
    GLuint vbo_points, vbo_color;
    // Shader
    CShader * shader;
    // windows height
    int win_height;
    // color
    QColor mycolor;
    // vertex
    std::vector<GLfloat> vertex;
private slots:
    void updateVbo();


public slots:
    void load(const std::string filename);
    void display(const int win_height);
    void loadShader();
};
}

#endif // GLFILAMENTS_H
