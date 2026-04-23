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
/**
        @author Jean-Charles Lambert <Jean-Charles.Lambert@lam.fr>
*/
#ifndef GLNEMOGLWINDOW_H
#define GLNEMOGLWINDOW_H

#include "camera.h"
#include "cshader.h"
#include "glaxesobject.h"
#include "glcolorbar.h"
#include "glcpoints.h"
#include "glcubeobject.h"
#include "globjectosd.h"
#include "globjectparticles.h"
#include "gloctree.h"
#include "glselection.h"
#include "gltexture.h"
#include "offscreenrenderer.h"
#include "particlesobject.h"
#include <GL/gl.h>
#include <QImage>
#include <QMutex>
#include <QOpenGLFunctions_3_3_Core>
#include <QOpenGLWidget>
#include <QRecursiveMutex>
#include <QSurface>
#include <glm/fwd.hpp>
#include <glm/gtc/type_ptr.hpp>

class fntTexFont;

namespace glnemo {
class GLGridObject;
class GlobalOptions;

class GLWindow : public QOpenGLWidget, protected QOpenGLFunctions_3_3_Core {
  Q_OBJECT
public:
  GLWindow(QWidget *, GlobalOptions *, QRecursiveMutex *, Camera *,
           CPointsetManager *);
  ~GLWindow();

  void bestZoomFit();
  void resize(const int w, const int h) { resizeGL(w, h); }
  void resizeOsd(const int w, const int h) { osd->setWH(w, h); }
  void resetView() { // reset view to initial
    resetMatScreen();
    resetMatScene();
    reset_screen_rotation = true;
    reset_scene_rotation = true;
    setRotationScreen(0, 0, 0);
    setRotationScene(0, 0, 0);
    setTranslation(0, 0, 0);
    resetEvents(true);
  }
  void gpvClear() { gpv.clear(); }
  static bool GLSL_support;
  static GLWindow *m_glWidget;
  static QOpenGLFunctions_3_3_Core *m_glFunctions;
  void setFBO(bool _b) { fbo = _b; }
  void setFBOSize(GLuint w, GLuint h) {
    texWidth = w;
    texHeight = h;
  }
  QImage grabFrameBufferObject() { return imgFBO; }
  void rotateAroundAxis(const int);
  void setMouseRot(const float x, const float y, const float z) {
    x_mouse = (int)y;
    y_mouse = (int)x;
    z_mouse = (int)z;
  }
  // select area
  GLSelection *gl_select;
  static void checkGLErrors(std::string s);
  // color bar
  GLColorbar *gl_colorbar;

  static void printMatrix(GLdouble *matrix, std::string text = "") {
    std::cerr << text << "\n";
    for (int i = 0; i < 4; i++) {
      for (int j = 0; j < 4; j++) {
        std::cerr << matrix[i * 4 + j] << " ";
      }
      std::cerr << "\n";
    }
    std::cerr << "\n----------\n";
  }

  static void printMatrix(GLfloat *matrix, std::string text = "") {
    std::cerr << text << "\n";
    for (int i = 0; i < 4; i++) {
      for (int j = 0; j < 4; j++) {
        std::cerr << matrix[i * 4 + j] << " ";
      }
      std::cerr << "\n";
    }
    std::cerr << "\n----------\n";
  }

  static void copyGlmtoMatrix(glm::mat4 src, double dest[16]) {
    for(int i = 0; i < 4; ++i) {
      for(int j = 0; j < 4; ++j) {
          dest[i * 4 + j] = static_cast<double>(src[i][j]);
      }
    }
  }
signals:
  void sigKeyMouse(const bool, const bool);
  void sigScreenshot();
  void leaveEvent();
  void sigMouseXY(const int x, const int y);
  void doneRendering();
  void selectTreeWidgetItem(int cpoint_id);
  void unselectTreeWidgetItem(int cpoint_id);
  void unselectTreeWidgetAll();
  void sendGLWindow(GLWindow *);
public slots:
  void update(ParticlesData *, ParticlesObjectVector *, GlobalOptions *,
              const bool update_old_obj = true);
  void update(ParticlesObjectVector *);
  void update();
  void updateVbo(const int);
  void updateBoundaryPhys(const int, const bool);
  void updateColorVbo(const int);
  void changeColorMap();
  void reverseColorMap();
  void rebuildGrid(bool ugl = true);
  void updateGrid(bool ugl = true);
  void updateGL();
  void updateOsdZrt(bool ugl = true);
  void forcePaintGL() {
    makeCurrent();
    paintGL();
    // doneCurrent();
  }
  void saveOffsreen(const QString &filePath) {
    m_fbo.saveToFile(this, filePath);
  };
  void select_all_particles_on_screen() { // from gui, interactive select, press
                                          // button select all particles
    gl_select->selectOnArea(pov->size(), mProj, mModel, viewport, true);
  }

