#ifndef STM_LINK_H
#define STM_LINK_H

#include "esp_err.h"

typedef enum {
    STM_COMMAND_OPEN = 0x01,
    STM_COMMAND_CLOSE = 0x02,
} stm_command_t;

esp_err_t stm_link_init(void);
esp_err_t stm_send_command(stm_command_t command);

#endif /* STM_LINK_H */
