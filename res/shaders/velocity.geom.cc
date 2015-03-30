// ============================================================================
// Copyright Jean-Charles LAMBERT - 2007-2015                                  
// e-mail:   Jean-Charles.Lambert@lam.fr                                      
// address:  Centre de donneeS Astrophysique de Marseille (CeSAM)              
//           Laboratoire d'Astrophysique de Marseille                          
//           Pôle de l'Etoile, site de Château-Gombert                         
//           38, rue Frédéric Joliot-Curie                                     
//           13388 Marseille cedex 13 France                                   
//           CNRS U.M.R 7326                                                   
// ============================================================================
// See the complete license in LICENSE and/or "http://www.cecill.info".        
// ============================================================================
//
// Pixel shader to display velocity vectors
// 
//
// with ATI hardware, uniform variable MUST be used by output          
// variables. That's why factor_size is used by gl_FrontColor    
//
// !!!!!Attribute variable CAN'T be modified (ex: gl_Color)!!!!!!
//
// ============================================================================
uniform float alpha;           // alpha color factor

attribute vec3 a_velocity; // velocity vector for each particles
// ============================================================================
void main()                                                            
{           
  vec4 col=vec4(0.0,0.0,0.0,0.0);
  col = vec4(gl_Color.r,gl_Color.g,gl_Color.b,gl_Color.a);

  gl_Position = ftransform();
  gl_FrontColor =  vec4( col.x ,
                         col.y ,
                         col.z ,
                         col.w * alpha);
}

// ============================================================================
