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
#include "glwindow.h"
#include "formobjectcontrol.h"
#include <iostream>
#include <sstream>
#include <QTableWidgetItem>
#include <QColorDialog>
#include <QRegExp>
#include <assert.h>
#include <QFileDialog>
#include <QMessageBox>
#include "globaloptions.h"
#include "gltexture.h"

namespace glnemo {
#define RT_VISIB 0
#define RT_COLOR 2
#define ST_VISIB 0
#define ST_COLOR 1
#define FT_VISIB 0
#define FT_COLOR 3

int last_row=0;
bool EMIT=true;  // EMIT only signal from user requests

// Object Start Table
int osrange=0;
int osselec=0;
int osfile =0;
// ============================================================================
// Constructor                                                                 
// create the 2 tables : Range and File                                        
FormObjectControl::FormObjectControl(CPointsetManager *_pointset_manager, GlobalOptions *global_options,
                                     QWidget *parent) : QDialog(parent)
{
  pointset_manager = _pointset_manager;
  go = global_options;
  ignoreCloseEvent = true;
  form.setupUi(this);
  current_data = NULL;
  // set number of rows per table (20)
  form.range_table->setRowCount(20);
  osrange=0;

  osselec=osrange+form.range_table->rowCount();

  form.range_table->setColumnWidth(0,25);   // Vis
  form.range_table->setColumnWidth(1,150);  // Range
  form.range_table->setColumnWidth(2,45);   // Color
  for (int i=0; i < form.range_table->rowCount(); i++) {
    form.range_table->setRowHeight(i,25);
  }
  // create object index array
  int no=form.range_table->rowCount();
  object_index = new int[no];
  for (int i=0; i<no;i++) object_index[i]=-1; // -1=>object does not exist
  current_object = 0;
  pov = NULL;
  first=true;
  lock = true;
  // insert checkbox and color widget into tables
  initTableWidget(form.range_table ,0,RT_VISIB,RT_COLOR);  // range
  // Selection mode and behaviour
  form.range_table->setSelectionMode(QAbstractItemView::SingleSelection);
  form.range_table->setSelectionBehavior(QAbstractItemView::SelectItems);
  // create range selection combobox to store objects' particles indexes.
  combobox = new QComboBoxTable(0,0,0);
  form.range_table->setCellWidget(0,RT_COLOR-1,combobox);
  // add default object range
  combobox->addItem("");
  combobox->addItem("0:99999");  // add default range 1
  combobox->addItem("0:199999"); // add default range 2
  connect(combobox,SIGNAL(comboActivated(const int, const int)),this,
          SLOT(checkComboLine(const int, const int)));
  // intialyze texture combobox according to the texture array
  // loop and load all embeded textures
  int i=0;
  while (GLTexture::TEXTURE[i][0]!=NULL) {
    form.texture_box->addItem(QIcon(GlobalOptions::RESPATH+GLTexture::TEXTURE[i][0]),GLTexture::TEXTURE[i][1]);
    std::cerr << "texture i="<<i<<" = " << GLTexture::TEXTURE[i][1].toStdString() << "\n";
    i++;
  }
  // check GLSL_support
  if (! GLWindow::GLSL_support) {
    form.gaz_glsl_check->setChecked(false);
    form.gaz_glsl_check->setDisabled(true);
  } else {
    form.gaz_glsl_check->setChecked(true);
  }

  //connect(combobox,SIGNAL(my_editTextChanged(const QString&, const int, const int)),
  //      this,SLOT(updateRange(const QString&, const int, const int)));
  //form.dens_histo_view->setParent(form.tab_density);
  dens_histo = new DensityHisto(form.dens_histo_view);
  form.dens_histo_view->setScene(dens_histo);
  form.dens_histo_view->repaint();
  //DEACTIVARED form.dens_glob_box->setDisabled(true);
  dens_color_bar = new DensityColorBar(go,form.dens_bar_view);
  form.dens_bar_view->setScene(dens_color_bar);

  form.objects_properties->setTabEnabled(1,false);
  form.objects_properties->setCurrentIndex(0); // set position to first tab

  my_mutex2 = new QMutex(QMutex::Recursive);

  auto custom_delete_cpointset_btn = new DeletePushButton();
  custom_delete_cpointset_btn->setObjectName(form.delete_cpointset_btn->objectName());
  custom_delete_cpointset_btn->setToolTip(form.delete_cpointset_btn->toolTip());
  custom_delete_cpointset_btn->setText(form.delete_cpointset_btn->text());
  form.delete_cpointset_layout->replaceWidget(form.delete_cpointset_btn, custom_delete_cpointset_btn);
  connect(custom_delete_cpointset_btn, SIGNAL(deleteClicked(bool)), this, SLOT(delete_cpointsets(bool)));

  auto custom_delete_cpoints_btn = new DeletePushButton();
  custom_delete_cpoints_btn->setObjectName(form.delete_cpoints_btn->objectName());
  custom_delete_cpoints_btn->setToolTip(form.delete_cpoints_btn->toolTip());
  custom_delete_cpoints_btn->setText(form.delete_cpoints_btn->text());
  form.delete_cpoints_layout->replaceWidget(form.delete_cpoints_btn, custom_delete_cpoints_btn);
  connect(custom_delete_cpoints_btn, SIGNAL(deleteClicked(bool)), this, SLOT(delete_cpoints(bool)));

  delete form.delete_cpoints_btn;
  delete form.delete_cpointset_btn;

  form.cpoints_set_treewidget->setSelectionMode(QAbstractItemView::SelectionMode::ExtendedSelection);
  form.cpoints_set_treewidget->setColumnHidden(1, true);

//  form.color_picker_button->setStyleSheet("QPushButton:disabled{background-color:#eeeeee}"); // FIXME does not work
}

// ============================================================================
// Destructor                                                                  
FormObjectControl::~FormObjectControl()
{
  delete [] object_index;
}
// ============================================================================
// initTableWidget()                                                           
// Initialize Range and File table                                             
void FormObjectControl::initTableWidget(QTableWidget * table, const int i_table,
                                        const int c_visib, const int c_col)
{
  for (int i=0; i<table->rowCount(); i++) {
#if 0
    ParticlesObject * po = new ParticlesObject();
    povrange.push_back(*po);
    delete po;
#endif
    // set checkbox widget
    QCheckBoxTable * checkbox = new QCheckBoxTable(i,i_table,this);
    assert(checkbox);
    if ( ! i_table)
      connect(checkbox,SIGNAL(changeVisib(const bool,const int ,const int )),
	      this,SLOT(updateVisib(const bool ,const int ,const int )));
    else
      connect(checkbox,SIGNAL(changeVisib(const bool,const int ,const int )),
	      this,SLOT(updateVisib(const bool ,const int ,const int )));
    checkbox->setChecked(false);
    table->setCellWidget(i,c_visib,checkbox);
    // set Color
    QTableWidgetItem  * cellcol=new QTableWidgetItem();
    cellcol->setBackground(Qt::white);
    table->setItem(i,c_col,cellcol);
    // set particles info
    QTableWidgetItem  * partcol=new QTableWidgetItem();
    partcol->setText(""); // put a blank text
    table->setItem(i,c_col-1,partcol);
  }
}
// ============================================================================
// resetTableWidget()                                                          
// RAZ Range and File table                                                    
void FormObjectControl::resetTableWidget(QTableWidget * table, const int i_table,
                                         const int c_col, const int row)
{
  // get item color
  QTableWidgetItem * cellcol = table->item(row,c_col);
  assert(cellcol);
  // set color
  cellcol->setBackground(Qt::white);
  //!!table->setItem(row,c_col,cellcol);
  // get visibility box
  QCheckBoxTable * checkbox = static_cast<QCheckBoxTable *>
  (table->cellWidget(row,i_table));
  assert(checkbox);
  // set visibility
  checkbox->setChecked(false);
  // set particles info
  QTableWidgetItem  * partrange=table->item(row,c_col-1);
  assert(partrange);
  partrange->setText(""); // put a blank text
  //!!table->setItem(row,c_col-1,partrange);
}
// ============================================================================
// update()                                                                    
// upate table with new objects                                                
void FormObjectControl::update(ParticlesData   * _p_data,
                               ParticlesObjectVector * _pov,
                               GlobalOptions         * _go,
			       bool                    reset_table)
{

  go           = _go;
  dens_color_bar->setGo(go);
  form.dynamic_cmap->setChecked(go->dynamic_cmap);
  if (go  && ! go->duplicate_mem) mutex_data->lock();
  lock=false;
  current_data = _p_data;
  nbody        = *(current_data->nbody);
  pov          = _pov;
  // get physical value data array
  phys_select = current_data->getPhysData();

  // parse all the objects to check if physic is present
  checkPhysic();

  int cpt=0;
  combobox->clear();
  for (int i=0; i < form.range_table->rowCount(); i++) {
    if (i < (int) pov->size()) { // remains objects
      object_index[i]=cpt++;
      QTableWidget * tw;
      if ((*pov)[i].selectFrom() == ParticlesObject::Range) { // Range
        tw = form.range_table;
        updateTable(tw,i,RT_VISIB ,RT_COLOR);
        updateRangeTable(i);
      }
#if 0
      else {                                                  // File
        tw = form.range_table;
        updateTable(tw,i,FT_VISIB,FT_COLOR);
        updateFileTable(i);
      }
#endif
      //updateObjectSettings(i);
    }
    else {               // not belonging to object list
      if (reset_table) {
	object_index[i]=-1;
	resetTableWidget(form.range_table,RT_VISIB,RT_COLOR,i);
      } else {
	//std::cerr << "combobox = " << (combobox->currentText()).toStdString() << "\n";
      }

    }
    //if (i) form.range_table->setCellWidget(i,1,NULL);

  }
  first=false;
  ///!!!!updateObjectSettings(0);
  ///MODIFICATION April,15 2011
  updateObjectSettings(last_row);
  EMIT=false;
  // stretch tab
  form.z_stretch_jit_cb->setChecked(go->z_stretch_jit);
  form.z_stretch_max_spin->setValue((double) go->z_stretch_max);
  form.z_stretch_slide->setValue(form.z_stretch_slide->maximum());
  EMIT=true;
  ///MODIFICATION April,15 2011
  form.range_table->setCurrentCell(current_object,1);
  // set active row
  lock=true;
  if (go  && ! go->duplicate_mem) mutex_data->unlock();

  on_range_table_cellClicked(last_row,1);
  //physicalSelected();
}
// ============================================================================
// updateTable()                                                               
void FormObjectControl::updateTable(QTableWidget * table, const int row,
                                        const int c_visib, const int c_col)
{
  //if (lock)
  if (go  && ! go->duplicate_mem) mutex_data->lock();
  int i_obj=object_index[row];      // object's index
  assert(i_obj < (int)pov->size());
  // update visible box
  QCheckBoxTable * checkbox = static_cast<QCheckBoxTable *>(table->cellWidget(row,c_visib));
  assert(checkbox);
  checkbox->setChecked((*pov)[i_obj].isVisible());
  // update color
  QTableWidgetItem * twi = table->item(row,c_col);
  assert(twi);
  twi->setBackground(QBrush((*pov)[i_obj].getColor())); // !!!!!!!
  //(*pov)[i_obj].setColor(twi->background().color());      // !!!!!!!
  //if (lock)
  if (go  && ! go->duplicate_mem) mutex_data->unlock();
}
// ============================================================================
// updateRangeTable()                                                          
void FormObjectControl::updateRangeTable(const int row)
{
  //if (lock)
  if (go  && ! go->duplicate_mem) mutex_data->lock();
  int i_obj=object_index[row];      // object's index
  assert(i_obj < (int)pov->size());
  QString text;
  if ((*pov)[i_obj].step == 1)
    text = QString("%1:%2"   ).arg((*pov)[i_obj].first,0,10).arg((*pov)[i_obj].last,0,10);
  else
    text = QString("%1:%2:%3").arg((*pov)[i_obj].first,0,10)
	                      .arg((*pov)[i_obj].last ,0,10)
	          	      .arg((*pov)[i_obj].step ,0,10);
  QTableWidgetItem * twi = form.range_table->item(row,1);
  assert(twi);
  twi->setText(text);     // put the object range corresponding to the object to the TABLE
  combobox->addItem(text);// add the object range in the combobox
  //if (lock)
  if (go  && ! go->duplicate_mem) mutex_data->unlock();
}
// ============================================================================
// updateFileTable()                                                           
void FormObjectControl::updateFileTable(const int row)
{
  if (row) {;}
}
// ============================================================================
// on_range_table_cellClicked()                                                
// process the Clicked event                                                   
int FormObjectControl::on_range_table_cellClicked(int row,int column)
{
  if (! pov) return 0;
  //if (lock)
  if (go  && ! go->duplicate_mem)
    mutex_data->lock();
  lock=false;

  bool valid=false;
  last_row = row; // save the current row clicked
  int i_obj= object_index[row]; // object's index
  //assert(i_obj < (int)pov->size());
  // -
  // --- process click event on [combo box cell]
  // -
  int rcolumn=1;
  // get the combobox from the active cell
  QComboBoxTable * my_combo = static_cast<QComboBoxTable *>
                        (form.range_table->cellWidget(row,rcolumn));

  // get back the text line from cell
  QTableWidgetItem *  item = (form.range_table->item(row,rcolumn)); // check item in range column !!
  assert(item);
  QString text="none";
  text=item->text();

  if ( ! my_combo ) { // no combobox yet from the cell
    if (form.range_table->cellWidget(combobox->row,rcolumn)) {
      QComboBoxTable * combobox1 = new QComboBoxTable(row,rcolumn,this); // new combo
      *combobox1 = *combobox;                                    // copy old to new
      form.range_table->removeCellWidget(combobox->row,rcolumn); // remove old
      delete combobox;
      combobox   = combobox1;                                    // link old to new
                                                                 // establish connection
      connect(combobox,
              SIGNAL(comboActivated(
                                   const int, const int)),this,
              SLOT(checkComboLine(const int, const int)));
    }
    combobox->row = row;                                         // adjust row
    combobox->display();
    combobox->setEditText(item->text());                         // add object to the combobox
    form.range_table->setCellWidget(row,rcolumn,combobox);       // put the combobox to the current cell
  }
  else {              // widget exist
    assert(combobox->row == row);
//    combobox->setEditText(item->text()); // add object to the combobox
//    combobox->display();
#if 0
    combobox->setEditText(item->text()); // add object to the combobox
    combobox->display();
#else
    if (object_index[row] != -1 )          // object exist
      combobox->setEditText(item->text()); // put range from table
    combobox->display();                   // display combobox
#endif
  }

  current_object = row;
  updateObjectSettings(row);   // Update object properties on the form

  // -
  // --- process click event on [Color cell]
  // -
  if (column == RT_COLOR) {
    form.range_table->setCurrentCell(row,column-1);
    // update color
    QTableWidgetItem * twi = form.range_table->item(row,column);
    assert(twi);
    QColor color=QColorDialog::getColor(twi->background().color());
    if (color.isValid() && pov && i_obj != -1) {
      valid=true;
      twi->setBackground(QBrush(color));
      ParticlesObject * pobj = &(*pov)[i_obj];
      pobj->setColor(color);
      emit gazColorObjectChanged(i_obj);
    }
  }
  if (column == RT_VISIB) {

  }
  current_object = row;

  if (valid) {
    if (i_obj < (int)pov->size()) {
      //emit gazAlphaObjectChanged(i_obj);
    }
    emit objectSettingsChanged();
  }
  lock=true;
  //if (lock)
  if (go  && ! go->duplicate_mem) mutex_data->unlock();
  return 1;
}
// ============================================================================
// checkComboLine()                                                            
// parse the range entered in the combobox. Create a new object if valid       
void FormObjectControl::checkComboLine(const int row, const int col)
{
  //if (lock)
  if (go  &&  ! go->duplicate_mem) mutex_data->lock();
  lock=false;
  QTableWidgetItem *  item = (form.range_table->item(row,col));
  assert(item);
  if (item) {
    //construct regexp
    QRegExp rx("^(\\d{1,})((:)(\\d{1,})){,1}((:)(\\d{1,})){,1}$");
    int match=rx.indexIn(combobox->currentText());
    if (match == -1) { // not match
    }
    else {
      int first,last,step=1;
      // get first
      std::istringstream iss((rx.cap(1)).toStdString());
      iss >> first;
      // get last
      if (rx.captureCount()>4) {
        std::istringstream  iss((rx.cap(4)).toStdString());
        iss >> last;
        // get step
        if (rx.captureCount()>=7) {
          std::istringstream  iss((rx.cap(7)).toStdString());
          iss >> step;
        }
      }
      else {
      last=first;
      }
#if 0
      std::cerr << "whole string =["<<(rx.cap(0)).toStdString()<<"\n";
      for (int i=0;i<=rx.captureCount();i++) {
        std::cerr << "cap ="<<(rx.cap(i)).toStdString()<<"\n";
      }
      std::cerr << "ncap="<<rx.captureCount()<<" first="<<first<<" last="<<last<<" step="<<step<<"\n";
#endif
      // check if the syntax is correct
      int npart=last-first+1; // #part
//      if (current_data && npart <= *(current_data->nbody) && npart>0) {    // valid object
      if (pov && npart <= nbody && npart>0) {    // valid object
        item->setText(combobox->currentText()); // fill cell
        int i_obj= object_index[row];      // object's index

        if ( i_obj != -1) { // it's an existing object
          assert(i_obj < (int)pov->size());
          ParticlesObject * pobj = &(*pov)[i_obj];
          if (! pobj->hasIndexesList()) { // only range allowed
              pobj->npart = npart;
              pobj->first = first;
              pobj->last  = last;
              pobj->step  = step;
              pobj->buildIndexList();   // build object index list
              current_object = row;
              // parse all the objects to check if physic is present
              //checkPhysic(); // uncomment this to activate physic rendering (2/sep/2011)
              updateObjectSettings(row);// update form !!! CAUSE OF CRASH
              emit objectUpdate();      // update OpenGL
          }
        }
        else {              // it's a new object
          ParticlesObject * po = new ParticlesObject(); // new object
          po->buildIndexList( npart,first,last,step);   // object's particles indexes
          // get color
          QTableWidgetItem * twi = form.range_table->item(row,RT_COLOR);
          assert(twi);
          // set color
          po->setColor(twi->background().color());
          // get visibility
          QCheckBoxTable * checkbox = static_cast<QCheckBoxTable *>
          (form.range_table->cellWidget(row,RT_VISIB));
          assert(checkbox);
          // set visibility
          po->setVisible(checkbox->isChecked());
          pov->push_back(*po);                    // insert object
          delete po;
          object_index[row] = pov->size()-1; // update object list
          current_object = row;              //
          updateObjectSettings(row);         // update form !!! CAUSE OF CRASH
          emit objectUpdate();               // update OpenGL object
        }
      }
      else {   // invalid object
      }
      // check if the syntax is correct
    }
  }
  else {
    std::cerr << "WARNING [checkComboLine] no item !!!!\n";
  }
  lock=true;
  //if (lock)
  if (go  && ! go->duplicate_mem) mutex_data->unlock();
}
// ============================================================================
// on_coon_commit_button_clicked()                                             
void FormObjectControl::on_range_table_cellPressed(int row,int column)
{
    if (row && column) {;}
//  std::cerr << "cell cellPressed : " << row << " X " << column << "\n";
}
// ============================================================================
// on_coon_commit_button_clicked()                                             
void FormObjectControl::on_range_table_itemClicked(QTableWidgetItem * item)
{
    if (item) {;}
//  std::cerr << "item Clicked : ";
}
// ============================================================================
// updateVisib()                                                               
void FormObjectControl::updateVisib(const bool visib,const int row,const int table)
{
  if (table) {;}
  //if (lock)
  if (go  && ! go->duplicate_mem) mutex_data->lock();
  int i_obj= object_index[row]; // object's index
  if (pov && i_obj != -1) {
    assert(i_obj < (int)pov->size());
    ParticlesObject * pobj = &(*pov)[i_obj];
    pobj->setVisible(visib);
    emit objectSettingsChanged();
  }
  //if (lock)
  if (go  && ! go->duplicate_mem) mutex_data->unlock();
}
// ============================================================================
// updateRange()                                                               
void FormObjectControl::updateRange(const QString& text, const int row, const int table)
{
  if (text=="") {;}
  if (table) {;}
  //if (lock)
  if (go  && ! go->duplicate_mem) mutex_data->lock();
  int i_obj= object_index[row]; // object's index
  if (pov && i_obj != -1) {
    assert(i_obj < (int)pov->size());
  }
  //if (lock)
  if (go  && ! go->duplicate_mem) mutex_data->unlock();
}
// ============================================================================
// updateObjectSettings()                                                      
// update all the Object's FORM properties(slides, buttons etc..) 
// from object's settings 
void FormObjectControl::updateObjectSettings( const int row)
{
  //if (go  && ! go->duplicate_mem) mutex_data->lock();
  my_mutex.lock();
  EMIT = false;
  int i_obj= object_index[row]; // object's index
  if (pov && i_obj != -1) {
    assert(i_obj < (int)pov->size());
    ParticlesObject * pobj = &(*pov)[i_obj];
    // Particles Settings
    form.part_check->setChecked(pobj->isPartEnable());
    form.part_slide_size->setValue((int) (pobj->getPartSize()*form.part_slide_size->maximum()/
                                   GlobalOptions::MAX_PARTICLES_SIZE));
    form.part_slide_alpha->setValue(pobj->getPartAlpha());
    // Gaz Settings
    form.gaz_check->setChecked(pobj->isGazEnable());
    form.gaz_slide_size->setValue((int) (pobj->getGazSize()*form.gaz_slide_size->maximum()/
                                   pobj->getGazSizeMax()));
    form.gaz_slide_alpha->setValue(pobj->getGazAlpha());
    form.texture_spin->setValue(pobj->getGazSizeMax());
    form.gaz_rot_check->setChecked(pobj->isGazRotate());
    if (GLWindow::GLSL_support)
      form.gaz_glsl_check->setChecked(pobj->isGazGlsl());
    form.texture_box->setCurrentIndex(pobj->getTextureIndex());
    // Velocity Settings
    form.vel_check->setChecked(pobj->isVelEnable());
    int x=(float) pobj->getVelSize() * (float)
                     form.vel_slide_size->maximum()/pobj->getVelSizeMax();
    form.vel_slide_size->setValue(x);
    form.vel_slide_alpha->setValue(pobj->getVelAlpha());
    form.vel_spin->setValue((pobj->getVelSizeMax()));
    // -- Orbits TAB
    form.odisplay_check->setChecked(pobj->isOrbitsEnable());
    form.orecord_check->setChecked(pobj->isOrbitsRecording());
    form.orbit_history_spin->setValue(pobj->getOrbitsHistory());
    form.orbit_max_spin->setValue(pobj->getOrbitsMax());
    // -- Physical quantity TAB
    if (!pobj->hasPhysic() ) {
      form.objects_properties->setTabEnabled(1,false); // disable physical tab
    } else {
      form.objects_properties->setTabEnabled(1,true);  // enable  physical tab
      form.dens_phys_radio->setEnabled    (current_data->rho!=NULL     ?true:false);
      form.temp_phys_radio->setEnabled    (current_data->temp!=NULL    ?true:false);
      form.tempdens_phys_radio->setEnabled(current_data->temp!=NULL    ?true:false);
      form.pressure_phys_radio->setEnabled(current_data->pressure!=NULL?true:false);
      form.velnorm_phys_radio->setEnabled(current_data->vel_norm!=NULL?true:false);
      setPhysicalTabName();
    }
    if (pobj->hasPhysic() && phys_select && phys_select->isValid()) {
      //std::cerr << "updateObjectSettings("<<row<<") getMax="<<log(phys_select->getMax())<< " getMin="<<
      //      log(phys_select->getMin())<<"\n";
      dens_histo->drawDensity(phys_select->data_histo);
      double diff_rho=(log(phys_select->getMax())-log(phys_select->getMin()))/100.;
      //min
      double minphys=pobj->getMinPhys(); // object's max phys value
      double maxphys=pobj->getMaxPhys(); // object's min phys value
      double gminphys=minphys,
             gmaxphys=maxphys;
      //std::cerr << "minphys ="<<minphys << " maxphys="<<maxphys<<"\n";
      if (go->phys_min_glob !=-1 && go->phys_max_glob !=-1) {
          // physical min/max values has been specified from the
          // command line, or user has interactively modified min/max
          gminphys = go->phys_min_glob;
          gmaxphys = go->phys_max_glob;
      }
      // compute min/max value for the object according to
      // the global value if it has been defined
      //min
      int min=rint((log(gminphys)-log(phys_select->getMin()))*1./diff_rho);
      form.dens_slide_min->setValue(min);
      //max
      int max=rint((log(gmaxphys)-log(phys_select->getMin()))*1./diff_rho);
      form.dens_slide_max->setValue(max);
      //pobj->setMaxPercenPhys(std::max(1,max-1));
#if 1
      setNewPhys(false);
      go->gcb_min = form.dens_slide_min->value();
      go->gcb_max = form.dens_slide_max->value();
#endif
      dens_histo->drawDensity(form.dens_slide_min->value(),form.dens_slide_max->value());
      dens_color_bar->draw(form.dens_slide_min->value(),form.dens_slide_max->value());
      //pobj->setMinPhys(minphys);
      //pobj->setMaxPhys(maxphys);
#if 0 // July 2012
      go->phys_min_glob = minphys;
      go->phys_max_glob = maxphys;
#else
      // there is no physical values set on the CL
      // then we set min/max glob from the first frame
      if (go->phys_min_glob ==-1 && go->phys_max_glob ==-1) {
        go->phys_min_glob = pobj->getMinPhys();
        go->phys_max_glob = pobj->getMaxPhys();
      }
#endif
    }
  }
  if ( i_obj == -1 ) { // no object selected
    // Particles Settings
    form.part_check->setChecked(true);
    form.part_slide_size->setValue(form.part_slide_size->maximum());
    form.part_slide_alpha->setValue(254);
    // Gaz Settings
    form.gaz_check->setChecked(false);
    form.gaz_slide_size->setValue(form.gaz_slide_size->maximum());
    form.gaz_slide_alpha->setValue(254);
    form.gaz_rot_check->setChecked(true);
    form.texture_spin->setValue(1.0);
    // Velocity Settings
    form.vel_check->setChecked(false);
    form.vel_slide_size->setValue(form.vel_slide_size->maximum());
    form.vel_slide_alpha->setValue(254);
    form.vel_spin->setValue(4);
    // -- Orbits TAB
    form.odisplay_check->setChecked(false);
    form.orecord_check->setChecked(false);
    form.orbit_history_spin->setValue(form.orbit_history_spin->maximum());
    form.orbit_max_spin->setValue(form.orbit_max_spin->maximum());

  }
  EMIT = true;
  //if (go  && ! go->duplicate_mem) mutex_data->unlock();
  my_mutex.unlock();
}
// ============================================================================
// checkPhysic()                                                                
void FormObjectControl::checkPhysic()
{
  if (pov && pov->size()>0) {
    for (int i=0; i<(int)pov->size();i++) {
      ParticlesObject * pobj = &(*pov)[i];
      pobj->setPhysic(false);
      // check if physic exist and set it
      for (int i=0; i < pobj->npart; i+=pobj->step) {
        int index=pobj->index_tab[i];
        if (phys_select && phys_select->isValid()) {
          if (phys_select->data[index] != -1) pobj->setPhysic(true);
        }
      }
      //
      if (pobj->hasPhysic() && phys_select && phys_select->isValid()) {
        if (go->phys_min_glob==-1 && go->phys_max_glob==-1) { // glob phys not defined
          if (pobj->getMinPhys()==-1. &&  // default parameter -1 -1
              pobj->getMaxPhys()==-1.) {  // it's a NEW object, so we set min/max phys
            pobj->setMinPhys(phys_select->getMin());
            pobj->setMaxPhys(phys_select->getMax());
          }
        } else { // global phys defined
          if (pobj->getMinPhys()==-1. &&            // default parameter for the object
              pobj->getMaxPhys()==-1.) {  //
            pobj->setMinPhys(go->phys_min_glob);
            pobj->setMaxPhys(go->phys_max_glob);
          }
        }
#if 0 // july 2012
        //min
        float minphys=pobj->getMinPhys();
        float maxphys=pobj->getMaxPhys();
        go->phys_min_glob = minphys;
        go->phys_max_glob = maxphys;
#endif
      }
    }
  }
}
// ============================================================================
// ON PARTICLES                                                                
// ============================================================================

// ============================================================================
// on_part_check_clicked()                                                     
void FormObjectControl::on_part_check_clicked(bool value)
{
  //if (lock)
  if (go  && ! go->duplicate_mem) mutex_data->lock();
  int i_obj = object_index[current_object];
  if (pov && pov->size()>0 && i_obj != -1) {  // at least one object
    assert(i_obj < (int)pov->size());
    ParticlesObject * pobj = &(*pov)[i_obj];
    pobj->setPart(value);
    if (EMIT) emit objectSettingsChanged();
  }
  //if (lock)
  if (go  && ! go->duplicate_mem) mutex_data->unlock();
}
// ============================================================================
// on_part_slide_size_valueChanged                                             
void FormObjectControl::on_part_slide_size_valueChanged(int value)
{
  //if (lock)
  if (go  && ! go->duplicate_mem) mutex_data->lock();
  int i_obj = object_index[current_object];
  if (pov && pov->size()>0 && i_obj != -1) {  // at least one object
    assert(i_obj < (int)pov->size());
    ParticlesObject * pobj = &(*pov)[i_obj];
    pobj->setPartSize((float)value*GlobalOptions::MAX_PARTICLES_SIZE/
                             form.part_slide_size->maximum());
    //std::cerr << "part value = " << value << "\n";
    if (EMIT) emit objectSettingsChanged();
  }
  //if (lock)
  if (go  && ! go->duplicate_mem) mutex_data->unlock();
}
// ============================================================================
// on_part_slide_alpha_valueChanged                                            
void FormObjectControl::on_part_slide_alpha_valueChanged(int value)
{
  //if (lock)
  if (go  && ! go->duplicate_mem) mutex_data->lock();
  int i_obj = object_index[current_object];
  if (pov && pov->size()>0 && i_obj != -1 ) {  // at least one object
    assert(i_obj < (int)pov->size());
    ParticlesObject * pobj = &(*pov)[i_obj];
    pobj->setPartAlpha(value);
    if (EMIT) emit objectSettingsChanged();
  }
  //if (lock)
  if (go  && ! go->duplicate_mem) mutex_data->unlock();
}
// ============================================================================
// ON GAZ                                                                      
// ============================================================================

// ============================================================================
// on_gaz_check_clicked()
void FormObjectControl::on_gaz_check_clicked(bool value)
{
  //if (lock)
  if (go  && ! go->duplicate_mem) mutex_data->lock();
  int i_obj = object_index[current_object];
  if (pov && pov->size()>0 && i_obj != -1 ) {  // at least one object
    assert(i_obj < (int)pov->size());
    ParticlesObject * pobj = &(*pov)[i_obj];
    //form.texture_spin->setValue(pobj->getGazSizeMax())
    //form.gaz_slide_size->setValue(pobj->getGazSize());
    pobj->setGaz(value);
    if (EMIT) emit objectSettingsChanged();
  }
  //if (lock)
  if (go  && ! go->duplicate_mem) mutex_data->unlock();
}
// ============================================================================
// on_gaz_slide_size_valueChanged
void FormObjectControl::on_gaz_slide_size_valueChanged(int value)
{
  //if (lock)
  if (go  && ! go->duplicate_mem) mutex_data->lock();
  int i_obj = object_index[current_object];
  if (pov && pov->size()>0 && i_obj != -1 ) {  // at least one object
    assert(i_obj < (int)pov->size());
    ParticlesObject * pobj = &(*pov)[i_obj];
    pobj->setGazSize(value*form.texture_spin->value()/
                             form.gaz_slide_size->maximum());
    if (EMIT) {
      //emit gazSizeObjectChanged(i_obj);
      emit objectSettingsChanged();
    }
  }
  //if (lock)
  if (go  && ! go->duplicate_mem) mutex_data->unlock();
}
// ============================================================================
// on_gaz_slide_alpha_valueChanged
void FormObjectControl::on_gaz_slide_alpha_valueChanged(int value)
{
  //if (lock)
  if (go  && ! go->duplicate_mem) mutex_data->lock();
  int i_obj = object_index[current_object];
  if (pov && pov->size()>0 && i_obj != -1) {  // at least one object
    assert(i_obj < (int)pov->size());
    ParticlesObject * pobj = &(*pov)[i_obj];
    pobj->setGazAlpha(value);
    if (EMIT) {
      //emit gazAlphaObjectChanged(i_obj);
      emit objectSettingsChanged();
    }
  }
  //if (lock)
  if (go  && ! go->duplicate_mem) mutex_data->unlock();
}
// ============================================================================
// on_gaz_rot_check_clicked()                                                  
void FormObjectControl::on_gaz_rot_check_clicked(bool value)
{
  //if (lock)
  if (go  && ! go->duplicate_mem) mutex_data->lock();
  int i_obj = object_index[current_object];
  if (pov && pov->size()>0 && i_obj != -1 ) {  // at least one object
    assert(i_obj < (int)pov->size());
    ParticlesObject * pobj = &(*pov)[i_obj];
    pobj->setGazRotate(value);
    if (EMIT) emit objectSettingsChanged();
  }
  //if (lock)
  if (go  && ! go->duplicate_mem) mutex_data->unlock();
}
// ============================================================================
// on_gaz_glsl_check_clicked()
void FormObjectControl::on_gaz_glsl_check_clicked(bool value)
{
  //if (lock)
  if (go  && ! go->duplicate_mem) mutex_data->lock();
  int i_obj = object_index[current_object];
  if (pov && pov->size()>0 && i_obj != -1 ) {  // at least one object
    assert(i_obj < (int)pov->size());
    ParticlesObject * pobj = &(*pov)[i_obj];
    //form.texture_spin->setValue(pobj->getGazSizeMax())
    //form.gaz_slide_size->setValue(pobj->getGazSize());
    pobj->setGazGlsl(value);
    if (EMIT) emit objectSettingsChanged();
  }
  //if (lock)
  if (go  && ! go->duplicate_mem) mutex_data->unlock();
}
// ============================================================================
// on_texture_spin_valueChanged
void FormObjectControl::on_texture_spin_valueChanged(double value)
{
  //if (lock)
  if (go  && ! go->duplicate_mem) mutex_data->lock();
  int i_obj = object_index[current_object];
  if (pov && pov->size()>0 && i_obj != -1 ) {  // at least one object
    assert(i_obj < (int)pov->size());
    ParticlesObject * pobj = &(*pov)[i_obj];
    //go->vel_vector_size = form.vel_slide_size->value();
    pobj->setGazSizeMax(value);
    pobj->setGazSize((float) form.gaz_slide_size->value()*(value/form.gaz_slide_size->maximum()));
    if (EMIT) {
      //emit gazSizeObjectChanged(i_obj);
      emit objectSettingsChanged();
    }
  }
  //if (lock)
  if (go  && ! go->duplicate_mem) mutex_data->unlock();
}
// ============================================================================
// ON VEL                                                                      
// ============================================================================

// ============================================================================
// on_vel_check_clicked()
void FormObjectControl::on_vel_check_clicked(bool value)
{
  //if (lock)
  if (go  && ! go->duplicate_mem) mutex_data->lock();
  int i_obj = object_index[current_object];
  if (pov && pov->size()>0 && i_obj != -1) {  // at least one object
    assert(i_obj < (int)pov->size());
    ParticlesObject * pobj = &(*pov)[i_obj];
    //pobj->setVelSize(((float) form.vel_spin->value()));
    pobj->setVel(value);
    if (EMIT) emit objectUpdateVel(i_obj);
    if (EMIT) emit objectSettingsChanged();
  }
  //if (lock)
  if (go  && ! go->duplicate_mem) mutex_data->unlock();
}
// ============================================================================
// on_vel_slide_size_valueChanged
void FormObjectControl::on_vel_slide_size_valueChanged(int value)
{
  //if (lock)
  if (go  && ! go->duplicate_mem) mutex_data->lock();
  int i_obj = object_index[current_object];
  if (pov && pov->size()>0 && i_obj != -1) {  // at least one object
    assert(i_obj < (int)pov->size());
    ParticlesObject * pobj = &(*pov)[i_obj];
    pobj->setVelSize((float) value*((float) form.vel_spin->value()/float(form.vel_slide_size->maximum())));
    if (form.vel_check->isChecked()) {
        if (EMIT) emit objectUpdateVel(i_obj);
        if (EMIT) emit objectSettingsChanged();
    }
  }
  //if (lock)
  if (go  && ! go->duplicate_mem) mutex_data->unlock();
}
// ============================================================================
// on_vel_slide_alpha_valueChanged
void FormObjectControl::on_vel_slide_alpha_valueChanged(int value)
{
  //if (lock)
  if (go  && ! go->duplicate_mem) mutex_data->lock();
  int i_obj = object_index[current_object];
  if (pov && pov->size()>0 && i_obj != -1) {  // at least one object
    assert(i_obj < (int)pov->size());
    ParticlesObject * pobj = &(*pov)[i_obj];
    pobj->setVelAlpha(value);
    if (form.vel_check->isChecked()) {
        if (EMIT) emit objectSettingsChanged();
    }
  }
  //if (lock)
  if (go  && ! go->duplicate_mem) mutex_data->unlock();
}
// ============================================================================
// on_vel_spin_valueChanged                                                    
void FormObjectControl::on_vel_spin_valueChanged(double value)
{
  //if (lock)
  if (go  && ! go->duplicate_mem) mutex_data->lock();
  int i_obj = object_index[current_object];
  if (pov && pov->size()>0 && i_obj != -1 ) {  // at least one object
    assert(i_obj < (int)pov->size());
    ParticlesObject * pobj = &(*pov)[i_obj];
    //go->vel_vector_size = form.vel_slide_size->value();
    pobj->setVelSizeMax(value);
    pobj->setVelSize((float) form.vel_slide_size->value()*((float) value/form.vel_slide_size->maximum()));
    if (form.vel_check->isChecked()) {
        if (EMIT) emit objectUpdateVel(i_obj);
        if (EMIT) emit objectSettingsChanged();
    }
  }
  //if (lock)
  if (go  && ! go->duplicate_mem) mutex_data->unlock();
}
// ============================================================================
// ON TEXTURE BOX                                                              
// ============================================================================

// ============================================================================
// on_texture_box_activated                                                    
void FormObjectControl::on_texture_box_activated(int index)
{
  //if (lock)
  if (go  && ! go->duplicate_mem) mutex_data->lock();
  int i_obj = object_index[current_object];
  if (pov && pov->size()>0 && i_obj != -1 ) {  // at least one object
    assert(i_obj < (int)pov->size());
    ParticlesObject * pobj = &(*pov)[i_obj];
    pobj->setTextureIndex(index);
    if (EMIT) emit textureObjectChanged(index,i_obj);
    if (EMIT) emit objectSettingsChanged();
  }
  //if (lock)
  if (go  && ! go->duplicate_mem) mutex_data->unlock();
}
// ============================================================================
// ON Orbits TAB                                                               
// ============================================================================

// ============================================================================
// on_orecord_check_clicked
void FormObjectControl::on_orecord_check_clicked(bool value)
{
  //if (lock)
  if (go  && ! go->duplicate_mem) mutex_data->lock();
  int i_obj = object_index[current_object];
  if (pov && pov->size()>0 && i_obj != -1 ) {  // at least one object
    assert(i_obj < (int)pov->size());
    ParticlesObject * pobj = &(*pov)[i_obj];
    pobj->setOrbitsRecord(value);
    if (EMIT) emit objectSettingsChanged();
  }
  //if (lock)
  if (go  && ! go->duplicate_mem) mutex_data->unlock();
}

// ============================================================================
// on_odisplay_check_ckicked                                                   
void FormObjectControl::on_odisplay_check_clicked(bool value)
{
  //if (lock)
  if (go  && ! go->duplicate_mem) mutex_data->lock();
  int i_obj = object_index[current_object];
  if (pov && pov->size()>0 && i_obj != -1 ) {  // at least one object
    assert(i_obj < (int)pov->size());
    ParticlesObject * pobj = &(*pov)[i_obj];
    pobj->setOrbits(value);
    if (EMIT) emit objectSettingsChanged();
  }
  //if (lock)
  if (go  && ! go->duplicate_mem) mutex_data->unlock();
}
// ============================================================================
// on_orbit_max_spin_valueChanged                                                    
void FormObjectControl::on_orbit_max_spin_valueChanged(int value)
{
  //if (lock)
  if (go  && ! go->duplicate_mem) mutex_data->lock();
  int i_obj = object_index[current_object];
  if (pov && pov->size()>0 && i_obj != -1 ) {  // at least one object
    assert(i_obj < (int)pov->size());
    ParticlesObject * pobj = &(*pov)[i_obj];
    //go->vel_vector_size = form.vel_slide_size->value();
    if (value > (int) pobj->npart) { // it's forbidden to exceed max orbits
      value=pobj->npart;
      form.orbit_max_spin->setValue(value);
    }
    pobj->setOrbitsMax(value);
    if (EMIT) emit objectSettingsChanged();
  }
  //if (lock)
  if (go  && ! go->duplicate_mem) mutex_data->unlock();
}
// ============================================================================
// on_orbit_history_spin_valueChanged                                          
void FormObjectControl::on_orbit_history_spin_valueChanged(int value)
{
  //if (lock)
  if (go  && ! go->duplicate_mem) mutex_data->lock();
  int i_obj = object_index[current_object];
  if (pov && pov->size()>0 && i_obj != -1 ) {  // at least one object
    assert(i_obj < (int)pov->size());
    ParticlesObject * pobj = &(*pov)[i_obj];
    //go->vel_vector_size = form.vel_slide_size->value();
    pobj->setOrbitsHistory(value);
    if (EMIT) emit objectSettingsChanged();
  }
  //if (lock)
  if (go  && ! go->duplicate_mem) mutex_data->unlock();
}
// ============================================================================
// ON PHYSICAL QUANTITY TAB                                                    
// ============================================================================
// ============================================================================
// change slide value on both direction
// add x to min and max (move both slide to right or left
// add min-y and max+y  (spread or shrink min and max)
void FormObjectControl::dens_slide_min_max(const int x, const int y)
{
  if (go  && ! go->duplicate_mem) mutex_data->lock();
  my_mutex2->lock();
  int i_obj = object_index[current_object];
  if (pov && pov->size()>0 && i_obj != -1 && phys_select)  {  // at least one object
    assert(i_obj < (int)pov->size());
    //std::cerr << "x="<<x<< "  y="<<y<<"\n";
    EMIT=FALSE;

    if (fabs(x)> fabs(y)) {
      // horizontal move
      form.dens_slide_min->setValue(form.dens_slide_min->value()+x);
      form.dens_slide_max->setValue(form.dens_slide_max->value()+x);
      // vertical move
    } else {
      if (y>0)
        form.dens_slide_min->setValue(form.dens_slide_min->value()+y);
      else {
        form.dens_slide_min->setValue(form.dens_slide_min->value()+y);
        form.dens_slide_max->setValue(form.dens_slide_max->value()-y);
      }
    }
    EMIT=TRUE;
    setNewPhys();
    go->gcb_min = form.dens_slide_min->value();
    go->gcb_max = form.dens_slide_max->value();
    emit changeBoundaryPhys(i_obj,EMIT);
  }
  my_mutex2->unlock();
  //if (lock)
  if (go  && ! go->duplicate_mem) mutex_data->unlock();
}

// ============================================================================
// on_dens_slide_min_valueChanged                                              
void FormObjectControl::on_dens_slide_min_valueChanged(int value)
{
  if (go  && ! go->duplicate_mem) mutex_data->lock();
  my_mutex2->lock();
  int i_obj = object_index[current_object];
  if (pov && pov->size()>0 && i_obj != -1 && phys_select)  {  // at least one object
    assert(i_obj < (int)pov->size());
    //std::cerr << "min value="<<value<<"\n";
    //ParticlesObject * pobj = &(*pov)[i_obj];
    if (value > form.dens_slide_max->value()) { // min < max !
      form.dens_slide_max->setValue(value+1);
    }

    //setNewPhys();
    ParticlesObject * pobj = &(*pov)[i_obj];
    if (value<0) value=0;
    if (value>=99) value=98;
    pobj->setMinPercenPhys(value);
    if (EMIT) {

      setNewPhys();
      go->gcb_min = form.dens_slide_min->value();
      go->gcb_max = form.dens_slide_max->value();
      emit changeBoundaryPhys(i_obj,EMIT);
      //emit updateThresholMinMax();
    }

    dens_histo->drawDensity(form.dens_slide_min->value(), form.dens_slide_max->value());
    dens_color_bar->draw(form.dens_slide_min->value(), form.dens_slide_max->value());

    //if (EMIT) emit changeBoundaryPhys(i_obj,EMIT);
  }
  my_mutex2->unlock();
  //if (lock)
  if (go  && ! go->duplicate_mem) mutex_data->unlock();
}
// ============================================================================
// on_dens_slide_max_valueChanged                                              
void FormObjectControl::on_dens_slide_max_valueChanged(int value)
{
  if (go  && ! go->duplicate_mem) mutex_data->lock();
  my_mutex2->lock();
  int i_obj = object_index[current_object];
  if (pov && pov->size()>0 && i_obj != -1 && phys_select)  {  // at least one object
    assert(i_obj < (int)pov->size());
    //std::cerr << "max value="<<value<<"\n";
    //ParticlesObject * pobj = &(*pov)[i_obj];
    if (value <= form.dens_slide_min->value()) { // min < max !
      form.dens_slide_min->setValue(std::max(value-1,0));
    }

    //setNewPhys();
    ParticlesObject * pobj = &(*pov)[i_obj];
    if (value<=0) value=1;
    if (value>=100) value=99;
    pobj->setMaxPercenPhys(value);
    if (EMIT) {

      setNewPhys();
      go->gcb_min = form.dens_slide_min->value();
      go->gcb_max = form.dens_slide_max->value();
      emit changeBoundaryPhys(i_obj, EMIT);
    }

    dens_histo->drawDensity(form.dens_slide_min->value(), form.dens_slide_max->value());
    dens_color_bar->draw(form.dens_slide_min->value(), form.dens_slide_max->value());

    //if (EMIT) emit changeBoundaryPhys(i_obj,EMIT);
  }
  my_mutex2->unlock();
  //if (lock)
  if (go  && ! go->duplicate_mem) mutex_data->unlock();
}
// ============================================================================
// setNewPhys
// Set new physical values for the object
void FormObjectControl::setNewPhys(bool update_glob)
{
  if (go  && ! go->duplicate_mem) mutex_data->lock();
  int i_obj = object_index[current_object];
  if (pov && pov->size()>0 && i_obj != -1 && phys_select)  {  // at least one object
    assert(i_obj < (int)pov->size());
    ParticlesObject * pobj = &(*pov)[i_obj];
    double diff_rho=(log(phys_select->getMax())-log(phys_select->getMin()))/100.;
    pobj->setMinPhys(exp(log(phys_select->getMin())+form.dens_slide_min->value()*diff_rho));
    pobj->setMaxPhys(exp(log(phys_select->getMin())+form.dens_slide_max->value()*diff_rho));
    if (update_glob) {
      // user has selected new values for physical data min/max
      // we save globally these values
      go->phys_min_glob = pobj->getMinPhys();
      go->phys_max_glob = pobj->getMaxPhys();
    }
  }
  if (go  && ! go->duplicate_mem) mutex_data->unlock();
}
// ============================================================================
// on_dens_apply_button_clicked()                                              
void FormObjectControl::on_phys_console_button_clicked()
{
  if (go  && ! go->duplicate_mem) mutex_data->lock();
  int i_obj = object_index[current_object];
  if (pov && pov->size()>0 && i_obj != -1 && phys_select)  {  // at least one object
    assert(i_obj < (int)pov->size());
    ParticlesObject * pobj = &(*pov)[i_obj];
#if 0
    if (1) { //DEACTIVAREDform.dens_loc_button->isChecked()) {
      go->phys_local = true;
      float diff_rho=(log(phys_select->getMax())-log(phys_select->getMin()))/100.;
      pobj->setMinPhys(exp(log(phys_select->getMin())+form.dens_slide_min->value()*diff_rho));
      pobj->setMaxPhys(exp(log(phys_select->getMin())+form.dens_slide_max->value()*diff_rho));
      std::cerr << ">> slide min=" << (log(pobj->getMinPhys())-log(phys_select->getMin()))*1/diff_rho << "\n";
    } else {
      go->phys_local = false;
      //DEACTIVARED go->phys_min_glob = (form.dens_min_user->text()).toFloat();
      //DEACTIVARED go->phys_max_glob = (form.dens_max_user->text()).toFloat();
    }
    if (EMIT) {
      emit densityProfileObjectChanged(i_obj);
      emit objectSettingsChanged();
    }
#endif
    std::cerr << "minphys="<<pobj->getMinPhys()<<"\n";
    std::cerr << "maxphys="<<pobj->getMaxPhys()<<"\n";
  }
  if (go  && ! go->duplicate_mem) mutex_data->unlock();
}
// ============================================================================
// on_dens_phys_radio_pressed()                                              
void FormObjectControl::on_dens_phys_radio_clicked()
{
  mutex_data->lock();
  current_data->setIpvs(PhysicalData::rho);
  emit updateIpvs(PhysicalData::rho);
  mutex_data->unlock();
  physicalSelected();
}
// ============================================================================
// on_temp_phys_radio_pressed()                                                
void FormObjectControl::on_temp_phys_radio_clicked()
{
  mutex_data->lock();
  current_data->setIpvs(PhysicalData::temperature);
  int i_obj = object_index[current_object];
  if (pov && pov->size()>0 && i_obj != -1 && phys_select)  {  // at least one object
    ParticlesObject * pobj = &(*pov)[i_obj];
    pobj->setRhoSorted(false);
    emit updateIpvs(PhysicalData::temperature);
  }
  mutex_data->unlock();
  physicalSelected();
}
// ============================================================================
// on_tempdens_phys_radio_pressed()                                                
void FormObjectControl::on_tempdens_phys_radio_clicked()
{
  mutex_data->lock();

  current_data->setIpvs(PhysicalData::temperaturesd);
  int i_obj = object_index[current_object];
  if (pov && pov->size()>0 && i_obj != -1 && phys_select)  {  // at least one object
    ParticlesObject * pobj = &(*pov)[i_obj];
    pobj->setRhoSorted(true);
    emit updateIpvs(PhysicalData::temperaturesd);
  }
  mutex_data->unlock();
  physicalSelected();
}
// ============================================================================
// on_pressure_phys_radio_pressed()                                            
void FormObjectControl::on_pressure_phys_radio_clicked()
{
  mutex_data->lock();
  current_data->setIpvs(PhysicalData::pressure);
  emit updateIpvs(PhysicalData::pressure);
  mutex_data->unlock();
  physicalSelected();
}
// ============================================================================
// on_velnorm_phys_radio_pressed()
void FormObjectControl::on_velnorm_phys_radio_clicked()
{
  mutex_data->lock();
  current_data->setIpvs(PhysicalData::velnorm);
  emit updateIpvs(PhysicalData::velnorm);
  mutex_data->unlock();
  physicalSelected();
}
// ============================================================================
// physicalSelected()                                                      
void FormObjectControl::physicalSelected()
{
  if (go  && ! go->duplicate_mem) mutex_data->lock();
  int i_obj = object_index[current_object];
  if (pov && pov->size()>0 && i_obj != -1 && phys_select)  {  // at least one object
    assert(i_obj < (int)pov->size());
    ParticlesObject * pobj = &(*pov)[i_obj];
    setPhysicalTabName();
    if (phys_select && phys_select->isValid()) {
      form.dens_slide_min->setValue(0);      // here we should put current object value (from PhysObject), not   0
      form.dens_slide_max->setValue(100);    // here we should put current object value (from PhysObject), not 100

      pobj->setMinPhys(phys_select->getMin());
      pobj->setMaxPhys(phys_select->getMax());

      dens_histo->drawDensity(phys_select->data_histo);
      double diff_rho=(log(phys_select->getMax())-log(phys_select->getMin()))/100.;
      form.dens_slide_min->setValue((log(pobj->getMinPhys())-log(phys_select->getMin()))*1./diff_rho);
      form.dens_slide_max->setValue((log(pobj->getMaxPhys())-log(phys_select->getMin()))*1./diff_rho);
      dens_histo->drawDensity(form.dens_slide_min->value(),form.dens_slide_max->value());
      dens_color_bar->draw(form.dens_slide_min->value(),form.dens_slide_max->value());


      //on_dens_apply_button_clicked();
      //on_phys_console_button_clicked();
      if (EMIT) {
        emit densityProfileObjectChanged(i_obj);
        emit objectSettingsChanged();
      }
    }
  }
  if (go  && ! go->duplicate_mem) mutex_data->unlock();
}
// ============================================================================
// setPhysicalTabName()                                                        
void FormObjectControl::setPhysicalTabName()
{
  phys_select = current_data->getPhysData();
  if (phys_select) {
    //int type=phys_select->getType(); // return the index of the selectd physical quantities
    int type=current_data->getIpvs(); // return the index of the selectd physical quantities
    switch (type) {
          case PhysicalData::rho :
            form.dens_phys_radio->setChecked(true);
            form.objects_properties->setTabText(1,"Density");
            break;
          case PhysicalData::temperature :
            form.objects_properties->setTabText(1,"Temperature");
            form.temp_phys_radio->setChecked(true);
            break;
           case PhysicalData::temperaturesd:
            form.objects_properties->setTabText(1,"Temperature/Dens");
            form.tempdens_phys_radio->setChecked(true);
            break;
          case PhysicalData::pressure :
            form.objects_properties->setTabText(1,"Pressure");
            form.pressure_phys_radio->setChecked(true);
            break;
          case PhysicalData::velnorm :
            form.objects_properties->setTabText(1,"VelocityNorm");
            form.velnorm_phys_radio->setChecked(true);
            break;
          }
  }
}
// ============================================================================
// -- Stretching Tab --
//

// ============================================================================
//
void FormObjectControl::on_z_stretch_slide_valueChanged(int value)
{
  if (go) {
    static double zsave=0.0;
    double ztransref;

    if (value>=1 && go->z_stretch_value!=0.0) {
      zsave=go->ztrans/go->z_stretch_value;
    }
    if (value>=1 && go->z_stretch_value==0.0) {
      ztransref=zsave;
    } else {
      ztransref=go->ztrans/go->z_stretch_value;
    }
    go->z_stretch_value = value*go->z_stretch_max/100.0;
    if (go->z_stretch_value!=0.0)
      go->ztrans=ztransref*go->z_stretch_value; // compute the new translation to keep centered on it
    if (EMIT) emit objectSettingsChanged();
  }

}
// ============================================================================
//
void FormObjectControl::on_z_stretch_jit_cb_clicked(bool b)
{
  if (go) {
    go->z_stretch_jit = b;
    if (EMIT) emit objectSettingsChanged();
  }
}
// ============================================================================
//
void FormObjectControl::on_z_stretch_max_spin_valueChanged(double value)
{
  if (go) {
    double ztransref=0.0;
    if (go->z_stretch_value!=0.0)
      ztransref=go->ztrans/go->z_stretch_value;
    go->z_stretch_max = value;
    go->z_stretch_value = form.z_stretch_slide->value()*go->z_stretch_max/100.0;
    if (go->z_stretch_value!=0.0)
      go->ztrans=ztransref*go->z_stretch_value; // compute the new translation to keep centered on it
    if (EMIT) emit objectSettingsChanged();
  }
}
// ============================================================================
// -- CPoints Tab --
//
// ============================================================================
//
void DeletePushButton::mousePressEvent(QMouseEvent *e) {
  emit deleteClicked(!(e->modifiers() == Qt::ShiftModifier));
  QPushButton::mousePressEvent(e);
}

void FormObjectControl::on_load_cpoints_file_clicked(bool) {
  QString file_path;
  file_path = QFileDialog::getOpenFileName(
          this, tr("Open characteristic point description file"));
  if (!file_path.isEmpty()) {
    if(!pointset_manager->loadFile(file_path.toStdString()))
      initCPointsTreeWidget();
    else
      QMessageBox::critical(this->window(), "Error", "Could not parse file " + file_path);
  }
}
// ============================================================================
//
CPointset *FormObjectControl::getPointsetFromItem(QTreeWidgetItem *item) {
  CPointset *cpointset;
  if (!item->parent()) { // item is a cpointset
    std::string pointset_name = item->text(0).toStdString();
    return (*pointset_manager)[pointset_name];
  } else
    return nullptr;
}
// ============================================================================
//
void FormObjectControl::initCPointsTreeWidget() {
  form.cpoints_set_treewidget->clear();
  for (auto cpoint_set : *pointset_manager) {
    createCpointsetTreeItem(cpoint_set.second);
  }
}
// ============================================================================
//
QTreeWidgetItem *FormObjectControl::createCpointsetTreeItem(CPointset *cpoint_set) {
  auto cpointset_item = new QTreeWidgetItem(
          form.cpoints_set_treewidget,
          QStringList() << QString::fromStdString(cpoint_set->getName()),
          QTreeWidgetItem::Type);

  for (auto const &cpoint_pair : cpoint_set->getCPoints()) {
    GLCPoint *cpoint = cpoint_pair.second;
    int cpoint_id = cpoint_pair.first;
    auto cpoint_item = new QTreeWidgetItem(
            cpointset_item,
            QStringList() << QString::fromStdString(cpoint->getName()) << QString::number(cpoint_id),
            QTreeWidgetItem::Type);
  }
  return cpointset_item;
}
// ============================================================================
//
void FormObjectControl::on_cpoints_display_cbx_stateChanged(int state) {
  QTreeWidgetItem *item = form.cpoints_set_treewidget->selectedItems()[0];
  if(item->parent())
    item = item->parent();
  CPointset *pointset = getPointsetFromItem(item);
  if (pointset) {
    if (state == Qt::Checked)
      pointset->setVisible(true);
    else if (state == Qt::Unchecked)
      pointset->setVisible(false);
    emit objectSettingsChanged();
  }
}
// ============================================================================
//
void FormObjectControl::on_cpoints_set_treewidget_itemSelectionChanged() {
  pointset_manager->unselectAll();
  QList<QTreeWidgetItem *> items = form.cpoints_set_treewidget->selectedItems();

  if (items.size() == 0) {
    form.edit_cpoint_parent_box->setEnabled(false);
    form.edit_cpointset_parent_box->setEnabled(false);
  } else if (items.size() == 1) {
    QTreeWidgetItem *item = items.first();

    if (!item->parent()) { // if top level item ie if a cpointset is selected
      form.edit_cpointset_parent_box->setEnabled(true);
      form.edit_cpointset_box->setEnabled(true);
      form.edit_cpoint_parent_box->setEnabled(false);

      CPointset *pointset = getPointsetFromItem(item);
      if (pointset) {
        setFormState(pointset);
        pointset->select();
      }
    } else { // else a cpoint is selected

      form.edit_cpoint_box->setEnabled(true);
      form.edit_cpoint_parent_box->setEnabled(true);
      form.edit_cpointset_parent_box->setEnabled(true);
      form.edit_cpointset_box->setEnabled(true);
      int cpoint_id = item->text(1).toInt();
      CPointset *pointset = getPointsetFromItem(item->parent());
      GLCPoint *cpoint = pointset->getCPoints().at(cpoint_id);
      form.edit_cpoint_coords_x->setValue(cpoint->getCoords()[0]);
      form.edit_cpoint_coords_y->setValue(cpoint->getCoords()[1]);
      form.edit_cpoint_coords_z->setValue(cpoint->getCoords()[2]);
      form.edit_cpoint_size->setValue(cpoint->getSize());
      form.edit_cpoint_name->setText(QString::fromStdString(cpoint->getName()));
      setFormState(pointset);
      pointset->selectCPoint(cpoint_id);
    }
  } else { // multiple items selected
    bool cpoints = false, cpointsets = false;
    CPointset* previous_parent = nullptr;

    for (auto item : items) {
      if (!item->parent()){
        cpointsets = true;
        auto cpointset = getPointsetFromItem(item);
        cpointset->select();
      }
      else{
        cpoints = true;
        CPointset* parent = getPointsetFromItem(item->parent());
        int cpoint_id = item->text(1).toInt();
        parent->selectCPoint(cpoint_id);

        if(!previous_parent) // assign to first parent
          previous_parent = parent;
        if(parent != previous_parent) // check if all cpoints are from the same set
          cpointsets = true;
        previous_parent = parent;
      }
    }

    if (cpointsets && !cpoints) { // only cpointsets
      form.edit_cpointset_parent_box->setEnabled(true);
      form.edit_cpointset_box->setEnabled(false);
      form.edit_cpoint_parent_box->setEnabled(false);
    } else if (!cpointsets && cpoints) { // only cpoints
      form.edit_cpointset_parent_box->setEnabled(true);
      form.edit_cpoint_parent_box->setEnabled(true);
      form.edit_cpoint_box->setEnabled(false);
    } else { //both types selected
      form.edit_cpointset_parent_box->setEnabled(false);
      form.edit_cpoint_parent_box->setEnabled(false);
    }
  }
  emit objectSettingsChanged();
}
// ============================================================================
//
void FormObjectControl::on_cpoints_threshold_slider_valueChanged(int threshold) {
  QTreeWidgetItem *item = form.cpoints_set_treewidget->selectedItems()[0];
  if(item->parent())
    item = item->parent();

  CPointset *pointset = getPointsetFromItem(item);
  if (pointset) {
    pointset->setThreshold(threshold);
    emit objectSettingsChanged();
  }
}

// ============================================================================
//
void FormObjectControl::on_add_cpoint_btn_clicked(bool) {
  QTreeWidgetItem *item = form.cpoints_set_treewidget->selectedItems()[0];
  bool select_created = false;
  if(item->parent())
    item = item->parent();
  else
    select_created = true;
  CPointset *pointset = getPointsetFromItem(item);
  if (pointset) {
    float size = form.add_cpoint_coords_size->value();
    if(size > 0){
      std::array<float, 3> coords = {
              static_cast<float>(form.add_cpoint_coords_x->value()),
              static_cast<float>(form.add_cpoint_coords_y->value()),
              static_cast<float>(form.add_cpoint_coords_z->value())
      };
      const string &point_text = form.add_cpoint_name->text().toStdString();
      GLCPoint *cpoint = pointset->addPoint(coords, size, point_text);
      auto new_item = new QTreeWidgetItem(QStringList() << QString::fromStdString(cpoint->getName()) << QString::number(cpoint->getId()));
      if(select_created)
        pointset->selectCPoint(cpoint->getId());
      item->insertChild(0, new_item);
      emit objectSettingsChanged();
    }
  }
}

// ============================================================================
//
void FormObjectControl::on_add_cpointset_clicked(bool) {
  CPointset *new_pointset = pointset_manager->createNewCPointset();
  auto item = new QTreeWidgetItem(form.cpoints_set_treewidget,
                                  QStringList() << QString::fromStdString(new_pointset->getName()), 0);
  form.cpoints_set_treewidget->setCurrentItem(item, 0);
}
// ============================================================================
//
void FormObjectControl::delete_cpointsets(bool need_confirmation) {
  auto items = form.cpoints_set_treewidget->selectedItems();
  if (items.size() == 1) {
    QTreeWidgetItem *item = items[0];
    std::string pointset_name = item->text(0).toStdString();
    QMessageBox::StandardButton reply;
    if (need_confirmation)
      reply = QMessageBox::question(this, "Delete cpointset", QString::fromStdString(
              "Do you really want to delete cpointset \"" + pointset_name + "\"?"),
                                    QMessageBox::Yes | QMessageBox::No);
    else reply = QMessageBox::Yes;
    if (reply == QMessageBox::Yes) {
      pointset_manager->deleteCPointset(pointset_name);
      delete item;
      emit objectSettingsChanged();
    }
  } else if (items.size() > 1) {
    QMessageBox::StandardButton reply;
    if (need_confirmation)
      reply = QMessageBox::question(this, "Delete cpointset", "Do you really want to delete " + QString::number(items.size()) + " cpointsets ?",
                                    QMessageBox::Yes | QMessageBox::No);
    else reply = QMessageBox::Yes;
    if (reply == QMessageBox::Yes) {
      for (auto item : items){
        std::string pointset_name = item->text(0).toStdString();
        pointset_manager->deleteCPointset(pointset_name);
        delete item;
      }
      emit objectSettingsChanged();
    }
  }
}

void FormObjectControl::delete_cpoints(bool need_confirmation) {
  auto items = form.cpoints_set_treewidget->selectedItems();

  if (items.size() == 1) {
    QTreeWidgetItem *item = items[0];
    QString cpoint_name = item->text(0);

    QMessageBox::StandardButton reply;
    if (need_confirmation)
      reply = QMessageBox::question(this, "Delete cpoints", "Do you really want to delete cpoints \"" + cpoint_name + "\"?",
                                    QMessageBox::Yes | QMessageBox::No);
    else reply = QMessageBox::Yes;

    if (reply == QMessageBox::Yes) {
      int cpoint_id = item->text(1).toInt();
      std::string parent_pointset_name = item->parent()->text(0).toStdString();
      pointset_manager->deleteCPoint(parent_pointset_name, cpoint_id);
      delete item;
      emit objectSettingsChanged();
    }
  } else if (items.size() > 1) {
    QMessageBox::StandardButton reply;
    if (need_confirmation)
      reply = QMessageBox::question(this, "Delete cpoints", QString::fromStdString(
              "Do you really want to delete " + std::to_string(items.size()) + " cpoints ?"),
                                    QMessageBox::Yes | QMessageBox::No);
    else reply = QMessageBox::Yes;

    if (reply == QMessageBox::Yes) {
      for (auto item : items) {
        auto parent_pointset_name = item->parent()->text(0).toStdString();
        int cpoint_id = item->text(1).toInt();
        pointset_manager->deleteCPoint(parent_pointset_name, cpoint_id);
        delete item;
      }
      emit objectSettingsChanged();
    }

  }
}

void FormObjectControl::shapeRadioClicked() {
  QTreeWidgetItem *item = form.cpoints_set_treewidget->selectedItems()[0];
  if(item->parent())
    item = item->parent();
  CPointset *pointset = getPointsetFromItem(item);
  QRadioButton *clickedBtn = qobject_cast<QRadioButton *>(sender());
  CPointset* new_pointset = nullptr;
  if (clickedBtn->objectName() == "shape_radio_disk")
    new_pointset = pointset_manager->changePointsetShape(pointset, "disk");
  else if (clickedBtn->objectName() == "shape_radio_square")
    new_pointset = pointset_manager->changePointsetShape(pointset, "square");
  else if (clickedBtn->objectName() == "shape_radio_tag")
    new_pointset = pointset_manager->changePointsetShape(pointset, "tag");
  else if (clickedBtn->objectName() == "shape_radio_sphere")
    new_pointset = pointset_manager->changePointsetShape(pointset, "sphere");

  if(new_pointset)
    setFormState(new_pointset);

  emit objectSettingsChanged();
}

void FormObjectControl::on_color_picker_button_clicked(bool) {
  QTreeWidgetItem *item = form.cpoints_set_treewidget->selectedItems()[0];
  if(item->parent())
    item = item->parent();
  CPointset *pointset = getPointsetFromItem(item);
  if (pointset) {
    QColor newColor = QColorDialog::getColor(pointset->getQColor());
    if (newColor.spec() != QColor::Invalid) {
      form.color_picker_button->setStyleSheet("background-color: " + newColor.name());
      pointset->setColor(newColor);
      emit objectSettingsChanged();
    }
  }

}

void FormObjectControl::on_export_cpoints_file_clicked(bool) {
  QString file_path;
  file_path = QFileDialog::getSaveFileName(
          this, tr("Save cpoints description file"));
  if (!file_path.isEmpty()) {
    pointset_manager->saveToFile(file_path.toStdString());
  }
}
void FormObjectControl::on_add_cpoint_center_coord_btn_clicked(bool) {
  form.add_cpoint_coords_x->setValue(-go->xtrans);
  form.add_cpoint_coords_y->setValue(-go->ytrans);
  form.add_cpoint_coords_z->setValue(-go->ztrans);
}


void FormObjectControl::on_edit_cpoint_coords_x_valueChanged(double x) {
  QTreeWidgetItem *item = form.cpoints_set_treewidget->selectedItems()[0];
  QTreeWidgetItem *parent_item = item->parent();
  CPointset *pointset = (*pointset_manager)[parent_item->text(0).toStdString()];
  int cpoint_id = item->text(1).toInt();
  pointset->setCpointCoordsX(cpoint_id, x);
  emit objectSettingsChanged();
}
void FormObjectControl::on_edit_cpoint_coords_y_valueChanged(double y) {
  QTreeWidgetItem *item = form.cpoints_set_treewidget->selectedItems()[0];
  QTreeWidgetItem *parent_item = item->parent();
  CPointset *pointset = (*pointset_manager)[parent_item->text(0).toStdString()];
  int cpoint_id = item->text(1).toInt();
  pointset->setCpointCoordsY(cpoint_id, y);
  emit objectSettingsChanged();
}

void FormObjectControl::on_edit_cpoint_coords_z_valueChanged(double z) {
  QTreeWidgetItem *item = form.cpoints_set_treewidget->selectedItems()[0];
  QTreeWidgetItem *parent_item = item->parent();
  CPointset *pointset = (*pointset_manager)[parent_item->text(0).toStdString()];
  int cpoint_id = item->text(1).toInt();
  pointset->setCpointCoordsZ(cpoint_id, z);
  emit objectSettingsChanged();
}
void FormObjectControl::on_edit_cpoint_size_valueChanged(double size) {
  QTreeWidgetItem *item = form.cpoints_set_treewidget->selectedItems()[0];
  QTreeWidgetItem *parent_item = item->parent();
  CPointset *pointset = (*pointset_manager)[parent_item->text(0).toStdString()];
  int cpoint_id = item->text(1).toInt();
  pointset->setCpointSize(cpoint_id, size);
  emit objectSettingsChanged();
}
void FormObjectControl::on_edit_cpointset_name_btn_clicked() {
  QTreeWidgetItem *item = form.cpoints_set_treewidget->selectedItems()[0];
  if(item->parent())
    item = item->parent();
  CPointset *pointset = getPointsetFromItem(item);
  auto new_name = form.edit_cpointset_name->text().toStdString();
  if (new_name != pointset->getName()) {
    try {
      pointset_manager->at(new_name);
      int ret = QMessageBox::critical(this, "Error",
                                      QString::fromStdString("CPointset " + new_name + " already exists"));
      form.edit_cpointset_name->setText(QString::fromStdString(pointset->getName()));
      emit objectSettingsChanged();
    } catch (std::out_of_range) {
      pointset_manager->setPointsetName(pointset->getName(), new_name);
      item->setData(0, Qt::DisplayRole, QString::fromStdString(new_name));
    }
  }
}

void FormObjectControl::on_edit_cpoint_name_textChanged() {
  auto item = form.cpoints_set_treewidget->selectedItems()[0];
  int cpoint_id = item->text(1).toInt();
  auto parent_pointset = getPointsetFromItem(item->parent());
  auto new_name = form.edit_cpoint_name->text().toStdString();
  parent_pointset->setCpointText(cpoint_id, new_name);
  item->setData(0, Qt::DisplayRole, QString::fromStdString(new_name));
  emit objectSettingsChanged();
}
void FormObjectControl::on_shape_show_name_cbx_stateChanged(int state) {
  QTreeWidgetItem *item = form.cpoints_set_treewidget->selectedItems()[0];
  if(item->parent())
    item = item->parent();
  CPointset *pointset = getPointsetFromItem(item);
  if (state == Qt::Checked)
    pointset->setNameVisible(true);
  else if (state == Qt::Unchecked)
    pointset->setNameVisible(false);
  emit objectSettingsChanged();
}
void FormObjectControl::on_edit_shape_fill_ratio_valueChanged(int value) {
  QTreeWidgetItem *item = form.cpoints_set_treewidget->selectedItems()[0];
  if(item->parent())
    item = item->parent();
  CPointset *pointset = getPointsetFromItem(item);
  pointset->setFillratio(value/100.);
  emit objectSettingsChanged();
}
void FormObjectControl::on_edit_shape_name_size_factor_valueChanged(int name_size) {
  QTreeWidgetItem *item = form.cpoints_set_treewidget->selectedItems()[0];
  if(item->parent())
    item = item->parent();
  CPointset *pointset = getPointsetFromItem(item);
  pointset->setNameSizeFactor(name_size/10.);
  emit objectSettingsChanged();
}
void FormObjectControl::on_edit_shape_name_offset_valueChanged(int name_offset) {
  QTreeWidgetItem *item = form.cpoints_set_treewidget->selectedItems()[0];
  if(item->parent())
    item = item->parent();
  CPointset *pointset = getPointsetFromItem(item);
  pointset->setNameOffset(name_offset/10.);
  emit objectSettingsChanged();
}
void FormObjectControl::on_edit_shape_nb_sphere_sections_valueChanged(int nb_sphere_sections) {
  QTreeWidgetItem *item = form.cpoints_set_treewidget->selectedItems()[0];
  if(item->parent())
    item = item->parent();
  CPointset *pointset = getPointsetFromItem(item);
  pointset->setNbSphereSections(nb_sphere_sections);
  emit objectSettingsChanged();
}

void FormObjectControl::on_edit_shape_name_angle_valueChanged(int name_angle) {
  QTreeWidgetItem *item = form.cpoints_set_treewidget->selectedItems()[0];
  if(item->parent())
    item = item->parent();
  CPointset *pointset = getPointsetFromItem(item);
  pointset->setNameAngle(name_angle-90);
  emit objectSettingsChanged();
}
void FormObjectControl::setFormState(CPointset *pointset) {
  CPointsetShapes shape = pointset->getShape();
  if (shape == CPointsetShapes::disk) {
    form.shape_radio_disk->setChecked(true);
    form.edit_shape_nb_sphere_sections->setEnabled(false);
    form.edit_shape_fill_ratio->setEnabled(true);
  } else if (shape == CPointsetShapes::square) {
    form.shape_radio_square->setChecked(true);
    form.edit_shape_nb_sphere_sections->setEnabled(false);
    form.edit_shape_fill_ratio->setEnabled(true);
  } else if (shape == CPointsetShapes::tag) {
    form.shape_radio_tag->setChecked(true);
    form.edit_shape_nb_sphere_sections->setEnabled(false);
    form.edit_shape_fill_ratio->setEnabled(false);
  } else if (shape == CPointsetShapes::sphere) {
    form.shape_radio_sphere->setChecked(true);
    form.edit_shape_nb_sphere_sections->setEnabled(true);
    form.edit_shape_fill_ratio->setEnabled(false);
  }
  form.shape_show_name_cbx->setChecked(pointset->isNameVisible());
  form.edit_shape_nb_sphere_sections->setValue(pointset->getNbSphereSections());
  form.edit_shape_name_offset->setValue(pointset->getNameOffset()*10);
  form.edit_shape_name_size_factor->setValue(pointset->getNameSizeFactor()*10);
  form.edit_shape_fill_ratio->setValue(pointset->getFillratio()*100);
  form.cpoints_display_cbx->setChecked(pointset->isVisible());
  form.cpoints_threshold_slider->setValue(pointset->getThreshold());
  form.color_picker_button->setStyleSheet("background-color:" + pointset->getQColor().name());
  form.edit_cpointset_name->setText(QString::fromStdString(pointset->getName()));
}
void FormObjectControl::disableCpointsTab() {
  form.tab_cpoints->setEnabled(false);
  form.tab_cpoints->setToolTip("Your GPU does not support this feature (OpenGL > 3.0 or EXT_gpu_shader4 is needed)");
}

}
