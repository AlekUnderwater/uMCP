// (C) Aleksandr Dikarev, 2019

#include <uMCP.h>

DCParams_Struct dcParams;

// nmea buffer from HOST
unsigned char ni_buffer[UMCP_NBUFFER_SIZE];
volatile unsigned int ni_cnt = 0;
volatile bool ni_ready = false;
volatile bool ni_pkt_start = false;

unsigned char no_buffer[UMCP_NBUFFER_SIZE];
unsigned int no_cnt = 0;
bool no_ready = false;

// buffer from HOST
unsigned char ih_ring[CR_RING_SIZE];
volatile unsigned int ih_rPos = 0;
volatile unsigned int ih_wPos = 0;
volatile unsigned int ih_Cnt = 0;
volatile unsigned int ih_TS = 0;

// buffer to HOST
unsigned char oh_buffer[UMCP_NBUFFER_SIZE];
unsigned int oh_cnt = 0;
bool oh_ready = false;

// buffer to LINE
unsigned char ol_buffer[UMCP_LBUFFER_SIZE];
unsigned int ol_cnt = 0;
bool ol_ready = false;

// incoming packet
unsigned char ip_datablock[UMCP_MAX_DATA_SIZE];
volatile bool ip_start = false;
volatile int ip_pos = 0;
volatile unsigned char ip_sid = 0;
volatile unsigned char ip_tid = 0;
volatile uMCP_PacketType ip_type = uMCP_PTYPE_INVALID;
volatile unsigned char ip_tcnt = 0;
volatile unsigned char ip_rcnt = 0;
volatile unsigned char ip_ahchk = 0;
volatile unsigned char ip_dcnt = 0;
volatile bool ip_ready = false;

unsigned char R, N, A;
bool selectDefaultState = CFG_SELECT_DEFAULT_STATE;
bool select = CFG_SELECT_DEFAULT_STATE;
unsigned char SID = 0, TID = 0;
unsigned char sid, tid, tcnt, rcnt, dcnt;
uMCP_PacketType pType;
uMCP_State state;
bool sack = false;
bool srep = false;
bool isTimerPendingOnTxFinish = false;
int lineBaudRate = CFG_LINEBAUDRATE;

unsigned int iTimer_Interval_tks[uMCP_Timer_INVALID];
unsigned int iTimer_ExpTime_tks[uMCP_Timer_INVALID];
bool iTimer_State[uMCP_Timer_INVALID];

unsigned char sentBlocksSize[255];
unsigned int sentBlocksRPos[255];
unsigned char sentBlocksCnt = 0;

// uMCP

// Interval timers management routines
void uMCP_ITimer_Init(uMCP_Timer_ID timerID, unsigned int interval_ms, bool istate)
{
	iTimer_Interval_tks[timerID] = MS2TKS(interval_ms);
	uMCP_ITimer_StateSet(timerID, istate);
}

void uMCP_ITimer_StateSet(uMCP_Timer_ID timerID, bool value)
{
	if (value)
	{
		iTimer_ExpTime_tks[timerID] = TAL_Ticks() + iTimer_Interval_tks[timerID];
		iTimer_State[timerID] = true;
	}
	else
	{
		iTimer_State[timerID] = false;
	}
}

void uMCP_ITimer_Expired(uMCP_Timer_ID timerID)
{
	if (timerID == uMCP_Timer_TMO)
	{
		uMCP_ITimer_StateSet(uMCP_Timer_TX, false);
		if (state == uMCP_STATE_ISTART)
		{
			uMCP_CtrlSend(uMCP_PTYPE_STR, 0, 0, true);
		}
		else if (state == uMCP_STATE_ASTART)
		{
			uMCP_CtrlSend(uMCP_PTYPE_STA, 0, 0, true);
		}
		else if (state == uMCP_STATE_RUNNING)
		{
			srep = true;
		}
	}
	else if (timerID == uMCP_Timer_SELECT)
	{
		uMCP_SELECT_Set(CFG_SELECT_DEFAULT_STATE);
	}
	else if (timerID == uMCP_Timer_TX)
	{
		if (isTimerPendingOnTxFinish && !iTimer_State[uMCP_Timer_TMO])
		{
			isTimerPendingOnTxFinish = false;
			uMCP_ITimer_StateSet(uMCP_Timer_TMO, true);
		}
	}
}

