/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * File Name          : freertos.c
 * Description        : Code for freertos applications
 ******************************************************************************
 * @attention
 *
 * <h2><center>&copy; Copyright (c) 2019 STMicroelectronics.
 * All rights reserved.</center></h2>
 *
 * This software component is licensed by ST under Ultimate Liberty license
 * SLA0044, the "License"; You may not use this file except in compliance with
 * the License. You may obtain a copy of the License at:
 *                             www.st.com/SLA0044
 *
 ******************************************************************************
 */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "FreeRTOS.h"
#include "cmsis_os.h"
#include "main.h"
#include "task.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "drive_task.h"
#include "encoder_task.h"
#include "orchard_rover_sys.h"
#include "remote_receive_task.h"

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN Variables */
osThreadId DriveTaskHandle;
osThreadId EncoderTaskHandle;
osThreadId RemoteReceiveTaskHandle;

// osThreadId SendTaskHandle;
// osThreadId ReceiveTaskHandle;
// osThreadId MiddleTaskHandle;
// // QueueHandle_t Key_Queue;            //消息队列
// SemaphoreHandle_t BinarySemaphore;  //二�?�信号量
// SemaphoreHandle_t
//     CountSemaphore;
//     //计数型信号量，cubeMX配置时默认不用计数信号量的，要手动Enable
// SemaphoreHandle_t MutexSemaphore;      //互斥信号�???
// EventGroupHandle_t EventGroupHandler;  //事件标志组句�???
// void SendTask(void const* argument);
// void ReceiveTask(void const* argument);
// void MiddleTask(void const* argument);

/* USER CODE END Variables */
osThreadId defaultTaskHandle;

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN FunctionPrototypes */

/* USER CODE END FunctionPrototypes */

void StartDefaultTask(void const* argument);

void MX_FREERTOS_Init(void); /* (MISRA C 2004 rule 8.1) */

/**
 * @brief  FreeRTOS initialization
 * @param  None
 * @retval None
 */
void MX_FREERTOS_Init(void) {
    /* USER CODE BEGIN Init */

    /* USER CODE END Init */

    /* USER CODE BEGIN RTOS_MUTEX */
    /* add mutexes, ... */
    /* USER CODE END RTOS_MUTEX */

    /* USER CODE BEGIN RTOS_SEMAPHORES */
    /* add semaphores, ... */
    /* USER CODE END RTOS_SEMAPHORES */

    /* USER CODE BEGIN RTOS_TIMERS */
    /* start timers, add new ones, ... */
    /* USER CODE END RTOS_TIMERS */

    /* USER CODE BEGIN RTOS_QUEUES */
    /* add queues, ... */
    /* USER CODE END RTOS_QUEUES */

    /* Create the thread(s) */
    /* definition and creation of defaultTask */
    osThreadDef(defaultTask, StartDefaultTask, osPriorityNormal, 0, 8);
    defaultTaskHandle = osThreadCreate(osThread(defaultTask), NULL);

    /* USER CODE BEGIN RTOS_THREADS */
    /* add threads, ... */

    // osThreadDef(Send_Task, SendTask, osPriorityNormal, 0, 64);
    // SendTaskHandle = osThreadCreate(osThread(Send_Task), NULL);

    // osThreadDef(Middle_Task, MiddleTask, osPriorityHigh, 0, 64);
    // MiddleTaskHandle = osThreadCreate(osThread(Middle_Task), NULL);

    // osThreadDef(Receive_Task, ReceiveTask, osPriorityBelowNormal, 0, 64);
    // ReceiveTaskHandle = osThreadCreate(osThread(Receive_Task), NULL);

    // freeRTOS创建任务时，128是指128个字，也就是128*4个字�?

    osThreadDef(EncoderTask, Encoder_Task, osPriorityHigh, 0, 128);
    EncoderTaskHandle = osThreadCreate(osThread(EncoderTask), NULL);

    osThreadDef(DriveTask, Drive_Task, osPriorityAboveNormal, 0, 128);
    DriveTaskHandle = osThreadCreate(osThread(DriveTask), NULL);

    osThreadDef(RemoteReceiveTask, Remote_Receive_Task, osPriorityNormal, 0,
                64);
    RemoteReceiveTaskHandle = osThreadCreate(osThread(RemoteReceiveTask), NULL);

    /* USER CODE END RTOS_THREADS */
}

