#include "app_status.h"

#include "FreeRTOS.h"
#include "semphr.h"

#include <string.h>

static AppStatus_t appStatus;
static SemaphoreHandle_t appStatusMutex = NULL;

void AppStatus_Init(void)
{
    appStatusMutex = xSemaphoreCreateMutex();

    appStatus.mode = APP_MODE_AUTO;
    appStatus.doorState = APP_DOOR_CLOSED;
    appStatus.alarmState = APP_ALARM_OFF;

    appStatus.distanceCm = 0;
    appStatus.temperatureC = 0;
    appStatus.humidity = 0;

    appStatus.uartOk = 1;
    appStatus.oledOk = 1;
    appStatus.sensorOk = 0;
}

void AppStatus_SetMode(AppMode_t mode)
{
    if (appStatusMutex != NULL) {
        xSemaphoreTake(appStatusMutex, portMAX_DELAY);
    }

    appStatus.mode = mode;

    if (appStatusMutex != NULL) {
        xSemaphoreGive(appStatusMutex);
    }
}

AppMode_t AppStatus_GetMode(void)
{
    AppMode_t mode;

    if (appStatusMutex != NULL) {
        xSemaphoreTake(appStatusMutex, portMAX_DELAY);
    }

    mode = appStatus.mode;

    if (appStatusMutex != NULL) {
        xSemaphoreGive(appStatusMutex);
    }

    return mode;
}

void AppStatus_SetDoorState(AppDoorState_t state)
{
    if (appStatusMutex != NULL) {
        xSemaphoreTake(appStatusMutex, portMAX_DELAY);
    }

    appStatus.doorState = state;

    if (appStatusMutex != NULL) {
        xSemaphoreGive(appStatusMutex);
    }
}

AppDoorState_t AppStatus_GetDoorState(void)
{
    AppDoorState_t state;

    if (appStatusMutex != NULL) {
        xSemaphoreTake(appStatusMutex, portMAX_DELAY);
    }

    state = appStatus.doorState;

    if (appStatusMutex != NULL) {
        xSemaphoreGive(appStatusMutex);
    }

    return state;
}

void AppStatus_SetAlarmState(AppAlarmState_t state)
{
    if (appStatusMutex != NULL) {
        xSemaphoreTake(appStatusMutex, portMAX_DELAY);
    }

    appStatus.alarmState = state;

    if (appStatusMutex != NULL) {
        xSemaphoreGive(appStatusMutex);
    }
}

AppAlarmState_t AppStatus_GetAlarmState(void)
{
    AppAlarmState_t state;

    if (appStatusMutex != NULL) {
        xSemaphoreTake(appStatusMutex, portMAX_DELAY);
    }

    state = appStatus.alarmState;

    if (appStatusMutex != NULL) {
        xSemaphoreGive(appStatusMutex);
    }

    return state;
}

void AppStatus_SetDistance(uint16_t distanceCm)
{
    if (appStatusMutex != NULL) {
        xSemaphoreTake(appStatusMutex, portMAX_DELAY);
    }

    appStatus.distanceCm = distanceCm;

    if (appStatusMutex != NULL) {
        xSemaphoreGive(appStatusMutex);
    }
}

uint16_t AppStatus_GetDistance(void)
{
    uint16_t distanceCm;

    if (appStatusMutex != NULL) {
        xSemaphoreTake(appStatusMutex, portMAX_DELAY);
    }

    distanceCm = appStatus.distanceCm;

    if (appStatusMutex != NULL) {
        xSemaphoreGive(appStatusMutex);
    }

    return distanceCm;
}

void AppStatus_SetTemperature(int16_t temperatureC)
{
    if (appStatusMutex != NULL) {
        xSemaphoreTake(appStatusMutex, portMAX_DELAY);
    }

    appStatus.temperatureC = temperatureC;

    if (appStatusMutex != NULL) {
        xSemaphoreGive(appStatusMutex);
    }
}

int16_t AppStatus_GetTemperature(void)
{
    int16_t temperatureC;

    if (appStatusMutex != NULL) {
        xSemaphoreTake(appStatusMutex, portMAX_DELAY);
    }

    temperatureC = appStatus.temperatureC;

    if (appStatusMutex != NULL) {
        xSemaphoreGive(appStatusMutex);
    }

    return temperatureC;
}

void AppStatus_SetHumidity(uint8_t humidity)
{
    if (appStatusMutex != NULL) {
        xSemaphoreTake(appStatusMutex, portMAX_DELAY);
    }

    appStatus.humidity = humidity;

    if (appStatusMutex != NULL) {
        xSemaphoreGive(appStatusMutex);
    }
}

uint8_t AppStatus_GetHumidity(void)
{
    uint8_t humidity;

    if (appStatusMutex != NULL) {
        xSemaphoreTake(appStatusMutex, portMAX_DELAY);
    }

    humidity = appStatus.humidity;

    if (appStatusMutex != NULL) {
        xSemaphoreGive(appStatusMutex);
    }

    return humidity;
}

void AppStatus_SetUartOk(uint8_t ok)
{
    if (appStatusMutex != NULL) {
        xSemaphoreTake(appStatusMutex, portMAX_DELAY);
    }

    appStatus.uartOk = ok ? 1 : 0;

    if (appStatusMutex != NULL) {
        xSemaphoreGive(appStatusMutex);
    }
}

void AppStatus_SetOledOk(uint8_t ok)
{
    if (appStatusMutex != NULL) {
        xSemaphoreTake(appStatusMutex, portMAX_DELAY);
    }

    appStatus.oledOk = ok ? 1 : 0;

    if (appStatusMutex != NULL) {
        xSemaphoreGive(appStatusMutex);
    }
}

void AppStatus_SetSensorOk(uint8_t ok)
{
    if (appStatusMutex != NULL) {
        xSemaphoreTake(appStatusMutex, portMAX_DELAY);
    }

    appStatus.sensorOk = ok ? 1 : 0;

    if (appStatusMutex != NULL) {
        xSemaphoreGive(appStatusMutex);
    }
}

void AppStatus_GetSnapshot(AppStatus_t *snapshot)
{
    if (snapshot == NULL) {
        return;
    }

    if (appStatusMutex != NULL) {
        xSemaphoreTake(appStatusMutex, portMAX_DELAY);
    }

    memcpy(snapshot, &appStatus, sizeof(AppStatus_t));

    if (appStatusMutex != NULL) {
        xSemaphoreGive(appStatusMutex);
    }
}

const char *AppStatus_ModeToString(AppMode_t mode)
{
    switch (mode) {
    case APP_MODE_AUTO:
        return "AUTO";

    case APP_MODE_MANUAL:
        return "MANUAL";

    default:
        return "UNKNOWN";
    }
}

const char *AppStatus_DoorStateToString(AppDoorState_t state)
{
    switch (state) {
    case APP_DOOR_CLOSED:
        return "CLOSED";

    case APP_DOOR_OPEN:
        return "OPEN";

    default:
        return "UNKNOWN";
    }
}

const char *AppStatus_AlarmStateToString(AppAlarmState_t state)
{
    switch (state) {
    case APP_ALARM_OFF:
        return "OFF";

    case APP_ALARM_ON:
        return "ON";

    default:
        return "UNKNOWN";
    }
}