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
	@author Jean-Charles Lambert <jean-charles.lambert@lam.fr>
 */
#ifndef GLNEMOGLOBJECTOSD_H
#define GLNEMOGLOBJECTOSD_H
#include <QObject>
#include <vector>
#include "gltextobject.h"
#include "gltextobject2.h"

namespace glnemo {
class GLTextObject;
class GLObjectOsd : public GLObject {
  Q_OBJECT
  public:
    GLObjectOsd(const int w, const int h,GLTextObject2 * _got2,
		const QColor &c=Qt::green, bool activated=TRUE);
   ~GLObjectOsd();

   enum OsdKeys {
     DataType  ,
     Title     ,
     Nbody     ,
     Time      ,
     Getdata   ,
     Zoom      ,
     Rot       ,
     Trans     ,
     Loading   ,
     Projection,
     n_OsdKeys
   };
  std::vector<GLTextObject2> * Osd_text;
  GLTextObject2 * got2;
  public slots:
    void setWH(int width, int height);
    void setFont(const std::string font);
    void setFont(const OsdKeys k, const std::string font);
    void setText(const OsdKeys k, const QString text);
    void setText(const OsdKeys k, const int);
    void setText(const OsdKeys k, const float);
    void setText(const OsdKeys k, const float, const float, const float);
    void setTextColor(const OsdKeys k,const QColor );
    void setColor(const QColor );
    void keysToggle(const OsdKeys k);
    void keysActivate(const OsdKeys k, const bool status);
    void updateDisplay();
    void updateDisplay(const OsdKeys k);
    void display(const int width, const int height);
    void updateColor(const QColor);
    
  private:
    static char * OsdText[n_OsdKeys];
    //fntRenderer font;
};

}

#endif
