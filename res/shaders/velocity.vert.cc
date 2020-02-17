// ============================================================================
// Copyright Jean-Charles LAMBERT - 2007-2020                                  
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

#version 120

uniform mat4 modelviewMatrix;
uniform mat4 projMatrix;

uniform float vel_factor;

attribute vec3 position;
attribute vec3 velocity;

// stretching
uniform float z_stretch_value;
uniform int   z_stretch_jit;

// basic noise
vec4 mod289(vec4 x);
vec4 perm(vec4 x);
float noise(vec3 p);

// ============================================================================
void main()                                                            
{
  //vec4 o_position = vec4(position.x* vel_factor,position.y* vel_factor,position.z* vel_factor, 1.0f);
  vec4 vert = vec4(position.xy,position.z * z_stretch_value,1.0);
  if (z_stretch_jit==1) { // random value requested
    vert.z = vert.z + z_stretch_value * noise(position);
  }

//  vec4 o_position = vec4(vert.x+velocity.x*vel_factor,
//                         vert.y+velocity.y*vel_factor,
//                         vert.z+velocity.z*vel_factor,
//                         1.0);
  vec4 o_position = vec4(vert.xyz+velocity.xyz*vel_factor,1.0);
  gl_Position = projMatrix*modelviewMatrix * o_position;

}
// ---- N O I S E -----
vec4 mod289(vec4 x)
{
  return x - floor(x * (1.0 / 289.0)) * 289.0;
}
vec4 perm(vec4 x)
{
  return mod289(((x * 34.0) + 1.0) * x);
}
float noise(vec3 p){
    vec3 a = floor(p);
    vec3 d = p - a;
    d = d * d * (3.0 - 2.0 * d);

    vec4 b = a.xxyy + vec4(0.0, 1.0, 0.0, 1.0);
    vec4 k1 = perm(b.xyxy);
    vec4 k2 = perm(k1.xyxy + b.zzww);

    vec4 c = k2 + a.zzzz;
    vec4 k3 = perm(c);
    vec4 k4 = perm(c + 1.0);

    vec4 o1 = fract(k3 * (1.0 / 41.0));
    vec4 o2 = fract(k4 * (1.0 / 41.0));

    vec4 o3 = o2 * d.z + o1 * (1.0 - d.z);
    vec2 o4 = o3.yw * d.x + o3.xz * (1.0 - d.x);

    return o4.y * d.y + o4.x * (1.0 - d.y);
}
// ============================================================================
