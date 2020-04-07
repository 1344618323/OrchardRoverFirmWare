#include "ultrasonic.h"

UltrasonicSensor rangeSensors[2];

void KS109_Uart_Callback_Handle(uint8_t *rx_buf, uint32_t len, uint8_t id)
{
    if (len >= 2)
        rangeSensors[id].range = (rx_buf[len-2] << 8) | (rx_buf[len-1]);
    else
        rangeSensors[id].range = 0;
    rangeSensors[id].rec_flag = 0;
}

// KS109串口通讯波特率 默认9600
// 串口指令格式 0xe8 延时20~100us 0x02 延时20~100us  指令
// 第一指令时序 0x9c 第二指令时序 0x95 第三指令时序 0x98
// 降噪指令： 一级降噪指令 0x71(默认出场设置)   完成后延迟2s以上
// 波束角：   15°~20°      0x7a(默认出场设置)   完成后延迟2s以上
// 距离探测： 0~11m(不带温度补偿，默认) 最大耗时68ms对应11m  本程序延迟50ms 0xb0
// 距离返回格式： 16位结果，先发高8位，后发低8位

void KS109_Init(void)
{
    HAL_Delay(2000);

    //降噪配置
    Transmit_Cmd_All_KS109(0xe8); //第一指令时序
    LeonardDelayUs(50);
    Transmit_Cmd_All_KS109(0x02);
    LeonardDelayUs(50);
    Transmit_Cmd_All_KS109(0x9c);
    LeonardDelayUs(50);
    Transmit_Cmd_All_KS109(0xe8); //第二指令时序
    LeonardDelayUs(50);
    Transmit_Cmd_All_KS109(0x02);
    LeonardDelayUs(50);
    Transmit_Cmd_All_KS109(0x95);
    LeonardDelayUs(50);
    Transmit_Cmd_All_KS109(0xe8); //第三指令时序
    LeonardDelayUs(50);
    Transmit_Cmd_All_KS109(0x02);
    LeonardDelayUs(50);
    Transmit_Cmd_All_KS109(0x98);
    LeonardDelayUs(50);
    Transmit_Cmd_All_KS109(0xe8); //一级降噪指令
    LeonardDelayUs(50);
    Transmit_Cmd_All_KS109(0x02);
    LeonardDelayUs(50);
    Transmit_Cmd_All_KS109(0x71);
    HAL_Delay(2500); //延迟2500ms

    //波束角配置
    Transmit_Cmd_All_KS109(0xe8); //第一指令时序
    LeonardDelayUs(50);
    Transmit_Cmd_All_KS109(0x02);
    LeonardDelayUs(50);
    Transmit_Cmd_All_KS109(0x9c);
    LeonardDelayUs(50);
    Transmit_Cmd_All_KS109(0xe8); //第二指令时序
    LeonardDelayUs(50);
    Transmit_Cmd_All_KS109(0x02);
    LeonardDelayUs(50);
    Transmit_Cmd_All_KS109(0x95);
    LeonardDelayUs(50);
    Transmit_Cmd_All_KS109(0xe8); //第三指令时序
    LeonardDelayUs(50);
    Transmit_Cmd_All_KS109(0x02);
    LeonardDelayUs(50);
    Transmit_Cmd_All_KS109(0x98);
    LeonardDelayUs(50);
    Transmit_Cmd_All_KS109(0xe8); //波束角设置15-20度
    LeonardDelayUs(50);
    Transmit_Cmd_All_KS109(0x02);
    LeonardDelayUs(50);
    Transmit_Cmd_All_KS109(0x7a);
    HAL_Delay(2500); //延迟2500ms
}

void Transmit_Cmd_All_KS109(uint8_t cmd)
{
    Transmit_KS109_Cmd(cmd, L_SENSOR_ID);
    Transmit_KS109_Cmd(cmd, R_SENSOR_ID);
}

void Read_KS109_Cmd(uint8_t id)
{
    //探测指令0xb0:0-11m距离，不带温度补偿
		// 0xb4 有温补
    Transmit_KS109_Cmd(0xe8, id);
    LeonardDelayUs(50);
    Transmit_KS109_Cmd(0x02, id);
    LeonardDelayUs(50);
    Transmit_KS109_Cmd(0xb4, id);
    rangeSensors[id].rec_flag = 1;
}

int16_t Read_KS109_Range(uint8_t id)
{
    if (rangeSensors[id].rec_flag == 1)
        return -1;
    else
        return rangeSensors[id].range;
}
