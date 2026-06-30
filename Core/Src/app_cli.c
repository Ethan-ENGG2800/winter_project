#include "app_cli.h"
#include "app_display.h"
#include "app_status.h"

#include "FreeRTOS.h"
#include "queue.h"
#include "semphr.h"
#include "task.h"

#include "usart.h"

#include <string.h>
#include <stdint.h>
#include <stdio.h>

#define CLI_RX_QUEUE_LENGTH   128
#define CLI_LINE_BUFFER_SIZE  64

static QueueHandle_t cliRxQueue = NULL;
static SemaphoreHandle_t uartTxMutex = NULL;

static uint8_t cliRxByte = 0;

static void Cli_SendString(const char *str);
static void Cli_SendPrompt(void);
static void Cli_ProcessCommand(char *cmd);

void AppCli_Init(void)
{
    cliRxQueue = xQueueCreate(CLI_RX_QUEUE_LENGTH, sizeof(uint8_t));
    uartTxMutex = xSemaphoreCreateMutex();

    /*
     * 开启第一次 UART 接收中断。
     * 后面每收到 1 字节，会在 callback 里重新开启下一次接收。
     */
    HAL_UART_Receive_IT(&huart2, &cliRxByte, 1);
}

void AppCli_RxCpltCallback(UART_HandleTypeDef *huart)
{
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;

    if (huart->Instance == USART2) {
        if (cliRxQueue != NULL) {
            xQueueSendFromISR(cliRxQueue, &cliRxByte, &xHigherPriorityTaskWoken);
        }

        HAL_UART_Receive_IT(&huart2, &cliRxByte, 1);

        portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
    }
}

void AppCli_Task(void *argument)
{
    uint8_t ch = 0;
    char lineBuffer[CLI_LINE_BUFFER_SIZE];
    uint8_t lineIndex = 0;

    memset(lineBuffer, 0, sizeof(lineBuffer));

    Cli_SendString("\r\n");
    Cli_SendString("================================\r\n");
    Cli_SendString(" FreeRTOS Smart Door CLI\r\n");
    Cli_SendString(" Type help for command list\r\n");
    Cli_SendString("================================\r\n");
    Cli_SendPrompt();

    for (;;) {
        if (xQueueReceive(cliRxQueue, &ch, portMAX_DELAY) == pdPASS) {

            /*
             * 回车或者换行：执行命令
             */
            if ((ch == '\r') || (ch == '\n')) {

                /*
                 * 防止 CRLF 被处理两次：
                 * 如果当前行为空，直接忽略。
                 */
                if (lineIndex == 0) {
                    continue;
                }

                lineBuffer[lineIndex] = '\0';

                Cli_SendString("\r\n");
                Cli_ProcessCommand(lineBuffer);

                lineIndex = 0;
                memset(lineBuffer, 0, sizeof(lineBuffer));

                Cli_SendPrompt();
            }

            /*
             * Backspace 或 DEL
             */
            else if ((ch == 0x08) || (ch == 0x7F)) {
                if (lineIndex > 0) {
                    lineIndex--;

                    /*
                     * 终端删除一个字符：
                     * \b 回退
                     * 空格覆盖
                     * \b 再回退
                     */
                    Cli_SendString("\b \b");
                }
            }

            /*
             * 可打印 ASCII 字符
             */
            else if ((ch >= 32) && (ch <= 126)) {
                if (lineIndex < CLI_LINE_BUFFER_SIZE - 1) {
                    lineBuffer[lineIndex] = (char)ch;
                    lineIndex++;

                    /*
                     * 回显输入字符
                     */
                    HAL_UART_Transmit(&huart2, &ch, 1, HAL_MAX_DELAY);
                } else {
                    Cli_SendString("\r\nError: command too long\r\n");

                    lineIndex = 0;
                    memset(lineBuffer, 0, sizeof(lineBuffer));

                    Cli_SendPrompt();
                }
            }

            /*
             * 其他控制字符忽略
             */
            else {
                /* Do nothing */
            }
        }
    }
}

static void Cli_SendString(const char *str)
{
    if (str == NULL) {
        return;
    }

    if (uartTxMutex != NULL) {
        xSemaphoreTake(uartTxMutex, portMAX_DELAY);
    }

    HAL_UART_Transmit(&huart2,
                      (uint8_t *)str,
                      strlen(str),
                      HAL_MAX_DELAY);

    if (uartTxMutex != NULL) {
        xSemaphoreGive(uartTxMutex);
    }
}

static void Cli_SendPrompt(void)
{
    Cli_SendString("> ");
}

