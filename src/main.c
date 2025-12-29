#include <FreeRTOS.h>
#include <newlib-freertos.h>
#include <task.h>
#include <stdio.h>

void helloTask(void *pvParameters)
{
  printf("Hello, FreeRTOS!\n");
  vTaskDelete(NULL);
}

int main(void)
{
  xTaskCreate(helloTask, "hello", 256, NULL, 1, NULL);
  vTaskStartScheduler();

  return 0;
}