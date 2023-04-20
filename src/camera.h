// ============================================================================
// Copyright Jean-Charles LAMBERT - 2007-2023                                  
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

#ifndef CAMERA_H
#define CAMERA_H
#include <QObject>
#include <QColor>
#include <QTimer>
//#include <QOpenGLWidget>
#include <QOpenGLFunctions>
#include <fstream>
#include "cshader.h"
#include "catmull_rom_spline.h"
#include "globject.h"
#include "globaloptions.h"
#include "gltexture.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

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
    // toogle spline mode according camera user option
    void toggleSplineMode(const bool _sm, const bool ugl=true) {
      spline_mode=_sm;
      if (!spline_mode) {
        Vec3D zero(0.,0.,0.);
        rv=zero;
      }
      if (ugl) { // updateGL required
        emit updateGL();
      }
    }
    void updateVectorUp(const int idx, const float value, const bool ugl=true) {
      up[idx] = value;
      if (ugl) { // updateGL required
        emit updateGL();
      }
    }
    //
    void setCamDisplayLoop(const bool _v) {
      play_loop = _v;
    }

    void startStopPlay(const bool);
    void display(const int win_height);
    void reset();
    
  signals:
    void updateGL();
    void sig_stop_play();

  private slots:
    void playGL() { emit updateGL();}

  private:
    void updateVbo();
    void displayVbo();
    void displayShaderCtrl();
    void checkGSLSupport();
    bool GLSL_support;
    GLTexture * texture;
    QOpenGLFunctions * ogl_function;
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
    // camera
    glm::vec3 last_up, up; // camera up vector
    bool spline_mode;
    bool play_loop; // loop on play ?
    
  };
}
#endif // CAMERA_H
