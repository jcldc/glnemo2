// ============================================================================
// Copyright Jean-Charles LAMBERT - 2007-2020
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
#include <QtGlobal>
#if QT_VERSION >= QT_VERSION_CHECK(5, 0, 0)
#include <GL/glew.h>
#include <QtGui>
#else
#include <QtGui>
#include <GL/glew.h>
#endif
#include <QtOpenGL>
#include <QMutex>
#include <assert.h>
#include <limits>
#include <math.h>
#include "glwindow.h"
#include "glgridobject.h"
#include "glcubeobject.h"
#include "globaloptions.h"
#include "particlesdata.h"
#include "particlesobject.h"
#include "tools3d.h"
#include "fnt.h"
#include "glcpoints.h"
#include <glm/gtx/string_cast.hpp>

#include <iostream>
#include <iomanip> // setprecision
#include <sstream> // stringstream


namespace glnemo {


    quat RotationBetweenVectors(vec3 start, vec3 dest){
        start = normalize(start);
        dest = normalize(dest);

        float cosTheta = dot(start, dest);
        vec3 rotationAxis;

        if (cosTheta < -1 + 0.001f){
            // special case when vectors in opposite directions:
            // there is no "ideal" rotation axis
            // So guess one; any will do as long as it's perpendicular to start
            rotationAxis = cross(vec3(0.0f, 0.0f, 1.0f), start);
            if (glm::length(rotationAxis) < 0.01 ) // bad luck, they were parallel, try again!
                rotationAxis = cross(vec3(1.0f, 0.0f, 0.0f), start);

            rotationAxis = normalize(rotationAxis);
            return glm::angleAxis(glm::radians(180.0f), rotationAxis);
        }

        rotationAxis = cross(start, dest);

        float s = sqrt( (1+cosTheta)*2 );
        float invs = 1 / s;

        return {
                s * 0.5f,
                rotationAxis.x * invs,
                rotationAxis.y * invs,
                rotationAxis.z * invs
        };

    }
#define DOF 4000000

  bool GLWindow::GLSL_support = false;
  GLuint framebuffer, renderbuffer;
  GLuint renderedTexture[2];
//  GLenum DrawBuffers[1] = {GL_COLOR_ATTACHMENT0};
  GLdouble GLWindow::mIdentity[16] = {1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1};
  //float store_options->ortho_range;

inline mat4 vr34ToGlm(const vr::HmdMatrix34_t& m) {
  mat4 result = mat4(
      m.m[0][0], m.m[1][0], m.m[2][0], 0.0,
      m.m[0][1], m.m[1][1], m.m[2][1], 0.0,
      m.m[0][2], m.m[1][2], m.m[2][2], 0.0,
      m.m[0][3], m.m[1][3], m.m[2][3], 1.0f);
  return result;
}

inline mat4 vr44ToGlm(const vr::HmdMatrix44_t& m) {
  mat4 result = mat4(
          m.m[0][0], m.m[1][0], m.m[2][0], m.m[3][0],
          m.m[0][1], m.m[1][1], m.m[2][1], m.m[3][1],
          m.m[0][2], m.m[1][2], m.m[2][2], m.m[3][2],
          m.m[0][3], m.m[1][3], m.m[2][3], m.m[3][3]);
  return result;
}


vr::IVRSystem* init_VR()
{

// Check whether there is an HMD plugged-in and the SteamVR runtime is installed
  if (vr::VR_IsHmdPresent())
  {
    std::cout << "An HMD was successfully found in the system" << std::endl;

    if (vr::VR_IsRuntimeInstalled()) {
      std::cout << "Runtime correctly installed" << std::endl;
    }
    else
    {
      std::cout << "Runtime was not found" << std::endl;
    }
  }
  else
  {
    std::cout << "No HMD was found in the system" << std::endl;
  }


  // ----------------
  // OpenVR variables
  // ----------------

  vr::IVRSystem* vr_context;
  vr::TrackedDevicePose_t tracked_device_pose[vr::k_unMaxTrackedDeviceCount];

  // ----------------
  // app variables
  // ----------------

  bool app_end = false;
  char driver_name[100];
  char driver_serial[100];
  string tracked_device_type[vr::k_unMaxTrackedDeviceCount];

  // Load the SteamVR Runtime
  vr::HmdError err;
  vr_context = vr::VR_Init(&err,vr::EVRApplicationType::VRApplication_Scene);
  vr_context == NULL ? cout << "Error while initializing SteamVR runtime. Error code is " << vr::VR_GetVRInitErrorAsSymbol(err) << endl : cout << "SteamVR runtime successfully initialized" << endl;

  // Obtain some basic information given by the runtime
  int base_stations_count = 0;
  for (uint32_t td=vr::k_unTrackedDeviceIndex_Hmd; td<vr::k_unMaxTrackedDeviceCount; td++) {

    if (vr_context->IsTrackedDeviceConnected(td))
    {
      vr::ETrackedDeviceClass tracked_device_class = vr_context->GetTrackedDeviceClass(td);

      auto td_type = vr::VRSystem()->GetTrackedDeviceClass(tracked_device_class);
      tracked_device_type[td] = td_type;

      cout << "Tracking device " << td << " is connected " << endl;
      char name[100];
      vr_context->GetStringTrackedDeviceProperty(td,vr::Prop_TrackingSystemName_String, name, 100);
      cout << "  Device type: " << td_type << ". Name: " << string(name) << endl;

      if (tracked_device_class == vr::ETrackedDeviceClass::TrackedDeviceClass_TrackingReference) base_stations_count++;

//      if (td == vr::k_unTrackedDeviceIndex_Hmd)
//      {
//        vr_context->GetStringTrackedDeviceProperty(vr::k_unTrackedDeviceIndex_Hmd,vr::Prop_TrackingSystemName_String, driver_name, 100);
//        vr_context->GetStringTrackedDeviceProperty(vr::k_unTrackedDeviceIndex_Hmd,vr::Prop_SerialNumber_String, driver_serial, 100);
//      }
    }
//    else
//      cout << "Tracking device " << td << " not connected" << endl;
  }

  // Check whether both base stations are found, not mandatory but just in case...
  if (base_stations_count < 2)
  {
    cout << "There was a problem indentifying the base stations, please check they are powered on" << endl;

  }

  if (!vr::VRCompositor())
  {
    throw std::runtime_error("Unable to initialize VR compositor!\n ");
  }
  else
    cout << "VR compositor ok";

	uint32_t rtWidth;
	uint32_t rtHeight;
  vr_context->GetRecommendedRenderTargetSize(&rtWidth, &rtHeight);


  std::clog<<"Initialized HMD with suggested render target size : " << rtWidth << "x" << rtHeight << std::endl;

  const float freq = vr_context->GetFloatTrackedDeviceProperty(vr::k_unTrackedDeviceIndex_Hmd, vr::Prop_DisplayFrequency_Float);

  std::clog << "display frequency : " << freq <<std::endl;
  return vr_context;


}

// ============================================================================
// Constructor
// BEWARE when parent constructor QGLWidget(QGLFormat(QGL::SampleBuffers),_parent)
// is called, we get antialiasing during screenshot capture but we can loose
// performance during rendering. You have been warned !!!!!
GLWindow::GLWindow(QWidget * _parent, GlobalOptions*_go, QMutex * _mutex, Camera *_camera, CPointsetManager * _pointset_manager, NewCamera* _new_camera) :QGLWidget(QGLFormat(QGL::SampleBuffers),_parent)
{
  vr_context = init_VR();

  m_scale = 1.f/10;
  world_scale = glm::vec3(m_scale, m_scale, m_scale);
  world_offset_position = glm::vec3(0, -60, 35);
  m_scene_matrix = glm::identity<mat4>();

  m_current_rotation = quat(1,0,0,0);

  // copy parameters
  parent        = _parent;
  store_options = _go;
  camera        = _camera;
  cpointset_manager = _pointset_manager;
  new_camera = _new_camera;
  //setAttribute(Qt::WA_NoSystemBackground);
  // reset coordinates
  resetEvents(true);
  is_mouse_pressed   = FALSE;
  is_mouse_zoom      = FALSE;
  is_key_pressed     = FALSE;
  is_a_key_pressed   = FALSE;
  is_shift_pressed   = FALSE;
  is_pressed_ctrl    = FALSE;
  gpv.clear();
  pov = NULL;
  p_data = NULL;
  mutex_data = _mutex;
  nframe = 0; // used during bench
  p_data = new ParticlesData();
  pov    = new ParticlesObjectVector();
  gl_select = new GLSelection();
  // rotatation mode is screen by default
  store_options->rotate_screen = true;
  // reset rotation matrixes
  resetMatScreen();
  resetMatScene();
  reset_screen_rotation = false;
  reset_scene_rotation  = false;
  // initialyse rotation variables
  last_xrot = last_urot = 0.;
  last_yrot = last_vrot = 0.;
  last_zrot = last_wrot = 0.;
  last_zoom = 0.;
  // SCENE/object matrix index
  i_umat = 0;
  i_vmat = 1;
  i_wmat = 2;

  connect(gl_select, SIGNAL(updateGL()), this, SLOT(updateGL()));
  //connect(gl_select, SIGNAL(updateZoom()), this, SLOT(osdZoom()));
  connect(gl_select, SIGNAL(updateZoom()), this, SLOT(updateOsdZrt()));
  connect(this,SIGNAL(sigScreenshot()),parent,SLOT(startAutoScreenshot()));
  setFocus();


//  this->resizeGL((int)rtWidth, (int)rtHeight);
  zoom_dynam = 0;
  // leave events : reset event when we leave opengl windows
  connect(this,SIGNAL(leaveEvent()),this,SLOT(resetEvents()));

  initializeGL();
  checkGLErrors("initializeGL");
  shader = NULL;
  vel_shader = NULL;
  vr_shader = NULL;
  vr = true;
  initShader();
  checkGLErrors("initShader");
  ////////

  // camera
  camera->loadShader();

  // grid

  new_grid_x = new Grid(0, &m_projection_matrix, &m_view_matrix, &m_scene_matrix, true, store_options->col_x_grid);
  new_grid_y = new Grid(1, &m_projection_matrix, &m_view_matrix, &m_scene_matrix, false, store_options->col_y_grid);
  new_grid_z = new Grid(2, &m_projection_matrix, &m_view_matrix, &m_scene_matrix, false, store_options->col_z_grid);

  // axes
  axes = new GLAxesObject();

  // cube
  cube  = new GLCubeObject(store_options->mesh_length*store_options->nb_meshs,store_options->col_cube,store_options->show_cube);
  // load texture
  GLTexture::loadTextureVector(gtv);

  // build display list in case of screenshot
  if (store_options->show_part && pov ) {
    //std::cerr << "GLWindow::initializeGL() => build display list\n";
    for (int i=0; i<(int)pov->size(); i++) {
      // !!!! DEACTIVATE gpv[i].buildDisplayList();;
      gpv[i].buildVelDisplayList();;
      gpv[i].setTexture();
      //gpv[i].buildVboPos();
    }
  }

  // Osd
  fntRenderer text;
  font = new fntTexFont(store_options->osd_font_name.toStdString().c_str());
  text.setFont(font);
  text.setPointSize(store_options->osd_font_size );
  osd = new GLObjectOsd(wwidth,wheight,text,store_options->osd_color);
  // colorbar
  gl_colorbar = new GLColorbar(store_options,true);
  uint32_t rtWidth;
  uint32_t rtHeight;
  vr_context->GetRecommendedRenderTargetSize(&rtWidth, &rtHeight);
  ////////
  // FBO
  // Set the width and height appropriately for you image
  fbo = true;
  //Set up a FBO with one renderbuffer attachment
  // init octree
  tree = new GLOctree(store_options);
  tree->setActivate(true);
  if (GLWindow::GLSL_support) {
    // framebuffer
    glGenFramebuffersEXT(1, &framebuffer);
    glBindFramebufferEXT(GL_FRAMEBUFFER, framebuffer);

    //texture

    for (int i =0; i < 2; i++) {
      glGenTextures(1, &renderedTexture[i]);
      glBindTexture(GL_TEXTURE_2D, renderedTexture[i]);
      glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, (GLsizei) rtWidth, (GLsizei) rtHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, 0);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

    }

    //render buffer for depth
    glGenRenderbuffersEXT(1, &renderbuffer);
    glBindRenderbufferEXT(GL_RENDERBUFFER, renderbuffer);
    glRenderbufferStorageEXT(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, (GLsizei)rtWidth, (GLsizei)rtHeight);
    glFramebufferRenderbufferEXT(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, renderbuffer);
//
//    // Set the list of draw buffers.
//    glDrawBuffersEXT(1, DrawBuffers); // "1" is the size of DrawBuffers
    glBindFramebufferEXT(GL_FRAMEBUFFER, 0);


  }
  checkGLErrors("GLWindow constructor");

