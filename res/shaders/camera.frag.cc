// ============================================================================
// Copyright Jean-Charles LAMBERT - 2007-2017
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
// Fragment shader for billboarding particles
//
// ============================================================================

#version 120

uniform sampler2D splatTexture;

void main()
{
    gl_FragColor = gl_Color * texture2D(splatTexture, gl_TexCoord[0].st);
}
// ============================================================================