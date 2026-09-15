#ifndef CONFIG_H
#define CONFIG_H

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
 * ============================================================
 * FREQUENCIES
 * ============================================================
 *
 * Satellite:
 *
 *     TX = 435.000 MHz
 *     RX = 437.375 MHz
 *
 * Ground station must use:
 *
 *     TX = 437.375 MHz
 *     RX = 435.000 MHz
 * ============================================================
 */

#define DEFAULT_TX_FREQUENCY_HZ        437375000UL
#define DEFAULT_RX_FREQUENCY_HZ        435000000UL

/*
 * ============================================================
 * DEFAULT AX.25 ADDRESS
 * ============================================================
 */

#define DEFAULT_SOURCE_CALLSIGN       "9NS2S2"
#define DEFAULT_SOURCE_SSID            1

#define DEFAULT_DEST_CALLSIGN         "GROUND"
#define DEFAULT_DEST_SSID              0

/*
 * ============================================================
 * GFSK / G3RUH
 * ============================================================
 */
//JC1 power 14dbm
#define RADIO_TX_POWER_DBM             14

/*
 * GMSK / G3RUH data rate: 4800 bps (matched to Ground Station).
 */
#define RADIO_BIT_RATE_BPS             4800

/*
 * GMSK frequency deviation (+/- 1.2 kHz for 4800 bps, BT 0.5, h=0.5).
 */
#define RADIO_FDEV_HZ                  1200

/*
 * SX126x GFSK bandwidth.
 */
#define RADIO_RX_BANDWIDTH_HZ         23400

/*
 * 128 bits = 16 bytes.
 */
#define RADIO_PREAMBLE_LENGTH_BITS     128

#define RADIO_TX_TIMEOUT_MS            3000

/*
 * ============================================================
 * PHYSICAL PACKET LENGTH
 *
 * The SX126x packet is deliberately fixed length.
 *
 * This is NOT an AX.25 requirement.
 * It is our radio transport configuration.
 * ============================================================
 */

#define RADIO_FIXED_PACKET_LEN         200

/*
 * Number of HDLC flags before actual frame.
 * 64 flags = 512 bits = 53 ms audio lock time at 4800 baud.
 */
#define PROTOCOL_LEADING_FLAG_COUNT    64

/*
 * ============================================================
 * RADIO CONFIG
 * ============================================================
 */

typedef struct
{
    uint32_t txFrequency;
    uint32_t rxFrequency;

    const char *sourceCallsign;
    uint8_t sourceSSID;

    const char *destCallsign;
    uint8_t destSSID;

    bool isSatelliteMode;

} RadioConfig_t;

/*
 * ============================================================
 * COMMANDS
 * ============================================================
 */

typedef enum
{
    CMD_NONE          = 0x00,

    CMD_REQUEST_BURST = 0x01,

    CMD_ACK           = 0x02,

    CMD_NACK          = 0x03,

    CMD_HEARTBEAT     = 0x04

} CommandOpcode_t;

#ifdef __cplusplus
}
#endif

#endif