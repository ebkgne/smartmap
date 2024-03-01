#include "PacketCreator.h"

#include "CITPDefines.h"

#include <QtDebug>

PacketCreator::PacketCreator(QObject *parent)
  : QObject(parent)
{
}

PacketCreator::~PacketCreator()
{
}

unsigned char * PacketCreator::createPLocPacket(PLocType plocType, 
						const QString &name, 
						const QString &state, 
						int &bufferLen)
{
  QString typeString("unknown");
  switch (plocType)
    {
    case LightingConsole:
      typeString = "LightingConsole";
      break;
    case MediaServer:
      typeString = "MediaServer";
      break;
    case Visualizer:
      typeString = "Visualizer";
      break;
    case OperationHub:
      typeString = "OperationHub";
      break;
    }

  // figure out the packet size, all strings need to be NULL terminated
  bufferLen = sizeof(struct CITP_PINF_PLoc) + typeString.size() + 1 + name.size() + 1 + state.size() + 1;
  
  unsigned char *buffer = new unsigned char[bufferLen];
  memset(buffer, 0, bufferLen);

  CITP_PINF_PLoc *packet = (CITP_PINF_PLoc*)buffer;

  // CITP header
  packet->CITPPINFHeader.CITPHeader.Cookie = COOKIE_CITP;
  packet->CITPPINFHeader.CITPHeader.VersionMajor = 0x01;
  packet->CITPPINFHeader.CITPHeader.VersionMinor = 0x00;
  packet->CITPPINFHeader.CITPHeader.Reserved[0] = 0x00;
  packet->CITPPINFHeader.CITPHeader.Reserved[1] = 0x00; 
  packet->CITPPINFHeader.CITPHeader.MessageSize = bufferLen;
  packet->CITPPINFHeader.CITPHeader.MessagePartCount = 0x01;
  packet->CITPPINFHeader.CITPHeader.MessagePart = 0x01; // XXX - doc says 0-based?
  packet->CITPPINFHeader.CITPHeader.ContentType = COOKIE_PINF;

  // PINF header
  packet->CITPPINFHeader.ContentType = COOKIE_PINF_PLOC;

  // PLoc data
  packet->ListeningTCPPort = 0x00;
  
  // type
  int offset = sizeof(struct CITP_PINF_PLoc);
  memcpy(buffer + offset, typeString.toAscii().constData(), typeString.size());
  
  // name
  offset += typeString.size() + 1;
  memcpy(buffer + offset, name.toAscii().constData(), name.size());

  // state
  offset += name.size() + 1;
  memcpy(buffer + offset, state.toAscii().constData(), state.size());

  return buffer;
}

// returns NULL on error. bufferLen is the length of the returned buffer.
unsigned char * PacketCreator::createUNamPacket(quint8 universeIndex, 
						const QString &name,
						int &bufferLen)
{
  // figure out the packet size, all strings need to be NULL terminated
  bufferLen = sizeof(struct CITP_SDMX_UNam) + name.size() + 1;
  
  unsigned char *buffer = new unsigned char[bufferLen];
  memset(buffer, 0, bufferLen);

  CITP_SDMX_UNam *packet = (CITP_SDMX_UNam *)buffer;

  // CITP header
  packet->CITPSDMXHeader.CITPHeader.Cookie = COOKIE_CITP;
  packet->CITPSDMXHeader.CITPHeader.VersionMajor = 0x01;
  packet->CITPSDMXHeader.CITPHeader.VersionMinor = 0x00;
  packet->CITPSDMXHeader.CITPHeader.Reserved[0] = 0x00;
  packet->CITPSDMXHeader.CITPHeader.Reserved[1] = 0x00; 
  packet->CITPSDMXHeader.CITPHeader.MessageSize = bufferLen;
  packet->CITPSDMXHeader.CITPHeader.MessagePartCount = 0x01;
  packet->CITPSDMXHeader.CITPHeader.MessagePart = 0x01; // XXX - doc says 0-based?
  packet->CITPSDMXHeader.CITPHeader.ContentType = COOKIE_SDMX;

  // SDMX header
  packet->CITPSDMXHeader.ContentType = COOKIE_SDMX_UNAM;

  // UNam data
  packet->UniverseIndex = universeIndex;
  
  // name
  int offset = sizeof(struct CITP_SDMX_UNam);
  memcpy(buffer + offset, name.toAscii().constData(), name.size());

  return buffer;
}