void uMCP_ITimers_Process()
{
	int tIdx = 0;
	for (tIdx = 0; tIdx < uMCP_Timer_INVALID; tIdx++)
	{
		if (iTimer_State[tIdx])
		{
			if (TAL_Ticks() >= iTimer_ExpTime_tks[tIdx])
			{
				iTimer_State[tIdx] = false;
				uMCP_ITimer_Expired((uMCP_Timer_ID)tIdx);
			}
		}
	}
}


// Host communication
void uMCP_HC_LACK_Write(unsigned char sntID, uMCP_LERR_Enum lErr)
{
	Str_WriterInit(no_buffer, &no_cnt, UMCP_NBUFFER_SIZE);
	Str_WriteByte(no_buffer, &no_cnt, NMEA_SNT_STR);
	Str_WriteStr(no_buffer, &no_cnt, MCP_PREFIX);
	Str_WriteByte(no_buffer, &no_cnt, IC_D2H_LACK);
	Str_WriteByte(no_buffer, &no_cnt, NMEA_PAR_SEP);
	Str_WriteByte(no_buffer, &no_cnt, sntID);
	Str_WriteByte(no_buffer, &no_cnt, NMEA_PAR_SEP);
	Str_WriteByte(no_buffer, &no_cnt, lErr);
	Str_WriteByte(no_buffer, &no_cnt, NMEA_PAR_SEP);

	Str_WriteByte(no_buffer, &no_cnt, NMEA_CHK_SEP);
	Str_WriteHexByte(no_buffer, &no_cnt, 0);
	Str_WriteStr(no_buffer, &no_cnt, NMEA_SNT_PST);

	NMEA_PktCheckSum_Update(no_buffer, no_cnt);
	no_ready = true;
}

void uMCP_HC_STAT_Write()
{
	Str_WriterInit(no_buffer, &no_cnt, UMCP_NBUFFER_SIZE);
	Str_WriteByte(no_buffer, &no_cnt, NMEA_SNT_STR);
	Str_WriteStr(no_buffer, &no_cnt, MCP_PREFIX);
	Str_WriteByte(no_buffer, &no_cnt, IC_D2H_STAT);
	Str_WriteByte(no_buffer, &no_cnt, NMEA_PAR_SEP);
	Str_WriteByte(no_buffer, &no_cnt, (unsigned char)state);
	Str_WriteByte(no_buffer, &no_cnt, NMEA_PAR_SEP);
	Str_WriteByte(no_buffer, &no_cnt, select);
	Str_WriteByte(no_buffer, &no_cnt, NMEA_PAR_SEP);

	Str_WriteByte(no_buffer, &no_cnt, NMEA_CHK_SEP);
	Str_WriteHexByte(no_buffer, &no_cnt, 0);
	Str_WriteStr(no_buffer, &no_cnt, NMEA_SNT_PST);

	NMEA_PktCheckSum_Update(no_buffer, no_cnt);
	no_ready = true;
}

void uMCP_HC_HDATA_Write(unsigned char sntID, unsigned char* data, unsigned int dSize)
{
	Str_WriterInit(oh_buffer, &oh_cnt, UMCP_NBUFFER_SIZE);
	Str_WriteByte(oh_buffer, &oh_cnt, NMEA_SNT_STR);
	Str_WriteStr(oh_buffer, &oh_cnt, MCP_PREFIX);
	Str_WriteByte(oh_buffer, &oh_cnt, sntID);
	Str_WriteByte(oh_buffer, &oh_cnt, NMEA_PAR_SEP);
	Str_WriteHexStr(oh_buffer, &oh_cnt, data, dSize);
	Str_WriteByte(oh_buffer, &oh_cnt, NMEA_CHK_SEP);
	Str_WriteHexByte(oh_buffer, &oh_cnt, 0);
	Str_WriteStr(oh_buffer, &oh_cnt, NMEA_SNT_PST);
	NMEA_PktCheckSum_Update(oh_buffer, oh_cnt);
	oh_ready = true;
}