/* USER CODE BEGIN Header_StartDefaultTask */
/**
 * @brief  Function implementing the defaultTask thread.
 * @param  argument: Not used
 * @retval None
 */
/* USER CODE END Header_StartDefaultTask */
void StartDefaultTask(void const* argument) {
    /* USER CODE BEGIN StartDefaultTask */
    /* Infinite loop */
    for (;;) {
        osDelay(1000);
    }
    /* USER CODE END StartDefaultTask */
}

/* Private application code --------------------------------------------------*/
/* USER CODE BEGIN Application */

// int32_t send_flag;
// uint32_t send_val;

// void SendTask(void const* argument) {
//     // Key_Queue = xQueueCreate(1, sizeof(uint8_t));//消息队列
//     // uint8_t key = 0;
//     // BinarySemaphore = xSemaphoreCreateBinary();
//     // CountSemaphore = xSemaphoreCreateCounting(2, 0);  //创建计数型信号量

//     // MutexSemaphore = xSemaphoreCreateMutex();  //创建互斥信号
//     //
//     xSemaphoreGive(MutexSemaphore);//试试将MutexSemaphore搞成二�?�信号量看看，注意二值信号量要先释放了，才能让任务们获取，这与互斥信号量不同

//     // EventGroupHandler = xEventGroupCreate();  //创建事件标志�???

//     while (1) {
//         //发给消息队列，等�???10ms，如果没有阻塞，key++
//         // BaseType_t err = xQueueSend(Key_Queue, &send_val, 1000);
//         // if (err != errQUEUE_FULL) {
//         //     send_val++;
//         // }

//         // xSemaphoreGive(BinarySemaphore);  //释放二�?�信号量
//         // send_val++;

//         // BaseType_t err = xSemaphoreGive(CountSemaphore);
//         //释放计数型信号量
//         // if (err == pdFALSE) {
//         //     send_flag = -1;
//         // } else {
//         //     send_flag = 1;
//         // }
//         // send_val = uxSemaphoreGetCount(CountSemaphore); //获取计数型信号量

//         // xSemaphoreTake(MutexSemaphore, portMAX_DELAY);  //获取互斥信号
//         // send_val++;
//         // HAL_Delay(100);
//         // xSemaphoreGive(MutexSemaphore);  //释放信号

//         /***设置事件�?***/
//         // if (send_val == 1)
//         //     xEventGroupSetBits(EventGroupHandler, 1);
//         // else if (send_val == 2)
//         //     xEventGroupSetBits(EventGroupHandler, 2);
//         // else if (send_val == 3)
//         //     xEventGroupSetBits(EventGroupHandler, 4);

//         /***任务通知***/
//         // xTaskNotifyGive(
//         //     ReceiveTaskHandle);  //任务通知模拟二�?�信号量，计数型信号�?

//         // BaseType_t err = xTaskNotify(
//         //任务通知模拟消息队列，队列长度只能是1
//         //     (TaskHandle_t)ReceiveTaskHandle,  //接收任务通知的任务句�?
//         //     (uint32_t)send_val,               //任务通知�?
//         //     (eNotifyAction)eSetValueWithOverwrite);
//         //覆写的方式发送任务�?�知
//         // send_val++;

//         // if (send_val == 1)
//         //     xTaskNotify(
//         //         (TaskHandle_t)ReceiveTaskHandle,  //接收任务通知的任务句�?
//         //         (uint32_t)1,                      //要更新的bit
//         //         (eNotifyAction)eSetBits);         //更新指定的bit
//         // else if (send_val == 2)
//         //     xTaskNotify(
//         //         (TaskHandle_t)ReceiveTaskHandle,  //接收任务通知的任务句�?
//         //         (uint32_t)2,                      //要更新的bit
//         //         (eNotifyAction)eSetBits);         //更新指定的bit

//         if (send_val == 1)
//             osSignalSet(ReceiveTaskHandle, 1);
//         else if (send_val == 2)
//             osSignalSet(ReceiveTaskHandle, 2);
//         send_val = 0;
//         osDelay(200);
//     }
// }

