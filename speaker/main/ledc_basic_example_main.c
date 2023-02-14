/* LEDC (LED Controller) basic example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/
#include <stdio.h>
#include "driver/ledc.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#define LEDC_MODE               LEDC_LOW_SPEED_MODE
//#define LEDC_OUTPUT_IO          (5) // Define the output GPIO
#define LEDC_DUTY               (4095) // Set duty to 50%. ((2 ** 13) - 1) * 50% = 4095
#define LEDC_FREQUENCY          (5000) // Frequency in Hertz. Set frequency at 5 kHz

#define BEEPER_GPIO 5
#define BEEPER_TIMER LEDC_TIMER_0
#define BEEPER_CHANNEL LEDC_CHANNEL_0
#define BEEPER_RESOLUTION LEDC_TIMER_13_BIT


void beeper_init()
{
    ledc_timer_config_t ledc_timer = {
        .duty_resolution = LEDC_TIMER_13_BIT,
        .freq_hz = 1000,
        .speed_mode = LEDC_HIGH_SPEED_MODE,
        .timer_num = LEDC_TIMER_0,
        .clk_cfg = LEDC_AUTO_CLK,
    };

    ledc_timer_config(&ledc_timer);

    ledc_channel_config_t channel_conf = {
        .channel = BEEPER_CHANNEL,
        .duty = 0,
        .gpio_num = BEEPER_GPIO,
        .intr_type = LEDC_INTR_DISABLE,
        .speed_mode = LEDC_HIGH_SPEED_MODE,
        .timer_sel = BEEPER_TIMER
    };
    ledc_channel_config(&channel_conf);
}

void beeper_set_frequency(int freq)
{
    ledc_timer_config_t timer_conf = {
        .duty_resolution = BEEPER_RESOLUTION,
        .freq_hz = freq,
        .speed_mode = LEDC_HIGH_SPEED_MODE,
        .timer_num = BEEPER_TIMER
    };
    ledc_timer_config(&timer_conf);
}

void beeper_on()
{
    ledc_set_duty(LEDC_HIGH_SPEED_MODE, BEEPER_CHANNEL, 4095);
    ledc_update_duty(LEDC_HIGH_SPEED_MODE, BEEPER_CHANNEL);
}

void beeper_off()
{
    ledc_set_duty(LEDC_HIGH_SPEED_MODE, BEEPER_CHANNEL, 0);
    ledc_update_duty(LEDC_HIGH_SPEED_MODE, BEEPER_CHANNEL);
}

void app_main(void)
{
    beeper_init();
    beeper_set_frequency(440);
    volatile bool approved = true;
    if (approved)
    {
        beeper_set_frequency(440);
        beeper_on();
        vTaskDelay(pdMS_TO_TICKS(2000));
        beeper_off();
        vTaskDelay(pdMS_TO_TICKS(5000));
    }
    else
    {
        while(1)
        {
            beeper_set_frequency(440);
            beeper_on();
            vTaskDelay(pdMS_TO_TICKS(500));
            beeper_off();
            vTaskDelay(pdMS_TO_TICKS(500));
        }        
    }
    while(1);
}
