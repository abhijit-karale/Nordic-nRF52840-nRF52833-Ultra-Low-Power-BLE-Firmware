/**
 * @file nrf_power.h
 * @brief Power management registers and SoftDevice power APIs
 */

#ifndef NRF_POWER_H__
#define NRF_POWER_H__

#include <stdint.h>
#include <stdbool.h>
#include "nrf_error.h"

#define POWER_RESETREAS_RESETPIN_Msk (0x1u << 0)
#define POWER_RESETREAS_DOG_Msk      (0x1u << 1)
#define POWER_RESETREAS_SREQ_Msk     (0x1u << 2)
#define POWER_RESETREAS_LOCKUP_Msk   (0x1u << 3)
#define POWER_RESETREAS_OFF_Msk      (0x1u << 16)
#define POWER_RESETREAS_LPCOMP_Msk   (0x1u << 17)
#define POWER_RESETREAS_DIF_Msk      (0x1u << 18)
#define POWER_RESETREAS_NFC_Msk      (0x1u << 19)

typedef struct {
    volatile uint32_t RESETREAS;
    volatile uint32_t RESERVED0[9];
    volatile uint32_t SYSTEMOFF;
    volatile uint32_t RESERVED1[15];
    volatile uint32_t GPREGRET;
    volatile uint32_t GPREGRET2;
    volatile uint32_t RESERVED2[14];
    volatile uint32_t DCDCEN;
} NRF_POWER_Type;

typedef struct {
    volatile uint32_t OUT;
    volatile uint32_t OUTSET;
    volatile uint32_t OUTCLR;
    volatile uint32_t IN;
    volatile uint32_t DIR;
    volatile uint32_t DIRSET;
    volatile uint32_t DIRCLR;
    volatile uint32_t LATCH;
    volatile uint32_t DETECTMODE;
    volatile uint32_t RESERVED[118];
    volatile uint32_t PIN_CNF[32];
} NRF_GPIO_Type;

/* Standard Nordic memory-mapped peripheral pointers (or mocked for test) */
extern NRF_POWER_Type mock_nrf_power;
extern NRF_GPIO_Type  mock_nrf_gpio0;
extern NRF_GPIO_Type  mock_nrf_gpio1;

#ifndef NRF_POWER
#define NRF_POWER (&mock_nrf_power)
#endif

#ifndef NRF_P0
#define NRF_P0 (&mock_nrf_gpio0)
#endif

#ifndef NRF_P1
#define NRF_P1 (&mock_nrf_gpio1)
#endif

typedef enum {
    NRF_POWER_DCDC_DISABLE = 0,
    NRF_POWER_DCDC_ENABLE  = 1
} nrf_power_dcdc_mode_t;

typedef enum {
    NRF_POWER_MODE_CONSTLAT = 0,
    NRF_POWER_MODE_LOWPWR   = 1
} nrf_power_mode_t;

/* SoftDevice Power Management Calls */
ret_code_t sd_power_dcdc_mode_set(nrf_power_dcdc_mode_t mode);
ret_code_t sd_power_mode_set(nrf_power_mode_t mode);
ret_code_t sd_power_system_off(void);
ret_code_t sd_power_gpregret_set(uint32_t gpregret_id, uint32_t val);
ret_code_t sd_power_gpregret_get(uint32_t gpregret_id, uint32_t * p_val);
ret_code_t sd_app_evt_wait(void);

#endif /* NRF_POWER_H__ */
