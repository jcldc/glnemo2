// ============================================================================
// Copyright Jean-Charles LAMBERT - 2007-2018                                  
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
/**
	@author Jean-Charles Lambert <Jean-Charles.Lambert@lam.fr>
*/
#ifndef GLNEMOGLWINDOW_H
#define GLNEMOGLWINDOW_H

#include  "cshader.h"
#include <QGLWidget>
#include <QImage>
#include <QMutex>
#include "particlesobject.h"
#include "globjectparticles.h"
#include "glcubeobject.h"
#include "glselection.h"
#include "globjectosd.h"
#include "gltexture.h"
#include "gloctree.h"
#include "glcolorbar.h"
#include "glaxesobject.h"
#include "camera.h"
#include "glcpoints.h"
#include "new_camera.h"
//#include "vr.h"

#include <openvr.h>

class fntTexFont;

namespace glnemo {
class GLGridObject;
class GLNewGridObject;
class GlobalOptions;

class GLWindow : public QGLWidget {
  Q_OBJECT

public:
    GLWindow(QWidget *, GlobalOptions *, QMutex * , Camera *, CPointsetManager *, NewCamera*);
    ~GLWindow();

    vr::IVRSystem* vr_context;
    void bestZoomFit();
    void resize(const int w, const int h ) { resizeGL(w,h);}
    void resizeOsd(const int w, const int h ) { osd->setWH(w,h);}
    void resetView() {    // reset view to initial
      resetMatScreen();
      resetMatScene();
      reset_screen_rotation = true;
      reset_scene_rotation  = true;
      setRotationScreen(0,0,0);
      setRotationScene(0,0,0);
      setTranslation(0,0,0);
      resetEvents(true);
    }
    void gpvClear() { gpv.clear(); }
    static bool GLSL_support;
    void setFBO(bool _b) { fbo = _b; }
    void setFBOSize(GLuint w, GLuint h) { texWidth=w; texHeight=h;}
    QImage grabFrameBufferObject() { return imgFBO;}
//    void renderVR();
    void rotateAroundAxis(const int);
    void setMouseRot(const float x,const float y, const float z) {
      x_mouse = (int) y;
      y_mouse = (int) x;
      z_mouse = (int) z;
    }
    // select area
    GLSelection * gl_select;
    static void checkGLErrors(std::string s);
    // color bar
    GLColorbar * gl_colorbar;
    
  signals:
    void sigKeyMouse(const bool, const bool);
    void sigScreenshot();
    void leaveEvent();
    void sigMouseXY(const int x, const int y);
    void doneRendering();
    void selectTreeWidgetItem(int cpoint_id);
    void unselectTreeWidgetItem(int cpoint_id);
    void unselectTreeWidgetAll();

    // vr
    void editGazSlideSizeValueByDelta(float delta);

public slots:
   void  update(ParticlesData   * ,
                ParticlesObjectVector * ,
                GlobalOptions         * ,
                const bool update_old_obj=true);
   void  update(ParticlesObjectVector * );
   void  update();
   void  updateVbo(const int);   
   void  updateBoundaryPhys(const int, const bool);
   void  updateColorVbo(const int);
   void  changeColorMap();
   void  reverseColorMap();
   void  rebuildGrid(bool ugl=true);
   void  updateGrid(bool ugl=true);
   void  updateGL();
   void  updateOsdZrt(bool ugl=true);
   void forcePaintGL() {
       makeCurrent();
       paintGL();
   }
   void select_all_particles_on_screen() { // from gui, interactive select, press button select all particles
       gl_select->selectOnArea(pov->size(),mProj,mModel,viewport,true);
   }

