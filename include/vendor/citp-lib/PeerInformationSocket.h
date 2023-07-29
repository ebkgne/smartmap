// -*- mode: C++ -*-
#ifndef _PEERINFORMATIONSOCKET_H_
#define _PEERINFORMATIONSOCKET_H_

#include <QUdpSocket>

class QTimer;
class Peer;

struct PeerDescripton;

class PeerInformationSocket : public QUdpSocket
{
  Q_OBJECT
public:
  PeerInformationSocket(QObject *parent=0);
  virtual ~PeerInformationSocket();

  bool init(const QString &name, const QString &state);

  // XXX - need updateState(QString)

  bool listPeers(QList<Peer*> &peerList);

private slots:
  void transmitPLoc();
  void handleReadReady();

private:
  QString m_name;
  QString m_state;

  QTimer *m_timer;

  unsigned char *m_packetBuffer;
  int m_packetBufferLen;

  void processPacket(const QHostAddress &sender, const QByteArray &packet);

  void addPeer(const QHostAddress &host, quint16 listeningPort, const QString &typeString, 
       const QString &name, const QString &state);
  QList<Peer*> m_peerList;

 signals:
  void peersUpdated();
};

#endif // _PEERINFORMATIONSOCKET_H_
