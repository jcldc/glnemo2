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
/**
        @author Jean-Charles Lambert <Jean-Charles.Lambert@lam.fr>
*/
#ifndef GLNEMOFORMOBJECTCONTROL_H
#define GLNEMOFORMOBJECTCONTROL_H

#include "ui_formobjectcontrol.h"
#include "glcpoints.h"
#include "particlesobject.h"
#include "particlesdata.h"
#include "globaloptions.h"
#include "densityhisto.h"
#include "densitycolorbar.h"
#include <QComboBox>
#include <QCloseEvent>
#include <QMutex>
#include <QResizeEvent>
#include <QStyledItemDelegate>

namespace glnemo {
  // =========================================================================
  // class DeletePushButton : used to delete cpointset
  class DeletePushButton : public QPushButton {
  Q_OBJECT
  protected:
    void mousePressEvent(QMouseEvent*e);
  signals:
    void deleteClicked(bool confirmation);
  };
  // ============================================================================
  // class QCheckBoxTable : control the visibility of the objects from the GUI
  class QCheckBoxTable: public QCheckBox {
    Q_OBJECT
      public:
    QCheckBoxTable(const int _r,const int _t,QWidget * parent = 0):QCheckBox(parent),
      row(_r), table(_t) {
        connect(this,SIGNAL(clicked(bool)),this,SLOT(myClicked(bool)));
      };
      const int row, table;
  signals:
      void changeVisib(const bool,const int ,const int );
  private slots:
    void myClicked(bool b) {
      //std::cerr << "my clicked row : "<< row <<" bool : " << b << "\n";
      emit changeVisib(b,row,table);
    };
    //~QCheckBoxTable();
  };
// ============================================================================
  // class QComboBoxTable : combobox from where the user give the particles range
  // of the object
  class QComboBoxTable: public QComboBox {
    Q_OBJECT
      public:
      const QComboBoxTable & operator=(const QComboBoxTable& m) {
        //row=m.row;
        //table=m.table;
        for (int i=0; i<m.count(); i++) {
          addItem(m.itemText(i));
        }
        return *this;
      }
     QComboBoxTable(const int _r,const int _t,QWidget * parent = 0):QComboBox(parent),
      row(_r), table(_t) {
        //connect(this,SIGNAL(editTextChanged ( const QString &)),this,
        //SLOT(my_editTextChanged ( const QString &)));
        //connect(this,SIGNAL(highlighted(int)),this,SLOT(myActivated(int)));
        connect(this,SIGNAL(activated(int)),this,SLOT(myActivated(int)));
        setEditable(true);
      };
      ~QComboBoxTable() {
        std::cerr << "<< destructor ~QComboBoxTable(), row="<<row<<"\n";
      }
      void display() {
#if 0
        for (int i=0; i<count(); i++) {
          std::cerr << "i="<<i<<" ==> "<<itemText(i).toStdString()<<"\n";
        }
#endif
      }
      int row, table;
  signals:
      void cellClicked(const int, const int);
      void comboActivated(const int, const int);
      void updateIpvs(const int);

  private slots:
    void  my_editTextChanged ( const QString & text ) {
      std::cerr << "[slot my_editTextChanged]  row : "<< row <<" text : " << text.toStdString() << "\n";
      emit cellClicked(row,1);
      //emit my_editTextChanged(text,row,table);
    };
    // ------------------------------------------------
    // myActivated is called when the comboboxtable is clicked
    // then the signal [comboActivated] is launched
    void myActivated(int _index) {
      QString text = itemText(_index);
      std::cerr << "[slot myActivated]  _index="<<_index<<" text : " << text.toStdString()
                << " row=" << row << "\n";
      emit comboActivated(row,1);
      //emit comboActivated(row,1,_index);
    }
  };
  // ============================================================================
  class FormObjectControl: public QDialog {
  Q_OBJECT
  public:
  FormObjectControl(CPointsetManager *_pointset_manager, GlobalOptions *global_options, QWidget *parent = 0);

