// ============================================================================
// Copyright Jean-Charles LAMBERT - 2007-2016                                  
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

#version 120

uniform float alpha;           // alpha color factor
//uniform vec3 color;
varying vec4 col;

void main()
{
    gl_FragColor = vec4(col.xyz, alpha);
}
// ============================================================================
