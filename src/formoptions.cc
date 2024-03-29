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
#include "formoptions.h"
#include <QFileDialog>
#include <iomanip>
#include <sstream>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QTextStream>
namespace glnemo {
  int FormOptions::windows_size[13][2] = {
    {2560,1600},{1920,1200},{1920,1080},{1600,1200},
    {1680,1050},{1440,900},{1280,1024},{1280,800},{1280,720},
    {1024,768},{800,600},{640,480},{320,200}
  };
// ============================================================================
// Constructor                                                                 
FormOptions::FormOptions(GlobalOptions * _go, QMutex * _mutex, QWidget *parent):QDialog(parent)
{
  if (parent) {;}  // remove compiler warning
  form.setupUi(this);
  EMIT=true;
  start=false;
  limited_timer = new QTimer(this);
  connect(limited_timer, SIGNAL(timeout()), this, SLOT(stop_bench()));

  // density tab
  go = _go;
  mutex_data = _mutex;
  // default screen resolution for offscreen rendering set to 1280x720
  form.screen_size->setCurrentIndex(8);
  form.frame_name_text->setText(QString(go->base_frame_name));
  // activate the first TAB by default
  form.options_dialog->setCurrentIndex(0);
  form.com->setChecked(go->auto_com);
  form.cod->setChecked(go->cod);
  // player tab
  form.frame_slide->setTracking(true);
  connect(form.frame_slide,SIGNAL(sliderPressed()) ,this,SLOT(lockFrame()));
  connect(form.frame_slide,SIGNAL(sliderReleased()),this,SLOT(unLockFrame()));
  connect(form.frame_dial ,SIGNAL(sliderPressed()) ,this,SLOT(lockFrame()));
  connect(form.frame_dial ,SIGNAL(sliderReleased()),this,SLOT(unLockFrame()));
  // camera tab
  playing_camera=false;
  // load camera path from ressource file
  // camera list from ressource file
  std::string cam_list=GlobalOptions::RESPATH.toStdString() + "/camera/camera.list";
  QFile infile(QString(cam_list.c_str()));
  if (infile.open(QIODevice::ReadOnly| QIODevice::Text)) {

    QTextStream in(&infile);
    QString line;
    do {
      line = in.readLine();
      if (!line.isNull()) {
        // load camera path from ressource file
        std::string cam_path=GlobalOptions::RESPATH.toStdString() + "/camera/"+ line.toStdString();
        QFileInfo ffile(QString(cam_path.c_str()));
        if (ffile.exists()) {
          form.cam_load_select->addItem(ffile.baseName());
        }
      }
    } while (!line.isNull());

    // load camera file in text area
    on_cam_load_select_activated(form.cam_load_select->itemText(0));
  }
  // frame spin box
  form.frame_spin->setKeyboardTracking(false);
  form.frame_spin->setButtonSymbols(QAbstractSpinBox::PlusMinus);
  QString css;
  // ---------- Grid tab
  // Color cube button
  css=QString("background:rgb(%1,%2,%3)").
        arg(go->col_cube.red()).
        arg(go->col_cube.green()).
        arg(go->col_cube.blue());  
  form.cube_color->setStyleSheet(css);
  // Color XY button
  css=QString("background:rgb(%1,%2,%3)").
        arg(go->col_x_grid.red()).
        arg(go->col_x_grid.green()).
        arg(go->col_x_grid.blue());  
  form.xy_grid_color->setStyleSheet(css);
  // Color YZ button
  css=QString("background:rgb(%1,%2,%3)").
        arg(go->col_y_grid.red()).
        arg(go->col_y_grid.green()).
        arg(go->col_y_grid.blue());  
  form.yz_grid_color->setStyleSheet(css);
  // Color XZ button
  css=QString("background:rgb(%1,%2,%3)").
        arg(go->col_z_grid.red()).
        arg(go->col_z_grid.green()).
        arg(go->col_z_grid.blue());  
  form.xz_grid_color->setStyleSheet(css);
  
  // ------------- OSD tab
  // font color
  css=QString("background:rgb(%1,%2,%3)").
        arg(go->osd_color.red()).
        arg(go->osd_color.green()).
        arg(go->osd_color.blue());  
  form.font_color->setStyleSheet(css);
  // background color
  css=QString("background:rgb(%1,%2,%3)").
        arg(go->background_color.red()).
        arg(go->background_color.green()).
        arg(go->background_color.blue());  
  form.background_color->setStyleSheet(css);
  
  // ------------- Colorbar tab
  // font color
  css=QString("background:rgb(%1,%2,%3)").
        arg(go->gcb_color.red()).
        arg(go->gcb_color.green()).
        arg(go->gcb_color.blue());  
  form.gcb_font_color->setStyleSheet(css);
  QDoubleValidator * double_validator= new QDoubleValidator(this);
   //double_validator->setBottom(0.00);
   //double_validator->setDecimals(12);
   //double_validator->setTop(100.00);
  form.gcb_factor->setValidator( double_validator);
  form.gcb_factor->setText("1");

  // ----- auto screnshots tab
  //form.radio_res_standard->clicked(true);
#if QT_VERSION >= QT_VERSION_CHECK(5, 0, 0)
  form.radio_res_standard->clicked(true);
#endif

  update();
}
// ============================================================================
// Destructor                                                                  
FormOptions::~FormOptions()
{
}
// ============================================================================
// update                                                                 
void FormOptions::update()
{
  EMIT=false;

  // Grid tabs
  form.show_grid_checkb->setChecked(go->show_grid);
  form.xy_checkb->setChecked(go->xy_grid);
  form.xz_checkb->setChecked(go->xz_grid);
  form.yz_checkb->setChecked(go->yz_grid);
  
  if (! form.mesh_length_spin->hasFocus())
    form.mesh_length_spin->setValue(go->mesh_length);
  if (! form.mesh_nb_spin->hasFocus() )
    form.mesh_nb_spin->setValue(go->nb_meshs);
  form.cube_checkb->setChecked(go->show_cube);

  // Play tab
  form.cod->setEnabled(go->rho_exist);

  // OSD tabs
  form.show_osd_checkb->setChecked(go->show_osd);
  form.osd_datatype->setChecked(go->osd_data_type);
  form.osd_title->setChecked(go->osd_title);
  form.osd_time->setChecked(go->osd_time);
  form.osd_nbody->setChecked(go->osd_nbody);
  form.osd_trans->setChecked(go->osd_trans);
  form.osd_zoom->setChecked(go->osd_zoom);
  form.osd_rot->setChecked(go->osd_rot);
  form.osd_proj->setChecked(go->osd_projection);
  if (! form.spin_font_size->hasFocus())
    form.spin_font_size->setValue(go->osd_font_size);
  form.title_name->setText(go->osd_title_name);
  if (! form.zoom_dspin->hasFocus())
    form.zoom_dspin->setValue(go->zoom);
  if (! form.rx_dspin->hasFocus())
    form.rx_dspin->setValue(go->xrot);
  if (! form.ry_dspin->hasFocus())
    form.ry_dspin->setValue(go->yrot);
  if (! form.rz_dspin->hasFocus())
    form.rz_dspin->setValue(go->zrot);
  if (! form.tx_dspin->hasFocus())
    form.tx_dspin->setValue(-go->xtrans);
  if (! form.ty_dspin->hasFocus())
    form.ty_dspin->setValue(-go->ytrans);
  if (! form.tz_dspin->hasFocus())
    form.tz_dspin->setValue(-go->ztrans);

  // ColorBar tab
  form.gcb_enable->setChecked(go->gcb_enable);
  if (! form.gcb_height->hasFocus() )
    form.gcb_height->setValue(go->gcb_pheight*100.);
  if (! form.gcb_width->hasFocus())
    form.gcb_width->setValue(go->gcb_pwidth*100.);
  form.gcb_log->setChecked(go->gcb_logmode);
  if (go->gcb_orientation==0) form.gcb_radio_north->setChecked(true);
  if (go->gcb_orientation==1) form.gcb_radio_est->setChecked(true);
  if (go->gcb_orientation==2) form.gcb_radio_south->setChecked(true);
  if (go->gcb_orientation==3) form.gcb_radio_west->setChecked(true);
  if (! form.gcb_spin_digit->hasFocus())
    form.gcb_spin_digit->setValue(go->gcb_ndigits);
  if (! form.gcb_spin_font_size->hasFocus())
    form.gcb_spin_font_size->setValue(go->gcb_font_size);
  if (! form.gcb_spin_offset->hasFocus())
    form.gcb_spin_offset->setValue(go->gcb_offset);

  // rotation/axis tab
  form.show_3daxis->setChecked(go->axes_enable);
  if (go->rotate_screen) 
    form.rot_screen->setChecked(true);
  else
    form.rot_world->setChecked(true);
  
  form.xsrotate->setChecked(go->xbrot);
  form.xsreverse->setChecked(go->ixrot==-1?true:false);
  
  form.ysrotate->setChecked(go->ybrot);
  form.ysreverse->setChecked(go->iyrot==-1?true:false);
  
  form.zsrotate->setChecked(go->zbrot);
  form.zsreverse->setChecked(go->izrot==-1?true:false);
  
  form.uwrotate->setChecked(go->ubrot);
  form.uwreverse->setChecked(go->iurot==-1?true:false);
  
  form.vwrotate->setChecked(go->vbrot);
  form.vwreverse->setChecked(go->ivrot==-1?true:false);
  
  form.wwrotate->setChecked(go->wbrot);
  form.wwreverse->setChecked(go->iwrot==-1?true:false);
  
  // OpenGL tab
  if (go->perspective) 
    form.radio_persp->setChecked(true);
  else
    form.radio_ortho->setChecked(true);
  
  // opaque disc
  form.cb_opaque_disc->setChecked(go->od_enable);
  if (! form.od_radius_spin->hasFocus())
    form.od_radius_spin->setValue(go->od_radius);
  form.cb_coronograph->setChecked(go->od_display);

  EMIT=true;
}
// ============================================================================
// updateFrame                                                                 
void FormOptions::updateFrame(const int frame, const int tot)
{
  form.fps_lcd->display(1000*frame/time.elapsed());
  form.total_lcd->display(tot);
  time.restart();
}
// ============================================================================
// updateFrame                                                                 
void FormOptions::on_bench_button_pressed()
{
  if ( ! start ) {
   if (form.limit_radio->isChecked())            // ask for limited time
     limited_timer->start((int)form.time_box->value()*1000);// limited timer start
  form.time_choice->setDisabled(true);
  form.fps_lcd->display(0);
  form.total_lcd->display(0);
  form.status_label->setText("Running");
  form.bench_button->setText("Stop");
  time.restart();
  emit start_bench(true);
  start = true;
 }
 else {
  stop_bench();
  start = false;
 }
  
}
// ============================================================================
// stop_bench                                                                  
void FormOptions::stop_bench()
{
  form.time_choice->setDisabled(false);
  form.status_label->setText("Not Running");
  form.bench_button->setText("Start");
  start=false;
  limited_timer->stop();
  emit start_bench(false);
}
// ============================================================================
// TAB INTERACTIVE SELECT OPTIONS                                              
// ============================================================================

// ============================================================================
// updateParticlesSelect                                                       
void FormOptions::updateParticlesSelect(const int nbody)
{
  form.nsel_edit->setText(QString("%1").arg(nbody));
}
// ============================================================================
// TAB CAMERA OPTIONS                                                          
// ============================================================================
void FormOptions::on_cam_play_pressed()
{
  //static bool playing=false;
  if (playing_camera) {
    form.cam_play->setText("Start");
  } 
  else {
    form.cam_play->setText("Stop");
  }
  emit startStopPlay(form.loop_check_box->isChecked());
  playing_camera = !playing_camera;
}
// ============================================================================
void FormOptions::on_cam_reset_pressed()
{
  emit cam_reset();
  form.cam_play->setText("Start");
  playing_camera=false;
  form.view_off_radio->setChecked(true);
  //form.view_on_radio->clicked(false);
  emit setCamDisplay(form.cam_pts_display->isChecked(),
                     form.cam_path_display->isChecked());
  emit update_gl();
}
// ============================================================================
// when commit button is pressed, text is placed in a temporary file
// and load as a camera path
void FormOptions::on_cam_commit_button_pressed()
{
  tmp_cam_file.remove();
  if (tmp_cam_file.open()) {

  }
  QTextStream out(&tmp_cam_file);
  out << form.cam_text_edit->toPlainText();
  tmp_cam_file.close();
  emit loadCameraPath(tmp_cam_file.fileName().toStdString(),form.spline_points->value(),(float) form.spline_scale->value());
  emit update_gl();
}
// ============================================================================
void FormOptions::on_cam_load_button_pressed()
{
  QString fileName;
  fileName=QFileDialog::getOpenFileName(this,tr("Select a camera path"));
  if (!fileName.isEmpty()) {

    emit loadCameraPath(fileName.toStdString(),form.spline_points->value(),(float) form.spline_scale->value());

    QFile infile(fileName);
    if (infile.open(QIODevice::ReadOnly | QIODevice::Text)) {
      QTextStream in(&infile);
      QString line;
      bool valid=false;
      line = in.readLine();
      if (line=="#camera_path") { // it's a valid camera path file
        infile.close();
        displayCameraFile(fileName);
      }
    }
  }
}
// ============================================================================
// save camera file edited
void FormOptions::on_cam_save_button_pressed()
{
  QString fileName= QFileDialog::getSaveFileName(this, tr("Save File"),"");
  if (!fileName.isEmpty()) {
    QFile file(fileName);
    if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
      QTextStream out(&file);
      out << form.cam_text_edit->toPlainText();
      file.close();
    }
  }
}
// ============================================================================
void FormOptions::on_cam_load_select_activated(const QString &text)
{
  QString select=text;
  if (select[0]!=QChar('/')) { // path from ressource file only
    select = GlobalOptions::RESPATH + "/camera/"+text;
  }
  //std::cerr << "camera path selected = "<<select.toStdString() << "\n";
  if (displayCameraFile(select)) {
    emit loadCameraPath(select.toStdString(),form.spline_points->value(),(float) form.spline_scale->value());
  }
}
// ============================================================================
bool FormOptions::displayCameraFile(const QString &infile)
{
  bool status=true;
  QFile file(infile);  //
  if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
    status=false;
  } else {
    QTextStream in(&file);

    QString mytext=in.readAll();
    form.cam_text_edit->clear(); // clear text
    form.cam_text_edit->insertPlainText(mytext); // display camera path
  }
  return status;
}

// ============================================================================
// TAB PLAY OPTIONS                                                            
// ============================================================================
void FormOptions::on_frame_name_pressed()
{
  QString fileName = QFileDialog::getSaveFileName(this,tr("Select Frame directory"),go->base_frame_name);
  if (!fileName.isEmpty()) {
    go->base_frame_name = fileName;
    form.frame_name_text->setText(QString(fileName));
  }
}
// ============================================================================
void FormOptions::play_pressed2(const int forcestop)
{
  static bool play=false;
  switch (forcestop) {
    case    0: play=true ; break;
    case    1: play=false; break;
    default  : play = ! play;
      emit playPressed();
      break;
  }

  if (play) {
    form.play->setText("STOP");
  } else {
    form.play->setText("PLAY");
  }
 // if (!forcestop)
 //   emit playPressed();
}
// ============================================================================
void FormOptions::on_screen_size_activated(int index)
{
  go->frame_width  = windows_size[index][0];
  go->frame_height = windows_size[index][1];
}
// ============================================================================
// TAB GRIDS and CUBE OPTIONS                                                            
// ============================================================================
}
