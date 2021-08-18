#include "api_os.h"
#include "api_event.h"

#include "stdint.h"

#include "stdbool.h"
#include "api_debug.h"

#include "api_hal_pm.h"

#include "app.h"

#include "api_hal_uart.h"
#include "api_hal_gpio.h"

#include "time.h"

#include "api_inc_network.h"
#include "api_info.h"
#include "api_sim.h"

#include "api_socket.h"
#include "api_network.h"
#include "api_sms.h"

#define VERSION "V1.0.0"

static HANDLE mainTaskHandle = NULL;
static HANDLE gpioTaskHandle = NULL;
static HANDLE uartTaskHandle = NULL;
static HANDLE rtcTaskHandle   = NULL;
static HANDLE smsTaskHandle   = NULL;

static uint8_t Network_stable = 0;

uint8_t sms_send(char number[], char* message)
{
	uint8_t* unicode = NULL;
	uint32_t unicodeLen;
	uint8_t ret = 0;

	// DBG_SMS("Sending to %s...", number);

	if(!SMS_LocalLanguage2Unicode(message, strlen(message), CHARSET_UTF_8, &unicode, &unicodeLen))
		uart_sender("Local to unicode fail");
	else
	{
		if(!SMS_SendMessage(number, unicode, unicodeLen, SIM0))
			uart_sender("$SEF!");
		else
		{
			uart_sender("$SES!");
			ret = 1;
		}

		OS_Free(unicode);
	}
	
	return ret;
}

void System_stady(void* param){
    GPIO_config_t gpioLedBlue=*(GPIO_config_t *)param;
    GPIO_LEVEL ledLevel;
    GPIO_GetLevel(gpioLedBlue,&ledLevel);
    ledLevel = (ledLevel==GPIO_LEVEL_HIGH)?GPIO_LEVEL_LOW:GPIO_LEVEL_HIGH;            
    GPIO_SetLevel(gpioLedBlue,ledLevel);        //Set level
    OS_StartCallbackTimer(gpioTaskHandle,2*1000,System_stady,param);
}
void start_blink_gpio(void* param){
    GPIO_config_t gpioLedBlue=*(GPIO_config_t *)param;
    GPIO_LEVEL ledLevel;
    GPIO_GetLevel(gpioLedBlue,&ledLevel);
    ledLevel = (ledLevel==GPIO_LEVEL_HIGH)?GPIO_LEVEL_LOW:GPIO_LEVEL_HIGH;            
    GPIO_SetLevel(gpioLedBlue,ledLevel);        //Set level
    OS_Sleep(1);
    OS_StartCallbackTimer(gpioTaskHandle,250,start_blink_gpio,param);
}
static void GpioEventDispatch(API_Event_t* pEvent,GPIO_config_t gpioLedBlue)
{
    switch(pEvent->id)
    {
        case API_EVENT_ID_SET_BLINKING:
            OS_StartCallbackTimer(gpioTaskHandle,250,start_blink_gpio,(void *)&gpioLedBlue);
            break;
        case API_EVENT_ID_RESET_BLINKING:
            OS_StopCallbackTimer(gpioTaskHandle,start_blink_gpio,(void *)&gpioLedBlue);  
            GPIO_SetLevel(gpioLedBlue,GPIO_LEVEL_LOW);
            break;
        default:
            break;
    }
}
void GPIO_TestTask()
{
    API_Event_t* event=NULL;

    GPIO_config_t gpioLedBlue1 = {
        .mode         = GPIO_MODE_OUTPUT,
        .pin          = GPIO_PIN27,
        .defaultLevel = GPIO_LEVEL_HIGH
    };
    GPIO_config_t gpioLedBlue2 = {
        .mode         = GPIO_MODE_OUTPUT,
        .pin          = GPIO_PIN28,
        .defaultLevel = GPIO_LEVEL_LOW
    };

    GPIO_Init(gpioLedBlue1);
    GPIO_Init(gpioLedBlue2);

    OS_StartCallbackTimer(gpioTaskHandle,2*1000,System_stady,(void *)&gpioLedBlue1);

    while(1)
    {
        if(OS_WaitEvent(gpioTaskHandle,(void *) &event, OS_TIME_OUT_WAIT_FOREVER))
        {
            GpioEventDispatch(event,gpioLedBlue2);
            OS_Free(event->pParam1);
            OS_Free(event->pParam2);
            OS_Free(event);
        }
    }
}

