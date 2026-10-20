/**
 * @file board_config.h
 * @brief Top-level board selector header
 */

#ifndef BOARD_CONFIG_H_
#define BOARD_CONFIG_H_

#if defined(BOARD_ULP_52840)
    #include "ulp_board_52840.h"
#elif defined(BOARD_ULP_52833)
    #include "ulp_board_52833.h"
#elif defined(BOARD_PCA10056)
    #include "pca10056.h"
#elif defined(BOARD_PCA10100)
    #include "pca10100.h"
#else
    /* Default to custom nRF52840 board */
    #include "ulp_board_52840.h"
#endif

#endif /* BOARD_CONFIG_H_ */