  vr::VRInput()->SetActionManifestPath( "C:/Users/Gregoire/CLionProjects/glnemo2/res/vr_controller_actions/hellovr_actions.json");

	vr::VRInput()->GetActionHandle( "/actions/demo/in/HideCubes", &m_actionHideCubes );
	vr::VRInput()->GetActionHandle( "/actions/demo/in/HideThisController", &m_actionHideThisController);

	vr::VRInput()->GetActionSetHandle( "/actions/demo", &m_actionsetDemo );

	vr::VRInput()->GetActionHandle( "/actions/demo/out/Haptic_Left", &m_rHand[Left].m_actionHaptic );
	vr::VRInput()->GetInputSourceHandle( "/user/hand/left", &m_rHand[Left].m_source );
	vr::VRInput()->GetActionHandle( "/actions/demo/in/Hand_Left", &m_rHand[Left].m_actionPose );
    vr::VRInput()->GetActionHandle( "/actions/demo/in/grip_left", &m_rHand[Left].m_grip );
    vr::VRInput()->GetActionHandle( "/actions/demo/in/AnalogInput_left", &m_rHand[Left].m_actionAnalongInput );

	vr::VRInput()->GetActionHandle( "/actions/demo/out/Haptic_Right", &m_rHand[Right].m_actionHaptic );
	vr::VRInput()->GetInputSourceHandle( "/user/hand/right", &m_rHand[Right].m_source );
	vr::VRInput()->GetActionHandle( "/actions/demo/in/Hand_Right", &m_rHand[Right].m_actionPose );
    vr::VRInput()->GetActionHandle( "/actions/demo/in/grip_right", &m_rHand[Right].m_grip );
    vr::VRInput()->GetActionHandle( "/actions/demo/in/AnalogInput_right", &m_rHand[Right].m_actionAnalongInput );

  auto play_timer   = new QTimer(this);
  connect(play_timer,SIGNAL(timeout()),this,SLOT(updateGL())); // update GL at every timeout()
  play_timer->start(11);

}

// ============================================================================
// Destructor
GLWindow::~GLWindow()
{
  delete new_grid_x;
  delete new_grid_y;
  delete new_grid_z;
  delete gl_select;
  delete cube;
  delete tree;
  delete axes;
  if (GLWindow::GLSL_support) {
    glDeleteRenderbuffersEXT(1, &renderbuffer);
    glDeleteRenderbuffersEXT(1, &framebuffer);
    if (shader) delete shader;
    if (vel_shader) delete vel_shader;
  }
  delete gl_colorbar;
  delete osd;
//  if (p_data)
//    delete p_data;
  vr::VR_Shutdown();
  std::cerr << "Destructor GLWindow::~GLWindow()\n";
}
#define COPY 0
// ============================================================================
// update
void GLWindow::updateGL()
{
  if ( !store_options->duplicate_mem) {
    mutex_data->lock();
    if (store_options->new_frame)
      update();
    else
      QGLWidget::update();
    mutex_data->unlock();
  }
  else QGLWidget::update();
}
//QMutex mutex1;

// ============================================================================
// update
void GLWindow::update(ParticlesData   * _p_data,
                      ParticlesObjectVector * _pov,
                      GlobalOptions         * _go,
                      const bool update_old_obj)
{
  store_options = _go;
  mutex_data->lock();

  if (store_options->duplicate_mem) {
    //pov->clear();
    //*pov    = *_pov;
  }
  //else pov    = _pov;
  pov    = _pov;

  if (p_data!=_p_data) {
    if (store_options->duplicate_mem)  *p_data = *_p_data;
    else                                p_data = _p_data;
  }
  // octree
  store_options->octree_enable = true;
  store_options->octree_display = true;
  store_options->octree_level = 0;
  //tree->update(p_data, _pov);
  gl_colorbar->update(&gpv,p_data->getPhysData(),store_options,mutex_data);

  for (unsigned int i=0; i<pov->size() ;i++) {
    if (i>=gpv.size()) {
      GLObjectParticles * gp = new GLObjectParticles(p_data,&((*pov)[i]),
                                                     store_options,&gtv,shader,vel_shader);
      //GLObjectParticles * gp = new GLObjectParticles(&p_data,pov[i],store_options);
      gpv.push_back(*gp);
      delete gp;
    } else {
      gpv[i].update(p_data,&((*pov)[i]),store_options, update_old_obj);
      //gpv[i].update(&p_data ,pov[i],store_options);

    }
  }

  store_options->new_frame=false;
  gl_select->update(&gpv,store_options,mutex_data);
  mutex_data->unlock();
  updateGL();

}
// ============================================================================
// update
void GLWindow::update(ParticlesObjectVector * _pov)
{
  if (store_options->duplicate_mem) {
    pov->clear();
    *pov    = *_pov;
  }
  else pov    = _pov;

}
// ============================================================================
// updateBondaryPhys
void GLWindow::updateBoundaryPhys(const int i_obj, const bool ugl)
{
  assert(i_obj < (int) gpv.size());
  gpv[i_obj].updateBoundaryPhys();
  if (ugl) updateGL();
}
// ============================================================================
// updateVbo
void GLWindow::updateVbo(const int i_obj)
{
  assert(i_obj < (int) gpv.size());
  gpv[i_obj].updateVbo();
}
// ============================================================================
// updateColorVbo
void GLWindow::updateColorVbo(const int i_obj)
{
  assert(i_obj < (int) gpv.size());
  gpv[i_obj].updateColorVbo();
}
// ============================================================================
// update
void GLWindow::update()
{
  if (pov) {
    update(p_data, pov, store_options, cpointset_manager);
  }
}
// ============================================================================
// update velocity vectors for the [index] object
void GLWindow::updateVel(const int index)
{
  if (pov && index < (int) pov->size()) {
    gpv[index].updateVel();
  }
}
// ============================================================================
// changeColorMap
void GLWindow::changeColorMap()
{
  for (unsigned int i=0; i<pov->size() ;i++) {
     //gpv[i].buildVboColor();
    gpv[i].updateColormap();
   }
  updateGL();
}
// ============================================================================
// reverseColorMap
void GLWindow::reverseColorMap()
{
  //store_options->reverse_cmap = !store_options->reverse_cmap;
//  for (unsigned int i=0; i<pov->size() ;i++) {
//    gpv[i].buildVboColor();
//  }
  updateGL();
}
// ============================================================================
// rebuildGrid
void GLWindow::rebuildGrid(bool ugl)
{
  new_grid_x->setNbLines(store_options->nb_meshs);
  new_grid_x->setLineGap(store_options->mesh_length);
  new_grid_x->genVertexBufferData();

  new_grid_y->setNbLines(store_options->nb_meshs);
  new_grid_y->setLineGap(store_options->mesh_length);
  new_grid_y->genVertexBufferData();

  new_grid_z->setNbLines(store_options->nb_meshs);
  new_grid_z->setLineGap(store_options->mesh_length);
  new_grid_z->genVertexBufferData();
//  cube->setSquareSize(store_options->nb_meshs*store_options->mesh_length);
  if (ugl) updateGL();
}
// ============================================================================
// updatedGrid
void GLWindow::updateGrid(bool ugl)
{
  new_grid_x->setDisplay(store_options->xy_grid);
  new_grid_x->setColor(store_options->col_x_grid);

  new_grid_y->setDisplay(store_options->yz_grid);
  new_grid_y->setColor(store_options->col_y_grid);

  new_grid_z->setDisplay(store_options->xz_grid);
  new_grid_z->setColor(store_options->col_z_grid);

  cube->setActivate(store_options->show_cube);
  cube->setColor(store_options->col_cube);

  if (ugl) updateGL();
}
// ============================================================================
// init Light
void GLWindow::initLight()
{
}
#define OSD 0

