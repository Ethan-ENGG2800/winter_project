#ifndef APP_DISPLAY_H
#define APP_DISPLAY_H

#ifdef __cplusplus
extern "C" {
#endif

#include "main.h"
#include <stdint.h>

    /*
     * font16x16 下：
     * ASCII 字符宽度通常是 8 px
     * 128 / 8 = 16 个 ASCII 字符
     * 所以这里用 17，包括最后的 '\0'
     */
#define APP_DISPLAY_LINE_LENGTH   17

    typedef struct {
        char line1[APP_DISPLAY_LINE_LENGTH];
        char line2[APP_DISPLAY_LINE_LENGTH];
        char line3[APP_DISPLAY_LINE_LENGTH];
        char line4[APP_DISPLAY_LINE_LENGTH];
    } AppDisplayMsg_t;

    void AppDisplay_Init(void);
    void AppDisplay_Task(void *argument);

    uint8_t AppDisplay_SendMessage(const AppDisplayMsg_t *msg);
    uint8_t AppDisplay_ShowText(const char *line1,
                                const char *line2,
                                const char *line3,
                                const char *line4);

#ifdef __cplusplus
}
#endif

#endif