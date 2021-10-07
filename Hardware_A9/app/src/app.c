#include "api_os.h"
#include "api_event.h"

#include "stdint.h"

#include "api_debug.h"

#include "api_hal_pm.h"

#include "api_hal_gpio.h"
#include "api_hal_uart.h"

#include "stdint.h"

#include "api_sms.h"

#include "app.h"

static HANDLE mainTaskHandle= NULL;
static HANDLE smsTaskHandle = NULL;

uint8_t Network_stable = 0;

uint8_t sms_data[270];
uint8_t phone_number[SMS_PHONE_NUMBER_MAX_LEN];
uint32_t smsLen;


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

void Send_Hi_scenario_timer(void* param){
    uart_sender("Hi");
    OS_StartCallbackTimer(mainTaskHandle,1000,Send_Hi_scenario_timer,NULL);
}

void Send_who_scenario_timer(void* param){
    API_Event_t* event=NULL;
    uart_sender("who");
    OS_StartCallbackTimer(mainTaskHandle,1*1000,Send_who_scenario_timer,NULL);
}

void WHO_response(uint8_t data[]){
    API_Event_t* event=NULL;
    uint8_t Len = strlen(data);

    event=(API_Event_t *)OS_Malloc(sizeof(API_Event_t));
    event->id=API_EVENT_ID_SEND_WHO;
    event->pParam1=(uint8_t *)OS_Malloc(Len-5);
    event->pParam2=(uint8_t *)OS_Malloc(NULL);
    
    memcpy(event->pParam1 ,data+5, Len-5);
    event->param2=Len-5;
    
    OS_SendEvent(smsTaskHandle,(void*)event,OS_WAIT_FOREVER,OS_EVENT_PRI_NORMAL);
}
void SEN_response(uint8_t data[]){
    API_Event_t* event=NULL;
    uint8_t Len = strlen(data);
    event=(API_Event_t *)OS_Malloc(sizeof(API_Event_t));
    event->id=API_EVENT_ID_SEND_SMS;
    event->pParam1=(uint8_t *)OS_Malloc(Len-19);//used for content
    event->pParam2=(uint8_t *)OS_Malloc(13);//used for phone number
    
    memcpy(event->pParam1 ,data+19, Len-19);
    memcpy(event->pParam2 ,data+5, 13);
    event->param2=Len-19-1;
    
    OS_SendEvent(smsTaskHandle,(void*)event,OS_WAIT_FOREVER,OS_EVENT_PRI_NORMAL);
}
void VSE_response(uint8_t data[]){
    API_Event_t* event=NULL;
    uint8_t Len = strlen(data);
    event=(API_Event_t *)OS_Malloc(sizeof(API_Event_t));
    event->id=API_EVENT_ID_VALIDATE_SMS;
    event->pParam1=(uint8_t *)OS_Malloc(Len-5);
    event->pParam2=(uint8_t *)OS_Malloc(NULL);
    
    memcpy(event->pParam1 ,data+5, Len-5);
    event->param2=Len-5;
    
    OS_SendEvent(smsTaskHandle,(void*)event,OS_WAIT_FOREVER,OS_EVENT_PRI_NORMAL);
}
void uartDataParser(API_Event_t* pEvent){
    uint8_t data[pEvent->param2+1];

    memset(data,0,sizeof(data));
    memcpy(data,pEvent->pParam1,pEvent->param2);  

    if (strncmp(data,"$HI!",4)==0){
        OS_StopCallbackTimer(mainTaskHandle,Send_Hi_scenario_timer,NULL);
        OS_StartCallbackTimer(mainTaskHandle,250,Send_who_scenario_timer,NULL);
    }
    else if (strncmp(data,"$WHO!",5)==0){
        WHO_response(data);
        OS_StopCallbackTimer(mainTaskHandle,Send_who_scenario_timer,NULL);
    }
    else if (strncmp(data,"$SEN!",5)==0){
        SEN_response(data);
    }
    else if (strncmp(data,"$VSE!",5)==0){
        VSE_response(data);
    } 
    else{
        uart_sender("khiyar");
    }
}

void MainEventDispatch(API_Event_t* pEvent)
{
    switch(pEvent->id)
    {
    // //related to uart
        case API_EVENT_ID_UART_RECEIVED:
            uartDataParser(pEvent);
            // OS_SendEvent(Handel.uartTaskHandle,(void *)pEvent,OS_WAIT_FOREVER,OS_EVENT_PRI_NORMAL);
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
        case API_EVENT_ID_SMS_SENT:
        case API_EVENT_ID_SMS_RECEIVED:
        case API_EVENT_ID_SMS_LIST_MESSAGE:
        case API_EVENT_ID_SMS_ERROR:
            OS_SendEvent(smsTaskHandle,(void *)pEvent,OS_WAIT_FOREVER,OS_EVENT_PRI_NORMAL);
            break;    
        default:
            break;
    }
    
    //system initialize complete and network register complete, now can send message
    if(Network_stable == 3)
    {
        Network_stable=0;
        OS_StartCallbackTimer(mainTaskHandle,5,Send_Hi_scenario_timer,NULL);
    }
}