void uart1DataParser(API_Event_t* pEvent){
    uint8_t data[pEvent->param2+1];
    uint8_t content[pEvent->param2-3];
    uint8_t phonenumber[SMS_PHONE_NUMBER_MAX_LEN];

    memset(data,0,sizeof(data));
    memset(content,0,sizeof(content));
    memset(phonenumber,0,sizeof(phonenumber));

    memcpy(data,pEvent->pParam1,pEvent->param2);   
    if (strncmp(data,"$SE!",4)==0){
        pEvent->id=API_EVENT_ID_SET_BLINKING;
        OS_SendEvent(gpioTaskHandle,pEvent,OS_WAIT_FOREVER,OS_EVENT_PRI_URGENT);
        OS_Sleep(1);

        memcpy(phonenumber,data+4,13);
        memcpy(content,data+18,pEvent->param2-17);
    
        sms_send(phonenumber,content);
    }
    else{
        uart_sender("khiyar");
    }

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
void UART_TestTask(){
    API_Event_t* event=NULL;

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
        if(OS_WaitEvent(uartTaskHandle,(void *) &event, OS_TIME_OUT_WAIT_FOREVER))
        {
            if (network_stable==3){
                            UartEventDispatch(event);

            }

            OS_Free(event->pParam1);
            OS_Free(event->pParam2);
            OS_Free(event);
        }
    }

}
void SMSInit()
{
    if(!SMS_SetFormat(SMS_FORMAT_TEXT,SIM0))
    {
        Trace(4,"sms set format error");
        return;
    }
    SMS_Parameter_t smsParam = {
        .fo = 17 ,
        .vp = 167,
        .pid= 0  ,
        .dcs= 8  ,//0:English 7bit, 4:English 8 bit, 8:Unicode 2 Bytes
    };
    if(!SMS_SetParameter(&smsParam,SIM0))
    {
        Trace(1,"sms set parameter error");
        return;
    }
    if(!SMS_SetNewMessageStorage(SMS_STORAGE_SIM_CARD))
    {
        Trace(1,"sms set message storage fail");
        return;
    }
}
uint8_t sms_clearAll()
{
	// 0 = there are still some SMSs to delete
	// 1 = all SMSs deleted

	SMS_Storage_Info_t storageInfo;
	
	if(SMS_GetStorageInfo(&storageInfo, SMS_STORAGE_SIM_CARD))
	{
		// DBG_SMS("SIM card storage: %u/%u", storageInfo.used, storageInfo.total);

		// Delete starting from first SMS to however many SMSs are stored (only properly works if there are no holes in stored message IDs)
		uint8_t i = 1;
		for(;i<storageInfo.used;i++)
		{
			// SMS_DeleteMessage() always returns true even when deleting non-existing SMSs?
			if(SMS_DeleteMessage(i, SMS_STATUS_ALL, SMS_STORAGE_SIM_CARD))
				uart_sender("Delete success");
			else
				uart_sender("Delete fail");
		}

		// Check to make sure all deleted
		// If still some remaining, then continue until max ID stored in SIM card
		for(;
			SMS_GetStorageInfo(&storageInfo, SMS_STORAGE_SIM_CARD) &&
			i<storageInfo.total+1 &&
			storageInfo.used > 0
		;i++)
		{
			// SMS_DeleteMessage() always returns true even when deleting non-existing SMSs?
			if(SMS_DeleteMessage(i, SMS_STATUS_ALL, SMS_STORAGE_SIM_CARD))
				uart_sender("Delete success");
			else
				uart_sender("Delete fail");
		}

		// Check to see if all SMSs have been deleted
		if(SMS_GetStorageInfo(&storageInfo, SMS_STORAGE_SIM_CARD))
		{
			if(storageInfo.used > 0)
			{
				uart_sender("Still SMSs left!");
				return 0;
			}
			
			return 1;
		}
	}

	uart_sender("Error getting storage info");
	return 0;
}
void SmsEventDispatch(API_Event_t* pEvent)
{
    switch(pEvent->id)
    {
        case API_EVENT_ID_SMS_RECEIVED:
            uart_sender("API_EVENT_ID_SMS_RECEIVED");
            Trace(2,"received message");
            SMS_Encode_Type_t encodeType = pEvent->param1;
            uint32_t contentLength = pEvent->param2;
            uint8_t* header = pEvent->pParam1;
            uint8_t* content = pEvent->pParam2;

            Trace(2,"message header:%s",header);
            Trace(2,"message content length:%d",contentLength);
            if(encodeType == SMS_ENCODE_TYPE_ASCII)
            {
                uart_sender("ascii sms recived");
                Trace(2,"message content:%s",content);
                uart_sender(content);
                // sms_send("+989129459183",content);
            }
            else
            {
                uart_sender("unicode sms recived");
                uint8_t* gbk = NULL;
                uint32_t gbkLen = 0;
                if(!SMS_Unicode2LocalLanguage(content,contentLength,CHARSET_UTF_8,&gbk,&gbkLen))
                    uart_sender("convert unicode to GBK fail!");
                else
                {
                    uart_sender(gbk);
                    // sms_send("+989129459183",gbk);
                    // OS_Sleep(5000);
                }
                OS_Free(gbk);
            }
            break;
        case API_EVENT_ID_SMS_LIST_MESSAGE:
        {
            uart_sender("API_EVENT_ID_SMS_LIST_MESSAGE");
            SMS_Message_Info_t* messageInfo = (SMS_Message_Info_t*)pEvent->pParam1;
            Trace(1,"message header index:%d,status:%d,number type:%d,number:%s,time:\"%u/%02u/%02u,%02u:%02u:%02u+%02d\"", messageInfo->index, messageInfo->status,
                                                                                        messageInfo->phoneNumberType, messageInfo->phoneNumber,
                                                                                        messageInfo->time.year, messageInfo->time.month, messageInfo->time.day,
                                                                                        messageInfo->time.hour, messageInfo->time.minute, messageInfo->time.second,
                                                                                        messageInfo->time.timeZone);
            Trace(1,"message content len:%d,data:%s",messageInfo->dataLen,messageInfo->data);
            UART_Write(UART1, messageInfo->data, messageInfo->dataLen);//use serial tool that support GBK decode if have Chinese, eg: https://github.com/Neutree/COMTool
            //need to free data here
            OS_Free(messageInfo->data);
            break;
        }
        case API_EVENT_ID_SMS_ERROR:
            uart_sender("API_EVENT_ID_SMS_ERROR");
            Trace(10,"SMS error occured! cause:%d",pEvent->param1);
        default:
            break;
    }
    sms_clearAll();
}
void SMS_TestTask(){
    API_Event_t* event=NULL;
    SMSInit();
    while(1)
    {
        if(OS_WaitEvent(smsTaskHandle, (void**)&event, OS_TIME_OUT_WAIT_FOREVER))
        {
            
            if (Network_stable==3){
                SmsEventDispatch(event);
            }

            OS_Free(event->pParam1);
            OS_Free(event->pParam2);
            OS_Free(event);
        }
    }
}

