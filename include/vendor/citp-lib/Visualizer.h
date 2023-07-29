// -*- mode: C++ -*-
#ifndef _VISUALIZER_H_
#define _VISUALIZER_H_

#include "Peer.h"

class Visualizer : public Peer
{
  Q_OBJECT
public:
  Visualizer(const QHostAddress &host, quint16 listeningPort, 
	     const QString &peerName, const QString &peerState, QObject *parent=0);
  virtual ~Visualizer();

  bool sendUniverseName(quint8 universeIndex, const QString &universeName);

  bool sendChannelData(bool blind, quint8 universeIndex, 
		       quint16 firstChannelIndex, quint16 channelCount, 
		       const quint8 *channelLevels);

  bool sendPatchMessage(quint16 fixtureIdentifier, quint8 universeIndex, quint16 channelIndex,
			quint16 channelCount, const QString &fixtureMake, const QString &fixtureName);

  // send empty list to unpatch all fixtures
  bool sendUnpatchMessage(const QList<quint16> &fixtureIdentifiers);

  // send empty list to request entire patch
  bool sendPatchRequest(const QList<quint16> &fixtureIdentifiers);

  bool sendSelectMessage(bool complete, const QList<quint16> &fixtureIdentifiers);
  bool sendDeselectMessage(const QList<quint16> &fixtureIdentifiers);

  // Alternate DMX source management
  bool sendAlternateSourceMessage_ArtNet(int network, int universe, int channel);
  bool sendAlternateSourceMessage_Avab(int network, int universe, int channel);
  bool sendAlternateSourceMessage_E131(int universe, int channel);
  bool sendAlternateSourceMessage_ETCNet2(int channel);


};

#endif // _VISUALIZER_H_
