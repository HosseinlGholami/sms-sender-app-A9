#include "api_os.h"
#include "api_event.h"

#include "stdint.h"

#include "stdbool.h"
#include "api_debug.h"

#include "api_hal_pm.h"

#include "app.h"

#include "api_hal_gpio.h"

#include "time.h"

#include "api_inc_network.h"
#include "api_info.h"
#include "api_sim.h"

#include "api_socket.h"
#include "api_network.h"
#include "api_sms.h"

Task_Handels Handel = {
    .mainTaskHandle=NULL,
    .gpioTaskHandle=NULL,
    .uartTaskHandle=NULL,
    .smsTaskHandle=NULL
};

uint8_t Network_stable = 0;

void MainEventDispatch(API_Event_t* pEvent)
{
    switch(pEvent->id)
    {
    // //related to uart
        case API_EVENT_ID_UART_RECEIVED:
            OS_SendEvent(Handel.uartTaskHandle,pEvent,OS_WAIT_FOREVER,OS_EVENT_PRI_NORMAL);
            break;
    //related to gsm network
        case API_EVENT_ID_NETWORK_REGISTERED_HOME:
            Network_stable |= 2;
            break;
        case API_EVENT_ID_NETWORK_REGISTERED_ROAMING:
            Network_stable |= 2;
            break;
        case API_EVENT_ID_SYSTEM_READY:
            Network_stable |= 1;
            break;
    //end related to gsm network
    
        default:
            break;
    }
    
    //system initialize complete and network register complete, now can send message
    if(Network_stable == 3)
    {
        // sms_clearAll();
        // uart_sender("Network is Stable");
    }
}

void MainTask(void *pData)
{
    API_Event_t* event=NULL;
    //  = OS_CreateTask(GPIO_TestTask,
    //     NULL, NULL, TEST_TASK_STACK_SIZE, TEST_TASK_PRIORITY, 0, 0, "GPIO Test Task");
    
    Handel.uartTaskHandle = OS_CreateTask(UART_TestTask,
        (void *)&Handel, NULL, TEST_TASK_STACK_SIZE, TEST_TASK_PRIORITY, 0, 0, "UART Test Task");

    // smsTaskHandle= OS_CreateTask(SMS_TestTask,
    //     NULL, NULL, TEST_TASK_STACK_SIZE*3, TEST_TASK_PRIORITY, 0, 0, "sms Test Task");
        
  
    while(1)
    {
        if(OS_WaitEvent(Handel.mainTaskHandle, (void**)&event, OS_TIME_OUT_WAIT_FOREVER))
        {
            MainEventDispatch(event);
            OS_Free(event->pParam1);
            OS_Free(event->pParam2);
            OS_Free(event);
        }
    }
}


void app_Main()
{
    Handel.mainTaskHandle = OS_CreateTask(MainTask ,
        NULL, NULL, MAIN_TASK_STACK_SIZE, MAIN_TASK_PRIORITY, 0, 0, MAIN_TASK_NAME);
    OS_SetUserMainHandle(&Handel.mainTaskHandle);
}
