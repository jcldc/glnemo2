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
#include <QtGlobal>
#include <QtGui>
#include <QtOpenGL>
#include <QFile>
#include <QString>

#include <glm/fwd.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <sstream>
#include "camera.h"
#include "glwindow.h"
#include <GL/glu.h>


namespace glnemo {
  // ============================================================================
  // constructor                                                                 
  Camera::Camera(GlobalOptions *so)
  {
    texture = NULL;
    shader = NULL;
    store_options = so;
    spline = new CRSpline();
    spline_mode = false;
    play_timer   = new QTimer(this);
    connect(play_timer,SIGNAL(timeout()),this,SLOT(playGL())); // update GL at every timeout()
    //checkGSLSupport(); // detect GSL support
    up=glm::vec3(0.0,0.0,1.0);
    play_loop=true;
    reset();
  }
  // ============================================================================
  // destructor                                                                 
  Camera::~Camera()
  {
    if (spline) delete spline;
  #if 0 // 330
    glDeleteLists( dplist_index, 1 );
  #endif
    if (shader ) {
      delete shader;
    }
  }

  // ============================================================================
  // loadShader
  void Camera::loadShader()
  {
    //GLWindow::m_glWidget->makeCurrent();
    QOpenGLExtraFunctions *f = QOpenGLContext::currentContext()->extraFunctions();

    if (GLWindow::GLSL_support) {
      f->glGenBuffers(1,&vbo_path);
      f->glGenBuffers(1,&vbo_ctrl);
      // particles shader
      shader = new CShader(GlobalOptions::RESPATH.toStdString()+"/shaders/glsl_330/camera.vert.cc",
                           GlobalOptions::RESPATH.toStdString()+"/shaders/glsl_330/camera.frag.cc");
      shader->init();
    }
    if (! texture) texture = new GLTexture();
    if (texture->load(GlobalOptions::RESPATH+"gaussian",NULL)) {
    } else {
      std::cerr << "\n\nUnable to load TEXTURE.....\n\n";
    }
    //GLWindow::m_glWidget->doneCurrent();
  }