void uMCP_STRT_Parse(unsigned int fromIdx)
{
	// $PMCP1,senderID,targetID,selectDefState,selIntMs,toutIntMs,lineBaudrate
	int i, lastDIdx = fromIdx, pIdx = 0;
	float val = 0.0f;
	int sID = -1, tID = -1;
	bool sdState = false;
	int selIntMs = -1, toutIntMs = -1, lbRate = -1;

	uMCP_LERR_Enum result = LERR_OK;

	for (i = fromIdx; i <= ni_cnt; i++)
	{
		if ((ni_buffer[i] == NMEA_PAR_SEP) || (i == ni_cnt) || (ni_buffer[i] == NMEA_CHK_SEP))
		{
			val = Str_ParseFloat(ni_buffer, lastDIdx + 1, i - 1);
			lastDIdx = i;
			switch (pIdx)
			{
				case 1:
				{
					sID = (int)val;
					break;
				}
				case 2:
				{
					tID = (int)val;
					break;
				}
				case 3:
				{
					sdState = (bool)(int)val;
					break;
				}
				case 4:
				{
					selIntMs = (int)val;
					break;
				}
				case 5:
				{
					toutIntMs = (bool)(int)val;
					break;
				}
				case 6:
				{
					lbRate = (int)val;
					break;
				}
			}

			pIdx++;
		}
	}


	if ((IS_BYTE(sID)) && (IS_BYTE(tID)) &&
		(IS_VALID_TINT(selIntMs)) && (IS_VALID_TINT(toutIntMs)) &&
		(lbRate > 0))
	{
		uMCP_STATE_Set(uMCP_STATE_HALTED);

		SID = sID;
		TID = tID;
		selectDefaultState = sdState;
		lineBaudRate = lbRate;

		uMCP_ITimer_Init(uMCP_Timer_SELECT, selIntMs, false);
		uMCP_ITimer_Init(uMCP_Timer_TMO, toutIntMs, false);

		uMCP_STATE_Set(uMCP_STATE_ISTART);
		uMCP_CtrlSend(uMCP_PTYPE_STR, 0, 0, true);
	}
	else
	{
		result = LERR_ARGUMENT_OUT_OF_RANGE;
	}

	uMCP_HC_LACK_Write(IC_H2D_STRT, result);
}

void uMCP_OnHostQuery()
{
	if (NMEA_PktCheckSum_Check(ni_buffer, ni_cnt))
	{
		int ptnPos = NMEA_Ptn_Search(ni_buffer, ni_cnt, (unsigned char*)MCP_PREFIX, MCP_PREFIX_LEN);

		if (ptnPos >= 0)
		{
			ptnPos += MCP_PREFIX_LEN;
			switch(ni_buffer[ptnPos])
			{
				// Host interface
				case IC_H2D_STRT:
				{
					uMCP_STRT_Parse(ptnPos);
					break;
				}
				case IC_H2D_SPKT:
				{
					int i;
					for (i = ptnPos + 1; i < ni_cnt; i += 2)
					{
						ih_ring[ih_wPos] = Str_ParseHexByte(ni_buffer, i);
						ih_wPos = (ih_wPos + 1) % CR_RING_SIZE;
						ih_Cnt++;
					}

					break;
				}
				default:
				{
					// unsupported sentence ID
					uMCP_HC_LACK_Write(ni_buffer[ptnPos], LERR_UNSUPPORTED);
					break;
				}
			}
		}
	}
	else
	{
		// Checksum error
		uMCP_HC_LACK_Write(IC_UNKNOWN, LERR_CHECKSUM);
	}

	ni_ready = false;
}


// Utils
bool uMCP_IsByteInRangeExclusive(unsigned char st, unsigned char nd, unsigned char val)
{
	bool result = false;
	unsigned char idx = st;
	unsigned char _nd = nd;
	_nd--;
	while ((idx != _nd) && (!result))
	{
		idx++;
		if (idx == val)
		{
			result = true;
		}
	}

	return result;
}


// uMCP state management routines
void uMCP_SELECT_Set(bool value)
{
	if (selectDefaultState)
	{
		uMCP_ITimer_StateSet(uMCP_Timer_SELECT, value != selectDefaultState);
	}

	select = value;
}

void uMCP_STATE_Set(uMCP_State value)
{
	if ((value == uMCP_STATE_ISTART) ||
		(value == uMCP_STATE_HALTED))
	{
		R = 0;
		N = 0;
		A = 0;
		sentBlocksCnt = 0;
	}
	state = value;
}


