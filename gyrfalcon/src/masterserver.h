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
#ifndef MASTERSERVER_H
#define MASTERSERVER_H

#include <QTcpServer>
#include <QMutex>
#include <QWaitCondition>
#include <body.h>
#include <client.h>

class MasterServer : public QTcpServer {


    public:
        MasterServer(int port, int maxConnexion, std::string sim_name, const falcON::snapshot * S);
        ~MasterServer();
        void updateData();
        void startList();

    private:
        int maxConnexion;
        int nbConnexionFree;
        int port;
        QList<Client *> clients;
        QList<QString> lstConnected;
        std::string sim_name;
        const falcON::snapshot * S;
        QMutex mutex;
        QWaitCondition cond;



};

#endif // MASTERSERVER_H
