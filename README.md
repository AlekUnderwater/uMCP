# uMCP
Lightweight communication protocol with guaranteed delivery

uMCP - a free interpretation of the DDCMP protocol, especially designed to
work with small baudrates and channels with high error probability - for example,
underwater acoustic channel etc.
uMCP is designed for better usability of uWAVE and RedLINE underwater acoustic
modems (but not only!).

It is very similar to DDCMP, but has some differencies:
- smaller overhead
- possible communication between tributary stations
- no NAKs, all error situations are managed by timeouts to decrease traffic
- easy pipelining

timeouts are causing transmission of REP (reply to message),
remote system answers with ACK (which, in fact, contains remote system state)
- if error is due to lost ACK, it will be corrected by ACK retransmission from the remote system
- if error is due to lost DATA message, it will be corrected by DATA message retransmission to the remote system
Retransmission performed without pipelining.
 

Message framing
 
SIGN  : 8 bit  (start signature) = 0xAD
SID   : 8 bit  (sender ID)
TID   : 8 bit  (target ID)
PTYPE : 8 bit  (packet type)

if PTYPE == STR || PTYPE == STA
- no extra data

else if PTYPE == REP
	TCNT  : 8 bit

else if PTYPE == ACK
	TCNT  : 8 bit
	RCNT  : 8 bit

else if PTYPE = DTA || PTYPE == DTE
	TCNT  : 8 bit
	RCNT  : 8 bit

endif

HCHK  : 8 bit (header checksum)

if PTYPE == DTA || PTYPE == DTE
	DCNT  : 8 bit (data counter)
	DATA  : 8 bit * DCNT
 
endif
 
