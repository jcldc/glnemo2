// ============================================================================
// Copyright Jean-Charles LAMBERT - 2007-2026                                  
// e-mail:   Jean-Charles.Lambert@lam.fr                                      
// address:  Centre de donneeS Astrophysique de Marseille (CeSAM)              
//           Laboratoire d'Astrophysique de Marseille                          
//           Pôle de l'Etoile, site de Château-Gombert                         
//           38, rue Frédéric Joliot-Curie                                     
//           13388 Marseille cedex 13 France                                   
//           CNRS U.M.R 7326                                                   
// ============================================================================
#version 330 core

uniform sampler2D splatTexture;
uniform int use_point;

in float v_to_discard;
in vec4 v_color;

out vec4 fragColor;

void main()                                                            
{           
  if (v_to_discard > 0.5) {
    discard;
  }
  
  // On récupère la texture systématiquement pour éviter que l'uniforme soit optimisé
  vec4 tex = texture(splatTexture, gl_PointCoord);

  if (use_point == 1) {
    // Rounding points with no textures, with smooth edge
    vec2 coord = gl_PointCoord - vec2(0.5);
    float dist = length(coord) * 2.0; // normalisé : 0 au centre, 1 au bord
    
    if (dist > 1.0) {
      discard; // transparent outside of the circle
    }
    
    // gaussien profile : alpha strong at the center, decrease gently 
    float density = exp(-4.0 * dist * dist); // ajuste le "4.0" pour la largeur du profil
    fragColor = vec4(v_color.rgb, v_color.a * density);

  } else {
    // Si NVIDIA renvoie (0,0) pour gl_PointCoord, tex sera la couleur du pixel (0,0) de la texture
    fragColor = v_color * tex;
  }
}
// ============================================================================
