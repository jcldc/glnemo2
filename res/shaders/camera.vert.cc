// ============================================================================
// Copyright Jean-Charles LAMBERT - 2007-2018
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
// Pixel shader for billboarding particles
//
//
// with ATI hardware, uniform variable MUST be used by output
// variables. That's why factor_size is used by gl_FrontColor
//
// !!!!!Attribute variable CAN'T be modified (ex: gl_Color)!!!!!!
//
// ============================================================================


#version 120

// Matrix
uniform mat4 modelviewMatrix;
uniform mat4 projMatrix;
uniform mat4 modelMatrix;
uniform mat4 viewMatrix;

attribute  float a_sprite_size;

void main()
{
  // alpha
  float alpha=1.0;

  // color
  vec4 col = vec4(gl_Color.r,gl_Color.g,gl_Color.b,gl_Color.a);

  // compute vertex Z value
  vec4 vert = vec4(gl_Vertex.xy,gl_Vertex.z,1.0);
  // size
  float point_size = a_sprite_size;
  gl_PointSize = point_size;
  //
  gl_TexCoord[0] = gl_MultiTexCoord0;
  gl_Position = projMatrix*modelviewMatrix * vec4(vert.xyz,1.0);
  gl_FrontColor =  vec4( col.x ,
                         col.y ,
                         col.z ,
                         col.w * alpha);
}
