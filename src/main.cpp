
extern "C"
{
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/task.h"
#include "esp_timer.h"
#include "esp_log.h"
#include "string.h"
}
#include <cstring>
#include <cstdint>
#include <string>
#include <iostream>

static const char *TAG = "main";

struct message
{
    int counter;
    char data[128];
};

struct commander_parameters
{
    QueueHandle_t xUdpQueue;
};

struct udp_server_parameters
{
    QueueHandle_t xUdpQueue;
};

void udp_server_task(void *pvParameters);
void commander_task(void *pvParameters);

extern "C" void app_main()
{
    BaseType_t xReturned;

    QueueHandle_t xUdpQueue = xQueueCreate(100, sizeof(message));

    udp_server_parameters udp_parm = {
        .xUdpQueue = xUdpQueue,
    };

    commander_parameters commander_parm = {
        .xUdpQueue = xUdpQueue,
    };

    xReturned = xTaskCreate(commander_task, "commander_task", 1024 * 16, &commander_parm, 11, NULL);
    if (xReturned != pdPASS)
    {
        ESP_LOGE(TAG, "Failed to create Commander Task");
    }

    xReturned = xTaskCreate(udp_server_task, "udp_task", 1024 * 16, &udp_parm, 11, NULL);
    if (xReturned != pdPASS)
    {
        ESP_LOGE(TAG, "Failed to create UDP Server Task");
    }

    vTaskStartScheduler();
}

void udp_server_task(void *pvParameters)
{
    // ESP_LOGI(TAG, "UDP Server Task starting up...");
    QueueHandle_t xUdpQueue;
    char rx_buffer[128]={};
    message udp_message;
    int stop = 0;
    TickType_t sendTime;

    udp_server_parameters *pTaskParameters = (udp_server_parameters *)pvParameters;
    if (NULL == pTaskParameters ||
        NULL == pTaskParameters->xUdpQueue)
    {
        // ESP_LOGE(TAG, "UDP Task parameters were missing, exiting.");
        vTaskDelete(NULL); // Delete self.
    }
    // ESP_LOGE(TAG, "UDP Task parameters were OK");

    xUdpQueue = pTaskParameters->xUdpQueue;

    // ESP_LOGE(TAG, "UDP OK");

    while (true)
    {
        if (!stop)
        {
            for (int x = 1; x < 100; x++)
            {
                sprintf(udp_message.data, "Text data:%i", x);
                udp_message.counter = x;
                sendTime = esp_timer_get_time();
                ESP_LOGW(TAG, "Write Time:%u  Int:%i  Str:%s", sendTime, udp_message.counter, udp_message.data);
                xQueueSend(xUdpQueue, &udp_message, pdMS_TO_TICKS(0));
                vTaskDelay(pdMS_TO_TICKS(5));
            }
            stop = 1;
        }
    }
}

void commander_task(void *pvParameters)
{
    // ESP_LOGD(TAG, "Commander Task starting...");
    message commander_message;
    QueueHandle_t xUdpQueue;
    TickType_t recvTime;

    commander_parameters *pTaskParameters = (commander_parameters *)pvParameters;
    if (NULL == pTaskParameters ||
        NULL == pTaskParameters->xUdpQueue)
    {
        ESP_LOGE(TAG, "Task parameters were missing, exiting.");
        vTaskDelete(NULL); // Delete self.
    }

    // ESP_LOGE(TAG, "Task parameters were OK.");

    xUdpQueue = pTaskParameters->xUdpQueue;

    // ESP_LOGE(TAG, "Got parameters");

    while (true)
    {
        if (xQueueReceive(xUdpQueue, &commander_message, portMAX_DELAY))
        {
            recvTime = esp_timer_get_time();
            ESP_LOGE(TAG, "Recv Time :%u  Int:%i   Str:%s",  recvTime, commander_message.counter, commander_message.data);
            vTaskDelay(pdMS_TO_TICKS(10));
        }
    }
}
