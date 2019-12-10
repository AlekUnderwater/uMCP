#include <uMCP_types.h>
#include <ustr.h>
#include <nmea.h>

void NMEA_PktCheckSum_Update(unsigned char* pkt, int size)
{
	int i;
	unsigned char acc = 0, b1, b2;
	for (i = 0; i < size; i++)
	{
		if (pkt[i] == NMEA_SNT_STR) acc = 0;
		else if (pkt[i] == NMEA_CHK_SEP)
		{
			b1 = acc / 16;
			if (b1 > 9) b1 += ('A' - 10); else b1 += '0';
			b2 = acc % 16;
			if (b2 > 9) b2 += ('A' - 10); else b2 += '0';
			pkt[i + 1] = b1;
			pkt[i + 2] = b2;

		}
		else acc ^= pkt[i];
	}
}

bool NMEA_PktCheckSum_Check(unsigned char* pkt, int size)
{
	int i = 0;
	unsigned char acc = 0;
	bool result = false;

	while ((i < size) && (pkt[i] != NMEA_CHK_SEP))
	{
		if (pkt[i] == NMEA_SNT_STR) acc = 0;
		else acc ^= pkt[i];
		i++;
	}

	if (pkt[i] == NMEA_CHK_SEP) result = (acc == Str_ParseHexByte(pkt, i + 1));
	else result = true;

	return result;
}

int NMEA_Ptn_Search(const unsigned char* pkt, int pktSize, const unsigned char* ptn, int ptnSize)
{
    int i, j;
	int prefixPos = -1;

	i = 0;
	while ((prefixPos < 0) && (i < (pktSize - ptnSize)))
	{
		j = 0;
		while ((j < ptnSize) && (pkt[i + j] == ptn[j])) { j++; }
		if (j == ptnSize) prefixPos = i; else i++;
	}

	return prefixPos;
}
