#include "app_display.h"

#include "FreeRTOS.h"
#include "queue.h"
#include "task.h"

#include "oled.h"
#include "font.h"

#include <string.h>

#define DISPLAY_QUEUE_LENGTH  4

#define APP_DISPLAY_FONT      (&font16x16)

static QueueHandle_t displayQueue = NULL;

static void AppDisplay_CopyLine(char *dst, const char *src)
{
    uint8_t i = 0;

    if (dst == NULL) {
        return;
    }

    if (src == NULL) {
        dst[0] = '\0';
        return;
    }

    for (i = 0; i < APP_DISPLAY_LINE_LENGTH - 1; i++) {
        if (src[i] == '\0') {
            break;
        }

        dst[i] = src[i];
    }

    dst[i] = '\0';
}

static void AppDisplay_DrawMessage(const AppDisplayMsg_t *msg)
{
    if (msg == NULL) {
        return;
    }

    OLED_NewFrame();

    OLED_PrintString(0, 0,  (char *)msg->line1, APP_DISPLAY_FONT, OLED_COLOR_NORMAL);
    OLED_PrintString(0, 16, (char *)msg->line2, APP_DISPLAY_FONT, OLED_COLOR_NORMAL);
    OLED_PrintString(0, 32, (char *)msg->line3, APP_DISPLAY_FONT, OLED_COLOR_NORMAL);
    OLED_PrintString(0, 48, (char *)msg->line4, APP_DISPLAY_FONT, OLED_COLOR_NORMAL);

    OLED_ShowFrame();
}

void AppDisplay_Init(void)
{
    displayQueue = xQueueCreate(DISPLAY_QUEUE_LENGTH, sizeof(AppDisplayMsg_t));
}

uint8_t AppDisplay_SendMessage(const AppDisplayMsg_t *msg)
{
    if ((displayQueue == NULL) || (msg == NULL)) {
        return 0;
    }

    if (xQueueSend(displayQueue, msg, pdMS_TO_TICKS(50)) == pdPASS) {
        return 1;
    }

    return 0;
}

uint8_t AppDisplay_ShowText(const char *line1,
                            const char *line2,
                            const char *line3,
                            const char *line4)
{
    AppDisplayMsg_t msg;

    AppDisplay_CopyLine(msg.line1, line1);
    AppDisplay_CopyLine(msg.line2, line2);
    AppDisplay_CopyLine(msg.line3, line3);
    AppDisplay_CopyLine(msg.line4, line4);

    return AppDisplay_SendMessage(&msg);
}

void AppDisplay_Task(void *argument)
{
    AppDisplayMsg_t msg;

    vTaskDelay(pdMS_TO_TICKS(50));

    OLED_Init();

    AppDisplay_ShowText("FreeRTOS Door",
                        "System Boot",
                        "UART: OK",
                        "OLED: OK");

    for (;;) {
        if (xQueueReceive(displayQueue, &msg, portMAX_DELAY) == pdPASS) {
            AppDisplay_DrawMessage(&msg);
        }
    }
}