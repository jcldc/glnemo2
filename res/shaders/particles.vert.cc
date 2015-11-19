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

// vertex position
attribute vec3 position;

// texture
uniform float alpha;           // alpha color factor                   
uniform float factor_size;     // texture size factor                  
uniform int   use_point;       // false=texture, true=point
uniform int   perspective;     // false=orthographic, true=perspective

// colormap
uniform vec3 colormap[100]; // rgb colormap
uniform int ncmap;          // colormap length
uniform float powalpha;     // alpha power value
uniform int reverse_cmap;   // use reversed color map ?

// physical values
uniform int data_phys_valid;// Is data phys valid ? 
uniform float data_phys_min; // minimum physical value
uniform float data_phys_max; // maximum physical value
uniform float zoom;

// special flag for spherical data
uniform int show_zneg;
uniform int coronograph;
uniform float viewport[4];
uniform float radius;

// stretching
uniform float z_stretch_value;
uniform int   z_stretch_jit;

// attribute values for all the vertex
attribute float a_sprite_size; // a different value for each particles 
attribute float a_phys_data;   // physical data value for each particles

// varying variable
//varying vec4 col;
varying float to_discard;

// functions declaration
vec4 computeColor();
bool isVisible();
// noise function
vec4 permute(vec4 x);
vec4 taylorInvSqrt(vec4 r);
float snoise(vec3 v);
// ============================================================================
void main()                                                            
{           
  vec4 col=vec4(0.0,0.0,0.0,0.0);
  to_discard=0.0;
  // compute color
  if (data_phys_valid==1) {
    col=computeColor();
  } else {
    if (show_zneg==1) {
      col = vec4(gl_Color.r,gl_Color.g,gl_Color.b,gl_Color.a);   
    } else {
      float a_alpha;
      if (isVisible()) {
	a_alpha = gl_Color.a;
      } else {
	a_alpha = 0.0;
      }
      col = vec4(gl_Color.r,gl_Color.g,gl_Color.b,a_alpha);   
    }
  }

  // compute texture size
  vec4 vert = vec4(position.xy,position.z * z_stretch_value,1.0);
  if (z_stretch_jit==1) { // random value requested
    vert.z = vert.z + z_stretch_value * snoise(position);
  }

  if (use_point==1) { // use point = same size whatever the distance from observer
    float pointSize =  a_sprite_size/a_sprite_size*factor_size;
    gl_PointSize = max(2., pointSize*1.);
  } 
  else {           // use texture, size change according to the distance
    float pointSize =  a_sprite_size*factor_size;
    vec3 pos_eye = vec3 ( modelviewMatrix * vert);
    if (perspective==1) {
      gl_PointSize = max(0.0000001, pointSize / (1.0 - pos_eye.z));
    } else {
      gl_PointSize = max(0.0000001, pointSize - pos_eye.z + pos_eye.z);
    }
  }
  gl_TexCoord[0] = gl_MultiTexCoord0;
  gl_Position = projMatrix*modelviewMatrix * vec4(vert.xyz,1.0);
//  if (1==0) {
//    gl_FrontColor =  vec4( gl_Color.r+col.x +float(factor_size)*0. + float(use_point)*0.,          
//                           gl_Color.g+col.y                                             ,         
//                           gl_Color.b+col.z                                             ,         
//                           (gl_Color.a+col.w) * alpha);
//  } else {
    gl_FrontColor =  vec4( col.x ,          
                           col.y ,         
                           col.z ,         
                           col.w * alpha);
  //}
}

// ============================================================================
vec4 computeColor() {
  vec4 col;
  
  if (data_phys_valid==1 && a_phys_data>0.0) {
    float logpri=log(a_phys_data);
    float log_rho=0.0;
            
    if ( (logpri >= data_phys_min) &&
         (logpri <= data_phys_max) &&
         (data_phys_max - data_phys_min) != 0.0) {
      log_rho = (logpri  - data_phys_min) / ( data_phys_max - data_phys_min);
    }
    // use float to avoid too many casting
    float fcindex;
    float fncmap=float(ncmap);
    
    if (reverse_cmap==0) { // normal colormap
      fcindex = log_rho*(fncmap-1.);
    } else {             // reverse colormap
      fcindex = fncmap-1.-(log_rho*(fncmap-1.));
    }
    int cindex;
    cindex=int(min(fcindex,fncmap-1.));
    cindex=int(max(0.,fcindex));
    //cindex=int(max(0.,cindex));
    col.x = colormap[cindex].x;    // red
    col.y = colormap[cindex].y;    // green
    col.z = colormap[cindex].z;    // blue
    vec4 vert = gl_Vertex;
    if (show_zneg==1) {
      if (log_rho>0.0)
        col.w = pow(log_rho,powalpha); // alpha
      else
        col.w = 0.;
     
    } else {
      // special test for spherical data (like SUN)
      // we display only postive z values
      vec3 pos_eye = vec3 (gl_ModelViewMatrix * vert);

      //if (log_rho>0.0 && (pos_eye.z-zoom)>0.0 && checkPartInDisc()) //vert.z>0.0)
      if (log_rho>0.0 && isVisible()) //vert.z>0.0)
        col.w = pow(log_rho,powalpha); // alpha
      else {
        col.w = 0.;
        to_discard=1.0;
      }
    }
      
  } else {
    col = vec4(gl_Color.r,gl_Color.g,gl_Color.b,1.0);
  }  
  return col;
}
// ============================================================================
// check if particles is off the disc
bool isVisible()
{
   bool ret=false;
   vec4 vert = vec4(position.xy,position.z * z_stretch_value,1.0);
   if (z_stretch_jit==1) { // random value requested
     vert.z = vert.z + z_stretch_value * snoise(position);
   }
   // transformation from the camera
   vec3 pos_eye = vec3 (gl_ModelViewMatrix * vert);

   if ((pos_eye.z-zoom)>0.0) { // particles front of the disc
       if (coronograph==1)
           ret=false;   // display an opaque disc in any case
       else
           ret=true;    // display particles front of the disc
   } else {
       if (radius > 0.) {
         // world vertex
         //vec4 vert = vec4(position,1.0);               // particles
         vec4 vori = vec4(0.    , 0., 0., 0.);  // center 0,0,0
         vec4 disc = vec4(radius, 0., 0., 0.);  // disc radius

         // billboarding matrix for the disc
         // we reset rotations
         mat4 matbboard = gl_ModelViewMatrix;
         for (int i=0; i<3; i++) {
             for (int j=0; j<3; j++) {
                 if (i==j) matbboard[i][j] = 1.0;
                 else      matbboard[i][j] = 0.0;
             }
         }

         // projected vertex
         vec4 pvert = gl_ModelViewProjectionMatrix * vert;
         vec4 pvori = gl_ModelViewProjectionMatrix * vori;
         vec4 pdisc = gl_ProjectionMatrix * matbboard * disc;

         // screen coordinates
         pdisc.x = (1.+pdisc.x) * viewport[2]/2. + viewport[0];
         pdisc.y = (1.+pdisc.y) * viewport[3]/2. + viewport[1];
         pvert.x = (1.+pvert.x) * viewport[2]/2. + viewport[0];
         pvert.y = (1.+pvert.y) * viewport[3]/2. + viewport[1];
         pvori.x = (1.+pvori.x) * viewport[2]/2. + viewport[0];
         pvori.y = (1.+pvori.y) * viewport[3]/2. + viewport[1];

         // distance
         float ddisc = distance(pdisc.xy,pvori.xy);
         float dvert = distance(pvert.xy,pvori.xy);
         if ((dvert)>(ddisc)) { // particles projection is off the disc
             ret=true;
         }
       }
   }
   return ret;
}

