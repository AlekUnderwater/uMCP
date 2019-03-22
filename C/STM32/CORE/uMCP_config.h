// (C) Aleksandr Dikarev, 2019

#ifndef _UMCP_CONFIG_H_
#define _UMCP_CONFIG_H_

#include <uMCP_types.h>

// Default state of SELECT flag. If it is true, node will regain the flag by timeout anyway
#define CFG_SELECT_DEFAULT_STATE         (true)

// Default SELECT interval
#define CFG_SELECT_INTERVAL_MS           (20000)

// Default timeout interval
#define CFG_TIMEOUT_INTERVAL_MS          (18000)

#define CFG_CR_TICK_FREQUENCY_HZ         (5000.0f)

// Line transmission speed in bits per second (80 for uWAVE and RedLINE modems)
#define CFG_LINEBAUDRATE                 (80)

// Default data packet size
#define CFG_DATABLOCK_SIZE               (64)

// Nagle algorithm delay
#define CFG_NAGLE_DELAY_MS               (100)

#define CFG_CR_DC_DEFAULT_BAUDRATE       (DC_BAUDRATE_9600)
#define CFG_CR_DC_DEFAULT_WORDLENGTH     (DC_WORD_LENGTH_8_BIT)
#define CFG_CR_DC_DEFAULT_STOPBITS       (DC_STOPBITS_1)
#define CFG_CR_DC_DEFAULT_PARITY         (DC_PARITY_NONE)
#define CFG_CR_DC_DEFAULT_HW_FLOWCONTROL (DC_HW_FLOW_CONTROL_NONE)

#define CFG_HOST_DC_ID                   (DC_3)
#define CFG_LINE_DC_ID                   (DC_1)

#define CFG_TAL_IWDG_ENABLED

#endif
