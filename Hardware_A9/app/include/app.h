#define MAIN_TASK_STACK_SIZE    (1024 * 2)
#define MAIN_TASK_PRIORITY      0 
#define MAIN_TASK_NAME         "MAIN Test Task"

#define TEST_TASK_STACK_SIZE    (1024 * 2)
#define TEST_TASK_PRIORITY      1


typedef struct {
    HANDLE mainTaskHandle;
    HANDLE gpioTaskHandle;
    HANDLE uartTaskHandle;
    HANDLE smsTaskHandle;

}Task_Handels;

void UART_TestTask(void* pData);

// void uart_sender(char data []);
// void uart_sender_int(int data);