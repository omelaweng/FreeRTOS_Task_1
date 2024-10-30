#include <stdio.h>
#include <stdbool.h>
#include <unistd.h>
#include "driver/gpio.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

TaskHandle_t MyFirstTaskHandle = NULL;
// 1. เพิ่ม MySecondTaskHandle
TaskHandle_t MySecondTaskHandle = NULL;
TaskHandle_t ISR = NULL;

// Define LED and Button pins properly using #define or constants
#define LED_PIN 27
#define PUSH_BUTTON_PIN 33

void IRAM_ATTR button_isr_handler(void *arg)
{
    xTaskResumeFromISR(ISR);
}

void interrupt_task(void *arg)
{
    bool led_status = false;
    while (1)
    {
        vTaskSuspend(NULL);
        led_status = !led_status;
        gpio_set_level(LED_PIN, led_status);
        printf("Button pressed!\n");
    }
}

void My_First_Task(void *arg)
{
    uint16_t i = 0;
    while (1)
    {
        printf("Hello My First Task %d\n", i);
        vTaskDelay(1000 / portTICK_PERIOD_MS);
        i++;

        if (i == 5)
        {
            vTaskSuspend(MySecondTaskHandle);
            printf("Second Task suspended\n");
        }

        if (i == 10)
        {
            vTaskResume(MySecondTaskHandle);
            printf("Second Task Resumed\n");
        }

        if (i == 15)
        {
            vTaskDelete(MySecondTaskHandle);
            printf("Second Task deleted\n");
        }

        if (i == 20)
        {
            printf("MyFirstTaskHandle will suspend itself\n");
            vTaskSuspend(NULL);
        }
    }
}

// 2. เพิ่ม task function
void My_Second_Task(void *arg)
{
    uint16_t i = 0;
    while (1)
    {
        printf("Hello My Second Task %d\n", i);
        vTaskDelay(1000 / portTICK_PERIOD_MS);
        i++;
    }
}

void app_main(void)
{
    // Configure the GPIO pins correctly using the given pin numbers
    esp_rom_gpio_pad_select_gpio(LED_PIN);
    esp_rom_gpio_pad_select_gpio(PUSH_BUTTON_PIN);

    // Correct direction configuration for LED and Button pins
    gpio_set_direction(LED_PIN, GPIO_MODE_OUTPUT);        // GPIO 27 as output (LED)
    gpio_set_direction(PUSH_BUTTON_PIN, GPIO_MODE_INPUT); // GPIO 33 as input (Button)

    // Set interrupt on rising edge for the button
    gpio_set_intr_type(PUSH_BUTTON_PIN, GPIO_INTR_POSEDGE);

    // Install ISR service
    gpio_install_isr_service(0);

    // Attach ISR handler for the button
    gpio_isr_handler_add(PUSH_BUTTON_PIN, button_isr_handler, NULL);

    // Create the interrupt task
    xTaskCreate(interrupt_task, "interrupt_task", 4096, NULL, 10, &ISR);
}