// move, translate and re-draw the whole scene according to the objects and
// features selected
long int CPT=0;

void GLWindow::paintGL()
{
  vr::VRCompositor()->WaitGetPoses(tracked_device_pose, vr::k_unMaxTrackedDeviceCount, nullptr, 0);
  HandleInput();
  for(int eye = 0; eye < 2; eye++) {
    m_projection_matrix = vr44ToGlm(vr_context->GetProjectionMatrix((vr::Hmd_Eye) eye, 0.2, 500));
    CPT++;
    //std::cerr << "GLWindow::paintGL() --> "<<CPT<<"\n";
    //std::cerr << "GLWindow::paintGL() auto_gl_screenshot="<<store_options->auto_gl_screenshot<<"\n";
    if (store_options->auto_gl_screenshot) {
      store_options->auto_gl_screenshot = false;
      emit sigScreenshot();
      //std::cerr << "GLWindow::paintGL() after EMIT"<<CPT<<"\n";
      store_options->auto_gl_screenshot = true;
    }
    if (!store_options->duplicate_mem)
      mutex_data->lock();
    if (fbo && GLWindow::GLSL_support) {
      //std::cerr << "FBO GLWindow::paintGL() --> "<<CPT<<"\n";
      //glGenFramebuffersEXT(1, &framebuffer);
      glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, framebuffer);
      glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, renderedTexture[eye], 0);
      //glGenRenderbuffersEXT(1, &renderbuffer);
//    glBindRenderbufferEXT(GL_RENDERBUFFER_EXT, renderbuffer);
//    glRenderbufferStorageEXT(GL_RENDERBUFFER_EXT, GL_RGBA8, texWidth, texHeight);
//    glFramebufferRenderbufferEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT,
//                  GL_RENDERBUFFER_EXT, renderbuffer);
      GLuint status = glCheckFramebufferStatusEXT(GL_FRAMEBUFFER_EXT);
      if (status != GL_FRAMEBUFFER_COMPLETE_EXT) {
      }
    }
    //setFocus();

    qglClearColor(store_options->background_color);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // set projection
    setProjection(0, 0, wwidth, wheight);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    //glEnable(GL_DEPTH_TEST);

    // rotation around scene/object axes
    float ru = store_options->urot - last_urot;
    float rv = store_options->vrot - last_vrot;
    float rw = store_options->wrot - last_wrot;

    // the following code compute OpenGL rotation
    // around UVW scene/object axes
    if (ru != 0 ||
        rv != 0 ||
        rw != 0) {
      glLoadIdentity();
      if (ru != 0)
        glRotatef(ru, mScene[0], mScene[1], mScene[2]);
      if (rv != 0)
        glRotatef(rv, mScene[4], mScene[5], mScene[6]);
      if (rw != 0)
        glRotatef(rw, mScene[8], mScene[9], mScene[10]);

      last_urot = store_options->urot;
      last_vrot = store_options->vrot;
      last_wrot = store_options->wrot;
      last_zoom = store_options->zoom;
      glMultMatrixd(mScene);
      glGetDoublev(GL_MODELVIEW_MATRIX, mScene);
    }

    // rotation around screen axes
    float rx = store_options->xrot - last_xrot;
    float ry = store_options->yrot - last_yrot;
    float rz = store_options->zrot - last_zrot;
    float dzoom = store_options->zoom - last_zoom;

    vr::VREvent_t vr_event;

    // Obtain hmd pose
    quat rot;

    glm::vec3 hmd_position;
    for (int nDevice = 0; nDevice < vr::k_unMaxTrackedDeviceCount; nDevice++) {
      if ((tracked_device_pose[nDevice].bDeviceIsConnected) && (tracked_device_pose[nDevice].bPoseIsValid) &&
          (nDevice == vr::k_unTrackedDeviceIndex_Hmd)) {

        auto hmd_pos_and_rot = vr34ToGlm(tracked_device_pose[nDevice].mDeviceToAbsoluteTracking);
        auto head_to_eye_m = vr34ToGlm(vr_context->GetEyeToHeadTransform((vr::Hmd_Eye) eye));
        auto absolute_eye_pos_m = hmd_pos_and_rot * head_to_eye_m;
        hmd_position = absolute_eye_pos_m[3];
        //hmd_position *= world_scale;
        //hmd_position += world_offset_position;

        rot = glm::quat(hmd_pos_and_rot);
      }
    }

    new_camera->setOrientation(rot);
    new_camera->setPosition(hmd_position);

    if (rx != 0 ||
        ry != 0 ||
        rz != 0 ||
        dzoom != 0) {
//    new_camera->rotate(rx, ry, rz);
      last_xrot = store_options->xrot;
      last_yrot = store_options->yrot;
      last_zrot = store_options->zrot;
      getMatrices();
    }
    if (reset_screen_rotation) {
      store_options->zoom = 4;
      new_camera->reset();
      getMatrices();
      reset_screen_rotation = false;
    }
    if (reset_scene_rotation) {
      glLoadIdentity();
      glGetDoublev(GL_MODELVIEW_MATRIX, mScene); // set to Identity
      reset_scene_rotation = false;
      last_urot = last_vrot = last_wrot = 0.0;
    }

    glLoadIdentity(); // reset OGL rotations
    // set camera

    // apply screen rotation on the whole system
    glMultMatrixd(mScreen);
    // apply scene/world rotation on the whole system
    glMultMatrixd(mScene);

    // Grid Anti aliasing
//#ifdef GL_MULTISAMPLE
    glEnable(GL_MULTISAMPLE);
//#endif
    if (1) { //line_aliased) {
      glEnable(GL_LINE_SMOOTH);
      glEnable(GL_POLYGON_SMOOTH);
      glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
      glHint(GL_POLYGON_SMOOTH_HINT, GL_NICEST);
      //glLineWidth (0.61);
      glLineWidth(1.0);
    } else {
      glDisable(GL_LINE_SMOOTH);
    }

  m_view_matrix = glm::inverse(new_camera->getViewMatrix());

  // grid display
  if (store_options->show_grid) {
    //glEnable( GL_DEPTH_TEST );
    glDisable(GL_BLEND);
    glDisable(GL_DEPTH_TEST);

    glEnable(GL_MULTISAMPLE);
    new_grid_x->display();
    new_grid_y->display();
    new_grid_z->display();
    // old grid
//    glDisable(GL_DEPTH_TEST);
//    glEnable(GL_BLEND);
//    gridx->display();
//    gridy->display();
//    gridz->display();
//    cube->display();
//    glDisable(GL_BLEND);
  }

    // sphere display
    if (0) {
      glDisable(GL_DEPTH_TEST);
      glEnable(GL_BLEND);
      GLUquadricObj *quadric = gluNewQuadric();
      gluQuadricDrawStyle(quadric, GLU_LINE);
      gluQuadricNormals(quadric, GLU_SMOOTH);
      GLdouble radius = GLdouble(store_options->mesh_length * store_options->nb_meshs / 2.0);
      GLint subdivisions = 16;
      gluSphere(quadric, radius, subdivisions, subdivisions);
      gluDeleteQuadric(quadric);
      glDisable(GL_BLEND);

    }
    // camera display path and control points
    camera->display(wheight);

    setModelMatrix(); // save ModelView  Matrix
    setProjMatrix();  // save Projection Matrix
