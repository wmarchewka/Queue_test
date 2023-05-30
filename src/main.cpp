
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
    std::string rx_buffer;
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

    udp_server_parameters udp_parameters = {
        .xUdpQueue = xUdpQueue,
    };

    commander_parameters commander_parameters = {
        .xUdpQueue = xUdpQueue,
    };

    xReturned = xTaskCreate(commander_task, "commander_task", 1024 * 8, &commander_parameters, 11, NULL);
    if (xReturned != pdPASS)
    {
        ESP_LOGE(TAG, "Failed to create Commander Task");
    }

    xReturned = xTaskCreate(udp_server_task, "udp_task", 1024 * 8, &udp_parameters, 12, NULL);
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
    char rx_buffer[128];
    message udp_message;
    int ret;

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
        for (int x = 1; x < 10; x++)
        {
            sprintf(rx_buffer, "%i", x);
            udp_message.rx_buffer = rx_buffer;
            printf("Write queue:%s\n", udp_message.rx_buffer.c_str());
            ret = xQueueSend(xUdpQueue, &udp_message, pdMS_TO_TICKS(0));
            vTaskDelay(pdMS_TO_TICKS(250));
        }
    }
}

void commander_task(void *pvParameters)
{
    // ESP_LOGD(TAG, "Commander Task starting...");
    message commander_message;
    QueueHandle_t xUdpQueue;

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
            printf("Recv Buffer:%s\n", commander_message.rx_buffer.c_str());
            vTaskDelay(pdMS_TO_TICKS(500));
        }
        else
        {
            printf("ERROR\n");
        }
    }
}