    ~FormObjectControl();
    void update(ParticlesData   * ,
                ParticlesObjectVector * ,
                GlobalOptions         * ,
	        bool reset_table=true  );
    void init(QMutex * _m) { mutex_data = _m;}
    void disableCpointsTab();
  signals:
  void objectSettingsChanged();
  void objectUpdateVel(const int);
  void objectUpdate();
  void gazSizeObjectChanged(const int);
  void gazAlphaObjectChanged(const int);
  void gazColorObjectChanged(const int);
  void densityProfileObjectChanged(const int);
  void textureObjectChanged(const int , const int);
  void leaveEvent();
  void updateIpvs(const int);
  // cmap signals
  void nextColorMap();
  void prevColorMap();
  void constantColorMap(const bool);
  void reverseColorMap(const bool);
  void customColormap();
  void changeBoundaryPhys(const int, const bool b);
  void cpointHelpClicked();
  public slots:
    void changeColorMap() {
      form.dynamic_cmap->setChecked(go->dynamic_cmap);
      form.reverse_cmap->setChecked(go->reverse_cmap);
      dens_color_bar->draw(form.dens_slide_min->value(),form.dens_slide_max->value());
    }
    void initCPointsTreeWidget();
    void selectTreeWidgetItem(int cpoint_id);
    void unselectTreeWidgetItem(int cpoint_id);
    void unselectTreeWidgetAll();
  private slots:
    void reject() {} // allow to de activate escape key to close the box
    // global slots
    void updateVisib(const bool _b,const int _r,const int _t);
    void updateRange(const QString&, const int, const int);
    void checkComboLine(const int, const int);
    // on range table
    int on_range_table_cellClicked(int,int);
    void on_range_table_cellPressed(int,int);
    void on_range_table_itemClicked(QTableWidgetItem * );
    // on particles
    void on_part_check_clicked(bool);
    void on_part_slide_size_valueChanged(int);
    void on_part_slide_alpha_valueChanged(int);
    // cpoints tab
    void on_load_cpoints_file_clicked(bool);
    void on_cpoints_display_cbx_stateChanged(int);
    void on_cpoints_set_treewidget_itemSelectionChanged();
    void on_cpoints_threshold_slider_valueChanged(int);
    void on_add_cpoint_btn_clicked(bool);
    void on_add_cpointset_clicked(bool);
    void on_shape_radio_disk_clicked(bool) {shapeRadioClicked();};
    void on_shape_radio_square_clicked(bool) {shapeRadioClicked();};
    void on_shape_radio_tag_clicked(bool) {shapeRadioClicked();};
    void on_shape_radio_sphere_clicked(bool) {shapeRadioClicked();};
    void shapeRadioClicked();
    void on_color_picker_button_clicked(bool);
    void on_export_cpoints_file_clicked(bool);
    void on_add_cpoint_center_coord_btn_clicked(bool);
    QTreeWidgetItem* createCpointsetTreeItem(CPointset*);
    void delete_cpointsets(bool need_confirmation);
    void delete_cpoints(bool need_confirmation);
    void on_edit_cpoint_coords_x_valueChanged(double);
    void on_edit_cpoint_coords_y_valueChanged(double);
    void on_edit_cpoint_coords_z_valueChanged(double);
    void on_edit_cpoint_size_valueChanged(double);
    void on_edit_cpointset_name_btn_clicked();
    void on_edit_cpoint_name_textChanged();
    void on_shape_show_name_cbx_stateChanged(int);
    void on_edit_shape_fill_ratio_valueChanged(int);
    void on_edit_shape_name_size_factor_valueChanged(int);
    void on_edit_shape_name_offset_valueChanged(int);
    void on_edit_shape_nb_sphere_sections_valueChanged(int);
    void on_edit_shape_name_angle_valueChanged(int);
    void setFormState(CPointset *pointset);
    void on_cpoint_help_clicked() {emit cpointHelpClicked();};
    // on gaz
    void on_gaz_check_clicked(bool);
    void on_gaz_slide_size_valueChanged(int);
    void on_gaz_slide_alpha_valueChanged(int);
    void on_gaz_rot_check_clicked(bool);
    void on_gaz_glsl_check_clicked(bool);
    void on_texture_spin_valueChanged(double);
    void on_texture_box_activated(int);
    // on velocity
    void on_vel_check_clicked(bool);
    void on_vel_slide_size_valueChanged(int);
    void on_vel_slide_alpha_valueChanged(int);
    // on velocity spin box
    void on_vel_spin_valueChanged(double);