// returns NULL on error. bufferLen is the length of the returned buffer.
unsigned char * PacketCreator::createChBkPacket(bool blind,
						quint8 universeIndex,
						quint16 firstChannelIndex,
						quint16 channelCount,
						const quint8 *channelLevels,
						int &bufferLen)
{
  // figure out the packet size, all strings need to be NULL terminated
  bufferLen = sizeof(struct CITP_SDMX_ChBk) + channelCount;
  
  unsigned char *buffer = new unsigned char[bufferLen];
  memset(buffer, 0, bufferLen);

  CITP_SDMX_ChBk *packet = (CITP_SDMX_ChBk *)buffer;

  // CITP header
  packet->CITPSDMXHeader.CITPHeader.Cookie = COOKIE_CITP;
  packet->CITPSDMXHeader.CITPHeader.VersionMajor = 0x01;
  packet->CITPSDMXHeader.CITPHeader.VersionMinor = 0x00;
  packet->CITPSDMXHeader.CITPHeader.Reserved[0] = 0x00;
  packet->CITPSDMXHeader.CITPHeader.Reserved[1] = 0x00; 
  packet->CITPSDMXHeader.CITPHeader.MessageSize = bufferLen;
  packet->CITPSDMXHeader.CITPHeader.MessagePartCount = 0x01;
  packet->CITPSDMXHeader.CITPHeader.MessagePart = 0x01; // XXX - doc says 0-based?
  packet->CITPSDMXHeader.CITPHeader.ContentType = COOKIE_SDMX;

  // SDMX header
  packet->CITPSDMXHeader.ContentType = COOKIE_SDMX_CHBK;

   // ChBk data
   packet->Blind = blind?0x01:0x00;
   packet->UniverseIndex = universeIndex;
   packet->FirstChannel = firstChannelIndex;
   packet->ChannelCount = channelCount;

  // channels
  int offset = sizeof(struct CITP_SDMX_ChBk);
  memcpy(buffer + offset, channelLevels, channelCount);

  return buffer;
}

// returns NULL on error. bufferLen is the length of the returned buffer.
unsigned char * PacketCreator::createSXSrPacket_ArtNet(int network,
						       int universe,
						       int channel,
						       int &bufferLen)
{
  QString str = ARTNET_STRING.arg(network).arg(universe).arg(channel);
  return createSXSrPacket(str, bufferLen);
}

// returns NULL on error. bufferLen is the length of the returned buffer.
unsigned char * PacketCreator::createSXSrPacket_Avab(int network,
						     int universe,
						     int channel,
						     int &bufferLen)
{
  QString str = AVAB_STRING.arg(network).arg(universe).arg(channel);
  return createSXSrPacket(str, bufferLen);
}

// returns NULL on error. bufferLen is the length of the returned buffer.
unsigned char * PacketCreator::createSXSrPacket_E131(int universe,
						     int channel,
						     int &bufferLen)
{
  QString str = E131_STRING.arg(universe).arg(channel);
  return createSXSrPacket(str, bufferLen);
}

// returns NULL on error. bufferLen is the length of the returned buffer.
unsigned char * PacketCreator::createSXSrPacket_ETCNet2(int channel,
							int &bufferLen)
{
  QString str = ETCNET2_STRING.arg(channel);
  return createSXSrPacket(str, bufferLen);
}


