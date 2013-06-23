#ifndef QWEB_CALLBACK_H
#define QWEB_CALLBACK_H

QW_RESULT QW_SessionAddrHandler(QW_CALLBACK_ACT State,int IntParam,QW_SESSION *pSession,QW_PACKET_HEADER *pPacket);
QW_RESULT QW_SessionQueryHandler(QW_CALLBACK_ACT State,int IntParam,QW_SESSION *pSession,QW_PACKET_HEADER *pPacket);
QW_RESULT QW_SessionInfoHandler(QW_CALLBACK_ACT State,int IntParam,QW_SESSION *pSession,QW_PACKET_HEADER *pPacket);
QW_RESULT QW_SessionQuitHandler(QW_CALLBACK_ACT State,int IntParam,QW_SESSION *pSession,QW_PACKET_HEADER *pPacket);
QW_RESULT QW_SessionDataHandler(QW_CALLBACK_ACT State,int IntParam,QW_SESSION *pSession,QW_PACKET_HEADER *pPacket);

#endif

