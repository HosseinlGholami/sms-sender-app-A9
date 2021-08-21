#include "api_os.h"
#include "api_event.h"
#include "app.h"
#include "api_sms.h"


#include "stdint.h"
#include "stdbool.h"

uint8_t sms_data[270];
uint8_t phone_number[SMS_PHONE_NUMBER_MAX_LEN];
uint32_t smsLen;

uint8_t sms_send(char number[])
{
	uint8_t* unicode = NULL;
	uint32_t unicodeLen;
	uint32_t ret = 0;
    // uart_sender("sms lenght:"); uart_sender_int(smsLen);
    if(!SMS_LocalLanguage2Unicode(sms_data, smsLen, CHARSET_UTF_8, &unicode, &unicodeLen))
        uart_sender("$decode problem!");
    else
    {   
        if (unicodeLen>255){
            unicodeLen=254;
        }
        // uart_sender("unicodelen"); uart_sender_int(unicodeLen);
        if(!SMS_SendMessage(number, unicode, unicodeLen, SIM0)){
            uart_sender("$SEE!");
        }
        else
        {
            uart_sender("$SES!");
            ret=1;
        }

        OS_Free(unicode);
    }
	return ret;
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

void send_who(API_Event_t* pEvent,Task_Handels* Handle){
    API_Event_t* event=NULL;
    memset(sms_data,0,sizeof(sms_data));
    memcpy(sms_data,pEvent->pParam1,pEvent->param2);
    smsLen=pEvent->param2;
    if(sms_send("+989129459183")>0){
        event=(API_Event_t *)OS_Malloc(sizeof(API_Event_t));
        event->pParam1=(uint8_t *)OS_Malloc(NULL);
        event->pParam2=(uint8_t *)OS_Malloc(NULL);
        event->id=API_EVENT_ID_STOP_WHO_TIMER;
        OS_SendEvent(Handle->mainTaskHandle,(void*)event,OS_WAIT_FOREVER,OS_EVENT_PRI_NORMAL);
    }
}

void send_sms(API_Event_t* pEvent,Task_Handels* Handle){
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

void validate_sms(API_Event_t* pEvent,Task_Handels* Handle){
    API_Event_t* event=NULL;
    uint8_t* unicode = NULL;
	uint32_t unicodeLen;
    memset(sms_data,0,sizeof(sms_data));
    memcpy(sms_data,pEvent->pParam1,pEvent->param2);
    smsLen=pEvent->param2;
    SMS_LocalLanguage2Unicode(sms_data, smsLen, CHARSET_UTF_8, &unicode, &unicodeLen);
    uart_sender_int(unicodeLen);
}

void SmsEventDispatch(API_Event_t* pEvent,Task_Handels* Handle)
{
    switch(pEvent->id)
    {
        case API_EVENT_ID_SEND_SMS:
            send_sms(pEvent,Handle);
            break;
        case API_EVENT_ID_VALIDATE_SMS:
            validate_sms(pEvent,Handle);
            break;
        case API_EVENT_ID_SEND_WHO:
            send_who(pEvent,Handle);
            break;
        case API_EVENT_ID_SMS_SENT:
            uart_sender("$SOK!");
            break;
        case API_EVENT_ID_SMS_RECEIVED:
            uart_sender("RSM");
            // SMS_Encode_Type_t encodeType = pEvent->param1;
            // uint32_t contentLength = pEvent->param2;
            // uint8_t* header = pEvent->pParam1;
            // uint8_t* content = pEvent->pParam2;

            // // Trace(2,"message header:%s",header);
            // // Trace(2,"message content length:%d",contentLength);
            // if(encodeType == SMS_ENCODE_TYPE_ASCII)
            // {
            //     uart_sender("ascii sms recived");
            //     // Trace(2,"message content:%s",content);
            //     uart_sender(content);
            //     // sms_send("+989129459183",content);
            // }
            // else
            // {
            //     uart_sender("unicode sms recived");
            //     uint8_t* gbk = NULL;
            //     uint32_t gbkLen = 0;
            //     if(!SMS_Unicode2LocalLanguage(content,contentLength,CHARSET_UTF_8,&gbk,&gbkLen))
            //         uart_sender("convert unicode to GBK fail!");
            //     else
            //     {
            //         uart_sender(gbk);
            //         // sms_send("+989129459183",gbk);
            //         // OS_Sleep(5000);
            //     }
            //     OS_Free(gbk);
            // }
            break;
        case API_EVENT_ID_SMS_LIST_MESSAGE:
        {
            uart_sender("API_EVENT_ID_SMS_LIST_MESSAGE");
            SMS_Message_Info_t* messageInfo = (SMS_Message_Info_t*)pEvent->pParam1;
            // Trace(1,"message header index:%d,status:%d,number type:%d,number:%s,time:\"%u/%02u/%02u,%02u:%02u:%02u+%02d\"", messageInfo->index, messageInfo->status,
            //                                                                             messageInfo->phoneNumberType, messageInfo->phoneNumber,
            //                                                                             messageInfo->time.year, messageInfo->time.month, messageInfo->time.day,
            //                                                                             messageInfo->time.hour, messageInfo->time.minute, messageInfo->time.second,
            //                                                                             messageInfo->time.timeZone);
            // Trace(1,"message content len:%d,data:%s",messageInfo->dataLen,messageInfo->data);
            uart_sender(messageInfo->data);
            // UART_Write(UART1, messageInfo->data, messageInfo->dataLen);//use serial tool that support GBK decode if have Chinese, eg: https://github.com/Neutree/COMTool
            //need to free data here
            OS_Free(messageInfo->data);
            break;
        }
        case API_EVENT_ID_SMS_ERROR:
            uart_sender("API_EVENT_ID_SMS_ERROR");
            // Trace(10,"SMS error occured! cause:%d",pEvent->param1);
        default:
            break;
    }
    sms_clearAll();
}

void SMS_TestTask(void* pData){
    API_Event_t* event=NULL;
    Task_Handels* Handle = (Task_Handels *) pData;

    SMSInit();
    while(1)
    {
        if(OS_WaitEvent(Handle->smsTaskHandle, (void**)&event, OS_TIME_OUT_WAIT_FOREVER))
        {
            SmsEventDispatch(event,Handle);
            OS_Free(event->pParam1);
            OS_Free(event->pParam2);
            OS_Free(event);
        }
    }
}