//    if(!nframe % 20) {
//      std::cout << "vr proj matrix :  " << glm::to_string(proj_matrix) << std::endl;
//      std::cout << "gl matrix :  " << glm::to_string(glm::make_mat4(mProj)) << std::endl;
//    }
//    proj_matrix = glm::make_mat4(mProj);
    // move the scene
    glTranslatef(store_options->xtrans, store_options->ytrans, store_options->ztrans);
    glGetDoublev(GL_MODELVIEW_MATRIX, (GLdouble *) mModel2);
    //printMatrix(mModel2,"GL_MODELVIEW_MATRIX 100");

    // nice points display
    glEnable(GL_POINT_SMOOTH);

    //glDepthFunc(GL_LESS);
  // Display objects (particles and velocity vectors)
;
  cpointset_manager->displayAll();

  glDisable(GL_MULTISAMPLE);
  // control blending on particles
    if (store_options->blending) {
      glEnable(GL_BLEND);
      glBlendFunc(GL_SRC_ALPHA, GL_ONE); // original
      //glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
      //glDepthFunc(GL_LESS);
    } else
      glDisable(GL_BLEND);
    // control depht buffer on particles
    if (store_options->dbuffer) glEnable(GL_DEPTH_TEST);
    else glDisable(GL_DEPTH_TEST);


    if (store_options->show_part && pov) {
      //mutex_data->lock();
      bool first = true;
      bool obj_has_physic = false;
      for (int i = 0; i < (int) pov->size(); i++) {
        gpv[i].display(mModel2, wheight, m_projection_matrix, m_scene_matrix);

        if (first) {
          const ParticlesObject *po = gpv[i].getPartObj();
          if (po->hasPhysic()) { //store_options->phys_min_glob!=-1 && store_options->phys_max_glob!=-1) {
            obj_has_physic = true;
            first = false;
          }
        }
      }


      if (obj_has_physic) {
        if (fbo) // offscreen rendering activated
          gl_colorbar->display(texWidth, texHeight);
        else
          gl_colorbar->display(QGLWidget::width(), QGLWidget::height());
      }

    //mutex_data->unlock();
  }

  // octree
  if (store_options->octree_display || 1) {
    tree->display();
  }

  // On Screen Display
  if (store_options->show_osd) osd->display();

  // display selected area
  gl_select->display(QGLWidget::width(),QGLWidget::height());

    // draw axes
    if (store_options->axes_enable)
      axes->display(mRot, mScene, wwidth, wheight,
                    store_options->axes_loc, store_options->axes_psize, store_options->perspective);

    // reset viewport to the windows size because axes object modidy it
    glViewport(0, 0, wwidth, wheight);
  }

//  if (fbo && GLWindow::GLSL_support) {
//    fbo = false;
    //imgFBO = grabFrameBuffer();
//    imgFBO = QImage( texWidth, texHeight,QImage::Format_RGB32);
//    glReadPixels( 0, 0, texWidth, texHeight, GL_RGBA, GL_UNSIGNED_BYTE, imgFBO.bits() );
    // Make the window the target
    vr::Texture_t leftEyeTexture = {(void*)renderedTexture[vr::Eye_Left], vr::ETextureType::TextureType_OpenGL, vr::ColorSpace_Linear};
		vr::Texture_t rightEyeTexture = {(void*)renderedTexture[vr::Eye_Right], vr::ETextureType::TextureType_OpenGL, vr::ColorSpace_Linear};

    vr::VRCompositor()->Submit(vr::Eye_Left, &leftEyeTexture);
		vr::VRCompositor()->Submit(vr::Eye_Right, &rightEyeTexture);

//		glFlush();
//
//		vr::VRCompositor()->PostPresentHandoff();

    glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);

   // Delete the renderbuffer attachment
   //glDeleteRenderbuffersEXT(1, &renderbuffer);
   //glDeleteRenderbuffersEXT(1, &framebuffer);
//  }

  if ( !store_options->duplicate_mem) mutex_data->unlock();

  nframe++; // count frames
  //glDrawPixels(gldata.width(), gldata.height(), GL_RGBA, GL_UNSIGNED_BYTE, gldata.bits());
  emit doneRendering();
}

void GLWindow::getMatrices() {
  const mat4 &view_matrix = inverse(new_camera->getViewMatrix());
  const mat4 &orientation_matrix = inverse(new_camera->getOrientationMatrix());
  const auto *view_matrix_ptr = (const float*) value_ptr(view_matrix);
  const auto *orientation_matrix_ptr = (const float*) value_ptr(orientation_matrix);
  for (int i = 0; i < 16; ++i){
    mScreen[i] = view_matrix_ptr[i];
    mRot[i] = orientation_matrix_ptr[i];
  }
}