// void MiddleTask(void const* argument) {
//     uint8_t* ptr;
//     osDelay(50);
//     for (;;) {
//         // ptr = pvPortMalloc(100);
//         // if (send_flag == 1)
//         //     vPortFree(ptr);
//         // send_val = xPortGetFreeHeapSize();

//         // send_flag = xEventGroupGetBits(EventGroupHandler);

//         // send_flag++;

//         // EventBits_t EventValue = xEventGroupWaitBits(
//         //     (EventGroupHandle_t)EventGroupHandler, (EventBits_t)7,
//         //     (BaseType_t)pdTRUE, (BaseType_t)pdTRUE,
//         //     (TickType_t)portMAX_DELAY);
//         // send_flag++;

//         osDelay(10);
//     }
// }

// uint32_t rec_val;

// void ReceiveTask(void const* argument) {
//     osDelay(50);
//     while (1) {
//         //获取信号量，如果取不到，就无限延时，任务阻塞
//         // uint8_t key;
//         // if (xQueueReceive(Key_Queue, &key, portMAX_DELAY)) {
//         //     rec_val = key;
//         // } else {
//         //     //不会进入这里
//         //     rec_val = 0;
//         // }

//         // BaseType_t err =
//         //     xSemaphoreTake(BinarySemaphore, portMAX_DELAY);  //获取信号
//         // if (err == pdTRUE)  //获取信号量成�???
//         // {
//         //     rec_val++;
//         // }

//         // xSemaphoreTake(CountSemaphore, portMAX_DELAY);  //等待数计数信号量
//         // rec_val = uxSemaphoreGetCount(CountSemaphore);

//         // xSemaphoreTake(MutexSemaphore, portMAX_DELAY);  //获取互斥信号
//         // rec_val++;
//         // HAL_Delay(1000);
//         // xSemaphoreGive(MutexSemaphore);  //释放信号

//         //等待事件组中的相应事件位
//         // EventBits_t EventValue = xEventGroupWaitBits(
//         //     (EventGroupHandle_t)EventGroupHandler, (EventBits_t)3,
//         //     (BaseType_t)pdTRUE, (BaseType_t)pdTRUE,
//         //     (TickType_t)portMAX_DELAY);
//         // rec_val++;

//         /***任务通知***/
//         // uint32_t NotifyValue = ulTaskNotifyTake(
//         //     pdTRUE,
//         //     portMAX_DELAY);  //模拟二�?�信号量，则返回值为1
//         //                      // uint32_t NotifyValue =
//         //                      ulTaskNotifyTake(pdFALSE,
//         //                      // portMAX_DELAY);
//         //                      //
//         //如果是模拟计数型信号量，则返回�?�为计数�?
//         // rec_val = NotifyValue;

//         // BaseType_t err =
//         //     xTaskNotifyWait((uint32_t)0x00,  //进入函数的时候不清除任务bit
//         //                     (uint32_t)ULONG_MAX,
//         //                     //�?出函数的时�?�清除所有的bit
//         //                     (uint32_t*)&rec_val,         //保存任务通知�?
//         //                     (TickType_t)portMAX_DELAY);  //阻塞时间

//         // uint32_t NotifyValue;

//         // BaseType_t err =
//         //     xTaskNotifyWait((uint32_t)0x00,  //进入函数的时候不清除任务bit
//         //                     (uint32_t)ULONG_MAX,
//         //                     //�?出函数的时�?�清除所有的bit
//         //                     (uint32_t*)&NotifyValue,     //保存任务通知�?
//         //                     (TickType_t)portMAX_DELAY);  //阻塞时间

//         // if (err == pdPASS)  //任务通知获取成功
//         // {
//         //     if ((NotifyValue & 1) != 0)  //事件0发生
//         //     {
//         //         rec_val = 1;
//         //     } else if ((NotifyValue & 2) != 0)  //事件1发生
//         //     {
//         //         rec_val = 2;
//         //     }
//         // }

//         osEvent event = osSignalWait(1 | 3, osWaitForever);
//         if (event.status == osEventSignal) {
//             if (event.value.signals & 1) {
//                 rec_val += 1;
//             }
//             if (event.value.signals & 2) {
//                 rec_val += 10;
//             }
//         }

//         osDelay(1000);
//     }
// }

/* USER CODE END Application */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
