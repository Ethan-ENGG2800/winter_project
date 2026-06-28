#include "app_cli.h"
#include "app_display.h"

#include "FreeRTOS.h"
#include "queue.h"
#include "semphr.h"
#include "task.h"

#include "usart.h"

#include <string.h>
#include <stdint.h>

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
        Cli_SendString("\r\n");

        AppDisplay_ShowText("CLI: help",
                            "help/status",
                            "echo/oled",
                            "System Ready");
        return;
    }

    /*
     * status
     */
    if (strcmp(cmd, "status") == 0) {
        Cli_SendString("RTOS: RUN\r\n");
        Cli_SendString("UART: OK\r\n");
        Cli_SendString("CLI: OK\r\n");
        Cli_SendString("OLED: OK\r\n");

        AppDisplay_ShowText("CLI: status",
                            "RTOS: RUN",
                            "UART: OK",
                            "OLED: OK");
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