// ============================================================================
void GLWindow::initShader()
{
  if (store_options->init_glsl) {
    const GLubyte* gl_version=glGetString ( GL_VERSION );
    std::cerr << "OpenGL version : ["<< gl_version << "]\n";
    int major = 0;
    int minor = 0;
    glGetIntegerv(GL_MAJOR_VERSION, &major);
    glGetIntegerv(GL_MINOR_VERSION, &minor);
    std::cerr << "OpenGL :"<< major << "." << minor << "\n";
    GLSL_support = true;
    std::cerr << "begining init shader\n";
    int err=glewInit();
    if (err==GLEW_OK && GLEW_ARB_multitexture && GLEW_ARB_vertex_shader && GLEW_ARB_fragment_shader && GL_VERSION_2_0)
      qDebug() << "Ready for GLSL\n";
    else {
      qDebug() << "BE CAREFULL : No GLSL support\n";
      GLSL_support = false;
      //exit(1);
    }

    if (GLSL_support ) {
      // check GLSL version supported
      const GLubyte* glsl_version=glGetString ( GL_SHADING_LANGUAGE_VERSION );
      std::cerr << "GLSL version supported : ["<< glsl_version << "]\n";
      //GLuint glsl_num;
      //glGetStringi(GL_SHADING_LANGUAGE_VERSION,glsl_num);
      //std::cerr << "GLSL version NUM : ["<< glsl_num << "]\n";
      // particles shader
      shader = new CShader(GlobalOptions::RESPATH.toStdString()+"/shaders/particles.vert.cc",
                           GlobalOptions::RESPATH.toStdString()+"/shaders/particles.frag.cc");
      shader->init();
      // velocity shader
      if (1) {

#if 0
          // Geometry shader OpenGL 3.30 and above only
          vel_shader = new CShader(GlobalOptions::RESPATH.toStdString()+"/shaders/velocity.vert330.cc",
                                   GlobalOptions::RESPATH.toStdString()+"/shaders/velocity.frag330.cc",
                                   GlobalOptions::RESPATH.toStdString()+"/shaders/velocity.geom330.cc");

#else
          vel_shader = new CShader(GlobalOptions::RESPATH.toStdString()+"/shaders/velocity.vert.cc",
                                   GlobalOptions::RESPATH.toStdString()+"/shaders/velocity.frag.cc");

#endif
          if (!vel_shader->init() ) {
              delete vel_shader;
              vel_shader=NULL;
          }
      }

    }

  }
  else { // Initialisation of GLSL not requested
    std::cerr << "GLSL desactivated from user request, slow rendering ...\n";
    GLSL_support = false;
  }
  std::cerr << "END OF INITSHADER \n";
}
// ============================================================================
// check OpenGL error message
void GLWindow::checkGLErrors(std::string s)
{
  GLenum error;
  while ((error = glGetError()) != GL_NO_ERROR) {
    std::cerr << "* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * \n";
    std::cerr << s << ": error - " << (char *) gluErrorString(error)<<"\n";
  }
}
// ============================================================================
// initialyse the openGl engine
void GLWindow::initializeGL()
{
  std::cerr << "\n>>>>>>>>> initializeGL()\n\n";
#if 0
  qglClearColor( Qt::black );		// Let OpenGL clear to black
  glEnable(GL_DEPTH_TEST);
  glEnable(GL_LINE_SMOOTH);
#ifdef GL_MULTISAMPLE
  glEnable(GL_MULTISAMPLE);
#endif
  glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
  //store_options->zoom = -10.;
  // Nice texture coordinate interpolation
  glHint( GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST );
  //PRINT_D std::cerr << "-- Initialize GL --\n";

#endif

  makeCurrent();   // activate OpenGL context, can build display list by now

}
// ============================================================================
// resize the opengl viewport according to the new window size
void GLWindow::resizeGL(int w, int h)
{
  wwidth = w;
  wheight= h;
  glViewport( 0, 0, (GLint)w, (GLint)h );
  //float ratio =  ((double )w) / ((double )h);
  glMatrixMode( GL_PROJECTION );
  glLoadIdentity();
  glFrustum(-1.0, 1.0, -1.0, 1.0, 1.0, (float) DOF);
  //gluPerspective(45.,ratio,0.0005,(float) DOF);
  glMatrixMode( GL_MODELVIEW );
  glLoadIdentity();
  osd->setWH(w,h);
  cpointset_manager->setScreenDim(w, h);
}
// ============================================================================
// set up the projection according to the width and height of the windows
void GLWindow::setProjection(const int x, const int y, const int width, const int height)
{
  glViewport( x, y, width, height);
  ratio =  ((double )width) / ((double )height);
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();

//  bool vr = true;
//  if(vr)
//  {
//
//  }

  if (store_options->perspective) {
//    gluPerspective(110.,ratio,0.0005,(float) DOF);
//    double mp[16];
//    glGetDoublev(GL_PROJECTION_MATRIX, (GLdouble *) mp);
//    for (int i=0;i<16;i++) std::cerr << "// "<< mp[i];
//    std::cerr << "\n";
//    const GLfloat zNear = 0.0005, zFar = (GLfloat) DOF, fov = 45.0;
//    m_projection_matrix_ptr = glm::perspective(glm::radians(fov), (GLfloat)ratio, zNear, zFar);
  }
  else {
    computeOrthoFactor();
    //std::cerr << "RANGE="<<store_options->ortho_range<<" zoom="<<store_options->zoom<<" zoomo="<<store_options->zoomo<<"\n";
    //std::cerr << "fx = "<< fx << " fy=" << fy <<  " range*fx*zoom0=" <<  store_options->zoomo*fx*store_options->ortho_range <<  "\n";
    ortho_right = store_options->ortho_range;
    ortho_left  =-store_options->ortho_range;
    ortho_top   = store_options->ortho_range;
    ortho_bottom=-store_options->ortho_range;
    //std::cerr << "zoom0 = " << store_options->zoomo << "\n";
    glOrtho(ortho_left   * fx  * store_options->zoomo,
            ortho_right  * fx  * store_options->zoomo,
            ortho_bottom * fy  * store_options->zoomo,
            ortho_top    * fy  * store_options->zoomo,
            -100000,100000);
            //(float) -DOF/2.,(float) -DOF/2.);
  }
  glGetIntegerv(GL_VIEWPORT,viewport);
}
// ============================================================================
// compute some factors for the orthographic projection
void GLWindow::computeOrthoFactor()
{
  if (ratio<1.0) {
    fx = 1.0  ; fy = 1./ratio;
  }
  else {
    fx = ratio; fy = 1.0;
  }
}
// ============================================================================
// reset rotation and translation to 0,0,0 coordinates
void GLWindow::resetEvents(bool pos)
{
  if (pos) {
    x_mouse= y_mouse= z_mouse=0;
    tx_mouse=ty_mouse=tz_mouse=0;
  }
  is_pressed_left_button  =FALSE;
  is_pressed_right_button =FALSE;
  is_pressed_middle_button=FALSE;
  is_mouse_pressed        =FALSE;
  is_translation          =FALSE;
  is_ctrl_pressed         =FALSE;
}
// ============================================================================
// manage rotation/translation according to mousePresssEvent
void GLWindow::mousePressEvent( QMouseEvent *e )
{
  setFocus();
  if ( e->button() == Qt::LeftButton ) {  // left button pressed
    if (is_shift_pressed)
      setCursor(Qt::CrossCursor);
    is_mouse_pressed       = TRUE;
    is_pressed_left_button = TRUE;
    setMouseTracking(TRUE);
    last_posx = e->x();
    last_posy = e->y();
    if  (is_translation) {;} //!parent->statusBar()->message("Translating X/Y");
    else                 {;} //!parent->statusBar()->message("Rotating X/Y");
  }
  if ( e->button() == Qt::RightButton ) { // right button pressed
    is_mouse_pressed        = TRUE;
    is_pressed_right_button = TRUE;
    setMouseTracking(TRUE);
    last_posz = e->x();
    if (is_translation) {;} //!parent->statusBar()->message("Translating Z");
    else                {;} //!parent->statusBar()->message("Rotating Z");
  }
  //if ( e->button() == Qt::MiddleButton ) {
  if ( e->button() == Qt::MidButton ) {
    //std::cerr << "Middle button pressed\n";
    is_mouse_pressed        = TRUE;
    is_pressed_middle_button= TRUE;
    setMouseTracking(TRUE);
    last_posx = e->x();
    last_posy = e->y();
  }
  emit sigKeyMouse( is_key_pressed, is_mouse_pressed);
  //!options_form->downloadOptions(store_options);
}
// ============================================================================
// GLObjectWindow::mouseReleaseEvent()
// manage mouseReleaseEvent
void GLWindow::mouseReleaseEvent(QMouseEvent *e) {
  if (e) { ; }  // do nothing... just to remove the warning :p
  is_pressed_left_button = FALSE;
  is_pressed_right_button = FALSE;
  is_mouse_pressed = FALSE;
  is_pressed_middle_button = FALSE;
  setMouseTracking(FALSE);
  //!statusBar()->message("Ready");
  //!options_form->downloadOptions(store_options);
#if DRAWBOX
  draw_box->draw( glbox,part_data, &psv, store_options,"");
#endif
  if (is_shift_pressed) {
    if (!store_options->duplicate_mem) mutex_data->lock();
    //JCL 07/21/2015 setPerspectiveMatrix(); // toggle to perspective matrix mode
    gl_select->selectOnArea(pov->size(), mProj, mModel, viewport);
    setPerspectiveMatrix(); // toggle to perspective matrix mode
    gl_select->zoomOnArea(mProj, mModel, viewport);
    osd->setText(GLObjectOsd::Zoom, (const float) store_options->zoom);
    osd->updateDisplay();
    if (!store_options->duplicate_mem) mutex_data->unlock();
  }
  if (is_a_key_pressed) {
    if (e->button() == Qt::LeftButton) {
      std::pair<CPointset*, GLCPoint*> cpoint_pair = cpointset_manager->getClickedCpoint(mModel2, mProj,
                                                                                         {e->x(), e->y()}, viewport,
                                                                                         DOF);
      auto closest_cpoint_parent_set = cpoint_pair.first;
      auto closest_cpoint = cpoint_pair.second;
      if(closest_cpoint){
        if (closest_cpoint->isSelected()) {
          closest_cpoint_parent_set->unselectCPoint(closest_cpoint->getId());
          emit unselectTreeWidgetItem(closest_cpoint->getId());
        } else {
          closest_cpoint_parent_set->selectCPoint(closest_cpoint->getId());
          emit selectTreeWidgetItem(closest_cpoint->getId());
        }
      }
    } else if (e->button() == Qt::RightButton) {
      cpointset_manager->unselectAll();
      emit unselectTreeWidgetAll();
    }
    updateGL();
  }
  setCursor(Qt::ArrowCursor);
  emit sigKeyMouse(is_key_pressed, is_mouse_pressed);
  //!draw_box->show();
}

