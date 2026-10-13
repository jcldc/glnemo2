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

// Matrix
uniform mat4 modelviewMatrix;
uniform mat4 projMatrix;

// attribute
layout (location = 0) in vec3 position;
in float a_sprite_size;

// uniform color
uniform vec4 color;

out vec4 v_color;

void main()
{
  // NVIDIA compatibility: ensure gl_PointSize has a healthy positive float.
  float size = (a_sprite_size > 0.01) ? a_sprite_size : 5.0;
  gl_PointSize = size;

  gl_Position = projMatrix * modelviewMatrix * vec4(position.xyz, 1.0);
  v_color = color;
}
