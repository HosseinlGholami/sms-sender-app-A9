#include "api_os.h"
#include "api_event.h"
#include "app.h"
#include "api_hal_uart.h"

#include "stdint.h"
#include "stdbool.h"



// #include "api_debug.h"


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


void uart1DataParser(API_Event_t* pEvent){
    // uint8_t data[pEvent->param2+1];
    // uint8_t content[pEvent->param2-3];
    // uint8_t phonenumber[SMS_PHONE_NUMBER_MAX_LEN];

    // memset(data,0,sizeof(data));
    // memset(content,0,sizeof(content));
    // memset(phonenumber,0,sizeof(phonenumber));

    // memcpy(data,pEvent->pParam1,pEvent->param2);   
    // if (strncmp(data,"$SE!",4)==0){
    //     pEvent->id=API_EVENT_ID_SET_BLINKING;
    //     OS_SendEvent(gpioTaskHandle,pEvent,OS_WAIT_FOREVER,OS_EVENT_PRI_URGENT);
    //     OS_Sleep(1);

    //     memcpy(phonenumber,data+4,13);
    //     memcpy(content,data+18,pEvent->param2-17);
    
    //     sms_send(phonenumber,content);
    // }
    // else{
        uart_sender("khiyar");
    // }

}

static void UartEventDispatch(API_Event_t* pEvent)
{
    switch(pEvent->id)
    {
        case API_EVENT_ID_UART_RECEIVED:
            if(pEvent->param1 == UART1)
            {
                uart1DataParser(pEvent);
            }
            break;
        default:
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
        OS_Sleep(100);
        if(OS_WaitEvent(Handle->uartTaskHandle,(void *) &event, OS_TIME_OUT_WAIT_FOREVER))
        {
            
            UartEventDispatch(event);

            OS_Free(event->pParam1);
            OS_Free(event->pParam2);
            OS_Free(event);
        }
    }

}