// ============================================================================
// GLObjectWindow::mouseMoveEvent()
// manage mouseMoveEvent
void GLWindow::mouseMoveEvent( QMouseEvent *e )
{
  int dx=0,dy=0,dz=0;
  setFocus();
  if (is_pressed_left_button && !is_a_key_pressed) {
    // offset displcacement
    dx = e->x()-last_posx;
    dy = e->y()-last_posy;
    //std::cerr << "dxdy="<< dx << " " << dy << "\n";
    // save last position
    last_posx = e->x();
    last_posy = e->y();
    if (is_shift_pressed && !is_mouse_zoom) { // user selection request
      gl_select->getMouse(e);
      updateGL();
    }
    else
      if (is_translation) {
        // total rotation
        glm::quat camera_orientation = new_camera->getOrientation();
        glm::vec3 right = camera_orientation*vec3(-dx, 0, 0);
        glm::vec3 up = camera_orientation*vec3(0, dy, 0);
        tx_mouse+=right.x;
        ty_mouse-=right.y;
        tz_mouse-=right.z;
        tx_mouse+=up.x;
        ty_mouse-=up.y;
        tz_mouse-=up.z;
      }
      else {
        if (is_mouse_zoom) {
          if(dx > 0)
            mouseWheelDown();
          else if(dx < 0)
            mouseWheelUp();
        }
        else {
          // total rotation
          x_mouse+=dx;
          y_mouse+=dy;
        }
      }
  }
  if ( !gl_select->isEnable()) {
    if ( is_pressed_right_button && !is_a_key_pressed) {
      // offset displcacement
      dz = e->x()-last_posz;
      // save last position
      last_posz = e->x();
      if (is_translation) {
        glm::quat camera_orientation = new_camera->getOrientation();
        glm::vec3 front = camera_orientation*vec3(0, 0, dz);
        tx_mouse+=front.x;
        ty_mouse-=front.y;
        tz_mouse-=front.z;
      }
      else {
        z_mouse-=dz;  // total translation
      }
    }
    if (is_translation) {
      setTranslation(tx_mouse,ty_mouse,tz_mouse);
      //setTranslation(dx,dy,-dz);
    }
    else {
      //std::cerr << "xyz mouse="<<x_mouse<<" "<<y_mouse<<" "<< z_mouse<<"\n";
      if (store_options->rotate_screen)
        setRotationScreen(y_mouse,x_mouse,z_mouse);
      else
        setRotationScene(y_mouse,x_mouse,z_mouse);
    }
  }
  //!options_form->downloadOptions(store_options);
  if (is_pressed_middle_button) {
    dx = e->x()-last_posx;
    dy = e->y()-last_posy;
    // save last position
    last_posx = e->x();
    last_posy = e->y();
    emit sigMouseXY(dx,dy);
    //std::cerr << "dx="<<dx<< "  dy="<<dy<<"\n";
  }
}
// ============================================================================
// manage zoom according to wheel event
void GLWindow::wheelEvent(QWheelEvent * e)
{
  if(e->delta() > 0)
    mouseWheelUp();
  else if(e->delta() < 0)
    mouseWheelDown();
}
// ============================================================================
// manage keyboard press events
void GLWindow::keyPressEvent(QKeyEvent * k)
{
  setFocus();
  if (k->key() == Qt::Key_Control ) {
    is_key_pressed = TRUE;
    is_translation = TRUE;
    is_pressed_ctrl= TRUE;
    getPixelTranslation(&tx_mouse,&ty_mouse,&tz_mouse);
    if (is_shift_pressed) {
      is_translation=FALSE;
      is_mouse_zoom=TRUE;
    }
  }
  if (k->key() == Qt::Key_A) {
    is_key_pressed = TRUE;
    is_a_key_pressed = TRUE;
    //!glbox->toggleLineAliased();
  }
  if (k->key() == Qt::Key_Plus) {
    is_key_pressed = TRUE;
    mouseWheelDown();
    //!statusBar()->message("Zoom IN");
  }
  if (k->key() == Qt::Key_Minus) {
    is_key_pressed = TRUE;
    mouseWheelUp();
    //!statusBar()->message("Zoom OUT");
  }
  if (k->key() == Qt::Key_Shift) {
    is_shift_pressed = TRUE;
    if (is_pressed_ctrl) {
      is_translation=FALSE;
      is_mouse_zoom=TRUE;
    }
  }
  if(k->key() == Qt::Key_W){
    is_key_pressed = TRUE;
    if(new_camera->getCameraMode() == CameraMode::free)
      new_camera->moveForward(16);
  }
  emit sigKeyMouse( is_key_pressed, is_mouse_pressed);
  //!options_form->downloadOptions(store_options);
}
// ============================================================================
// manage keyboard release events
void GLWindow::keyReleaseEvent(QKeyEvent * k)
{
  if (k->key() == Qt::Key_Control ) {
    is_translation = FALSE;
    is_pressed_ctrl= FALSE;
    is_mouse_zoom  = FALSE;
  }
  if (k->key() == Qt::Key_Shift) {
    is_shift_pressed = FALSE;
    is_mouse_zoom  = FALSE;
    gl_select->reset();
    updateGL();
  }
  if (k->key() == Qt::Key_A) {
    is_a_key_pressed = FALSE;
  }
  is_key_pressed = FALSE;
  emit sigKeyMouse( is_key_pressed, is_mouse_pressed);
  //!options_form->downloadOptions(store_options);
}
// ============================================================================
//  Set the rotation angle of the object to n degrees around the X,Y,Z axis of the screen
void GLWindow::setRotationScreen( const int x, const int y, const int z )
{
  // rotate angles
  GLfloat xRot = (GLfloat)(x % 360);
  GLfloat yRot = (GLfloat)(y % 360);
  GLfloat zRot = (GLfloat)(z % 360);
  // hud display
  osd->setText(GLObjectOsd::Rot,xRot,yRot,zRot);
  osd->updateDisplay();
  // save values
  store_options->xrot =x;
  store_options->yrot =y;
  store_options->zrot =z;
  updateGL();
}
// ============================================================================
//  Set the rotation angle of the object to n degrees around the U,V,W axis of the scene/object
void GLWindow::setRotationScene( const int x, const int y, const int z )
{
  // rotate angles
  GLfloat xRot = (GLfloat)(x % 360);
  GLfloat yRot = (GLfloat)(y % 360);
  GLfloat zRot = (GLfloat)(z % 360);
//  // hud display
//  osd->setText(GLObjectOsd::Rot,xRot,yRot,zRot);
//  osd->updateDisplay();
  // save values
  store_options->urot =xRot;
  store_options->vrot =yRot;
  store_options->wrot =zRot;
  updateGL();
}
// -----------------------------------------------------------------------------
// rotateAroundAxis()
void GLWindow::rotateAroundAxis(const int axis)
{
  if (!is_key_pressed              && // no interactive user request
      !is_mouse_pressed) {

      switch (axis) {
      case 0: // X
              store_options->xrot += store_options->ixrot;
              y_mouse = (int) store_options->xrot;
              break;
      case 1: // Y
              store_options->yrot += store_options->iyrot;
              x_mouse = (int) store_options->yrot;
              break;
      case 2: // Z
              store_options->zrot += store_options->izrot;
              z_mouse = (int) store_options->zrot;
              break;
      case 3: // U
              store_options->urot += store_options->iurot;
              //y_mouse = (int) store_options->urot;
              i_umat = 0;
              i_vmat = 1;
              i_wmat = 2;
              break;
      case 4: // V
              store_options->vrot += store_options->ivrot;
              //x_mouse = (int) store_options->vrot;
              i_umat = 4;
              i_vmat = 5;
              i_wmat = 6;
              break;
      case 5: // W
              store_options->wrot += store_options->iwrot;
              //z_mouse = (int) store_options->wrot;
              i_umat = 8;
              i_vmat = 9;
              i_wmat = 10;
              break;
      }
      if (axis < 3) {
        setRotationScreen(store_options->xrot,store_options->yrot,store_options->zrot);
      } else {
        setRotationScene(store_options->urot,store_options->vrot,store_options->wrot);
      }
      //setRotation(x,y,z);
      //updateGL();
  }
}
// -----------------------------------------------------------------------------
// translateAlongAxis()
void GLWindow::translateAlongAxis(const int axis)
{
  if (!is_key_pressed              && // no interactive user request
      !is_mouse_pressed) {
      switch (axis) {
      case 0: tx_mouse++;  // X
              setTranslation(tx_mouse,ty_mouse,tz_mouse);
              break;
      case 1: ty_mouse++;  // Y
              setTranslation(tx_mouse,ty_mouse,tz_mouse);
              break;
      case 2: tz_mouse--;  // Z
              setTranslation(tx_mouse,ty_mouse,tz_mouse);
              break;
      }
    updateGL();
  }
}
// ============================================================================
// GLBox::getPixelTranslation()
// return x,y,z pixel translation according to current translation viewport
// coordinates and zoom
void GLWindow::getPixelTranslation(float *x, float *y, float *z)
{
  GLint	Viewport[4];
  glGetIntegerv(GL_VIEWPORT,Viewport);
  *x = (float) (-store_options->xtrans * Viewport[2]/store_options->zoom);
  *y = (float) ( store_options->ytrans * Viewport[3]/store_options->zoom);
  *z = (float) ( store_options->ztrans * Viewport[2]/store_options->zoom);
}
// ============================================================================
// GLBox::setTranslation()
// Set the translation angle
void GLWindow::setTranslation(const float x, const float y, const float z )
{
  GLint	Viewport[4];
  glGetIntegerv(GL_VIEWPORT,Viewport);
#if 0
  ixTrans = x;
  iyTrans = y;
  izTrans = z;
#endif
  // compute translation
  GLfloat xTrans = (GLfloat)(-x*store_options->zoom/(Viewport[2]));
  GLfloat yTrans = (GLfloat)( y*store_options->zoom/(Viewport[3])); //Viewport[3]*ratio));
  GLfloat zTrans = (GLfloat)( z*store_options->zoom/(Viewport[2]));
  // display on HUD
  osd->setText(GLObjectOsd::Trans,-xTrans,-yTrans,-zTrans);
  osd->updateDisplay();
  // save
  store_options->xtrans=xTrans;
  store_options->ytrans=yTrans;
  store_options->ztrans=zTrans;
  store_options->mat4_model = glm::mat4();
  store_options->mat4_model = glm::translate(store_options->mat4_model,glm::vec3(xTrans,yTrans,zTrans));
  updateGL();
}
// -----------------------------------------------------------------------------
// updateOsd()
void GLWindow::updateOsdZrt(bool ugl)
{
  GlobalOptions * g = store_options;
  setOsd(GLObjectOsd::Zoom,(const float) store_options->zoom,
                    g->osd_zoom,false);
  setOsd(GLObjectOsd::Rot,(const float) store_options->xrot,
                    (const float) store_options->yrot,
                    (const float) store_options->zrot, g->osd_rot,false);
  setOsd(GLObjectOsd::Trans,(const float) -store_options->xtrans,
                    (const float) -store_options->ytrans,
                    (const float) -store_options->ztrans,g->osd_trans,false);

  osd->updateDisplay();
  if (ugl) {
   updateGL();
  }

}
void GLWindow::mouseWheelUp()
{
  const CameraMode mode = new_camera->getCameraMode();
  if(mode == arcball){
    float new_zoom = new_camera->increaseZoom();
    store_options->zoom = new_zoom;
    store_options->zoomo = new_zoom;
    osdZoom();
  }
  // else{
  //   store_options->speed = new_camera->increaseSpeed();
  // }
}
void GLWindow::mouseWheelDown()
{
  const CameraMode mode = new_camera->getCameraMode();
  if(mode == arcball){
    float new_zoom = new_camera->decreaseZoom();
    store_options->zoom = new_zoom;
    store_options->zoomo = new_zoom;
    osdZoom();
  }
  // else{
  //   store_options->speed = new_camera->decreaseSpeed();
  // }
}
// ============================================================================
// setup zoom according to a z value
void GLWindow::setZoom(const int z)
{
  if (z>0) {
    store_options->zoom  *= 1.1;//1.1025; //1.1;
    store_options->zoomo *= 1.1;//1.1025; //1.1;
  } else {
    store_options->zoom  *= 0.9;//0.8075; //0.9;
    store_options->zoomo *= 0.9;//0.8075; //0.9;
  }
  osdZoom();
}
// ============================================================================
// update OSD zoom value
void GLWindow::osdZoom(bool ugl)
{
  osd->setText(GLObjectOsd::Zoom,(const float) store_options->zoom);
  osd->updateDisplay();
  if (ugl) updateGL();
}
// ============================================================================
// GLWindow::setOsd()
// Set Text value to the specified HudObject
void GLWindow::setOsd(const GLObjectOsd::OsdKeys k, const QString text, bool show, bool ugl)
{
  osd->keysActivate(k,show);
  osd->setText(k,text);
  if (ugl) updateGL();
}
// ============================================================================
// GLWindow::setOsd()
// Set Float value to the specified HudObject
void GLWindow::setOsd(const GLObjectOsd::OsdKeys k, const float value, bool show, bool ugl)
{
  osd->keysActivate(k,show);
  osd->setText(k,(const float) value);
  if (ugl) updateGL();
}
// ============================================================================
// GLWindow::setOsd()
// Set Float value to the specified HudObject
void GLWindow::setOsd(const GLObjectOsd::OsdKeys k, const float value1,
                      const float value2, const float value3, bool show,bool ugl)
{
  osd->keysActivate(k,show);
  osd->setText(k,(const float) value1,(const float) value2,(const float) value3);
  if (ugl) updateGL();
}
// ============================================================================
// GLWindow::setOsd()
// Set Int value to the specified HudObject
void GLWindow::setOsd(const GLObjectOsd::OsdKeys k, const int value, bool show, bool ugl)
{
  osd->keysActivate(k,show);
  osd->setText(k,(const int) value);
  if (ugl) updateGL();
}
// ============================================================================
// GLWindow::changeOsdFont()
// Change OSD font
void GLWindow::changeOsdFont()
{
  fntRenderer text;
  if (font) delete font;
  font = new fntTexFont(store_options->osd_font_name.toStdString().c_str());
  text.setFont(font);
  text.setPointSize(store_options->osd_font_size );
  osd->setFont(text);
  osd->setColor(store_options->osd_color);
  updateGL();
}
// ============================================================================
// set texture on the object
void GLWindow::setTextureObject(const int tex, const int obj)
{
  if (obj < (int) gpv.size() ) {
    gpv[obj].setTexture(tex);
  }
}
// ============================================================================
// setZoom()
// setup zoom according to a z value
void GLWindow::setZoom(const float z)
{
  const float zoom = z;
  store_options->zoom=zoom;
  osdZoom();
}
// ============================================================================
// >> HERE WE FORCE PERSPECTIVE PROJECTION
//    TO COMPUTE BOTH BEST ZOOM FOR ORTHO
//    AND PERSPECTVE PROJECTION
//
//    we must force perspective projection to have
//    the good projection and modelview matrix
void GLWindow::setPerspectiveMatrix()
{
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  gluPerspective(45.,ratio,0.0005,(float) DOF);
#if 1
  glMatrixMode( GL_MODELVIEW );
  glLoadIdentity();
  camera->setEye(0.0,  0.0,  -store_options->zoom);
  camera->moveTo();
  // apply screen rotation on the whole system
  glMultMatrixd (mScreen);
  // apply scene/world rotation on the whole system
  glMultMatrixd (mScene);
  setModelMatrix(); // save ModelView  Matrix
#endif
  setProjMatrix();  // save Projection Matrix
}

