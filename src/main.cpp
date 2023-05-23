
extern "C"
{
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "string.h"
}
#include <string>

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

    QueueHandle_t xUdpQueue = xQueueCreate(20, sizeof(message));

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
    QueueHandle_t x_TestQueue;
    char rx_buffer[128]={};
    message udp_message;


    udp_server_parameters *pTaskParameters = (udp_server_parameters *)pvParameters;
    if (NULL == pTaskParameters ||
        NULL == pTaskParameters->xUdpQueue)
    {
        // ESP_LOGE(TAG, "UDP Task parameters were missing, exiting.");
        vTaskDelete(NULL); // Delete self.
    }
    // ESP_LOGE(TAG, "UDP Task parameters were OK");

    x_TestQueue = pTaskParameters->xUdpQueue;

    // ESP_LOGE(TAG, "UDP OK");

    while (true)
    {
        for (int x = 1; x < 10; x++)
        {
            sprintf(rx_buffer, "Text data:%i", x);
            printf("%s\n", rx_buffer);
            udp_message.counter = x;
            memcpy(udp_message.data, rx_buffer,  strlen(rx_buffer));
            printf("Write int:%i\n", udp_message.counter);
            printf("Write char:%s\n", udp_message.data);
            xQueueSend(x_TestQueue, &udp_message, pdMS_TO_TICKS(0));
            vTaskDelay(pdMS_TO_TICKS(500));
        }
    }
}

void commander_task(void *pvParameters)
{
    // ESP_LOGD(TAG, "Commander Task starting...");
    message commander_message;
    QueueHandle_t x_TestQueue;

    commander_parameters *pTaskParameters = (commander_parameters *)pvParameters;
    if (NULL == pTaskParameters ||
        NULL == pTaskParameters->xUdpQueue)
    {
        ESP_LOGE(TAG, "Task parameters were missing, exiting.");
        vTaskDelete(NULL); // Delete self.
    }

    // ESP_LOGE(TAG, "Task parameters were OK.");

    x_TestQueue = pTaskParameters->xUdpQueue;

    // ESP_LOGE(TAG, "Got parameters");

    while (true)
    {
        if (xQueueReceive(x_TestQueue, &commander_message, portMAX_DELAY))
        {
            printf("Recv Buffer int:%i\n", commander_message.counter);
            printf("Recv Buffer str:%s\n", commander_message.data);
            vTaskDelay(pdMS_TO_TICKS(1000));
        }
    }
}