// basic protocol routines
void uMCP_DataSend(bool isDTE, unsigned char tcnt, unsigned char rcnt, unsigned int rPos, unsigned int cnt, bool isStartTimer)
{
	uMCP_PacketType ptype = isDTE ? uMCP_PTYPE_DTE : uMCP_PTYPE_DTA;

	Str_WriterInit(ol_buffer, &ol_cnt, UMCP_LBUFFER_SIZE);
	StrB_WriteByte(ol_buffer, &ol_cnt, UMCP_SIGN);
	StrB_WriteByte(ol_buffer, &ol_cnt, ptype);
	StrB_WriteByte(ol_buffer, &ol_cnt, SID);
	StrB_WriteByte(ol_buffer, &ol_cnt, TID);
	StrB_WriteByte(ol_buffer, &ol_cnt, tcnt);
	StrB_WriteByte(ol_buffer, &ol_cnt, rcnt);
	StrB_WriteByte(ol_buffer, &ol_cnt, CRC8_Get(ol_buffer, 0, ol_cnt));

	unsigned int dstart_idx = ol_cnt;
	StrB_WriteByte(ol_buffer, &ol_cnt, (unsigned char)cnt);

	int i;
	unsigned int rposc = rPos;
	for (i = 0; i < cnt; i++)
	{
		ol_buffer[ol_cnt++] = ih_ring[rposc];
		rposc = (rposc + 1) % CR_RING_SIZE;
	}

	StrB_WriteByte(ol_buffer, &ol_cnt, CRC8_Get(ol_buffer, dstart_idx, cnt + 1));

	ol_ready = true;
	isTimerPendingOnTxFinish = isStartTimer;

	uMCP_ITimer_Init(uMCP_Timer_TX, UMCP_FIXED_TX_DELAY_MS + 1000 *	(int)(((float)(ol_cnt * 8)) / CFG_LINEBAUDRATE), true);
	uMCP_SELECT_Set(!isDTE);
}

void uMCP_NextDataBlockSend()
{
	bool isDTE = (sentBlocksCnt >= UMCP_PIPELINE_LIMIT) || (ih_Cnt < CFG_DATABLOCK_SIZE);
	unsigned char pcnt = (ih_Cnt < CFG_DATABLOCK_SIZE) ? ih_Cnt : CFG_DATABLOCK_SIZE;

	uMCP_DataSend(isDTE, N, R, ih_rPos, pcnt, isDTE);

	sentBlocksRPos[N] = ih_rPos;
	sentBlocksSize[N] = pcnt;
	sentBlocksCnt++;

	ih_rPos = (ih_rPos + pcnt) % CR_RING_SIZE;
	ih_Cnt -= pcnt;
}

void uMCP_DataBlockResend(unsigned char blockId, bool isDTE, bool isStartTimer)
{
	if (sentBlocksSize[blockId] != 0)
	{
		uMCP_DataSend(isDTE, blockId, R, sentBlocksRPos[blockId], sentBlocksSize[blockId], isStartTimer);
	}
	// else
	//    wtf???
}

void uMCP_CtrlSend(uMCP_PacketType ptype, unsigned char tcnt, unsigned char rcnt, bool isStartTimer)
{
	Str_WriterInit(ol_buffer, &ol_cnt, UMCP_LBUFFER_SIZE);
	StrB_WriteByte(ol_buffer, &ol_cnt, UMCP_SIGN);
	StrB_WriteByte(ol_buffer, &ol_cnt, ptype);
	StrB_WriteByte(ol_buffer, &ol_cnt, SID);
	StrB_WriteByte(ol_buffer, &ol_cnt, TID);

	if ((ptype == uMCP_PTYPE_REP) || (ptype == uMCP_PTYPE_ACK))
	{
		StrB_WriteByte(ol_buffer, &ol_cnt, tcnt);
	    if (ptype == uMCP_PTYPE_ACK)
	    {
	    	StrB_WriteByte(ol_buffer, &ol_cnt, rcnt);
	    }
	}

	StrB_WriteByte(ol_buffer, &ol_cnt, CRC8_Get(ol_buffer, 0, ol_cnt));
	ol_ready = true;
	isTimerPendingOnTxFinish = isStartTimer;

	uMCP_ITimer_Init(uMCP_Timer_TX, UMCP_FIXED_TX_DELAY_MS + 1000 *	(int)(((float)(ol_cnt * 8)) / CFG_LINEBAUDRATE), true);
	uMCP_SELECT_Set(false);
}

