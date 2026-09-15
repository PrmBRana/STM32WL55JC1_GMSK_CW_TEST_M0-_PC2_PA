#include "satellite_app.h"

/*
 * ============================================================================
 * SATELLITE MISSION MODES
 * ============================================================================
 */
typedef enum {
    SAT_MODE_CW_BEACON = 0,
    SAT_MODE_GMSK_BURST,
    SAT_MODE_STATUS_TELEMETRY,
} SatelliteMode_t;

/*
 * ============================================================================
 * MAIN ENTRY POINT
 * ============================================================================
 * All satellite parameters, frequencies, callsigns, and timings are configured
 * in satellite_app.h. Satellite_Init(NULL) applies these defaults.
 * ============================================================================
 */
int main(void) {
    /* 1. Point Cortex-M0+ vector table to CPU2 flash at 0x08032000 */
    SCB->VTOR = 0x08032000;

    /* 2. Enable IPCC, SUBGHZSPI, and SRAM2 peripheral bus clocks in CPU2 domain */
    (*(volatile uint32_t *)0x58000150UL) |= (1UL << 0) | (1UL << 25);

    /* 3. Enable GPIOA, GPIOB, GPIOC peripheral bus clocks in CPU2 domain */
    (*(volatile uint32_t *)0x5800014CUL) |= 0x87;

    /* 4. Enable USART1 bus clock in CPU2 domain */
    (*(volatile uint32_t *)0x58000160UL) |= (1UL << 14);

    /* 5. Disable and mask IPCC interrupts in CPU2 domain to prevent unhandled IRQ traps */
    (*(volatile uint32_t *)0x58000C10UL) = 0x00000000; /* IPCC_C2CR: Disable RXOIE and TXFIE */
    (*(volatile uint32_t *)0x58000C14UL) = 0xFFFFFFFF; /* IPCC_C2MR: Mask all channels */
    NVIC_DisableIRQ((IRQn_Type)1);                      /* Disable IPCC IRQ1 in NVIC */

    /* 6. Initialize satellite system with configuration defined in satellite_app.h */
    Satellite_Init(NULL);

    /* Retrieve active configuration pointer for runtime parameters and delays */
    const SatelliteConfig_t *cfg = Satellite_GetConfig();

    uint32_t cycle = 0;
    volatile SatelliteMode_t current_mode = SAT_MODE_CW_BEACON;

    /* 2. Main mission executive loop: sequences CW, GMSK burst, and Telemetry */
    while (1) {
        cycle++;

        /* --------------------------------------------------------------------
         * MISSION STEP 1: CW Morse Beacon Session
         * -------------------------------------------------------------------- */
        current_mode = SAT_MODE_CW_BEACON;
        (void)current_mode;
        Satellite_Run_CW_Session(cycle);

        /* Guard delay between CW and GMSK */
        Satellite_DelayMs(cfg->cycleGuardDelayMs);

        /* --------------------------------------------------------------------
         * MISSION STEP 2: GMSK AX.25 Burst Session
         * -------------------------------------------------------------------- */
        current_mode = SAT_MODE_GMSK_BURST;
        Satellite_Run_GMSK_Burst_Session(cycle);

        /* Guard delay between GMSK and Telemetry */
        Satellite_DelayMs(cfg->cycleGuardDelayMs);

        /* --------------------------------------------------------------------
         * MISSION STEP 3: Status Telemetry Packet
         * -------------------------------------------------------------------- */
        current_mode = SAT_MODE_STATUS_TELEMETRY;
        char status_msg[64];
        snprintf(status_msg, sizeof(status_msg), "[STATUS] Cycle #%lu completed (Total PKT: %lu)",
                 (unsigned long)cycle, (unsigned long)Satellite_GetTotalPacketsSent());
        Satellite_Send_AX25_String_Timeout(status_msg, cfg->txTimeoutMs);

        /* Delay after telemetry transmission */
        Satellite_DelayMs(cfg->telemetryIntervalMs);
    }

    return 0;
}