unsigned char * PacketCreator::createSXSrPacket(const QString &str, int &bufferLen)
{

  // figure out the packet size, all strings need to be NULL terminated
  bufferLen = sizeof(struct CITP_SDMX_SXSr) + str.size() + 1;
  
  unsigned char *buffer = new unsigned char[bufferLen];
  memset(buffer, 0, bufferLen);

  CITP_SDMX_SXSr *packet = (CITP_SDMX_SXSr *)buffer;

  // CITP header
  packet->CITPSDMXHeader.CITPHeader.Cookie = COOKIE_CITP;
  packet->CITPSDMXHeader.CITPHeader.VersionMajor = 0x01;
  packet->CITPSDMXHeader.CITPHeader.VersionMinor = 0x00;
  packet->CITPSDMXHeader.CITPHeader.Reserved[0] = 0x00;
  packet->CITPSDMXHeader.CITPHeader.Reserved[1] = 0x00; 
  packet->CITPSDMXHeader.CITPHeader.MessageSize = bufferLen;
  packet->CITPSDMXHeader.CITPHeader.MessagePartCount = 0x01;
  packet->CITPSDMXHeader.CITPHeader.MessagePart = 0x01; // XXX - doc says 0-based?
  packet->CITPSDMXHeader.CITPHeader.ContentType = COOKIE_SDMX;

  // SXSr header
  packet->CITPSDMXHeader.ContentType = COOKIE_SDMX_SXSR;

  // SXSr data
  int offset = sizeof(struct CITP_SDMX_SXSr);
  memcpy(buffer + offset, str.toAscii().constData(), str.size());

  return buffer;
}

unsigned char * PacketCreator::createPtchPacket(quint16 fixtureIdentifier, 
						quint8 universeIndex, 
						quint16 channelIndex,
						quint16 channelCount, 
						const QString &fixtureMake, 
						const QString &fixtureName,
						int &bufferLen)
{
  // figure out the packet size, all strings need to be NULL terminated
  int stringSize = 0;
  if (!fixtureMake.isEmpty())
    {
      stringSize += fixtureMake.size() + 1;
    }
  stringSize += fixtureName.size() + 1;
  bufferLen = sizeof(struct CITP_FPTC_Ptch) + stringSize;
  
  unsigned char *buffer = new unsigned char[bufferLen];
  memset(buffer, 0, bufferLen);

  CITP_FPTC_Ptch *packet = (CITP_FPTC_Ptch *)buffer;

  // CITP header
  packet->CITPFPTCHeader.CITPHeader.Cookie = COOKIE_CITP;
  packet->CITPFPTCHeader.CITPHeader.VersionMajor = 0x01;
  packet->CITPFPTCHeader.CITPHeader.VersionMinor = 0x00;
  packet->CITPFPTCHeader.CITPHeader.Reserved[0] = 0x00;
  packet->CITPFPTCHeader.CITPHeader.Reserved[1] = 0x00; 
  packet->CITPFPTCHeader.CITPHeader.MessageSize = bufferLen;
  packet->CITPFPTCHeader.CITPHeader.MessagePartCount = 0x01;
  packet->CITPFPTCHeader.CITPHeader.MessagePart = 0x01; // XXX - doc says 0-based?
  packet->CITPFPTCHeader.CITPHeader.ContentType = COOKIE_FPTC;

  // FPTC header
  packet->CITPFPTCHeader.ContentType = COOKIE_FPTC_PTCH;
  packet->CITPFPTCHeader.ContentHint = 0x00; // XXX - is this right?

   // Ptch data
  packet->FixtureIdentifier = fixtureIdentifier;
  packet->Universe = universeIndex;
  packet->Channel = channelIndex;
  packet->ChannelCount = channelCount;
  
  // fixture make
  int offset = sizeof(struct CITP_FPTC_Ptch);
  if (!fixtureMake.isEmpty())
    {
      memcpy(buffer + offset, fixtureMake.constData(), fixtureMake.size());
      offset += fixtureMake.size() + 1;
    }

  memcpy(buffer + offset, fixtureName.constData(), fixtureName.size());

  return buffer;
}

