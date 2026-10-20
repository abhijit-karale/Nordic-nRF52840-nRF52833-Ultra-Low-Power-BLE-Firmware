/**
 * @file nrf_pwr_mgmt.h
 * @brief Power management module declarations
 */

#ifndef NRF_PWR_MGMT_H__
#define NRF_PWR_MGMT_H__

#include <stdint.h>
#include <stdbool.h>
#include "nrf_error.h"

ret_code_t nrf_pwr_mgmt_init(void);
void nrf_pwr_mgmt_run(void);

#endif /* NRF_PWR_MGMT_H__ */
