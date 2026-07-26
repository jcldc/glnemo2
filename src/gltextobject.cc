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
#include "gltextobject.h"
#include <glm/fwd.hpp>

namespace glnemo {

using namespace std;


GLTextObject::GLTextObject(bool activated):GLObject()
    {
      //dplist_index = glGenLists( 1 ); // create a new display List
      is_activated = activated;
      x = y = x_text = 0;
    }

  // ============================================================================
  // GLTextObject::setText()                                                     
  // set label and text                                                          
  void GLTextObject::setText(const QString &p_label,const QString& p_text)
  {
    label = p_label;
    text  = p_text;
  }
  // ============================================================================
  // GLTextObject::getLabelWidth()                                               
  // return label width in pixels                                                
  int GLTextObject::getLabelWidth()
  {
    float l,r,b,t;
    glnemo::TextBoundingBox box = m_gtr->getTextBoundingBox(label.toStdString().c_str(),x,y,1);
    return (box.width);
  }
  // ============================================================================
  // GLTextObject::getTextWidth()                                                
  // return text width in pixels                                                 
  int GLTextObject::getTextWidth()
  {
    float l,r,b,t;
    glnemo::TextBoundingBox box = m_gtr->getTextBoundingBox(text.toStdString().c_str(),x,y,1);
    return (box.width);
  }
  // ============================================================================
  // GLTextObject::getHeight()                                                   
  // return font height in pixels                                                
  int GLTextObject::getHeight()
  {
    float l,r,b,t;
    glnemo::TextBoundingBox box = m_gtr->getTextBoundingBox(text.toStdString().c_str(),x,y,1);
    return (box.height+3); // Add +3 pixels in height to seprate letters
  }
  // ============================================================================
  // GLTextObject::setPos()                                                      
  // specify new positions x and y (in pixels) for the given text                
  void GLTextObject::setPos(const int new_x, const int new_y, 
                            const int new_x_text)
  {
    x = new_x;
    y = new_y;
    x_text = new_x_text;
  }
  // ============================================================================
  // GLTextObject::display()                                                     
  // display text object if activated                                            
  void GLTextObject::display()
  {
    if (width) {;} // remove compiler warning
    if (is_activated) {   
      float r, g, b, a;
      mycolor.getRgbF(&r, &g, &b, &a);
      glm::vec4 gcolor(r,g,b,a);
      if (! text.toStdString().empty()) {
        m_gtr->draw(label.toStdString(),x,height-y,1,gcolor);
        // text
        m_gtr->draw(text.toStdString(),x_text,height-y,1,gcolor);
      }
    }
  }
}


