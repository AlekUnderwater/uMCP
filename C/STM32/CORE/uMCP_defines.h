// (C) Aleksandr Dikarev, 2019

#ifndef _UMCP_DEFINES_H_
#define _UMCP_DEFINES_H_

#include <uMCP_config.h>
#include <ff.h>
#include <ustr.h>
#include <crc.h>
#include <nmea.h>

#define CR_CORE_MONIKER             "uMCP\0"
#define CR_CORE_VERSION             (0x0100)

#define CR_TICKS_PER_MS             ((unsigned int)(CFG_CR_TICK_FREQUENCY_HZ / 1000.0f))
#define CR_TICKS_PER_SEC            ((int)CFG_CR_TICK_FREQUENCY_HZ)
#define CFG_TAL_IWDG_RELOAD_TICKS   (CFG_TAL_IWDG_RELOAD_MS * CR_TICKS_PER_MS)

#define UMCP_SIGN                   (0xAD)
#define UMCP_MAX_DATA_SIZE          (255)
#define UMCP_MAX_OVERHEAD           (9)
#define UMCP_MAX_PACKETSIZE         (UMCP_MAX_DATA_SIZE + UMCP_MAX_OVERHEAD)
#define UMCP_FIXED_TX_DELAY_MS      (50)
#define UMCP_MAX_SENT_BLOCKS        (255)
#define UMCP_PIPELINE_LIMIT         (16)

#define CR_RING_SIZE                (UMCP_MAX_SENT_BLOCKS * UMCP_MAX_DATA_SIZE)
#define CR_DC_TX_BUFFER_SIZE        (255)

#define UMCP_NBUFFER_SIZE           (512)
#define UMCP_LBUFFER_SIZE           (UMCP_MAX_PACKETSIZE)
#define UMCP_NAGLE_DELAY_TKS        (CFG_NAGLE_DELAY_MS * CR_TICKS_PER_MS)

#define IS_BYTE(value)              (((value) >= 0) && ((value) <= 255))
#define IS_VALID_TINT(value)        (((value) >= 100) && ((value) <= 120000))

#define MS2TKS(ms)                  (ms * CR_TICKS_PER_MS)
#define TKS2SEC(ticks)              (((float)(ticks)) / (float)CFG_CR_TICK_FREQUENCY_HZ)
#define TKS2MS(ticks)               (((float)(ticks)) / CR_TICKS_PER_MS)

#endif