// ============================================================================
// Best Zoom fit
// fit all the particles on the screen from perspective view
void GLWindow::bestZoomFit()
{
  if ( !store_options->duplicate_mem) mutex_data->lock();

  glGetIntegerv(GL_VIEWPORT,viewport);
  ratio =  ((double )viewport[2]) / ((double )viewport[3]);

  setPerspectiveMatrix(); // toggle to perspective matric mode


  Tools3D::bestZoomFromObject(mProj,mModel,
                              viewport, pov, p_data, store_options);

  ortho_right = store_options->ortho_range;
  ortho_left  =-store_options->ortho_range;
  ortho_top   = store_options->ortho_range;
  ortho_bottom=-store_options->ortho_range;
  //store_options->zoomo = 1.;

  osdZoom();
  if ( !store_options->duplicate_mem) mutex_data->unlock();
}

//---------------------------------------------------------------------------------------------------------------------
// Purpose: Returns true if the action is active and had a rising edge
//---------------------------------------------------------------------------------------------------------------------
bool GetDigitalActionRisingEdge(vr::VRActionHandle_t action, vr::VRInputValueHandle_t *pDevicePath = nullptr )
{
	vr::InputDigitalActionData_t actionData;
	vr::VRInput()->GetDigitalActionData(action, &actionData, sizeof(actionData), vr::k_ulInvalidInputValueHandle );
	if (pDevicePath)
	{
		*pDevicePath = vr::k_ulInvalidInputValueHandle;
		if (actionData.bActive)
		{
			vr::InputOriginInfo_t originInfo;
			if (vr::VRInputError_None == vr::VRInput()->GetOriginTrackedDeviceInfo(actionData.activeOrigin, &originInfo, sizeof(originInfo)))
			{
				*pDevicePath = originInfo.devicePath;
			}
		}
	}
	return actionData.bActive && actionData.bChanged && actionData.bState;
}

//---------------------------------------------------------------------------------------------------------------------
// Purpose: Returns true if the action is active and its state is true
//---------------------------------------------------------------------------------------------------------------------
bool GetDigitalActionState(vr::VRActionHandle_t action, vr::VRInputValueHandle_t *pDevicePath = nullptr )
{
	vr::InputDigitalActionData_t actionData;
	vr::VRInput()->GetDigitalActionData(action, &actionData, sizeof(actionData), vr::k_ulInvalidInputValueHandle );
	if (pDevicePath)
	{
		*pDevicePath = vr::k_ulInvalidInputValueHandle;
		if (actionData.bActive)
		{
			vr::InputOriginInfo_t originInfo;
			if (vr::VRInputError_None == vr::VRInput()->GetOriginTrackedDeviceInfo(actionData.activeOrigin, &originInfo, sizeof(originInfo)))
			{
				*pDevicePath = originInfo.devicePath;
			}
		}
	}
	return actionData.bActive && actionData.bState;
}

