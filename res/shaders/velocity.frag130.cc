// ============================================================================
// Copyright Jean-Charles LAMBERT - 2007-2023                                  
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
// Fragment shader for velocity vectors
// 
// ============================================================================

#version 130

uniform float alpha;           // alpha color factor
out vec4 fcolor;
uniform vec3 color;

void main()
{
    fcolor = vec4(color.xyz, alpha);
}
// ============================================================================