  // ============================================================================
  // reset
  void Camera::reset()
  {
    play_timer->stop();
    ex=ey=ez=0.0;     // eyes
    cx=cy=cz=0.0;     // center
    ux=uz=0.0;uy=1.0; // up vectors
    last_up=glm::vec3(ux,uz,uz);

    display_ctrl = false; // toggle display ctrl
    display_path = false; // toggle display path
    play         = false; // toggle animation
    index_frame  = 0;
    Vec3D zero(0.,0.,0.);
    rv=zero;
    spline_mode = false; // control view (spline or viewer)
    //up=glm::vec3(ux,0.0,1.0);

  }
  // ============================================================================
  // init
  void Camera::init(std::string filename, const int _p, const float _s)
  {
    npoints = _p; // #interpolated points
    scale   = _s; // scale factor applied on top of ctrl points
  #if 0 // 330
    dplist_index = glGenLists( 1 );    // get a new display list index
  #endif
    if (filename != "") {
      loadSplinePoints(filename);
    }
  }
  // ============================================================================
  // setEye                                                                      
  void Camera::setEye(const float x, const float y, const float z)
  {
    ex=x;ey=y;ez=z;
    if (!play && (rv.x!=0. || rv.y!=0. || rv.z!=0)) {
      ex=rv.x;
      ey=rv.y;
      uz=ez;
      ez=rv.z;
    }
  }
  // ============================================================================
  //  setCenter                                                                  
  void Camera::setCenter(const float x, const float y, const float z)
  {
    cx=x;cy=y;cz=z;
  }
  // ============================================================================
  //  setUp                                                                      
  void Camera::setUp(const float x, const float y, const float z)
  {
    ux=x;uy=y;uz=z;
  }
  // ============================================================================
  //  moveTo                                                                     
  void Camera::moveTo()
  {
    //glm::vec3 up(ux,0.0,1.0);

    if (!play && ! spline_mode) {
      store_options->mat4_view = glm::lookAt(glm::vec3(ex, ey, ez),
                glm::vec3(cx, cy, cz),
                glm::vec3(ux, uy, uz));
    }
    else {
      index_frame = index_frame%npoints;
      //std::cerr << "frame : "<< index_frame << "\n";
      float  t=(float)index_frame / (float)npoints;
      rv = spline->GetInterpolatedSplinePoint(t)*scale;


#if 0 // camera crossed vect along path
      if (index_frame < (npoints-1)) {
        float  t=(float)(index_frame+1) / (float)npoints;
        Vec3D rv2 = spline->GetInterpolatedSplinePoint(t)*scale;

        Vec3D vdisp = rv2-rv; // displacement vector

        Vec3D center(cx,cy,cz);
        Vec3D veyes = center-rv; // point to center view direction vector

        // compute up vector orthogonal to vdisp and veyes
        up = glm::cross(glm::vec3(veyes.x,veyes.y,veyes.z),
                        glm::vec3(vdisp.x,vdisp.y,vdisp.z));

        up = glm::abs(up);
        if (up.x==0. && up.y==0. && up.z==0.) {
          up = glm::vec3(ux,0.0,1.0);
        }
        last_up = up;
      } else {
        up = last_up;
      }
#endif
      store_options->mat4_view = glm::lookAt(glm::vec3(rv.x, rv.y, rv.z),
                glm::vec3(cx, cy, cz),
                glm::vec3(up.x,up.y,up.z));
                //ux, 0.0, 1.0);// ez); // ez, why ????!!!!!!
      if (play) {
        index_frame++;
        if (!play_loop && index_frame >= npoints ) {
          // we reach end of path
          emit sig_stop_play(); // tell to formoption to stop playing
        }
      }
      //std::cerr << "r : "<<rv.x<<" "<<rv.y<<" "<<rv.z<<"\n";
    }
#if 0
    std::cerr << "e : "<<ex<<" "<<ey<<" "<<ez<<"\n";
    std::cerr << "c : "<<cx<<" "<<cy<<" "<<cz<<"\n";
    std::cerr << "u : "<<ux<<" "<<uy<<" "<<uz<<"\n";
    std::cerr << "up: "<<up.x<<" "<<up.y<<" "<<up.z<<"\n";
#endif
  }
  // ============================================================================
  //  loadSplinePoints                                                           
  int Camera::loadSplinePoints(std::string filen)
  {
    //GLWindow::m_glWidget->makeCurrent();
    QFile infile(QString(filen.c_str()));
    if (!infile.open(QIODevice::ReadOnly | QIODevice::Text))
      return 0;
    // clear ctrl points
    spline->clearCPoints();
        
    QTextStream in(&infile);
    QString line;
    bool valid=false;
    line = in.readLine();
    if (line=="#camera_path") { // it's a valid camera path file
      valid=true;
      do {
        line = in.readLine();
        if (!line.isNull() && !line.isEmpty()) {
          //std::cerr << "line :" << line.toStdString() <<"\n";
          std::istringstream ss(line.toStdString());
          float x,y,z;
          ss >> x;
          ss >> y;
          ss >> z;
          Vec3D v(x,y,z);
          spline->AddSplinePoint(v);
        }
      } while (!line.isNull());
    }
    infile.close();
    if (valid) {
      if (GLWindow::GLSL_support) { // update vbo
        updateVbo();
      } else {
        buildDisplayList();
      }
    }
    //GLWindow::m_glWidget->doneCurrent();
    return valid;
  }
  // ============================================================================
  //  updateVbo
  // build VBO for control points
  void Camera::updateVbo()
  {
    std::vector<GLfloat>  vpos;
    QOpenGLExtraFunctions *f = QOpenGLContext::currentContext()->extraFunctions();
    // ---> interpolated points, aka PATH
    for (int i=0; i<npoints; i++) {
      float  t=(float)(i) / (float)npoints;
      Vec3D rv = spline->GetInterpolatedSplinePoint(t)*scale;
      vpos.push_back(rv.x);
      vpos.push_back(rv.y);
      vpos.push_back(rv.z);
    }
    // bind VBO buffer for sending data
    f->glBindBuffer(GL_ARRAY_BUFFER_ARB, vbo_path);
    // upload Positions (and Velocities) to VBO
    f->glBufferData(GL_ARRAY_BUFFER_ARB,vpos.size()*sizeof(GLfloat),&vpos[0], GL_STATIC_DRAW_ARB);
    // unbind
    f->glBindBuffer(GL_ARRAY_BUFFER_ARB, 0);

    // ---> control points
    vpos.clear();
    for (int i=0; i<spline->GetNumPoints(); i++) {
      Vec3D rv = spline->GetNthPoint(i)*scale;
      vpos.push_back(rv.x);
      vpos.push_back(rv.y);
      vpos.push_back(rv.z);
    }
    // bind VBO buffer for sending data
    f->glBindBuffer(GL_ARRAY_BUFFER_ARB, vbo_ctrl);
    // upload Positions (and Velocities) to VBO
    f->glBufferData(GL_ARRAY_BUFFER_ARB,vpos.size()*sizeof(GLfloat),&vpos[0], GL_STATIC_DRAW_ARB);
    // unbind
    f->glBindBuffer(GL_ARRAY_BUFFER_ARB, 0);
  }
  // ============================================================================
  //  buildDisplayList                                                           
  void Camera::buildDisplayList()
  {
  #if 0 // 330
    glNewList( dplist_index, GL_COMPILE );
    glBegin(GL_LINE_STRIP);
    for (int i=0; i<npoints; i++) {
      float  t=(float)i / (float)npoints;
      Vec3D rv = spline->GetInterpolatedSplinePoint(t)*scale;    
      glVertex3f(rv.x, rv.y, rv.z);  
    }
    glEnd();
    glEndList();
  #endif
  }
  // ============================================================================
  // displayCameraPath                                                           
  void Camera::displayCameraPath()
  {
    if (spline->GetNumPoints() > 0 ) {
      glEnable (GL_LINE_SMOOTH);
      glEnable (GL_BLEND);
      glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
      glHint (GL_LINE_SMOOTH_HINT, GL_DONT_CARE);
      glLineWidth (1.5);
      GLObject::display(dplist_index);
      glDisable(GL_BLEND);
    }
  }
  // ============================================================================
  // displayVbo
  // display ctrl and camera path
  void Camera::displayVbo()
  {
    QOpenGLExtraFunctions *f = QOpenGLContext::currentContext()->extraFunctions();
    
    // Mandatory for Core Profile: VAO
    GLuint vao;
    f->glGenVertexArrays(1, &vao);
    f->glBindVertexArray(vao);

    // color
    mycolor = Qt::yellow;
    float col[4];
    col[0] = mycolor.redF();
    col[1] = mycolor.greenF();
    col[2] = mycolor.blueF();
    col[3] = mycolor.alphaF();

    glEnable(GL_PROGRAM_POINT_SIZE); //glEnable(0x8861);

    // ------------------------------
    // start shader
    // ------------------------------

    shader->start();

    // Send color uniform
    shader->sendUniformXfv("color", 4, 1, col);

    // texture
    f->glActiveTexture(GL_TEXTURE0);
    texture->glBindTexture();  // bind texture

    // send matrix
    shader->sendUniformXfv("projMatrix",16,1,
                           glm::value_ptr(store_options->mat4_proj));
    glm::mat4 mv=store_options->mat4_view*store_options->mat4_model;
    shader->sendUniformXfv("modelviewMatrix",16,1,glm::value_ptr(mv));

    // Send data to Pixel Shader
    shader->sendUniformi("splatTexture", 0);

    // get attribute location for sprite size
    int a_sprite_size = f->glGetAttribLocation(shader->getProgramId(), "a_sprite_size");
    if ( a_sprite_size != -1) {
        f->glVertexAttrib1f(a_sprite_size, 5.0f);
    }

    int vpositions = f->glGetAttribLocation(shader->getProgramId(), "position");

    // send vertex positions only
    if (display_path && vpositions != -1) {
      // Don't use texture for path (lines)
      shader->sendUniformi("use_texture", 0);
      
      // setup lines
      glDisable(GL_DEPTH_TEST);
      glEnable (GL_LINE_SMOOTH);
      glEnable (GL_BLEND);
      glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
      glHint (GL_LINE_SMOOTH_HINT, GL_DONT_CARE);
      glLineWidth (4.5f);

      f->glBindBuffer(GL_ARRAY_BUFFER, vbo_path);
      f->glEnableVertexAttribArray(vpositions);
      f->glVertexAttribPointer(vpositions, 3, GL_FLOAT, GL_FALSE, 0, (void *) 0);
      f->glDrawArrays(GL_LINE_STRIP, 0, npoints);
      f->glDisableVertexAttribArray(vpositions);
      f->glBindBuffer(GL_ARRAY_BUFFER, 0);
    }

    if (display_ctrl && vpositions != -1) {
      // Use texture for control points
      shader->sendUniformi("use_texture", 1);
      
      glEnable(GL_BLEND);
      glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
      
      f->glBindBuffer(GL_ARRAY_BUFFER, vbo_ctrl);
      f->glEnableVertexAttribArray(vpositions);
      f->glVertexAttribPointer(vpositions, 3, GL_FLOAT, GL_FALSE, 0, (void *) 0);
      f->glDrawArrays(GL_POINTS, 0, spline->GetNumPoints());
      f->glDisableVertexAttribArray(vpositions);
      f->glBindBuffer(GL_ARRAY_BUFFER, 0);
    }

    shader->stop();

    f->glBindVertexArray(0); f->glDeleteVertexArrays(1, &vao);
    glDisable(GL_BLEND);
    glDepthMask(GL_TRUE);
    glEnable(GL_DEPTH_TEST);
  }
  // ============================================================================
  // display                                                                     
  // display ctrl and camera path                                                
  void Camera::display(const int _win_height)
  {
    //GLWindow::m_glWidget->makeCurrent();
    win_height = _win_height;
    if (GLWindow::GLSL_support) {
      if (spline->GetNumPoints() > 0 ) {
        displayVbo();
      }
    } else {
      if (display_path) displayCameraPath();
      if (display_ctrl) {;}
    }
    //GLWindow::m_glWidget->doneCurrent();
  }
  // ============================================================================
  // setSplineParam                                                              
  // set Spline Parameters                                                       
  // npoints : # interpolated points in the spline                               
  // scale   : scale factor applied on top of each control points                
  void Camera::setSplineParam(const int _p, const double _s, const bool ugl)
  { 
    npoints=_p; 
    scale  =_s;
    if (GLWindow::GLSL_support) {
      updateVbo();
    } else {
      buildDisplayList();
    }
    if (ugl) { // updateGL required
      emit updateGL();
    }
  }
  // ============================================================================
  // setCamDisplay                                                               
  // toggle ctrl and path camera display                                         
  void Camera::setCamDisplay(const bool ctrl, const bool path, const bool ugl)
  {
    display_ctrl = ctrl;
    display_path = path;
    if (ugl) { // updateGL required
      emit updateGL();
    }
  }
  // ============================================================================
  // startStopPlay                                                               
  void Camera::startStopPlay(const bool _b)
  {
    play_loop = _b;
    if (!play) {
      play_timer->start(10);
    }
    else {
      play_timer->stop();
    }
    play = !play;
  }
}
