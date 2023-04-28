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
#include <QtGlobal>
//#if QT_VERSION >= QT_VERSION_CHECK(5, 0, 0)
//#include <GL/glew.h>
#include <QtGui>
//#endif
#include <QtOpenGL>
#include <QFile>
#include <QString>

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
    glDeleteLists( dplist_index, 1 );
    if (shader ) {
      delete shader;
    }
  }

  // ============================================================================
  // loadShader
  void Camera::loadShader()
  {
    QOpenGLExtraFunctions *f = QOpenGLContext::currentContext()->extraFunctions();

    if (GLWindow::GLSL_support) {
      f->glGenBuffers(1,&vbo_path);
      f->glGenBuffers(1,&vbo_ctrl);
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
    dplist_index = glGenLists( 1 );    // get a new display list index
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
      gluLookAt(ex, ey, ez,
                cx, cy, cz,
                ux, uy, uz);
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
      gluLookAt(rv.x, rv.y, rv.z,
                cx, cy, cz,
                up.x,up.y,up.z);
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
    glNewList( dplist_index, GL_COMPILE );
    glBegin(GL_LINE_STRIP);
    for (int i=0; i<npoints; i++) {
      float  t=(float)i / (float)npoints;
      Vec3D rv = spline->GetInterpolatedSplinePoint(t)*scale;    
      glVertex3f(rv.x, rv.y, rv.z);  
    }
    glEnd();
    glEndList();
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
    // color
    mycolor = Qt::yellow;
    glColor4ub(mycolor.red(), mycolor.green(), mycolor.blue(),mycolor.alpha());

    glTexEnvi(GL_POINT_SPRITE,GL_COORD_REPLACE,GL_TRUE);

    // ------------------------------
    // start shader
    // ------------------------------

    shader->start();

    // texture
    f->glActiveTexture(GL_TEXTURE0_ARB);
    texture->glBindTexture();  // bind texture

    // send matrix
    GLfloat proj[16];
    f->glGetFloatv( GL_PROJECTION_MATRIX,proj);
    shader->sendUniformXfv("projMatrix",16,1,&proj[0]);
    GLfloat mview[16];
    f->glGetFloatv( GL_MODELVIEW_MATRIX,mview);
    shader->sendUniformXfv("modelviewMatrix",16,1,&mview[0]);

    // Send data to Pixel Shader
    shader->sendUniformi("splatTexture",0);

    // get attribute location for sprite size
    int a_sprite_size = f->glGetAttribLocation(shader->getProgramId(), "a_sprite_size");
    f->glVertexAttrib1f(a_sprite_size,5.0);
    if ( a_sprite_size == -1) {
      std::cerr << "Error occured when getting \"a_sprite_size\" attribute\n";
      exit(1);
    }


    // send vertex positions only
    if (display_path) {
      // setup lines
      glDisable(GL_DEPTH_TEST);
      glEnable (GL_LINE_SMOOTH);
      glEnable (GL_BLEND);
      glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
      glHint (GL_LINE_SMOOTH_HINT, GL_DONT_CARE);
      glLineWidth (4.5);

      f->glBindBuffer(GL_ARRAY_BUFFER_ARB, vbo_path);
      glEnableClientState(GL_VERTEX_ARRAY);
      glVertexPointer((GLint) 3, GL_FLOAT, (GLsizei) 0, (void *) 0);
      glDrawArrays(GL_LINE_STRIP, 0, npoints);
      f->glBindBuffer(GL_ARRAY_BUFFER_ARB, 0);
    }

    if (display_ctrl) {
      // setup point sprites
      glEnable(GL_POINT_SPRITE_ARB);
      glEnable(GL_VERTEX_PROGRAM_POINT_SIZE_NV);
      glEnable(GL_POINT_SMOOTH);

      f->glBindBuffer(GL_ARRAY_BUFFER_ARB, vbo_ctrl);
      glEnableClientState(GL_VERTEX_ARRAY);
      glVertexPointer((GLint) 3, GL_FLOAT, (GLsizei) 0, (void *) 0);
      glDrawArrays(GL_POINTS, 0, spline->GetNumPoints());
      f->glBindBuffer(GL_ARRAY_BUFFER_ARB, 0);
    }

    shader->stop();

    glDisableClientState(GL_VERTEX_ARRAY);
    glDisable(GL_POINT_SPRITE_ARB);
    glDisable(GL_BLEND);
    glDepthMask(GL_TRUE);
    glEnable(GL_DEPTH_TEST);
  }
  // ============================================================================
  // display                                                                     
  // display ctrl and camera path                                                
  void Camera::display(const int _win_height)
  {
    win_height = _win_height;
    if (GLWindow::GLSL_support) {
      if (spline->GetNumPoints() > 0 ) {
        displayVbo();
      }
    } else {
      if (display_path) displayCameraPath();
      if (display_ctrl) {;}
    }
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