  void osdZoom(bool ugl = true);
  void setOsd(const GLObjectOsd::OsdKeys k, const QString text, bool show,
              bool b = true);
  void setOsd(const GLObjectOsd::OsdKeys k, const int value, bool show,
              bool b = true);
  void setOsd(const GLObjectOsd::OsdKeys k, const float value, bool show,
              bool b = true);
  void setOsd(const GLObjectOsd::OsdKeys k, const float value1,
              const float value2, const float value3, bool show, bool b = true);
  void changeOsdFont();
  void toggleRotateScreen() {

    last_posx = last_posy = last_posz = 0;
    if (store_options->rotate_screen) {
      y_mouse = store_options->xrot;
      x_mouse = store_options->yrot;
      z_mouse = store_options->zrot;
    } else {
      y_mouse = store_options->urot;
      x_mouse = store_options->vrot;
      z_mouse = store_options->wrot;
    }
  }

  void resetFrame() { nframe = 0; }
  int getFrame() { return nframe; }
  void updateColorbarFont() { gl_colorbar->updateFont(); }

protected:
  void initializeGL() override;
  void paintGL();
  void resizeGL(int w, int h);
  QOpenGLContext *gl_context;
private slots:
  void updateVel(const int); // update velocity vector
  void updateIpvs(const int ipvs = -1) {
    p_data->setIpvs(ipvs);
    gl_colorbar->update(&gpv, p_data->getPhysData(), store_options, mutex_data);
  }
  void leaveEvent(QEvent *event) {
    if (event) {
      ;
    }
    emit leaveEvent();
  }
  void rotateAroundX() { rotateAroundAxis(0); }
  void rotateAroundY() { rotateAroundAxis(1); }
  void rotateAroundZ() { rotateAroundAxis(2); }
  void rotateAroundU() { rotateAroundAxis(3); }
  void rotateAroundV() { rotateAroundAxis(4); }
  void rotateAroundW() { rotateAroundAxis(5); }
  void resetMatrix(const bool b = true) {
    if (b) {
      resetMatScreen();
      resetMatScene();
      // reset_screen_rotation = true;
      // reset_scene_rotation  = true;
      last_xrot = last_yrot = last_zrot = 0.0;
      last_posx = last_posy = last_posz = 0.0;
      y_mouse = store_options->xrot;
      x_mouse = store_options->yrot;
      z_mouse = store_options->zrot;
    }
  }
  void translateX() { translateAlongAxis(0); }
  void translateY() { translateAlongAxis(1); }
  void translateZ() { translateAlongAxis(2); }
  void setTextureObject(const int, const int);
  void resetEvents(bool pos = false);
  void resetMatScreen() { 
    memcpy(mScreen, mIdentity, 16 * sizeof(GLdouble)); 
    m_screen = glm::mat4(1.0f);
  }
  void resetMatScene() { 
    memcpy(mScene, mIdentity, 16 * sizeof(GLdouble)); 
    m_scene = glm::mat4(1.0f);
  }

private:
  // OpenGL
  QSet<QByteArray> gl_extensions;
  const GLubyte *gl_version;
  int gl_minor, gl_major;

  // my parent
  QWidget *parent;
  // global options
  GlobalOptions *store_options;
  // grid variables
  GLGridObject *gridx, *gridy, *gridz;
  GLCubeObject *cube;
  // axes
  GLAxesObject *axes;
  // Vectors
  GLObjectParticlesVector gpv;
  ParticlesObjectVector *pov;
  CPointsetManager *cpointset_manager;

