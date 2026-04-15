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

// texture
uniform float alpha;                   
uniform float factor_size;              
uniform int   use_point;       
uniform int   perspective;     

// colormap
uniform vec3 colormap[100]; 
uniform int ncmap;          
uniform float powalpha;     
uniform int reverse_cmap;   

// physical values
uniform int data_phys_valid;
uniform float data_phys_min; 
uniform float data_phys_max; 
uniform float zoom;

// special flag for spherical data
uniform int show_zneg;
uniform int coronograph;
uniform float viewport[4];
uniform float radius;

// stretching
uniform float z_stretch_value;
uniform int   z_stretch_jit;

// attributes (matching C++ glGetAttribLocation)
in vec3 position;
in float a_sprite_size;
in float a_phys_data;
// Note: color and texCoord are handled by fixed pipeline or uniforms in your C++
// but for 330 core we use uniforms or specific attributes.
uniform vec4 color; 

// outputs
out float v_to_discard;
out vec4 v_color;

// functions declaration
vec4 computeColor();
bool isVisible();
vec4 mod289(vec4 x);
vec4 perm(vec4 x);
float noise(vec3 p);

void main()                                                            
{           
  v_to_discard = 0.0;
  vec4 local_col;

  // compute color
  if (data_phys_valid == 1) {
    local_col = computeColor();
  } else {
    if (show_zneg == 1) {
      local_col = color;
    } else {
      float a_alpha = isVisible() ? color.a : 0.0;
      local_col = vec4(color.rgb, a_alpha);
    }
  }

  // compute vertex position
  vec4 vert = vec4(position.x, position.y, position.z * z_stretch_value, 1.0);
  if (z_stretch_jit == 1) {
    vert.z = vert.z + z_stretch_value * noise(position);
  }

  // compute Point Size
  float pSize = factor_size;
  if (use_point == 0) {
    pSize = a_sprite_size * factor_size;
    vec3 pos_eye = vec3(modelviewMatrix * vert);
    if (perspective == 1) {
      pSize = pSize / max(0.01, (1.0 - pos_eye.z));
    }
  }
  gl_PointSize = max(1.0, pSize);

  gl_Position = projMatrix * modelviewMatrix * vert;
  v_color = vec4(local_col.rgb, local_col.a * alpha);
}

vec4 computeColor() {
  vec4 res = vec4(color.rgb, 1.0);
  if (data_phys_valid == 1 && a_phys_data > 0.0) {
    float logpri = log(a_phys_data);
    float log_rho = 0.0;
    if (abs(data_phys_max - data_phys_min) > 0.0001) {
      log_rho = clamp((logpri - data_phys_min) / (data_phys_max - data_phys_min), 0.0, 1.0);
    }
    float fncmap = float(ncmap);
    float fcindex = (reverse_cmap == 0) ? log_rho * (fncmap - 1.0) : (fncmap - 1.0) - (log_rho * (fncmap - 1.0));
    int cindex = int(clamp(fcindex, 0.0, fncmap - 1.0));
    
    res.rgb = colormap[cindex];
    
    if (show_zneg == 1) {
      res.a = (log_rho > 0.0) ? pow(log_rho, powalpha) : 0.0;
    } else {
      vec3 pos_eye = vec3(modelviewMatrix * vec4(position, 1.0));
      if (log_rho > 0.0 && isVisible()) {
        res.a = pow(log_rho, powalpha);
      } else {
        res.a = 0.0;
        v_to_discard = 1.0;
      }
    }
  }
  return res;
}

bool isVisible() {
   vec4 vert = vec4(position.x, position.y, position.z * z_stretch_value, 1.0);
   if (z_stretch_jit == 1) vert.z = vert.z + z_stretch_value * noise(position);
   
   vec3 pos_eye = vec3(modelviewMatrix * vert);
   if ((pos_eye.z - zoom) > 0.0) {
       return (coronograph == 1) ? false : true;
   }
   if (radius > 0.0) {
       mat4 matbboard = modelviewMatrix;
       matbboard[0] = vec4(1.0, 0.0, 0.0, 0.0); 
       matbboard[1] = vec4(0.0, 1.0, 0.0, 0.0); 
       matbboard[2] = vec4(0.0, 0.0, 1.0, 0.0);
       
       vec4 pvert = (projMatrix * modelviewMatrix) * vert;
       vec4 pvori = (projMatrix * modelviewMatrix) * vec4(0.0, 0.0, 0.0, 1.0);
       vec4 pdisc = projMatrix * matbboard * vec4(radius, 0.0, 0.0, 1.0);

       vec2 s_pvert = (pvert.xy / pvert.w + 1.0) * vec2(viewport[2], viewport[3]) * 0.5 + vec2(viewport[0], viewport[1]);
       vec2 s_pvori = (pvori.xy / pvori.w + 1.0) * vec2(viewport[2], viewport[3]) * 0.5 + vec2(viewport[0], viewport[1]);
       vec2 s_pdisc = (pdisc.xy / pdisc.w + 1.0) * vec2(viewport[2], viewport[3]) * 0.5 + vec2(viewport[0], viewport[1]);

       return distance(s_pvert, s_pvori) > distance(s_pdisc, s_pvori);
   }
   return true;
}

vec4 mod289(vec4 x) { return x - floor(x * (1.0 / 289.0)) * 289.0; }
vec4 perm(vec4 x) { return mod289(((x * 34.0) + 1.0) * x); }
float noise(vec3 p){
    vec3 a = floor(p); vec3 d = p - a;
    d = d * d * (3.0 - 2.0 * d);
    vec4 b = vec4(a.x, a.x, a.y, a.y) + vec4(0.0, 1.0, 0.0, 1.0);
    vec4 k1 = perm(vec4(b.x, b.y, b.x, b.y)); 
    vec4 k2 = perm(vec4(k1.x, k1.y, k1.x, k1.y) + vec4(b.z, b.z, b.w, b.w));
    vec4 c = k2 + vec4(a.z, a.z, a.z, a.z);
    vec4 k3 = perm(c); vec4 k4 = perm(c + 1.0);
    vec4 o1 = fract(k3 * (1.0 / 41.0)); vec4 o2 = fract(k4 * (1.0 / 41.0));
    vec4 o3 = o2 * d.z + o1 * (1.0 - d.z);
    vec2 o4 = vec2(o3.y, o3.w) * d.x + vec2(o3.x, o3.z) * (1.0 - d.x);
    return o4.y * d.y + o4.x * (1.0 - d.y);
}