void MainEventDispatch(API_Event_t* pEvent)
{
    bool res;
    switch(pEvent->id)
    {
    //related to uart
        case API_EVENT_ID_UART_RECEIVED:
            res = OS_SendEvent(uartTaskHandle,pEvent,OS_WAIT_FOREVER,OS_EVENT_PRI_NORMAL);
            break;
    //related to gsm network
    {
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
    }
    //related to sms
        case API_EVENT_ID_SMS_SENT:
            uart_sender("$SOK!");
            pEvent->id=API_EVENT_ID_RESET_BLINKING;
            OS_SendEvent(gpioTaskHandle,pEvent,OS_WAIT_FOREVER,OS_EVENT_PRI_URGENT);
            break;
        case API_EVENT_ID_SMS_RECEIVED:
            res = OS_SendEvent(smsTaskHandle,pEvent,OS_WAIT_FOREVER,OS_EVENT_PRI_URGENT);
            Trace(4,"yoooooooooo API_EVENT_ID_SMS_RECEIVED va ersal : %d ",res);
            break;
        case API_EVENT_ID_SMS_LIST_MESSAGE:
            res = OS_SendEvent(smsTaskHandle,pEvent,OS_WAIT_FOREVER,OS_EVENT_PRI_NORMAL);
            Trace(4,"yoooooooooo API_EVENT_ID_SMS_LIST_MESSAGE va ersal : %d ",res);
            break;
        case API_EVENT_ID_SMS_ERROR:
            if ((SMS_Error_t) pEvent->param1==SMS_ERROR_DECODE_ERROR){
                res = OS_SendEvent(smsTaskHandle,pEvent,OS_WAIT_FOREVER,OS_EVENT_PRI_NORMAL);
                Trace(4,"yoooooooooo API_EVENT_ID_SMS_ERROR va ersal : %d ",res);
            }
            break;
        // end related to sms
        default:
            break;
    }
    //system initialize complete and network register complete, now can send message
    if(Network_stable == 3)
    {
        sms_clearAll();
        uart_sender("Network is Stable");
    }
}

void MainTask(void *pData)
{
    API_Event_t* event=NULL;
    gpioTaskHandle = OS_CreateTask(GPIO_TestTask,
        NULL, NULL, TEST_TASK_STACK_SIZE, TEST_TASK_PRIORITY, 0, 0, "GPIO Test Task");
    
    uartTaskHandle = OS_CreateTask(UART_TestTask,
        NULL, NULL, TEST_TASK_STACK_SIZE, TEST_TASK_PRIORITY, 0, 0, "UART Test Task");

    smsTaskHandle= OS_CreateTask(SMS_TestTask,
        NULL, NULL, TEST_TASK_STACK_SIZE*3, TEST_TASK_PRIORITY, 0, 0, "sms Test Task");
        
  
    while(1)
    {
        if(OS_WaitEvent(mainTaskHandle, (void**)&event, OS_TIME_OUT_WAIT_FOREVER))
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
    mainTaskHandle = OS_CreateTask(MainTask ,
        NULL, NULL, MAIN_TASK_STACK_SIZE, MAIN_TASK_PRIORITY, 0, 0, MAIN_TASK_NAME);
    OS_SetUserMainHandle(&mainTaskHandle);
}