unsigned char * PacketCreator::createSPtcPacket(const QList<quint16> &fixtureIdentifiers, int &bufferLen)
{
  // figure out the packet size
  const quint16 fixtureCount = fixtureIdentifiers.size();
  bufferLen = sizeof(struct CITP_FPTC_SPtc) + (fixtureCount * sizeof(uint16));

  unsigned char *buffer = new unsigned char[bufferLen];
  memset(buffer, 0, bufferLen);

  CITP_FPTC_SPtc *packet = (CITP_FPTC_SPtc *)buffer;

  // CITP header
  packet->CITPFPTCHeader.CITPHeader.Cookie = COOKIE_CITP;
  packet->CITPFPTCHeader.CITPHeader.VersionMajor = 0x01;
  packet->CITPFPTCHeader.CITPHeader.VersionMinor = 0x00;
  packet->CITPFPTCHeader.CITPHeader.Reserved[0] = 0x00;
  packet->CITPFPTCHeader.CITPHeader.Reserved[1] = 0x00; 
  packet->CITPFPTCHeader.CITPHeader.MessageSize = bufferLen;
  packet->CITPFPTCHeader.CITPHeader.MessagePartCount = 0x01;
  packet->CITPFPTCHeader.CITPHeader.MessagePart = 0x01; // XXX - doc says 0-based?
  packet->CITPFPTCHeader.CITPHeader.ContentType = COOKIE_FPTC;

  // FPTC header
  packet->CITPFPTCHeader.ContentType = COOKIE_FPTC_SPTC;
  packet->CITPFPTCHeader.ContentHint = 0x00000000; // XXX - how should we figure this out?

   // SPtc data
  packet->FixtureCount = fixtureCount;
  int offset = sizeof(struct CITP_FPTC_SPtc);
  foreach (quint16 fixId, fixtureIdentifiers)
    {
      buffer[offset++] = fixId & 0xFF;
      buffer[offset++] = (fixId >> 8) & 0xFF;
    }

  return buffer;
}

unsigned char * PacketCreator::createUPtcPacket(const QList<quint16> &fixtureIdentifiers, int &bufferLen)
{
  // figure out the packet size
  const quint16 fixtureCount = fixtureIdentifiers.size();
  bufferLen = sizeof(struct CITP_FPTC_UPtc) + (fixtureCount * sizeof(uint16));

  unsigned char *buffer = new unsigned char[bufferLen];
  memset(buffer, 0, bufferLen);

  CITP_FPTC_UPtc *packet = (CITP_FPTC_UPtc *)buffer;

  // CITP header
  packet->CITPFPTCHeader.CITPHeader.Cookie = COOKIE_CITP;
  packet->CITPFPTCHeader.CITPHeader.VersionMajor = 0x01;
  packet->CITPFPTCHeader.CITPHeader.VersionMinor = 0x00;
  packet->CITPFPTCHeader.CITPHeader.Reserved[0] = 0x00;
  packet->CITPFPTCHeader.CITPHeader.Reserved[1] = 0x00; 
  packet->CITPFPTCHeader.CITPHeader.MessageSize = bufferLen;
  packet->CITPFPTCHeader.CITPHeader.MessagePartCount = 0x01;
  packet->CITPFPTCHeader.CITPHeader.MessagePart = 0x01; // XXX - doc says 0-based?
  packet->CITPFPTCHeader.CITPHeader.ContentType = COOKIE_FPTC;

  // FPTC header
  packet->CITPFPTCHeader.ContentType = COOKIE_FPTC_UPTC;
  packet->CITPFPTCHeader.ContentHint = 0x00000000; // XXX - how should we figure this out?

   // SPtc data
  packet->FixtureCount = fixtureCount;
  int offset = sizeof(struct CITP_FPTC_UPtc);
  foreach (quint16 fixId, fixtureIdentifiers)
    {
      buffer[offset++] = fixId & 0xFF;
      buffer[offset++] = (fixId >> 8) & 0xFF;
    }

  return buffer;
}