void uMCP_AcknowledgeSentItems(int to)
{
	unsigned char a;
	unsigned char from = A;
	for (a = from; a != to; a++, A++)
	{
		sentBlocksSize[A] = 0; // sent block is free
		sentBlocksCnt--;
	}
}

void uMCP_Protocol_Perform()
{
	 if (state == uMCP_STATE_RUNNING)
	 {
		if ((!iTimer_State[uMCP_Timer_TX]) &&
			(!iTimer_State[uMCP_Timer_TMO]) &&
			(select))
		{
			if (ih_Cnt == 0)
			{
				if (srep)
				{
					uMCP_CtrlSend(uMCP_PTYPE_REP, N, 0, true);
					srep = false;
				}
				else if (sentBlocksCnt > 0)
				{
					uMCP_DataBlockResend(A + 1, true, true);
				}
				else if ((!selectDefaultState) || (sack))
				{
					uMCP_CtrlSend(uMCP_PTYPE_ACK, N, R, false);
					sack = false;
				}
			}
			else if (ih_Cnt > 0)
			{
				if ((ih_Cnt >= CFG_DATABLOCK_SIZE) || (TAL_Ticks() >= ih_TS + UMCP_NAGLE_DELAY_TKS))
				{
					N++;
					uMCP_NextDataBlockSend();
				}
			}
		}
	}
}

void uMCP_OnIncomingPacket()
{
	sid = ip_sid;
	tid = ip_tid;
	tcnt = ip_tcnt;
	rcnt = ip_rcnt;
	dcnt = ip_dcnt;
	pType = ip_type;
	ip_ready = false;

	switch (pType)
	{
	case uMCP_PTYPE_STR:
	{
		uMCP_SELECT_Set(true);
		if ((state == uMCP_STATE_HALTED) || (state == uMCP_STATE_ISTART))
		{
			uMCP_STATE_Set(uMCP_STATE_ASTART);
			uMCP_ITimer_StateSet(uMCP_Timer_TX, false);
			uMCP_ITimer_StateSet(uMCP_Timer_TMO, false);
			uMCP_CtrlSend(uMCP_PTYPE_STA, 0, 0, true);
		}
		else if (state == uMCP_STATE_ASTART)
		{
			uMCP_STATE_Set(uMCP_STATE_RUNNING);
			uMCP_ITimer_StateSet(uMCP_Timer_TX, false);
			uMCP_ITimer_StateSet(uMCP_Timer_TMO, false);
			uMCP_CtrlSend(uMCP_PTYPE_ACK, 0, 0, false);
		}
		else
		{
			uMCP_STATE_Set(uMCP_STATE_HALTED);
		}
		break;
	}
	case uMCP_PTYPE_STA:
	{
		uMCP_SELECT_Set(true);
		if ((state == uMCP_STATE_ISTART) || (state == uMCP_STATE_ASTART) || (state == uMCP_STATE_RUNNING))
		{
			uMCP_STATE_Set(uMCP_STATE_RUNNING);
			uMCP_ITimer_StateSet(uMCP_Timer_TX, false);
			uMCP_ITimer_StateSet(uMCP_Timer_TMO, false);
			uMCP_CtrlSend(uMCP_PTYPE_ACK, 0, 0, false);
		}
		break;
	}
	case uMCP_PTYPE_REP:
	{
		if (state == uMCP_STATE_RUNNING)
		{
			sack = true;
			srep = false;
			uMCP_SELECT_Set(true);
		}
		break;
	}
	case uMCP_PTYPE_ACK:
	{
		if (state == uMCP_STATE_ASTART)
		{
			if (rcnt == 0)
			{
				uMCP_ITimer_StateSet(uMCP_Timer_TX, false);
				uMCP_ITimer_StateSet(uMCP_Timer_TMO, false);
				uMCP_STATE_Set(uMCP_STATE_RUNNING);
			}
		}
		else if (state == uMCP_STATE_RUNNING)
		{
			srep = false;
			uMCP_ITimer_StateSet(uMCP_Timer_TX, false);
			uMCP_ITimer_StateSet(uMCP_Timer_TMO, false);
			if ((rcnt == N) || (uMCP_IsByteInRangeExclusive(A, N, rcnt)))
			{
				uMCP_AcknowledgeSentItems(rcnt);
			}
		}

		uMCP_SELECT_Set(true);
		break;
	}
	case uMCP_PTYPE_DTA:
	case uMCP_PTYPE_DTE:
	{
		if (state == uMCP_STATE_RUNNING)
		{
			if (tcnt <= (unsigned char)(R + 1))
			{
				if (tcnt == (unsigned char)(R + 1))
				{
					R++;

#ifdef CFG_IS_NMEA

					uMCP_HC_HDATA_Write(IC_D2H_RPKT, ip_datablock, dcnt);
#else
					ff_copy_u8(ip_datablock, oh_buffer, dcnt);
					oh_cnt = dcnt;
					oh_ready = true;
#endif

				}
				sack = true;
			}

			uMCP_ITimer_StateSet(uMCP_Timer_TX, false);
			uMCP_ITimer_StateSet(uMCP_Timer_TMO, false);

			if ((rcnt == N) || (uMCP_IsByteInRangeExclusive(A, N, rcnt)))
			{
				uMCP_AcknowledgeSentItems(rcnt);
			}

			if (pType == uMCP_PTYPE_DTA)
			{
				uMCP_ITimer_StateSet(uMCP_Timer_TMO, true);
			}

			uMCP_SELECT_Set(pType == uMCP_PTYPE_DTE);
		}
		break;
	}
	default:
		break;
	}
}