static void Cli_ProcessCommand(char *cmd)
{
    if (cmd == NULL) {
        return;
    }

    if (strlen(cmd) == 0) {
        return;
    }

    /*
     * help
     */
    if (strcmp(cmd, "help") == 0) {
        Cli_SendString("\r\nCommands:\r\n");
        Cli_SendString("  help          - Show command list\r\n");
        Cli_SendString("  status        - Show system status\r\n");
        Cli_SendString("  clear         - Clear terminal\r\n");
        Cli_SendString("  echo <text>   - Echo text\r\n");
        Cli_SendString("  oled <text>   - Show text on OLED\r\n");
        Cli_SendString("  mode auto     - Set auto mode\r\n");
        Cli_SendString("  mode manual   - Set manual mode\r\n");
        Cli_SendString("  door open     - Set door state open\r\n");
        Cli_SendString("  door close    - Set door state closed\r\n");
        Cli_SendString("  alarm on      - Set alarm on\r\n");
        Cli_SendString("  alarm off     - Set alarm off\r\n");
        Cli_SendString("\r\n");

        AppDisplay_ShowText("CLI: help",
                            "status/mode",
                            "door/alarm",
                            "oled/echo");
        return;
    }

    /*
     * status
     */
    if (strcmp(cmd, "status") == 0) {
        AppStatus_t status;
        char buffer[64];

        AppStatus_GetSnapshot(&status);

        Cli_SendString("\r\nSystem Status:\r\n");

        snprintf(buffer, sizeof(buffer), "  Mode: %s\r\n",
                 AppStatus_ModeToString(status.mode));
        Cli_SendString(buffer);

        snprintf(buffer, sizeof(buffer), "  Door: %s\r\n",
                 AppStatus_DoorStateToString(status.doorState));
        Cli_SendString(buffer);

        snprintf(buffer, sizeof(buffer), "  Alarm: %s\r\n",
                 AppStatus_AlarmStateToString(status.alarmState));
        Cli_SendString(buffer);

        snprintf(buffer, sizeof(buffer), "  Distance: %u cm\r\n",
                 status.distanceCm);
        Cli_SendString(buffer);

        snprintf(buffer, sizeof(buffer), "  Temp: %d C\r\n",
                 status.temperatureC);
        Cli_SendString(buffer);

        snprintf(buffer, sizeof(buffer), "  Humidity: %u %%\r\n",
                 status.humidity);
        Cli_SendString(buffer);

        snprintf(buffer, sizeof(buffer), "  UART: %s\r\n",
                 status.uartOk ? "OK" : "ERR");
        Cli_SendString(buffer);

        snprintf(buffer, sizeof(buffer), "  OLED: %s\r\n",
                 status.oledOk ? "OK" : "ERR");
        Cli_SendString(buffer);

        snprintf(buffer, sizeof(buffer), "  Sensor: %s\r\n",
                 status.sensorOk ? "OK" : "ERR");
        Cli_SendString(buffer);

        AppDisplay_ShowText("System Status",
                            AppStatus_ModeToString(status.mode),
                            AppStatus_DoorStateToString(status.doorState),
                            AppStatus_AlarmStateToString(status.alarmState));

        return;
    }

    /*
     * clear
     */
    if (strcmp(cmd, "clear") == 0) {
        /*
         * ANSI escape code:
         * \033[2J 清屏
         * \033[H  光标回到左上角
         */
        Cli_SendString("\033[2J\033[H");

        AppDisplay_ShowText("CLI: clear",
                            "Terminal",
                            "cleared",
                            "");
        return;
    }

    /*
     * echo <text>
     */
    if (strncmp(cmd, "echo ", 5) == 0) {
        const char *text = &cmd[5];

        Cli_SendString(text);
        Cli_SendString("\r\n");

        AppDisplay_ShowText("CLI: echo",
                            text,
                            "",
                            "");
        return;
    }

    /*
     * oled <text>
     */
    if (strncmp(cmd, "oled ", 5) == 0) {
        const char *text = &cmd[5];

        AppDisplay_ShowText("CLI MSG:",
                            text,
                            "",
                            "");

        Cli_SendString("OLED message updated\r\n");
        return;
    }

    if (strcmp(cmd, "mode auto") == 0) {
        AppStatus_SetMode(APP_MODE_AUTO);

        Cli_SendString("Mode set to AUTO\r\n");

        AppDisplay_ShowText("Mode Changed",
                            "Mode: AUTO",
                            "",
                            "");

        return;
    }

    if (strcmp(cmd, "mode manual") == 0) {
        AppStatus_SetMode(APP_MODE_MANUAL);

        Cli_SendString("Mode set to MANUAL\r\n");

        AppDisplay_ShowText("Mode Changed",
                            "Mode: MANUAL",
                            "",
                            "");

        return;
    }

    if (strcmp(cmd, "door open") == 0) {
        AppStatus_SetDoorState(APP_DOOR_OPEN);

        Cli_SendString("Door state set to OPEN\r\n");

        AppDisplay_ShowText("Door State",
                            "Door: OPEN",
                            "",
                            "");

        return;
    }

    if (strcmp(cmd, "door close") == 0) {
        AppStatus_SetDoorState(APP_DOOR_CLOSED);

        Cli_SendString("Door state set to CLOSED\r\n");

        AppDisplay_ShowText("Door State",
                            "Door: CLOSED",
                            "",
                            "");

        return;
    }

    if (strcmp(cmd, "alarm on") == 0) {
        AppStatus_SetAlarmState(APP_ALARM_ON);

        Cli_SendString("Alarm set to ON\r\n");

        AppDisplay_ShowText("Alarm State",
                            "Alarm: ON",
                            "",
                            "");

        return;
    }

    if (strcmp(cmd, "alarm off") == 0) {
        AppStatus_SetAlarmState(APP_ALARM_OFF);

        Cli_SendString("Alarm set to OFF\r\n");

        AppDisplay_ShowText("Alarm State",
                            "Alarm: OFF",
                            "",
                            "");

        return;
    }

    /*
     * 未知命令
     */
    Cli_SendString("Unknown command: ");
    Cli_SendString(cmd);
    Cli_SendString("\r\n");

    AppDisplay_ShowText("Unknown CMD:",
                        cmd,
                        "Type help",
                        "");
}