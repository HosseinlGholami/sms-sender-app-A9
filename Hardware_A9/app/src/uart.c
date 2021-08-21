#include "api_os.h"
#include "api_event.h"
#include "app.h"
#include "api_hal_uart.h"

#include "stdint.h"
#include "stdbool.h"


void uart_sender(char data []){
    char buffer [100];
    memset(buffer,0,sizeof(buffer));
    sprintf(buffer,"%s\r\n",data);
    UART_Write(UART1,buffer,strlen(buffer));
}
void uart_sender_int(int data ){
    char buffer [100];
    memset(buffer,0,sizeof(buffer));
    sprintf(buffer,"%d\r\n",data);
    UART_Write(UART1,buffer,strlen(buffer));
}
void HI_response(uint8_t data[],Task_Handels * pData){
    API_Event_t* event=NULL;
    event=(API_Event_t *)OS_Malloc(sizeof(API_Event_t));
    event->id=API_EVENT_ID_STOP_HI_TIMER;
    OS_SendEvent(pData->mainTaskHandle,(void*)event,OS_WAIT_FOREVER,OS_EVENT_PRI_NORMAL);
}
void WHO_response(uint8_t data[],Task_Handels * pData){
    API_Event_t* event=NULL;
    uint8_t Len = strlen(data);
    event=(API_Event_t *)OS_Malloc(sizeof(API_Event_t));
    event->id=API_EVENT_ID_SEND_WHO;
    event->pParam1=(uint8_t *)OS_Malloc(Len-5);
    event->pParam2=(uint8_t *)OS_Malloc(NULL);
    
    memcpy(event->pParam1 ,data+5, Len-5);
    event->param2=Len-5;
    
    OS_SendEvent(pData->smsTaskHandle,(void*)event,OS_WAIT_FOREVER,OS_EVENT_PRI_NORMAL);

}
void SEN_response(uint8_t data[],Task_Handels * pData){
    API_Event_t* event=NULL;
    uint8_t Len = strlen(data);
    event=(API_Event_t *)OS_Malloc(sizeof(API_Event_t));
    event->id=API_EVENT_ID_SEND_SMS;
    event->pParam1=(uint8_t *)OS_Malloc(Len-19);//used for content
    event->pParam2=(uint8_t *)OS_Malloc(13);//used for phone number
    
    memcpy(event->pParam1 ,data+19, Len-19);
    memcpy(event->pParam2 ,data+5, 13);
    event->param2=Len-19-1;
    
    OS_SendEvent(pData->smsTaskHandle,(void*)event,OS_WAIT_FOREVER,OS_EVENT_PRI_NORMAL);
}
VSE_response(uint8_t data[],Task_Handels * pData){
    API_Event_t* event=NULL;
    uint8_t Len = strlen(data);
    event=(API_Event_t *)OS_Malloc(sizeof(API_Event_t));
    event->id=API_EVENT_ID_VALIDATE_SMS;
    event->pParam1=(uint8_t *)OS_Malloc(Len-5);
    event->pParam2=(uint8_t *)OS_Malloc(NULL);
    
    memcpy(event->pParam1 ,data+5, Len-5);
    event->param2=Len-5;
    
    OS_SendEvent(pData->smsTaskHandle,(void*)event,OS_WAIT_FOREVER,OS_EVENT_PRI_NORMAL);

}

void uart1DataParser(API_Event_t* pEvent,Task_Handels * pData){
    uint8_t data[pEvent->param2+1];
    memset(data,0,sizeof(data));
    memcpy(data,pEvent->pParam1,pEvent->param2);  

    if (strncmp(data,"$HI!",4)==0){
        HI_response(data,pData);
    }
    else if (strncmp(data,"$WHO!",5)==0){
        WHO_response(data,pData);
    }
    else if (strncmp(data,"$SEN!",5)==0){
        SEN_response(data,pData);
    }
    else if (strncmp(data,"$VSE!",5)==0){
        VSE_response(data,pData);
    } 
    else{
        uart_sender("khiyar");
    }
    

}


static void UartEventDispatch(API_Event_t* pEvent,Task_Handels * pData)
{
    switch(pEvent->id)
    {
        case API_EVENT_ID_UART_RECEIVED:
            if(pEvent->param1 == UART1)
                uart1DataParser(pEvent,pData);
            break;
        case API_EVENT_ID_UART_WRTIE:
                uart_sender((char *) pEvent->pParam1);
            break;
        default:
            uart_sender("Han");
            break;
    }
}

void UART_TestTask(void* pData){
    API_Event_t* event=NULL;
    Task_Handels* Handle = (Task_Handels *) pData;

    UART_Config_t uartConfig = {
        .baudRate = UART_BAUD_RATE_115200,
        .dataBits = UART_DATA_BITS_8,
        .stopBits = UART_STOP_BITS_1,
        .parity   = UART_PARITY_NONE,
        .rxCallback = NULL,
        .useEvent = true,
    };

    UART_Init(UART1,uartConfig);

    while(1)
    {
        if(OS_WaitEvent(Handle->uartTaskHandle,(void**)&event, OS_TIME_OUT_WAIT_FOREVER))
        {
            UartEventDispatch(event,Handle);
            OS_Free(event->pParam1);
            OS_Free(event->pParam2);
            OS_Free(event);
        }
    }

}