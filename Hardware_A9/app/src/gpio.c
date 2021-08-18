// void System_stady(void* param){
//     GPIO_config_t gpioLedBlue=*(GPIO_config_t *)param;
//     GPIO_LEVEL ledLevel;
//     GPIO_GetLevel(gpioLedBlue,&ledLevel);
//     ledLevel = (ledLevel==GPIO_LEVEL_HIGH)?GPIO_LEVEL_LOW:GPIO_LEVEL_HIGH;            
//     GPIO_SetLevel(gpioLedBlue,ledLevel);        //Set level
//     OS_StartCallbackTimer(gpioTaskHandle,2*1000,System_stady,param);
// }
// void start_blink_gpio(void* param){
//     GPIO_config_t gpioLedBlue=*(GPIO_config_t *)param;
//     GPIO_LEVEL ledLevel;
//     GPIO_GetLevel(gpioLedBlue,&ledLevel);
//     ledLevel = (ledLevel==GPIO_LEVEL_HIGH)?GPIO_LEVEL_LOW:GPIO_LEVEL_HIGH;            
//     GPIO_SetLevel(gpioLedBlue,ledLevel);        //Set level
//     OS_Sleep(1);
//     OS_StartCallbackTimer(gpioTaskHandle,250,start_blink_gpio,param);
// }
// static void GpioEventDispatch(API_Event_t* pEvent,GPIO_config_t gpioLedBlue)
// {
//     switch(pEvent->id)
//     {
//         case API_EVENT_ID_SET_BLINKING:
//             OS_StartCallbackTimer(gpioTaskHandle,250,start_blink_gpio,(void *)&gpioLedBlue);
//             break;
//         case API_EVENT_ID_RESET_BLINKING:
//             OS_StopCallbackTimer(gpioTaskHandle,start_blink_gpio,(void *)&gpioLedBlue);  
//             GPIO_SetLevel(gpioLedBlue,GPIO_LEVEL_LOW);
//             break;
//         default:
//             break;
//     }
// }
// void GPIO_TestTask()
// {
//     API_Event_t* event=NULL;

//     GPIO_config_t gpioLedBlue1 = {
//         .mode         = GPIO_MODE_OUTPUT,
//         .pin          = GPIO_PIN27,
//         .defaultLevel = GPIO_LEVEL_HIGH
//     };
//     GPIO_config_t gpioLedBlue2 = {
//         .mode         = GPIO_MODE_OUTPUT,
//         .pin          = GPIO_PIN28,
//         .defaultLevel = GPIO_LEVEL_LOW
//     };

//     GPIO_Init(gpioLedBlue1);
//     GPIO_Init(gpioLedBlue2);

//     OS_StartCallbackTimer(gpioTaskHandle,2*1000,System_stady,(void *)&gpioLedBlue1);

//     while(1)
//     {
//         if(OS_WaitEvent(gpioTaskHandle,(void *) &event, OS_TIME_OUT_WAIT_FOREVER))
//         {
//             GpioEventDispatch(event,gpioLedBlue2);
//             OS_Free(event->pParam1);
//             OS_Free(event->pParam2);
//             OS_Free(event);
//         }
//     }
// }

