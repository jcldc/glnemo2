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
#include<QTcpSocket>
#include<QByteArray>
#include <assert.h>
#include "sendreseau.h"
#include <iostream>

namespace network {

    sendReseau::sendReseau() {

    }

    // ============================================================================
    // Set th socket to communicate
    // ============================================================================
    void sendReseau::setSocket(QTcpSocket* sock) {

        client = sock;

    }

    // ============================================================================
    // Send any type of data
    // ============================================================================
    void sendReseau::sendData(int taille, char *val) {

        client->write(val,taille);
        int bytes;
        while ((bytes=client->bytesToWrite())>0  && client->state() == QAbstractSocket::ConnectedState) {
            client->waitForBytesWritten(300);
        }

    }


}