    // -- Stretching Tab --
    void on_z_stretch_slide_valueChanged(int);
    void on_z_stretch_jit_cb_clicked(bool);
    void on_z_stretch_max_spin_valueChanged(double);

    // -- Orbits Tab --
    void on_orecord_check_clicked(bool);
    void on_odisplay_check_clicked(bool);
    void on_orbit_max_spin_valueChanged(int);
    void on_orbit_history_spin_valueChanged(int);

    // -- Physical quantities  Tab --
    void dens_slide_min_max(const int, const int);
    void on_dens_slide_min_valueChanged(int);
    void on_dens_slide_max_valueChanged(int);
    void on_phys_console_button_clicked();
    void setPhysicalTabName();
    void setNewPhys(bool update_glob=true);
    //
    void on_dens_phys_radio_clicked();
    void on_temp_phys_radio_clicked();
    void on_tempdens_phys_radio_clicked();
    void on_pressure_phys_radio_clicked();
    void on_velnorm_phys_radio_clicked();

    // colormap group
    void on_next_cmap_clicked() {
       emit nextColorMap();
    }
    void on_prev_cmap_clicked(){
       emit prevColorMap();
    }
    void on_custom_cmap_clicked() {

    }
    void on_dynamic_cmap_clicked(bool b) {
      emit constantColorMap(b);
    }
    void on_reverse_cmap_clicked(bool b) {
      emit reverseColorMap(b);
    }
    //
    void on_objects_properties_currentChanged(int index) {
      if (index==1) { // tab density
        //std::cerr << "index tab="<<index<<" dens_histo_view width="<<form.dens_histo_view->width()<<"\n";
        //form.dens_histo_view->resize(form.dens_histo_view->width(),form.dens_histo_view->height());
        form.dens_histo_view->repaint();
        dens_histo->drawDensity(form.dens_slide_min->value(),form.dens_slide_max->value());
        form.dens_bar_view->resize(form.dens_bar_view->width(),form.dens_bar_view->height());
        dens_color_bar->draw(form.dens_slide_min->value(),form.dens_slide_max->value());
      }
    }

  private:
    CPointset *getPointsetFromItem(QTreeWidgetItem *item);
    void checkPhysic();
    void physicalSelected();
    void leaveEvent ( QEvent * event ) {
      if (event) {;} // remove compiler warning
      emit leaveEvent();
    }
    void resizeEvent ( QResizeEvent * event ) {
      form.dens_histo_view->repaint();
      dens_histo->resizeEvent(event);
      dens_histo->drawDensity(form.dens_slide_min->value(),form.dens_slide_max->value());
      dens_color_bar->resizeEvent(event);
      dens_color_bar->draw(form.dens_slide_min->value(),form.dens_slide_max->value());
    }
    bool ignoreCloseEvent;
    Ui::FormObjectControl form;
    ParticlesData * current_data;
    ParticlesObjectVector * pov;
    CPointsetManager *pointset_manager;
    GlobalOptions * go;
    QComboBoxTable * combobox;
    QString cbox_text;
    // store object index
    int * object_index,nbody;
    // range table stuff
    void updateRangeTable(const int);
    // file table stuffs
    void updateFileTable(const int);
    // global stuff
    void updateTable(QTableWidget *, const int, const int, const int);
    void initTableWidget(QTableWidget *, const int, const int, const int);
    void resetTableWidget(QTableWidget *, const int, const int, const int);
    // object settings
    int current_object;  // current object selected
    void updateObjectSettings(const int);
    bool first;
    QMutex * mutex_data,my_mutex;
    QMutex * my_mutex2;
    bool lock;
    DensityHisto * dens_histo;
    DensityColorBar * dens_color_bar;
    PhysicalData * phys_select;
};

}

#endif
