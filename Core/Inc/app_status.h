#ifndef APP_STATUS_H
#define APP_STATUS_H

#ifdef __cplusplus
extern "C" {
#endif

#include "main.h"
#include <stdint.h>

    typedef enum {
        APP_MODE_AUTO = 0,
        APP_MODE_MANUAL
    } AppMode_t;

    typedef enum {
        APP_DOOR_CLOSED = 0,
        APP_DOOR_OPEN
    } AppDoorState_t;

    typedef enum {
        APP_ALARM_OFF = 0,
        APP_ALARM_ON
    } AppAlarmState_t;

    typedef struct {
        AppMode_t mode;
        AppDoorState_t doorState;
        AppAlarmState_t alarmState;

        uint16_t distanceCm;
        int16_t temperatureC;
        uint8_t humidity;

        uint8_t uartOk;
        uint8_t oledOk;
        uint8_t sensorOk;
    } AppStatus_t;

    void AppStatus_Init(void);

    void AppStatus_SetMode(AppMode_t mode);
    AppMode_t AppStatus_GetMode(void);

    void AppStatus_SetDoorState(AppDoorState_t state);
    AppDoorState_t AppStatus_GetDoorState(void);

    void AppStatus_SetAlarmState(AppAlarmState_t state);
    AppAlarmState_t AppStatus_GetAlarmState(void);

    void AppStatus_SetDistance(uint16_t distanceCm);
    uint16_t AppStatus_GetDistance(void);

    void AppStatus_SetTemperature(int16_t temperatureC);
    int16_t AppStatus_GetTemperature(void);

    void AppStatus_SetHumidity(uint8_t humidity);
    uint8_t AppStatus_GetHumidity(void);

    void AppStatus_SetUartOk(uint8_t ok);
    void AppStatus_SetOledOk(uint8_t ok);
    void AppStatus_SetSensorOk(uint8_t ok);

    void AppStatus_GetSnapshot(AppStatus_t *snapshot);

    const char *AppStatus_ModeToString(AppMode_t mode);
    const char *AppStatus_DoorStateToString(AppDoorState_t state);
    const char *AppStatus_AlarmStateToString(AppAlarmState_t state);

#ifdef __cplusplus
}
#endif

#endif