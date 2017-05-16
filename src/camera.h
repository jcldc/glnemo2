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

#ifndef CAMERA_H
#define CAMERA_H
#include <QObject>
#include <QColor>
#include <QTimer>
//#include <QGLWidget>
#include <fstream>
#include "cshader.h"
#include "catmull_rom_spline.h"
#include "globject.h"
#include "globaloptions.h"
#include "gltexture.h"

namespace glnemo {

/**
	@author Jean-Charles Lambert <jean-charles.lambert@lam.fr>
*/
  class Camera: public GLObject{
    Q_OBJECT
  public:
    Camera(GlobalOptions * so);
    ~Camera();
    // init
    void init(std::string filename="", const int _p=2000, const float _s=1.0); // MUST BE CALLED AFTER MAKECURRENT()
    void loadShader();

    // positions and mouvement
    void setEye(const float, const float, const float);
    void setCenter(const float, const float, const float);
    void setUp(const float, const float, const float);
    void moveTo();
    // spline
    int loadSplinePoints(std::string file);
    
    // SLOTS
  public slots:
    void setSplineParam(const int ,const double, const bool ugl=true);  // set spline parameters
    void setCamDisplay(const bool , const bool, const bool ugl=true);  // toggle camera display
    void loadPath(const std::string filename, const int p, const float s) {
      init(filename,p,s);
    }

    void startStopPlay();
    void display(const int win_height);
    void reset();
    
  signals:
    void updateGL();
    
  private slots:
    void playGL() { emit updateGL();}

  private:
    void updateVbo();
    void displayVbo();
    void displayShaderCtrl();
    void checkGSLSupport();
    bool GLSL_support;
    GLTexture * texture;
  private:
    GlobalOptions * store_options;
    float
        ex, ey, ez,
        cx, cy, cz,
        ux, uy, uz;
    Vec3D rv;
    CRSpline * spline;
    // playing timer
    QTimer * play_timer;
    int npoints; // # interpolated points
    float scale; // Scale factor applied on control points
    int index_frame; 
    bool display_ctrl, display_path;
    bool play;   // true if playing
    void buildDisplayList();
    void displayCameraPath();
    // VBO
    GLuint vbo_path, vbo_ctrl;
    // Shader
    CShader * shader;
    // windows height
    int win_height;
    // color
    QColor mycolor;
    
  };
}
#endif // CAMERA_H
