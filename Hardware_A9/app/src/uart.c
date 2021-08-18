#include "app.h"

#include "api_os.h"
#include "api_event.h"

#include "stdint.h"
#include "stdbool.h"
#include "api_debug.h"


#include "api_hal_uart.h"


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

