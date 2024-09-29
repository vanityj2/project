#include "BEEP.h"
#include "LED.h"
#include "delay.h"
#include "usart.h"
#include "DHT11.h"
#include "timer.h"
#include "OLED.h"
#include "BH1750.h"
#include "key.h"
#include "sys.h"
#include "exti.h"
#include "stdio.h"
#include "esp8266.h"
#include "onenet.h"
u8 Flag=0;
u8 Free=10;
u8 humiH;
u8 humiL;
u8 tempH;
u8 tempL;
u8 LED_Status = 0;
extern char oledBuf[20];
u8 ESP8266_INIT_OK = 0;
float Light=0;
char PUB_BUF[256];//上传数据的buf
const char *devSubTopic[] = {"/iot/4346/sub"};
const char devPubTopic[] = {"/iot/4346/pub"};
int main(void)
{
	unsigned short timeCount = 0;	//发送间隔变量
	unsigned char *dataPtr = NULL;
    BEEP_Init();
    delay_init();
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
    LED_Init();
    KEY_Init();
    EXTIX_Init();
    DHT11_Init();
    BH1750_Init();
    Usart1_Init(115200);
    Usart2_Init(115200);
    
    OLED_Init();
    OLED_ColorTurn(0);
    OLED_DisplayTurn(0);
    OLED_Clear();
    TIM2_Int_Init(4999,7199);
    TIM3_Int_Init(2499,7199);
    UsartPrintf(USART_DEBUG," Hardware init ok\r\n");
	
	ESP8266_Init();
	while(OneNet_DevLink())
    delay_ms(500);
    BEEP=0;
    delay_ms(250);
    BEEP=1;
    OneNet_Subscribe(devSubTopic, 1);
 while(1)
 {
	if(timeCount % 40 == 0)//1000ms / 25 = 40 一秒执行一次
	{
		DHT11_Read_Data(&humiH,&humiL,&tempH,&tempL);
		UsartPrintf(USART1, "温度: %d.%d  湿度:%d.%d",tempH,tempL,humiH,humiL);
		if(!i2c_CheckDevice(BH1750_Addr))
		{
		  Light=LIght_Intensity();
			UsartPrintf(USART_DEBUG,"当前光照度为:%.1f lx\r\n",Light);
		}
		
		if(Free==10)
		{
		if((tempH<30) && (humiH<90) && (Light<1000)) Flag=0;
		else Flag=1;
		}
		if(Free<10) Free++;
		
		if(++timeCount >= 200)
		{
		 	LED_Status = GPIO_ReadInputDataBit(GPIOA,GPIO_Pin_4);
			UsartPrintf(USART_DEBUG,"OneNet_Publish\r\n");
			sprintf(PUB_BUF,"{\"Hum\":%d.%d,\"Temp\":%d.%d,\"Light\":%.1f,\"LED\":%d,\"Beep\":%d}",humiH,humiL,tempH,tempL,Light,LED_Status?0:1,Flag);
			OneNet_Publish(devPubTopic,PUB_BUF);
			timeCount = 0;
	        ESP8266_Clear();
		}
		dataPtr = ESP8266_GetIPD(3);
		if(dataPtr != NULL)
			OneNet_RevPro(dataPtr);
			delay_ms(10);
    
}
}
}
