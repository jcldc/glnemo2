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
// Geometry shader to display velocity vectors
// 
//
// !!!!!Attribute variable CAN'T be modified (ex: gl_Color)!!!!!!
//
// ============================================================================
#version 330 core

uniform float alpha;           // alpha color factor
//uniform mat4 modelviewMatrix, projMatrix;

in vec3 a_velocity[]; // velocity vector for each particles

layout (points) in;
layout (line_strip, max_vertices = 2) out;

in VS_OUT {
    vec4 color;
} gs_in[];

out vec4 fColor;
// ============================================================================
void buildVelocityVectors(vec4 position)
{           
    fColor = gs_in[0].color; // gs_in[0] since there's only one input vertex

    // draw first point
    //gl_Position = projMatrix * modelviewMatrix * position; // at the vertex position
    gl_Position = position; // at the vertex position
    EmitVertex();


    // draw second point
    vec3 vel =  a_velocity[0];
    //gl_Position = projMatrix * modelviewMatrix * (position + vec4(vel,1.0)); // at vertex position + vel vector
    gl_Position = (position + vec4(vel,1.0)); // at vertex position + vel vector
    EmitVertex();

    EndPrimitive();
}


void main()
{
  buildVelocityVectors(gl_in[0].gl_Position);
}

// ============================================================================
