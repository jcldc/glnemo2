// ============================================================================
// Copyright Jean-Charles LAMBERT - 2007-2014
//           Yannick Dalbin
// e-mail:   Jean-Charles.Lambert@lam.fr
// address:  Dynamique des galaxies
//           Laboratoire d'Astrophysique de Marseille
//           P�le de l'Etoile, site de Ch�teau-Gombert
//           38, rue Fr�d�ric Joliot-Curie
//           13388 Marseille cedex 13 France
//           CNRS U.M.R 7326
// ============================================================================
// See the complete license in LICENSE and/or "http://www.cecill.info".
// ============================================================================
#include <QString>

#include "global.h"

namespace network {

    // ============================================================================
    // Any global propertise for assure communication between client and server
    // ============================================================================
    qint16 Global::HELLO                           = 1;
    qint16 Global::NBODY                          = 2;
    qint16 Global::TIME                              = 3;
    qint16 Global::SIZEARRAY                     = 4;
    qint16 Global::POS                               = 5;
    qint16 Global::VEL                                = 6;
    qint16 Global::SELECTBODY                  = 7;
    qint16 Global::SELECTVEL                     = 8;
    qint16 Global::ENDMESSAGES               = 9;
    qint16 Global::SERVEURFULL                 = 255;

    QString Global::IDENTCLIENT				= "GLNEMO-2-IAMREADY";
    QString Global::IDENTSERVEUR			= "GLNEMO-2-CONNECTION-OK";

}
