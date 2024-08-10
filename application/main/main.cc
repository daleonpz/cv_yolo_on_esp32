/* Copyright 2019 The TensorFlow Authors. All Rights Reserved.

   Licensed under the Apache License, Version 2.0 (the "License");
   you may not use this file except in compliance with the License.
   You may obtain a copy of the License at

http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
==============================================================================*/

#include "ble_service.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include "esp_system.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "main_functions.h"

void tf_main(void) {
  setup();
  while (true) {
    loop();
  }
}

#define GPIO_LED_RED GPIO_NUM_21
#define GPIO_LED_WHITE GPIO_NUM_22

void gpio_led_init(void) {
  gpio_config_t io_conf;
  io_conf.mode = GPIO_MODE_OUTPUT;
  io_conf.pull_up_en = GPIO_PULLUP_ENABLE;
  io_conf.intr_type = GPIO_INTR_DISABLE;
  io_conf.pin_bit_mask = (1LL << GPIO_LED_RED) | (1LL << GPIO_LED_WHITE);
  gpio_config(&io_conf);
}

void gpio_led_toggle(void) {}

void gpio_led_task(void *pvParameter) {
  static int toggle = 0;
  while (true) {
    toggle = !toggle;
    gpio_set_level(GPIO_LED_RED, toggle);
    gpio_set_level(GPIO_LED_WHITE, toggle);
    vTaskDelay(1000 / portTICK_PERIOD_MS);
  }
}

void vTasksendNotification(void *pvParameters) {
  BLE_Service *ble_service = new BLE_Service();
  ble_service->Start();
  while (1) {
    if (ble_service->AreThereSubscribers()) {
      ble_service->SendNotification();
    } else {
      printf("No one subscribed to notifications\n");
    }
    vTaskDelay(2000 / portTICK_PERIOD_MS);
  }
  vTaskDelete(NULL);
}

extern "C" void app_main() {
  gpio_led_init();  // To save Stack size with gpio_led_task

  // xTaskCreatePinnedToCore instead of xTaskCreate to split tasks between cores
  // otherwise, core 0 will be overloaded
  xTaskCreatePinnedToCore((TaskFunction_t)&tf_main, "tf_main", 5 * 1024, NULL,
                          8, NULL, 0);
  xTaskCreatePinnedToCore((TaskFunction_t)&gpio_led_task, "gpio_led_task",
                          configMINIMAL_STACK_SIZE, NULL, 8, NULL, 1);
  //     xTaskCreatePinnedToCore((TaskFunction_t)&vTasksendNotification,
  //     "vTasksendNotification", 2*1024, NULL, 2, NULL, 1);
  vTaskDelete(NULL);
}
