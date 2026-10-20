/**
 * @file nrf_error.h
 * @brief Common Nordic error codes compliant with nRF5 SDK 17.1.0
 */

#ifndef NRF_ERROR_H__
#define NRF_ERROR_H__

#include <stdint.h>

#define NRF_SUCCESS                           (0)
#define NRF_ERROR_SVC_HANDLER_MISSING         (1)
#define NRF_ERROR_SOFTDEVICE_NOT_ENABLED      (2)
#define NRF_ERROR_INTERNAL                    (3)
#define NRF_ERROR_NO_MEM                      (4)
#define NRF_ERROR_NOT_FOUND                   (5)
#define NRF_ERROR_NOT_SUPPORTED               (6)
#define NRF_ERROR_INVALID_PARAM               (7)
#define NRF_ERROR_INVALID_STATE               (8)
#define NRF_ERROR_INVALID_LENGTH              (9)
#define NRF_ERROR_INVALID_FLAGS               (10)
#define NRF_ERROR_INVALID_DATA                (11)
#define NRF_ERROR_DATA_SIZE                   (12)
#define NRF_ERROR_TIMEOUT                     (13)
#define NRF_ERROR_NULL                        (14)
#define NRF_ERROR_FORBIDDEN                   (15)
#define NRF_ERROR_INVALID_ADDR                (16)
#define NRF_ERROR_BUSY                        (17)
#define NRF_ERROR_CONN_COUNT                  (18)
#define NRF_ERROR_RESOURCES                   (19)

typedef uint32_t ret_code_t;

#define APP_ERROR_CHECK(ERR_CODE)                           \
    do {                                                    \
        ret_code_t __err = (ERR_CODE);                      \
        if (__err != NRF_SUCCESS) {                         \
            app_error_fault_handler(__err, (uint32_t)__LINE__, (uint32_t)(uintptr_t)__FILE__); \
        }                                                   \
    } while (0)

void app_error_fault_handler(ret_code_t error_code, uint32_t line_num, uint32_t info);

#endif /* NRF_ERROR_H__ */
