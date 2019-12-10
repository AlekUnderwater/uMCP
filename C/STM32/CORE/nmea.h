

#ifndef _NMEA_H_
#define _NMEA_H_

#define NMEA_SNT_STR            '$'
#define NMEA_SNT_END            '\n'
#define NMEA_SNT_PST            "\r\n\0"
#define NMEA_PAR_SEP            ','
#define NMEA_CHK_SEP            '*'
#define NMEA_DEC_SEP            '.'

#define MCP_PREFIX_LEN          (4)
#define MCP_PREFIX              "PMCP\0"

#define IC_D2H_LACK             '0'        // $PMCP0,sentenceID,errCode  - local command ACK
#define IC_H2D_STRT             '1'        // $PMCP1,senderID,targetID,selectDefState,selIntMs,toutIntMs - restart protocol with specified params
#define IC_D2H_RACK             '2'        // $PMCP2,h--h // sent packet acknowledged
#define IC_D2H_RPKT             '3'        // $PMCP3,h--h // packet received
#define IC_H2D_SPKT             '4'        // $PMCP4,h--h // send packet
#define IC_D2H_STAT             '5'        // $PMCP5,state,select // protocol state changed

#define IC_UNKNOWN              '-'

void NMEA_PktCheckSum_Update(unsigned char* pkt, int size);
bool NMEA_PktCheckSum_Check(unsigned char* pkt, int size);
int NMEA_Ptn_Search(const unsigned char* pkt, int pktSize, const unsigned char* ptn, int ptnSize);

#endif