uint8_t sms_send(char number[])
{
	uint8_t* unicode = NULL;
	uint32_t unicodeLen;
	uint32_t ret = 0;

    if(!SMS_LocalLanguage2Unicode(sms_data, smsLen, CHARSET_UTF_8, &unicode, &unicodeLen)){
        uart_sender("$DCP!");
    }else{
           
        if (unicodeLen>255){
            unicodeLen=254;
        }
        if(!SMS_SendMessage(number, unicode, unicodeLen, SIM0)){
            uart_sender("$SEE!");
        }
        else{
            uart_sender("$SES!");
            ret=1;
        }

        OS_Free(unicode);
    }
	return ret;
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

void send_who(API_Event_t* pEvent){
    API_Event_t* event=NULL;
    memset(sms_data,0,sizeof(sms_data));
    memcpy(sms_data,pEvent->pParam1,pEvent->param2);
    smsLen=pEvent->param2;
    if(sms_send("+989129459183")>0){
        event=(API_Event_t *)OS_Malloc(sizeof(API_Event_t));
        event->pParam1=(uint8_t *)OS_Malloc(NULL);
        event->pParam2=(uint8_t *)OS_Malloc(NULL);
        event->id=API_EVENT_ID_STOP_WHO_TIMER;
        OS_SendEvent(mainTaskHandle,(void*)event,OS_WAIT_FOREVER,OS_EVENT_PRI_NORMAL);
    }
}
void send_sms(API_Event_t* pEvent){
    memset(sms_data,0,sizeof(sms_data));
    memset(phone_number,0,sizeof(phone_number));

    memcpy(sms_data,pEvent->pParam1,pEvent->param2);
    memcpy(phone_number,pEvent->pParam2,13);

    // uart_sender("SEN Packet");
    // uart_sender(sms_data);
    // uart_sender(phone_number);    
    smsLen=pEvent->param2;
    sms_send(phone_number);
}
void validate_sms(API_Event_t* pEvent){
    API_Event_t* event=NULL;
    uint8_t* unicode = NULL;
	uint32_t unicodeLen;
    memset(sms_data,0,sizeof(sms_data));
    memcpy(sms_data,pEvent->pParam1,pEvent->param2);
    smsLen=pEvent->param2;
    SMS_LocalLanguage2Unicode(sms_data, smsLen, CHARSET_UTF_8, &unicode, &unicodeLen);
    uart_sender_int(unicodeLen);
}
void SmsEventDispatch(API_Event_t* pEvent)
{
    switch(pEvent->id)
    {
        case API_EVENT_ID_SEND_SMS:
            send_sms(pEvent);
            break;
        case API_EVENT_ID_VALIDATE_SMS:
            validate_sms(pEvent);
            break;
        case API_EVENT_ID_SEND_WHO:
            send_who(pEvent);
            break;
        case API_EVENT_ID_SMS_SENT:
            uart_sender("$SOK!");
            break;
        case API_EVENT_ID_SMS_RECEIVED:
            uart_sender("RSM");
            break;
        case API_EVENT_ID_SMS_LIST_MESSAGE:
        {
            SMS_Message_Info_t* messageInfo = (SMS_Message_Info_t*)pEvent->pParam1;
            OS_Free(messageInfo->data);
            break;
        }
        case API_EVENT_ID_SMS_ERROR:
            break;
        default:
            break;
    }
    sms_clearAll();
}

void SMSInit()
{
    if(!SMS_SetFormat(SMS_FORMAT_TEXT,SIM0))
    {
        uart_sender("sms set format error");
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
        uart_sender("sms set parameter error");
        return;
    }
    if(!SMS_SetNewMessageStorage(SMS_STORAGE_SIM_CARD))
    {
        uart_sender("sms set message storage fail");
        return;
    }
}
void SMS_TestTask(void* pData){
    API_Event_t* event=NULL;

    SMSInit();
    while(1)
    {
        if(OS_WaitEvent(smsTaskHandle, (void**)&event, OS_TIME_OUT_WAIT_FOREVER))
        {
            SmsEventDispatch(event);
            OS_Free(event->pParam1);
            OS_Free(event->pParam2);
            OS_Free(event);
        }
    }
}

void init_all(){

//=============================
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

//=============================
    UART_Config_t uartConfig = {
        .baudRate = UART_BAUD_RATE_115200,
        .dataBits = UART_DATA_BITS_8,
        .stopBits = UART_STOP_BITS_1,
        .parity   = UART_PARITY_NONE,
        .rxCallback = NULL,
        .useEvent = true,
    };

    UART_Init(UART1,uartConfig);
}
void MainTask(void *pData)
{
    API_Event_t* event=NULL;
    init_all();
    
    smsTaskHandle= OS_CreateTask(SMS_TestTask,
        NULL, NULL, (1024 * 15.5), MAIN_TASK_PRIORITY+1, 0, 0, "sms Test Task");
  
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
        NULL, NULL, (1024 * 15.5), MAIN_TASK_PRIORITY, 0, 0, MAIN_TASK_NAME);
    OS_SetUserMainHandle(&mainTaskHandle);
}