// Events
void uMCP_DC_Output_Process()
{
	if ((oh_ready) && (!IsDCLock[CFG_HOST_DC_ID]))
	{
		TAL_DC_Write_Block(CFG_HOST_DC_ID, oh_buffer, oh_cnt);
		oh_ready = false;
	}
	else if ((no_ready) && (!IsDCLock[CFG_HOST_DC_ID]))
	{
		TAL_DC_Write_Block(CFG_HOST_DC_ID, no_buffer, no_cnt);
		no_ready = false;
	}

	if ((ol_ready) && (!IsDCLock[CFG_LINE_DC_ID]))
	{
		TAL_DC_Write_Block(CFG_LINE_DC_ID, ol_buffer, ol_cnt);
		ol_ready = false;
	}
}

void uMCP_OnNewByte(DCID_Enum chID, unsigned char c)
{
	if (chID == CFG_HOST_DC_ID)
	{
#ifdef CFG_IS_NMEA

		if (!ni_ready)
		{
			if (c == NMEA_SNT_STR)
			{
				ni_pkt_start = true;
				ff_fill_u8(ni_buffer, 0, UMCP_NBUFFER_SIZE);
				ni_cnt = 0;
				ni_buffer[ni_cnt++] = c;
			}
			else if (ni_pkt_start)
			{
				if (c == NMEA_SNT_END)
				{
					ni_ready = true;
					ni_pkt_start = false;
				}
				else
				{
					if (ni_cnt++ > UMCP_NBUFFER_SIZE)
					{
						ni_pkt_start = false;
					}
					else
					{
						ni_buffer[ni_cnt] = c;
					}
				}
			}
		}

#else
		if (ih_Cnt <= CR_RING_SIZE)
		{
			ih_ring[ih_wPos] = c;
			ih_wPos = (ih_wPos + 1) % CR_RING_SIZE;
			ih_Cnt++;
			ih_TS = TAL_Ticks();
		}
#endif
	}
	else if (chID == CFG_LINE_DC_ID)
	{
		if (ip_start)
		{
			if (ip_pos == 1)
			{
				ip_ahchk = CRC8_Update(ip_ahchk, c);
				ip_type = (uMCP_PacketType)c;
				if (ip_type == uMCP_PTYPE_INVALID)
				{
					ip_start = false;
				}
				else
				{
					ip_pos++;
				}
			}
			else if (ip_pos <= 3)
			{
				ip_ahchk = CRC8_Update(ip_ahchk, c);
				if (ip_pos == 3)
				{
					ip_tid = c;
				}
				else if (ip_pos == 2)
				{
					ip_sid = c;
				}
				ip_pos++;
			}
			else
			{
				if ((ip_type == uMCP_PTYPE_STR) || (ip_type == uMCP_PTYPE_STA))
				{
					if (ip_ahchk == c) // CRC OK
					{
						ip_start = false;
						ip_ready = true;
					}
				}
				else
				{
					if (ip_pos == 4)
					{
						ip_tcnt = c;
						ip_pos++;
						ip_ahchk = CRC8_Update(ip_ahchk, c);
					}
					else
					{
						if (ip_type == uMCP_PTYPE_REP)
						{
							if (ip_ahchk == c)
							{
								ip_ready = true;
							}
							ip_start = false;
						}
						else
						{
							if (ip_pos == 5)
							{
								ip_rcnt = c;
								ip_pos++;
								ip_ahchk = CRC8_Update(ip_ahchk, c);
							}
							else
							{
								if (ip_pos == 6) // header checksum
								{
									if (ip_ahchk == c)
									{
										if (ip_type == uMCP_PTYPE_ACK)
										{
											ip_ready = true;
											ip_start = false;
										}
										else
										{
											ip_pos++;
										}
									}
									else
									{
										ip_start = false;
									}
								}
								else
								{
									if (ip_pos == 7) // dcnt
									{
										ip_dcnt = c;
										if (ip_dcnt >= 1)
										{
											ip_ahchk = CRC8_Update(0xff, c);
											ip_pos++;
										}
										else
										{
											ip_start = false;
											ip_ready = true;
											ip_type = uMCP_PTYPE_ACK;
										}
									}
									else if (ip_pos == 8 + ip_dcnt)
									{
										if (ip_ahchk == c) // data block CRC ok
										{
											ip_ready = true;
										}
										else
										{
											ip_type = uMCP_PTYPE_ACK;
										}
										ip_start = false;
									}
									else
									{
										ip_datablock[ip_pos - 8] = c;
										ip_ahchk = CRC8_Update(ip_ahchk, c);
										ip_pos++;
									}
								}
							}
						}
					}
				}
			}
		}
		else if (c == UMCP_SIGN)
		{
			ip_start = true;
			ip_pos = 1;
			ip_ahchk = CRC8_Update(0xff, UMCP_SIGN);
			ip_type = uMCP_PTYPE_INVALID;
			ip_dcnt = 0;
		}
	}
}