unsigned char * PacketCreator::createSelePacket(const QList<quint16> &fixtureIdentifiers, 
						bool complete, int &bufferLen)
{
  // figure out the packet size
  const quint16 fixtureCount = fixtureIdentifiers.size();
  bufferLen = sizeof(struct CITP_FSEL_Sele) + (fixtureCount * sizeof(uint16));

  unsigned char *buffer = new unsigned char[bufferLen];
  memset(buffer, 0, bufferLen);

  CITP_FSEL_Sele *packet = (CITP_FSEL_Sele *)buffer;

  // CITP header
  packet->CITPFSELHeader.CITPHeader.Cookie = COOKIE_CITP;
  packet->CITPFSELHeader.CITPHeader.VersionMajor = 0x01;
  packet->CITPFSELHeader.CITPHeader.VersionMinor = 0x00;
  packet->CITPFSELHeader.CITPHeader.Reserved[0] = 0x00;
  packet->CITPFSELHeader.CITPHeader.Reserved[1] = 0x00; 
  packet->CITPFSELHeader.CITPHeader.MessageSize = bufferLen;
  packet->CITPFSELHeader.CITPHeader.MessagePartCount = 0x01;
  packet->CITPFSELHeader.CITPHeader.MessagePart = 0x01; // XXX - doc says 0-based?
  packet->CITPFSELHeader.CITPHeader.ContentType = COOKIE_FSEL;

  // FSEL header
  packet->CITPFSELHeader.ContentType = COOKIE_FSEL_SELE;

   // Sele data
  packet->Complete = complete?0xFF:0x00;
  packet->FixtureCount = fixtureCount;
  int offset = sizeof(struct CITP_FSEL_Sele);
  foreach (quint16 fixId, fixtureIdentifiers)
    {
      buffer[offset++] = fixId & 0xFF;
      buffer[offset++] = (fixId >> 8) & 0xFF;
    }

  return buffer;
}

unsigned char * PacketCreator::createDeSePacket(const QList<quint16> &fixtureIdentifiers, int &bufferLen)
{
  // figure out the packet size
  const quint16 fixtureCount = fixtureIdentifiers.size();
  bufferLen = sizeof(struct CITP_FSEL_DeSe) + (fixtureCount * sizeof(uint16));

  unsigned char *buffer = new unsigned char[bufferLen];
  memset(buffer, 0, bufferLen);

  CITP_FSEL_DeSe *packet = (CITP_FSEL_DeSe *)buffer;

  // CITP header
  packet->CITPFSELHeader.CITPHeader.Cookie = COOKIE_CITP;
  packet->CITPFSELHeader.CITPHeader.VersionMajor = 0x01;
  packet->CITPFSELHeader.CITPHeader.VersionMinor = 0x00;
  packet->CITPFSELHeader.CITPHeader.Reserved[0] = 0x00;
  packet->CITPFSELHeader.CITPHeader.Reserved[1] = 0x00; 
  packet->CITPFSELHeader.CITPHeader.MessageSize = bufferLen;
  packet->CITPFSELHeader.CITPHeader.MessagePartCount = 0x01;
  packet->CITPFSELHeader.CITPHeader.MessagePart = 0x01; // XXX - doc says 0-based?
  packet->CITPFSELHeader.CITPHeader.ContentType = COOKIE_FSEL;

  // FSEL header
  packet->CITPFSELHeader.ContentType = COOKIE_FSEL_DESE;

   // Dese data
  packet->FixtureCount = fixtureCount;
  int offset = sizeof(struct CITP_FSEL_DeSe);
  foreach (quint16 fixId, fixtureIdentifiers)
    {
      buffer[offset++] = fixId & 0xFF;
      buffer[offset++] = (fixId >> 8) & 0xFF;
    }

  return buffer;
}
