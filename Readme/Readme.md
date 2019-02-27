# Readme

### 疑难解答

+ PB3/4无法拉低

  ~~~c
  //PB4默认用作调试口，如果用作普通的IO，需要加上以下两句 
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO,ENABLE);
  GPIO_PinRemapConfig(GPIO_Remap_SWJ_JTAGDisable, ENABLE);
  ~~~

+ 注意使能外设时钟要仔细检查，AP1/APB2混乱并不会报错

+ 读取SX1278状态不要多次调用Radio->Process()，应使用返回值来判断，否则无法有效接收到标志位

+ GPIO模拟IIC注意时序

| 应用               | 外设      |      |
| ------------------ | --------- | ---- |
| 电池电压检测       | ADC2通道0 |      |
| TVOC传感器电压检测 | ADC1通道1 |      |
| LEDR               | PB2       |      |
| LEDG               | PB3       |      |
| LEDB               | PB4       |      |
|                    |           |      |
|                    |           |      |

### 程序流程图

1. lora节点上电，初始化外设，若初始化有错，红灯常亮
2. 启动注册过程，发送一次注册请求，等待集中器注册响应，若5秒后没有收到注册响应，状态灯红灯闪烁；若成功注册，绿灯常亮
3. lora进入接收状态，等待集中器的信息以作进一步响应