void uMCP_PPNode_Init()
{
	// Interval timers
	uMCP_ITimer_Init(uMCP_Timer_SELECT, CFG_SELECT_INTERVAL_MS, false);
	uMCP_ITimer_Init(uMCP_Timer_TMO, CFG_TIMEOUT_INTERVAL_MS, false);

	// Data channels
	dcParams.Baudrate       = CFG_CR_DC_DEFAULT_BAUDRATE;
	dcParams.Wordlength     = CFG_CR_DC_DEFAULT_WORDLENGTH;
	dcParams.Stopbits       = CFG_CR_DC_DEFAULT_STOPBITS;
	dcParams.Parity         = CFG_CR_DC_DEFAULT_PARITY;
	dcParams.HW_FlowControl = CFG_CR_DC_DEFAULT_HW_FLOWCONTROL;
	TAL_DC_Config(CFG_HOST_DC_ID, &dcParams);
	TAL_DC_Config(CFG_LINE_DC_ID, &dcParams);

	TAL_IWDG_Init();
}

void uMCP_PPNode_Run()
{
	while (1)
	{
		uMCP_DC_Output_Process();
		uMCP_ITimers_Process();

		if (ip_ready)
		{
			uMCP_OnIncomingPacket();
		}

#ifdef CFG_IS_NMEA
		if (ni_ready)
			uMCP_OnHostQuery();

		if (state != uMCP_STATE_HALTED)
			uMCP_Protocol_Perform();
#else

		// Protocol autostart if buffer is not empty
		if ((state == uMCP_STATE_HALTED) && (ih_Cnt > 0))
		{
			uMCP_STATE_Set(uMCP_STATE_ISTART);
			uMCP_CtrlSend(uMCP_PTYPE_STR, 0, 0, true);
		}
		else if (state == uMCP_STATE_RUNNING)
		{
			uMCP_Protocol_Perform();
		}

#endif

#ifdef CFG_TAL_IWDG_ENABLED
		TAL_IWDG_Reload();
#endif
	}
}
