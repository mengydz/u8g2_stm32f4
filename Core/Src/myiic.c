/**************************************************************************
  * @brief     : 模拟IIC驱动程序
  * @author    : 何运富(yunfu.he@leadchina.cn)
  * @copyright : LeadChina
  * @version   : V1.0
  * @note      : 注意事项
  * @history   : 20200211
***************************************************************************/
#include "myiic.h"
#include "tim.h"
/**
*FM24V10 引脚定义
*SCL:PH8
*SDA:PE12
**/


/*******************************************************************************
* 函数名 : void delay_iic(uint8_t Micros)
* 描  述 : us 延时函数
* 输  入 : Micros:延时多少个us
* 输  出 : 无
* 返回值 : 无
*******************************************************************************/
void delay_iic(uint8_t us)
{
    for(uint16_t _delay=0;_delay<us;_delay++)
    for(uint8_t _delay_n=0;_delay_n<2;_delay_n++);
}

/*******************************************************************************
* 函数名 : void MyIIC_Init(void)
* 描  述 : 模拟IIC引脚初始化
* 输  入 : 无
* 输  出 : 无
* 返回值 : 无
*******************************************************************************/
void MyIIC_Init(void)
{
	GPIO_InitTypeDef GPIO_Initure;

	__HAL_RCC_GPIOB_CLK_ENABLE();   //使能GPIOE时钟
	__HAL_RCC_GPIOE_CLK_ENABLE();   //使能GPIOE时钟

	GPIO_Initure.Pin=GPIO_PIN_11;	//sck
	GPIO_Initure.Mode=GPIO_MODE_OUTPUT_PP;  //推挽输出
	GPIO_Initure.Pull=GPIO_PULLUP;          //上拉
	GPIO_Initure.Speed=GPIO_SPEED_FAST;     //快速
	HAL_GPIO_Init(GPIOB,&GPIO_Initure);

	GPIO_Initure.Pin=GPIO_PIN_15;	//sda
	GPIO_Initure.Mode=GPIO_MODE_OUTPUT_PP;  //推挽输出
	GPIO_Initure.Pull=GPIO_PULLUP;          //上拉
	GPIO_Initure.Speed=GPIO_SPEED_FAST;     //快速
	HAL_GPIO_Init(GPIOE,&GPIO_Initure);

	MyIIC_SDA(1);
	MyIIC_SCL(1);
}

/*******************************************************************************
* 函数名 : void MyIIC_Start(void)
* 描  述 : 模拟IIC开始信号
* 输  入 : 无
* 输  出 : 无
* 返回值 : 无
*******************************************************************************/
void MyIIC_Start(void)
{
	MyIIC_SDA_OUT();     //sda线输出
	MyIIC_SDA(1);
	MyIIC_SCL(1);
	delay_iic(1);
 	MyIIC_SDA(0);//START:when CLK is high,DATA change form high to low
	delay_iic(1);
	MyIIC_SCL(0);//钳住I2C总线，准备发送或接收数据
	delay_iic(1);
}

/*******************************************************************************
* 函数名 : void MyIIC_Stop(void)
* 描  述 : 模拟IIC停止信号
* 输  入 : 无
* 输  出 : 无
* 返回值 : 无
*******************************************************************************/
void MyIIC_Stop(void)
{
	MyIIC_SDA_OUT();//sda线输出
	MyIIC_SCL(0);
	MyIIC_SDA(0);//STOP:when CLK is high DATA change form low to high
 	delay_iic(1);
	MyIIC_SCL(1);
	delay_iic(1);
	MyIIC_SDA(1);//发送I2C总线结束信号
	delay_iic(1);
}

/*******************************************************************************
* 函数名 : uint8_t MyIIC_Wait_Ack(void)
* 描  述 : 等待应答信号到来
* 输  入 : 无
* 输  出 : 无
* 返回值 : 1，接收应答失败
*          0，接收应答成功
*******************************************************************************/
uint8_t MyIIC_Wait_Ack(void)
{
	uint8_t ucErrTime=0;
	MyIIC_SDA_IN();      //SDA设置为输入
	MyIIC_SDA(1);delay_iic(1);
	MyIIC_SCL(1);delay_iic(1);
	while(MyIIC_READ_SDA)
	{
		ucErrTime++;
		if(ucErrTime>250)
		{
			MyIIC_Stop();
			return 1;
		}
	}
	MyIIC_SCL(0);//时钟输出0
	return 0;
}

/*******************************************************************************
* 函数名 : void MyIIC_Ack(void)
* 描  述 : 产生 ACK 应答
* 输  入 : 无
* 输  出 : 无
* 返回值 : 无
*******************************************************************************/
void MyIIC_Ack(void)
{
	MyIIC_SCL(0);
	MyIIC_SDA_OUT();
	MyIIC_SDA(0);
	delay_iic(1);
	MyIIC_SCL(1);
	delay_iic(1);
	MyIIC_SCL(0);
}

/*******************************************************************************
* 函数名 : void MyIIC_NAck(void)
* 描  述 : 不产生 ACK 应答
* 输  入 : 无
* 输  出 : 无
* 返回值 : 1，接收应答失败
*          0，接收应答成功
*******************************************************************************/
void MyIIC_NAck(void)
{
	MyIIC_SCL(0);
	MyIIC_SDA_OUT();
	MyIIC_SDA(1);
	delay_iic(1);
	MyIIC_SCL(1);
	delay_iic(1);
	MyIIC_SCL(0);
}

/*******************************************************************************
* 函数名 : void MyIIC_Send_Byte(uint8_t txd)
* 描  述 : MyIIC发送一个字节
* 输  入 : 无
* 输  出 : 无
* 返回值 : 1，有应答
*          0，无应答
*******************************************************************************/
void MyIIC_Write_Data(uint8_t txd)
{
	uint8_t t;
	MyIIC_SDA_OUT();
	MyIIC_SCL(0);//拉低时钟开始数据传输
	for(t=0;t<8;t++)
	{
		MyIIC_SDA((txd&0x80)>>7);
		txd<<=1;
		delay_iic(1);   //对TEA5767这三个延时都是必须的
		MyIIC_SCL(1);
		delay_iic(1);
		MyIIC_SCL(0);
		delay_iic(1);
	}
}
/*******************************************************************************
* 函数名 : uint8_t MyIIC_Read_Byte(unsigned char ack)
* 描  述 : 读取一个字节
* 输  入 : ack:1，发送ACK;0，发送nACK
* 输  出 : 无
* 返回值 : 1，接收应答失败
*          0，接收应答成功
*******************************************************************************/
uint8_t MyIIC_Read_Data(unsigned char ack)
{
	unsigned char i,receive=0;
	MyIIC_SDA_IN();//SDA设置为输入
	for(i=0;i<8;i++ )
	{
		MyIIC_SCL(0);
		delay_iic(1);
		MyIIC_SCL(1);
		receive<<=1;
		if(MyIIC_READ_SDA) receive++;
			delay_iic(1);
	}
	if (!ack)
		MyIIC_NAck();//发送nACK
	else
		MyIIC_Ack(); //发送ACK
	return receive;
}

void MyIIC_Send_Bytes(uint8_t *ptr, uint8_t len)
{
	MyIIC_Start();
	MyIIC_Write_Data(0x78);	//iic addr
	MyIIC_Wait_Ack();
	for(uint8_t cnt=0;cnt<len;cnt++)
	{
		MyIIC_Write_Data(*(ptr+cnt));	//iic addr
		MyIIC_Wait_Ack();
	}
	MyIIC_Stop();
}

