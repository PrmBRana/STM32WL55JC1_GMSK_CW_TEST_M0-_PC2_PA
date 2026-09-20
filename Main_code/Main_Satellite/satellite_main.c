#include "satellite_app.h"
#include <stdio.h>

/*
 * ============================================================================
 * SATELLITE MISSION PHASES
 * ============================================================================
 */
typedef enum {
    SAT_MODE_CW_BEACON = 0,     /* Continuous Wave (Morse code carrier keying) */
    SAT_MODE_GMSK_BURST,        /* High-speed GMSK AX.25 UI telemetry frames  */
    SAT_MODE_STATUS_TELEMETRY,  /* Cycle completion status packet             */
} SatelliteMode_t;

/*
 * ============================================================================
 * MAIN ENTRY POINT (CORTEX-M0+ / CPU2)
 * ============================================================================
 * Hardware, clock tree, radio front-end, and protocols are initialized in
 * clearly separated standard functions.
 * All default parameters, frequencies, and delays are defined in satellite_app.h.
 * ============================================================================
 */
int main(void) {
    /*
     * ------------------------------------------------------------------------
     * STAGE 1: Low-Level CPU2 Hardware & Bus Initialization
     * ------------------------------------------------------------------------
     * 1. Point Cortex-M0+ vector table (SCB->VTOR) to CPU2 flash (0x08032000).
     * 2. Enable CPU2 domain peripheral bus clocks (GPIOA/B/C/H, Flash, IPCC,
     *    USART1 console, and SUBGHZSPI radio interface).
     * 3. Disable and mask IPCC interrupts to prevent unhandled inter-core traps.
     * ------------------------------------------------------------------------
     */
    Satellite_Hardware_Init();

    /*
     * ------------------------------------------------------------------------
     * STAGE 2: Satellite Subsystem & Application Initialization
     * ------------------------------------------------------------------------
     * 1. Initialize SysTick timer and HAL base.
     * 2. Bring up USART1 debug console (PA9 TX / PA10 RX @ 115200 baud).
     * 3. Configure RF switches (PC4, PC5), 3.3V PA (PC2), 5V PA (PC3), 5V DC/DC (PA0).
     * 4. Initialize Sub-GHz radio transceiver (SX1262) and NVIC radio IRQ.
     * 5. Apply mission parameters, callsigns, and frequency configurations.
     * ------------------------------------------------------------------------
     */
    Satellite_Init(NULL);

    /* Retrieve active configuration pointer for runtime parameters and delays */
    const SatelliteConfig_t *cfg = Satellite_GetConfig();

    uint32_t cycle = 0;
    volatile SatelliteMode_t current_mode = SAT_MODE_CW_BEACON;

    /*
     * ------------------------------------------------------------------------
     * STAGE 3: Satellite Mission Executive Loop
     * ------------------------------------------------------------------------
     * Continuously sequences:
     *   Phase 1: CW Morse Beacon Session
     *   Phase 2: GMSK AX.25 UI Frame Burst Session
     *   Phase 3: Mission Cycle Status Telemetry Packet
     * ------------------------------------------------------------------------
     */
    while (1) {
        cycle++;

        /* --------------------------------------------------------------------
         * MISSION PHASE 1: 1-Minute 3.3V CW Morse Beacon Session (PC2 Active)
         * -------------------------------------------------------------------- */
        current_mode = SAT_MODE_CW_BEACON;
        (void)current_mode;
        Satellite_Run_CW_Session(cycle);

        /* Inter-mission guard delay (1 second) */
        Satellite_DelayMs(cfg->cycleGuardDelayMs);

        /* --------------------------------------------------------------------
         * MISSION PHASE 2: 1-Minute 5V GMSK AX.25 Burst Session (PA0 + PC3 Active)
         * -------------------------------------------------------------------- */
        current_mode = SAT_MODE_GMSK_BURST;
        Satellite_Run_GMSK_Burst_Session(cycle);

        /* Inter-mission guard delay (1 second) */
        Satellite_DelayMs(cfg->cycleGuardDelayMs);
    }

    return 0;
}