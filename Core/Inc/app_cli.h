#ifndef APP_CLI_H
#define APP_CLI_H

#ifdef __cplusplus
extern "C" {
#endif

#include "main.h"

    void AppCli_Init(void);
    void AppCli_RxCpltCallback(UART_HandleTypeDef *huart);
    void AppCli_Task(void *argument);

#ifdef __cplusplus
}
#endif

#endif