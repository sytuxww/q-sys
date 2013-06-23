#ifndef Q_WEB_TRANSMISSION_H
#define Q_WEB_TRANSMISSION_H

#include "QWebPacket.h"

bool QW_HwSetup(void);
u8 QW_SendPacket(QW_PACKET_HEADER *pPacket);
u8 QW_RecvPacket(QW_PACKET_HEADER *pPacket);
void QWeb_SetAddrMode(u8 MyAddr,bool RecvBroadcost);

#endif