// ---- N O I S E -----
vec4 permute(vec4 x)
{
  return mod(((x*34.0)+1.0)*x, 289.0);
}

vec4 taylorInvSqrt(vec4 r)
{
  return 1.79284291400159 - 0.85373472095314 * r;
}

float snoise(vec3 v)
  {
  const vec2  C = vec2(1.0/6.0, 1.0/3.0) ;
  const vec4  D = vec4(0.0, 0.5, 1.0, 2.0);

// First corner
  vec3 i  = floor(v + dot(v, C.yyy) );
  vec3 x0 =   v - i + dot(i, C.xxx) ;

// Other corners
  vec3 g = step(x0.yzx, x0.xyz);
  vec3 l = 1.0 - g;
  vec3 i1 = min( g.xyz, l.zxy );
  vec3 i2 = max( g.xyz, l.zxy );

  //  x0 = x0 - 0. + 0.0 * C
  vec3 x1 = x0 - i1 + 1.0 * C.xxx;
  vec3 x2 = x0 - i2 + 2.0 * C.xxx;
  vec3 x3 = x0 - 1. + 3.0 * C.xxx;

// Permutations
  i = mod(i, 289.0 );
  vec4 p = permute( permute( permute(
             i.z + vec4(0.0, i1.z, i2.z, 1.0 ))
           + i.y + vec4(0.0, i1.y, i2.y, 1.0 ))
           + i.x + vec4(0.0, i1.x, i2.x, 1.0 ));

// Gradients
// ( N*N points uniformly over a square, mapped onto an octahedron.)
  float n_ = 1.0/7.0; // N=7
  vec3  ns = n_ * D.wyz - D.xzx;

  vec4 j = p - 49.0 * floor(p * ns.z *ns.z);  //  mod(p,N*N)

  vec4 x_ = floor(j * ns.z);
  vec4 y_ = floor(j - 7.0 * x_ );    // mod(j,N)

  vec4 x = x_ *ns.x + ns.yyyy;
  vec4 y = y_ *ns.x + ns.yyyy;
  vec4 h = 1.0 - abs(x) - abs(y);

  vec4 b0 = vec4( x.xy, y.xy );
  vec4 b1 = vec4( x.zw, y.zw );

  //vec4 s0 = vec4(lessThan(b0,0.0))*2.0 - 1.0;
  //vec4 s1 = vec4(lessThan(b1,0.0))*2.0 - 1.0;
  vec4 s0 = floor(b0)*2.0 + 1.0;
  vec4 s1 = floor(b1)*2.0 + 1.0;
  vec4 sh = -step(h, vec4(0.0));

  vec4 a0 = b0.xzyw + s0.xzyw*sh.xxyy ;
  vec4 a1 = b1.xzyw + s1.xzyw*sh.zzww ;

  vec3 p0 = vec3(a0.xy,h.x);
  vec3 p1 = vec3(a0.zw,h.y);
  vec3 p2 = vec3(a1.xy,h.z);
  vec3 p3 = vec3(a1.zw,h.w);

//Normalise gradients
  vec4 norm = taylorInvSqrt(vec4(dot(p0,p0), dot(p1,p1), dot(p2, p2), dot(p3,p3)));
  p0 *= norm.x;
  p1 *= norm.y;
  p2 *= norm.z;
  p3 *= norm.w;

// Mix final noise value
  vec4 m = max(0.6 - vec4(dot(x0,x0), dot(x1,x1), dot(x2,x2), dot(x3,x3)), 0.0);
  m = m * m;
  return 42.0 * dot( m*m, vec4( dot(p0,x0), dot(p1,x1),
                                dot(p2,x2), dot(p3,x3) ) );
  }
