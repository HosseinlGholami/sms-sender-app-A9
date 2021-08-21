#include "api_os.h"
#include "api_event.h"

#include "stdint.h"

#include "stdbool.h"
#include "api_debug.h"

#include "api_hal_pm.h"

#include "app.h"



// #include "api_inc_network.h"
// #include "api_info.h"
// #include "api_sim.h"


Task_Handels Handel = {
    .mainTaskHandle=NULL,
    .gpioTaskHandle=NULL,
    .uartTaskHandle=NULL,
    .smsTaskHandle=NULL
};

uint8_t Network_stable = 0;

void Hi_scenario_timer(void* param){
    API_Event_t* event=NULL;
    event=(API_Event_t *)OS_Malloc(sizeof(API_Event_t));
    event->id=API_EVENT_ID_UART_WRTIE;
    event->pParam1=(char *)OS_Malloc(sizeof("Hi"));
    event->pParam2=(char *)OS_Malloc(NULL);
    
    event->pParam1="Hi";
    OS_SendEvent(Handel.uartTaskHandle,(void*)event,OS_WAIT_FOREVER,OS_EVENT_PRI_NORMAL);
    uart_sender("in timer");
    OS_StartCallbackTimer(Handel.mainTaskHandle,3*1000,Hi_scenario_timer,NULL);
}

void Stop_Hi_scenario_timer(void* param){
    API_Event_t* event=NULL;
    event=(API_Event_t *)OS_Malloc(sizeof(API_Event_t));
    event->id=API_EVENT_ID_UART_WRTIE;
    event->pParam1=(char *)OS_Malloc(sizeof("who"));
    event->pParam2=(char *)OS_Malloc(NULL);
    
    event->pParam1="who";
    OS_SendEvent(Handel.uartTaskHandle,(void*)event,OS_WAIT_FOREVER,OS_EVENT_PRI_NORMAL);
    uart_sender("in timer");
    OS_StartCallbackTimer(Handel.mainTaskHandle,3*1000,Stop_Hi_scenario_timer,NULL);
}

void MainEventDispatch(API_Event_t* pEvent)
{
    switch(pEvent->id)
    {
    // //related to uart
        case API_EVENT_ID_UART_RECEIVED:
            OS_SendEvent(Handel.uartTaskHandle,(void *)pEvent,OS_WAIT_FOREVER,OS_EVENT_PRI_NORMAL);
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
        case API_EVENT_ID_STOP_HI_TIMER:
            OS_StopCallbackTimer(Handel.mainTaskHandle,Hi_scenario_timer,NULL);
            OS_StartCallbackTimer(Handel.mainTaskHandle,250,Stop_Hi_scenario_timer,NULL);
            break;
        case API_EVENT_ID_STOP_WHO_TIMER:
            OS_StopCallbackTimer(Handel.mainTaskHandle,Stop_Hi_scenario_timer,NULL);
            break;    
    //end related to gsm network
        case API_EVENT_ID_SMS_SENT:
        case API_EVENT_ID_SMS_RECEIVED:
        case API_EVENT_ID_SMS_LIST_MESSAGE:
        case API_EVENT_ID_SMS_ERROR:
            OS_SendEvent(Handel.smsTaskHandle,(void *)pEvent,OS_WAIT_FOREVER,OS_EVENT_PRI_NORMAL);
            break;    
        default:
            break;
    }
    
    //system initialize complete and network register complete, now can send message
    if(Network_stable == 3)
    {
        Network_stable=0;
        OS_StartCallbackTimer(Handel.mainTaskHandle,5,Hi_scenario_timer,NULL);
    }
}

void MainTask(void *pData)
{
    API_Event_t* event=NULL;

    //  = OS_CreateTask(GPIO_TestTask,
    //     NULL, NULL, TEST_TASK_STACK_SIZE, TEST_TASK_PRIORITY, 0, 0, "GPIO Test Task");
    
    Handel.uartTaskHandle = OS_CreateTask(UART_TestTask,
        (void *)&Handel, NULL, MAIN_TASK_STACK_SIZE*7, TEST_TASK_PRIORITY, 0, 0, "UART Test Task");

    Handel.smsTaskHandle= OS_CreateTask(SMS_TestTask,
        (void *)&Handel, NULL, MAIN_TASK_STACK_SIZE*7, TEST_TASK_PRIORITY, 0, 0, "sms Test Task");
  
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
        NULL, NULL, MAIN_TASK_STACK_SIZE*7.5, MAIN_TASK_PRIORITY, 0, 0, MAIN_TASK_NAME);
    OS_SetUserMainHandle(&Handel.mainTaskHandle);
}