  ParticlesData *p_data;
  // projections
  void setProjection(const int x, const int y, const int w, const int h);
  void computeOrthoFactor();
  float ortho_left, ortho_right, ortho_bottom, ortho_top;
  float ratio, fx, fy;
  int wwidth, wheight;
  GLuint texWidth, texHeight;
  QImage imgFBO;
  bool fbo;
  // events
  void mousePressEvent(QMouseEvent *e);
  void mouseReleaseEvent(QMouseEvent *e);
  void mouseMoveEvent(QMouseEvent *e);
  void wheelEvent(QWheelEvent *e);
  void keyReleaseEvent(QKeyEvent *k);
  void keyPressEvent(QKeyEvent *k);

  bool is_pressed_left_button;
  bool is_pressed_right_button;
  bool is_pressed_middle_button;
  bool is_pressed_ctrl;
  bool is_mouse_pressed;
  bool is_mouse_zoom;
  bool is_key_pressed;
  bool is_a_key_pressed;
  bool is_ctrl_pressed;

  void translateAlongAxis(const int);

  // transformations (rotation, translation)
  bool is_translation;
  int x_mouse, y_mouse, z_mouse, tx_mouse, ty_mouse, tz_mouse, last_posx,
      last_posy, last_posz;
  float last_xrot, last_yrot, last_zrot;
  float last_urot, last_vrot, last_wrot;
  int i_umat, i_vmat, i_wmat; // index of the SCENE/Object rotation matrix

  void setRotationScreen(const int x, const int y, const int z);
  void setRotationScene(const int u, const int v, const int w);
  void getPixelTranslation(int *x, int *y, int *z);
  void setTranslation(const int x, const int y, const int z);
  void setZoom(const int z);
  void setZoom(const float z);
  int zoom_dynam;

    // gl matrix
  GLdouble mProj[16], mModel[16], mModel2[16], mScreen[16], mScene[16],
      mRot[16];
  glm::mat4 m_screen, m_scene, m_rot;
  GLdouble static mIdentity[16];
  int viewport[4];
  bool reset_screen_rotation, reset_scene_rotation;
  void setProjMatrix() {
    glGetDoublev(GL_PROJECTION_MATRIX, (GLdouble *)mProj);
   // GLWindow::printMatrix(mProj, ">> mProj");
    //GLWindow::printMatrix(glm::value_ptr(store_options->mat4_proj), ">> mat4_proj");
    GLWindow::copyGlmtoMatrix(store_options->mat4_proj, mProj);
    //GLWindow::printMatrix(mProj, ">> new mProj");
    // GLWindow::printMatrix(mm3, ">> mm3 diff");
  }
  void setModelMatrix() {
    glGetDoublev(GL_MODELVIEW_MATRIX, (GLdouble *)mModel);
    //GLWindow::printMatrix(mModel, "setModelMatrix => mModel");
    glm::mat4 mv=store_options->mat4_model*store_options->mat4_view;
    //GLWindow::printMatrix(glm::value_ptr(mv), "setModelMatrix => mv");
    GLWindow::copyGlmtoMatrix( mv, mModel);
    //GLWindow::printMatrix(mModel, "setModelMatrix => mModel transformed");
    // GLWindow::printMatrix(mm, "mm to mModel");
    // GLWindow::printMatrix(glm::value_ptr(mv), ">> mat4_modeliview");
   }
  void setViewPort() { glGetIntegerv(GL_VIEWPORT, viewport); }

  void setPerspectiveMatrix();
  // OSD
  GLObjectOsd *osd;
  QImage image, gldata;
  // Font
  fntTexFont *font;
  // Font
  // fntRenderer * text;
  // Texture vector
  GLTextureVector gtv;
  // Thread
  QRecursiveMutex *mutex_data;

  bool is_shift_pressed;
  // bench
  int nframe;
  //   Shaders
  CShader *shader, *vel_shader;
  void initShader();
  unsigned int m_vertexShader, m_pixelShader;

  static const char vertexShader[];
  static const char pixelShader[];
  // Lights
  void initLight();
  // octree
  GLOctree *tree;
  // Camera
  Camera *camera;
  // OffscreeRendering
  OffscreenRenderer m_fbo;
};
} // namespace glnemo

#endif
