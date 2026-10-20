/**
 * @file app_main.h
 * @brief Application Entry Point and Subsystem Coordinator
 */

#ifndef APP_MAIN_H_
#define APP_MAIN_H_

#include <stdint.h>
#include <stdbool.h>
#include "sdk_common.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Early hardware board initialization (parks unused pins).
 */
void board_early_init(void);

/**
 * @brief SoftDevice and BLE stack initialization.
 */
ret_code_t ble_stack_init(void);

/**
 * @brief GAP and GATT services setup.
 */
ret_code_t gap_gatt_services_init(void);

/**
 * @brief Application main function.
 */
int main(void);

#ifdef __cplusplus
}
#endif

#endif /* APP_MAIN_H_ */