bool GLWindow::HandleInput() {
    bool bRet = false;

    // Process SteamVR events
//	vr::VREvent_t event;
//	while( vr_context->PollNextEvent( &event, sizeof( event ) ) )
//	{
//		ProcessVREvent( event );
//	}

    // Process SteamVR action state
    // UpdateActionState is called each frame to update the state of the actions themselves. The application
    // controls which action sets are active with the provided array of VRActiveActionSet_t structs.
    vr::VRActiveActionSet_t actionSet = {0};
    actionSet.ulActionSet = m_actionsetDemo;
    vr::VRInput()->UpdateActionState(&actionSet, sizeof(actionSet), 1);

//	m_bShowCubes = !GetDigitalActionState( m_actionHideCubes );


    vr::InputAnalogActionData_t analogData[2];

    if (vr::VRInput()->GetAnalogActionData(m_rHand[Right].m_actionAnalongInput, &analogData[Right], sizeof(analogData[Right]),
                                           vr::k_ulInvalidInputValueHandle) == vr::VRInputError_None &&
        analogData[Right].bActive) {
        if (analogData[Right].x != 0) {
            emit editGazSlideSizeValueByDelta(analogData[Right].x / 50);
            //m_scale += analogData.x/1000;
            //world_scale = glm::vec3(m_scale, m_scale, m_scale);
        }

    }

    if (vr::VRInput()->GetAnalogActionData(m_rHand[Left].m_actionAnalongInput, &analogData[Left], sizeof(analogData[Left]),
                                         vr::k_ulInvalidInputValueHandle) == vr::VRInputError_None &&
      analogData[Left].bActive) {
      if (analogData[Left].x != 0 || analogData[Left].y != 0)
          emit editDensSlideByDelta(analogData[Left].x, analogData[Left].y);
  }

    vr::InputPoseActionData_t poseData[2];
    glm::vec3 controller_position[2];
    glm::mat4 pose[2];

    for (EHand eHand = Left; eHand <= Right; ((int &) eHand)++) {
        if (GetDigitalActionRisingEdge(m_rHand[eHand].m_grip)) {
            if (vr::VRInput()->GetPoseActionDataForNextFrame(m_rHand[Right].m_actionPose,
                                                             vr::TrackingUniverseStanding,
                                                             &poseData[Right], sizeof(poseData[Right]),
                                                             vr::k_ulInvalidInputValueHandle) != vr::VRInputError_None || !poseData[Right].bActive || !poseData[Right].pose.bPoseIsValid) {
            } else {
                m_rHand[Right].m_initial_controller_position = glm::vec3(vr34ToGlm(poseData[Right].pose.mDeviceToAbsoluteTracking)[3]);
                m_rHand[Right].m_previous_orientation = quat_cast(vr34ToGlm(poseData[Right].pose.mDeviceToAbsoluteTracking));
            }
            if (vr::VRInput()->GetPoseActionDataForNextFrame(m_rHand[Left].m_actionPose,
                                                             vr::TrackingUniverseStanding,
                                                             &poseData[Left], sizeof(poseData[Left]),
                                                             vr::k_ulInvalidInputValueHandle) != vr::VRInputError_None || !poseData[Left].bActive || !poseData[Left].pose.bPoseIsValid) {
            } else {
                m_rHand[Left].m_initial_controller_position = glm::vec3(vr34ToGlm(poseData[Left].pose.mDeviceToAbsoluteTracking)[3]);
                m_rHand[Left].m_previous_orientation = quat_cast(vr34ToGlm(poseData[Left].pose.mDeviceToAbsoluteTracking));
            }
            m_initial_scene_position = glm::vec3(m_scene_matrix[3]);
            m_initial_scale = m_scale;
            m_initial_scene_orientation = glm::quat(m_scene_matrix);
            m_prev_controller_orientation = m_rHand[Left].m_initial_controller_position - m_rHand[Right].m_initial_controller_position;
        }
    }

    if (GetDigitalActionState(m_rHand[Left].m_grip) && GetDigitalActionState(m_rHand[Right].m_grip)) {
        // process left

        if (vr::VRInput()->GetPoseActionDataForNextFrame(m_rHand[Left].m_actionPose,
                                                         vr::TrackingUniverseStanding,
                                                         &poseData[Left], sizeof(poseData[Left]),
                                                         vr::k_ulInvalidInputValueHandle) != vr::VRInputError_None || !poseData[Left].bActive || !poseData[Left].pose.bPoseIsValid) {
        } else {
            pose[Left] = vr34ToGlm(poseData[Left].pose.mDeviceToAbsoluteTracking);
            controller_position[Left] = glm::vec3(pose[Left][3]);
        }

        // process right
        if (vr::VRInput()->GetPoseActionDataForNextFrame(m_rHand[Right].m_actionPose,
                                                         vr::TrackingUniverseStanding,
                                                         &poseData[Right], sizeof(poseData[Right]),
                                                         vr::k_ulInvalidInputValueHandle) != vr::VRInputError_None || !poseData[Right].bActive || !poseData[Right].pose.bPoseIsValid ) {
        } else {
            pose[Right] = vr34ToGlm(poseData[Right].pose.mDeviceToAbsoluteTracking);
            controller_position[Right] = glm::vec3(pose[Right][3]);
        }

        // reset scene matrix
        m_scene_matrix = glm::identity<glm::mat4>();

        // apply combined orientation
        auto current_orientation = controller_position[Left] - controller_position[Right];
        m_current_rotation = normalize(normalize((RotationBetweenVectors(m_prev_controller_orientation, current_orientation))) * m_current_rotation);
        m_prev_controller_orientation = current_orientation;
/*
        // apply both hands orientation separately
        auto current_right_orientation = quat_cast(pose[Right]);
        auto current_left_orientation = quat_cast(pose[Left]);

        auto delta_right_orientation = current_right_orientation * inverse(m_rHand[Right].m_previous_orientation);
        auto delta_left_orientation = current_left_orientation * inverse(m_rHand[Left].m_previous_orientation);

        auto scaled_delta_right_orientation = slerp(quat(1,0,0,0), delta_right_orientation, 0.5f);
        auto scaled_delta_left_orientation = slerp(quat(1,0,0,0), delta_left_orientation, 0.5f);

        m_current_rotation = scaled_delta_right_orientation * m_current_rotation;
        m_current_rotation = scaled_delta_left_orientation * m_current_rotation;

        m_rHand[Right].m_previous_orientation = current_right_orientation;
        m_rHand[Left].m_previous_orientation = current_left_orientation;
*/
        // apply orientation to scene matrix
        m_scene_matrix = mat4_cast(m_current_rotation) * m_scene_matrix;

        // translate by deplacement of the middle of the two controllers
        auto mean_controller_position = (controller_position[Right] - m_rHand[Right].m_initial_controller_position + controller_position[Left] - m_rHand[Left].m_initial_controller_position) / 2.f ;
        auto new_scene_position = mean_controller_position  + m_initial_scene_position;

        m_scene_matrix[3][0] = new_scene_position[0];
        m_scene_matrix[3][1] = new_scene_position[1];
        m_scene_matrix[3][2] = new_scene_position[2];

        // scale by distance between controllers
        m_scale = m_initial_scale + (glm::length(controller_position[Right] - controller_position[Left]) - length(m_rHand[Right].m_initial_controller_position - m_rHand[Left].m_initial_controller_position))  *m_initial_scale;

        if (m_scale < 0.00001)
            m_scale = 0.00001;

        world_scale = glm::vec3(m_scale, m_scale, m_scale);

        // apply scale
        m_scene_matrix = glm::scale(m_scene_matrix, world_scale);

    }



//
//	m_rHand[Left].m_bShowController = true;
//	m_rHand[Right].m_bShowController = true;
//
//	vr::VRInputValueHandle_t ulHideDevice;
//	if ( GetDigitalActionState( m_actionHideThisController, &ulHideDevice ) )
//	{
//		if ( ulHideDevice == m_rHand[Left].m_source )
//		{
//			m_rHand[Left].m_bShowController = false;
//		}
//		if ( ulHideDevice == m_rHand[Right].m_source )
//		{
//			m_rHand[Right].m_bShowController = false;
//		}
//	}
//

	return bRet;
}

// before openvr
//void GLWindow::renderVR() {
//
//  if (!store_options->duplicate_mem)
//    mutex_data->lock();
//
//  QImage cubemap_faces[6];
//  store_options->axes_enable = false;
//  store_options->show_osd = false;
//  new_camera->setCameraMode(CameraMode::free);
//  for (CubemapFace face = first; face != last; face = static_cast<CubemapFace>(face + 1)) {
//    new_camera->setOrientation(face);
//    setFBO(true);                              // activate Frame Buffer Object
//    forcePaintGL();  // draw in FBO
//
//    grabFrameBufferObject().mirrored().rgbSwapped().save("/home/kalterkrieg/Documents/lam/glnemo/cubemap/"+QString::number(face)+".png"); // convert FBO to img
//  }
//
//  glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, framebuffer);
//  //glGenRenderbuffersEXT(1, &renderbuffer);
//  glBindRenderbufferEXT(GL_RENDERBUFFER_EXT, renderbuffer);
//  glRenderbufferStorageEXT(GL_RENDERBUFFER_EXT, GL_RGBA8, final_image_size[0], final_image_size[1]);
//  glFramebufferRenderbufferEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT,
//                               GL_RENDERBUFFER_EXT, renderbuffer);
//  GLuint status = glCheckFramebufferStatusEXT(GL_FRAMEBUFFER_EXT);
//  if (status != GL_FRAMEBUFFER_COMPLETE_EXT) {
//    std::cerr << "ERROR FRAMEBUFFER VR\n";
//  }
//
//  GLuint cubemap_tex_id;
//  glGenTextures(1, &cubemap_tex_id);
//  glActiveTextureARB(GL_TEXTURE0);
//  glBindTextureEXT(GL_TEXTURE_CUBE_MAP, cubemap_tex_id);
//
//  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
//  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
//  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
//  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
//  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
//
//  for (CubemapFace face = first; face != last; face = static_cast<CubemapFace>(face + 1)) {
//    glTexImage2D(
//            GL_TEXTURE_CUBE_MAP_POSITIVE_X + face,
//            0, GL_RGB, cubemap_faces[face].width(), cubemap_faces[face].height(), 0, GL_RGBA, GL_UNSIGNED_BYTE,
//            cubemap_faces[face].bits()
//    );
//  }
//
//  qglClearColor(store_options->background_color);
//  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
//
//  vr_shader->start();
//
//  vr_shader->sendUniformi("tex", 0);
//
//  //        glGenerateMipmapEXT(GL_TEXTURE_2D);
//  glDrawArraysEXT(GL_TRIANGLE_STRIP, 0, 4);
//  vr_shader->stop();
//
//  imgFBO = QImage(final_image_size[0], final_image_size[1], QImage::Format_RGB32);
//  glReadPixels(0, 0, final_image_size[0], final_image_size[1], GL_RGBA, GL_UNSIGNED_BYTE, imgFBO.bits());
//
//  if (!store_options->duplicate_mem) mutex_data->unlock();
//  glBindTexture(GL_TEXTURE_2D, 0);
//  glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
//}

} // namespace glnemo