   void  osdZoom(bool ugl=true);
   void setOsd(const GLObjectOsd::OsdKeys k,const QString text, bool show,bool b=true);
   void setOsd(const GLObjectOsd::OsdKeys k,const int value, bool show, bool b=true);
   void setOsd(const GLObjectOsd::OsdKeys k,const float value, bool show, bool b=true);
   void setOsd(const GLObjectOsd::OsdKeys k, const float value1, 
                      const float value2, const float value3,  bool show,bool b=true);
   void changeOsdFont();     
   void toggleRotateScreen() {
     
     last_posx = last_posy = last_posz =0;
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

   void resetFrame() { nframe=0; }
   int getFrame() { return nframe;}
protected:
  void	initializeGL();
  void	paintGL();
  void	resizeGL( int w, int h );

private slots:
  void updateVel(const  int); // update velocity vector
  void updateIpvs(const int ipvs=-1) {
    p_data->setIpvs(ipvs);
    gl_colorbar->update(&gpv,p_data->getPhysData(),store_options,mutex_data);
  }
  void leaveEvent ( QEvent * event ) {
      if (event) {;}
      emit leaveEvent();
    }
  void rotateAroundX() { rotateAroundAxis(0);}
  void rotateAroundY() { rotateAroundAxis(1);}
  void rotateAroundZ() { rotateAroundAxis(2);}
  void rotateAroundU() { rotateAroundAxis(3);}
  void rotateAroundV() { rotateAroundAxis(4);}
  void rotateAroundW() { rotateAroundAxis(5);}
  void resetMatrix(const bool b=true) {
    if (b) {
      resetMatScreen();
      resetMatScene();
      //reset_screen_rotation = true;
      //reset_scene_rotation  = true;
      last_xrot = last_yrot = last_zrot = 0.0;
      last_posx = last_posy = last_posz = 0.0;
      y_mouse=store_options->xrot;
      x_mouse=store_options->yrot;
      z_mouse=store_options->zrot;
    }
  }
  void translateX()    { translateAlongAxis(0); }
  void translateY()    { translateAlongAxis(1); }
  void translateZ()    { translateAlongAxis(2); }
  void setTextureObject(const int, const int);
  void resetEvents(bool pos=false);
  void resetMatScreen() {
    memcpy(mScreen,mIdentity, 16*sizeof(GLdouble));
  }
  void resetMatScene() {
    memcpy(mScene,mIdentity, 16*sizeof(GLdouble));
  }

private:


  vr::TrackedDevicePose_t tracked_device_pose[vr::k_unMaxTrackedDeviceCount];

  // ----------------
  // app variables
  // ----------------

  bool app_end = false;
  string driver_name, driver_serial;
  string tracked_device_type[vr::k_unMaxTrackedDeviceCount];
  // my parent
  QWidget * parent;
  // global options
  GlobalOptions * store_options;
  // grid variables
  GLGridObject * gridx, * gridy, * gridz;
  GLNewGridObject *new_grid;
  GLCubeObject * cube;
  // axes
  GLAxesObject * axes;
  // Vectors
  GLObjectParticlesVector gpv;
  ParticlesObjectVector * pov;
  CPointsetManager* cpointset_manager;

  ParticlesData   * p_data;
  // projections
  void setProjection(const int x, const int y, const int w, const int h );
  void computeOrthoFactor();
  float ortho_left,ortho_right,ortho_bottom,ortho_top;
  float ratio, fx,fy;
  int wwidth, wheight;
  int vr_width, vr_height;
  GLuint texWidth, texHeight;
  QImage imgFBO;
  bool fbo;
  // events
  void mousePressEvent  ( QMouseEvent *e );
  void mouseReleaseEvent( QMouseEvent *e );
  void mouseMoveEvent   ( QMouseEvent *e );
  void wheelEvent       ( QWheelEvent *e );
  void keyReleaseEvent  ( QKeyEvent   *k );
  void keyPressEvent    ( QKeyEvent   *k );
  
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
  int  x_mouse, y_mouse, z_mouse,
      last_posx, last_posy, last_posz;
  float tx_mouse,ty_mouse,tz_mouse;
  float last_xrot, last_yrot, last_zrot;
  float last_urot, last_vrot, last_wrot;
  float last_zoom;
  int   i_umat, i_vmat, i_wmat; // index of the SCENE/Object rotation matrix
  
  void setRotationScreen( const int x, const int y, const int z );
  void setRotationScene ( const int u, const int v, const int w );
  void getPixelTranslation(float *x, float *y, float *z);
  void setTranslation(const float x, const float y, const float z );
  void setZoom(const int z);
  void setZoom(const float z);
  void mouseWheelUp();
  void mouseWheelDown();
  int zoom_dynam;
  // gl matrix
  //
  glm::mat4 m_projection_matrix, m_model_matrix, m_screen_matrix, m_scene_matrix, m_rotation_matrix;
  GLdouble mProj[16], mModel[16], mModel2[16],
  mScreen[16], mScene[16], mRot[16];
  GLdouble static mIdentity[16];
  int viewport[4];
  bool reset_screen_rotation, reset_scene_rotation;
  void setProjMatrix()  {
    glGetDoublev(GL_PROJECTION_MATRIX, (GLdouble *) mProj);
  }
  void setModelMatrix() {
    glGetDoublev(GL_MODELVIEW_MATRIX, (GLdouble *) mModel);
  }
  void setViewPort() {
    glGetIntegerv(GL_VIEWPORT,viewport);
  }

  void printMatrix( GLdouble * matrix, std::string text="" ) {
    std::cerr << text << "\n";
    for (int i=0;i<4;i++) {
      for (int j=0;j<4;j++) {
        std::cerr << matrix[i*4+j] << " ";
        // 0 4 8  12
        // 1 5 9  13
        // 2 6 10 14
        // 3 7 11 15       3: xtrans 7: ytrans  11: zoom & ztrans
      }
      std::cerr << "\n";
    }
    std::cerr << "\n----------\n";
  }

  void setPerspectiveMatrix();
  // OSD
  GLObjectOsd * osd;
  QImage image,gldata;
  // Font
  fntTexFont * font;
  // Font
  //fntRenderer * text;
  // Texture vector
  GLTextureVector gtv;
  // Thread
  QMutex * mutex_data;
  
  bool is_shift_pressed;
  // bench
  int nframe;
  int vr_nb_frames;
  bool vr;
  // Shaders
  CShader * shader, * vel_shader, * vr_shader;
  void initShader();
  unsigned int m_vertexShader, m_pixelShader;

  static const char vertexShader[];
  static const char pixelShader[];
  // Lights
  void initLight();
  // octree
  GLOctree * tree;
  // Camera
  Camera * camera;
  NewCamera * new_camera;
  void getMatrices();

  int world_scale_value;
  glm::vec3 world_scale;
  glm::vec3 world_offset_position;

	vr::TrackedDevicePose_t m_rTrackedDevicePose[ vr::k_unMaxTrackedDeviceCount ];
//	Matrix4 m_rmat4DevicePose[ vr::k_unMaxTrackedDeviceCount ];

	struct ControllerInfo_t
	{
		vr::VRInputValueHandle_t m_source = vr::k_ulInvalidInputValueHandle;
		vr::VRActionHandle_t m_actionPose = vr::k_ulInvalidActionHandle;
		vr::VRActionHandle_t m_actionHaptic = vr::k_ulInvalidActionHandle;
//		Matrix4 m_rmat4Pose;
//		CGLRenderModel *m_pRenderModel = nullptr;
		std::string m_sRenderModelName;
		bool m_bShowController;
	};

	enum EHand
	{
		Left = 0,
		Right = 1,
	};
	ControllerInfo_t m_rHand[2];

	vr::VRActionHandle_t m_actionHideCubes = vr::k_ulInvalidActionHandle;
	vr::VRActionHandle_t m_actionHideThisController = vr::k_ulInvalidActionHandle;
	vr::VRActionHandle_t m_actionTriggerHaptic = vr::k_ulInvalidActionHandle;
	vr::VRActionHandle_t m_actionAnalongInput = vr::k_ulInvalidActionHandle;

	vr::VRActionSetHandle_t m_actionsetDemo = vr::k_ulInvalidActionSetHandle;
	vec2 m_vAnalogValue = {-2, -2};

  void ProcessVREvent( const vr::VREvent_t & event );
  bool HandleInput();


};
} // namespace glnemo

#endif
