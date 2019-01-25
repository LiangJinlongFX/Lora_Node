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

