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
#ifndef GLOBAL_H
#define GLOBAL_H

namespace network {

    class Global {

        public:
            static qint16 HELLO;
            static qint16 TIME;
            static qint16 NBODY;
            static qint16 SIZEARRAY;
            static qint16 POS;
            static qint16 VEL;
            static qint16 SELECTBODY;
            static qint16 SELECTVEL;
            static qint16 ENDMESSAGES;
            static qint16 SERVEURFULL;


            static QString IDENTCLIENT;
            static QString IDENTSERVEUR;

    };

}

#endif // GLOBAL_H
