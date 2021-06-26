#include<reg52.h> //包含头文件，一般情况不需要改动，头文件包含特殊功能寄存器的定义
#include <intrins.h>

#define uchar unsigned char
#define uint  unsigned int

sbit    CLK=P1^0;        //max6921
sbit    DIN=P1^1;        //max6921
sbit    LOAD=P1^2;       //max6921
sbit    speak=P3^7;   //定义蜂鸣器控制口
sbit    clk_ds1302=P3^6; //定义控制DS1302的时钟线
sbit    io_ds1302=P3^5;  //定义控制DS1302的串行数据
sbit    rest_ds1302=P3^4;//定义控制DS1302的复位
sbit    s1_bit=P1^3;     //定义S1控制口
sbit    s2_bit=P1^4;     //定义S2控制口
sbit    s3_bit=P1^5;     //定义S3控制口


/*void Delay(unsigned int t)
{
 while(--t);
}*/

void  w_1byte_ds1302(uchar t);//向DS1302写一个字节的数据
void  set_ds1302();//设置时间
void  get_ds1302();//读取当前时间
void  judge_s1();//S1按键处理函数
void  judge_s2();//S2按键处理函数
void  judge_s3();//S3按键处理函数
void  Init_Timer0();
void  Init_Timer1();
void  delay_3us();//延时3微妙函数的声明
void  delay_50us(uint t);//延时50*T微妙函数的声明
void  dis(uchar s0,uchar s1,uchar s3,uchar s4,uchar s6,uchar s7);//显示子程序
void  dis_san(uchar s0,uchar s1,uchar s3,uchar s4,uchar s6,uchar s7,uchar san);//闪烁显示子程序
void  dis_led();//LED处理函数
void  judge_dis();//显示处理函数
void  judge_clock();//显示处理函数

uchar t0_crycle;
 
uchar r_1byte_ds1302();//从DS1302读一个字节的数据
uchar clock_en;//闹钟关闭和开启的标志，1开启，0关闭
uint  speak_count;//蜂鸣器

uint TMP;           //全局变量
uint idx;
uchar flag1,second_flag,zancun1,zancun2,zancun3;  //标志位1  秒标志位  暂存变量
uchar zancun4,zancun5,zancun6;                    //暂存变量
uchar countdown_second;                            //准备（倒记）秒
uchar countdown_hour,countdown_minute;
uchar clock_flag,countdown_flag;

uint  vdata[8]={0x1000,0x0800,0x0400,0x2000,0x0200,0x4000,0x0100,0x8000};
uchar hour_count,minute_count,second_count,msecond_count;
uchar clock_hour,clock_minute;
uchar msecond_minute,msecond_second,msecond_msecond,msecond_flag;//秒表相关参数
uchar tab23[3];//={0x40,0x59,0x23,0x28,0x11,0x06,0x09};//上电时默认的时间
uchar tft(uchar t)
{
 switch(t)
  {
 case 0: return(0xbb); break;//  [0]
 case 1: return(0x12); break;//  [1]
 case 2: return(0xae); break;//  [2]
 case 3: return(0xb6); break;//  [3]
 case 4: return(0x17); break;//  [4]
 case 5: return(0xb5); break;//  [5]
 case 6: return(0xbd); break;//  [6]
 case 7: return(0x92); break;//  [7]
 case 8: return(0xbf); break;//  [8]
 case 9: return(0xb7); break;//  [9]
 case 10: return(0xad); break;//  [E]
 case 11: return(0x2d); break;//  [t]
 case 12: return(0x00); break;//  [ ]
 case 13: return(0xa9); break;//  [C]
 case 14: return(0x04); break;//  [-]
 case 15: return(0x00); break;//  [ ]
 default:
                break;
   }
 }


/*------------------------------------------------
                    主程序
------------------------------------------------*/

void main()
{
 TMP=0;
 DIN=0;
 CLK=0;
 flag1=0;    
 zancun3=0;
 speak=1;//关闭蜂鸣器
 speak_count=0;
 msecond_minute=0;//置秒表相关参数为0
 msecond_second=0;
 msecond_msecond=0;
 clock_hour=0;
 clock_minute=0;
 clock_flag=0;
 countdown_flag=0;//倒计时标志位为0
 clock_en=0;//开机时默认关闭闹钟
 Init_Timer0();
 Init_Timer1();
 
 while(1)
   {  
      get_ds1302();
      judge_dis();//显示处理
      judge_s1();
      judge_s2();
      judge_s3();
      judge_clock();//闹钟处理程序
   }
}

//****************************************
//时钟显示程序
void  dis(uchar s0,uchar s1,uchar s3,uchar s4,uchar s6,uchar s7)
{
     vdata[0] =((unsigned int)vdata[0]&0xff00)  | (tft(s0)&0xff);
     vdata[1] =((unsigned int)vdata[1]&0xff00)  | (tft(s1)&0xff);
     vdata[3] =((unsigned int)vdata[3]&0xff00)  | (tft(s3)&0xff); 
     vdata[4] =((unsigned int)vdata[4]&0xff00)  | (tft(s4)&0xff);
     vdata[6] =((unsigned int)vdata[6]&0xff00)  | (tft(s6)&0xff);
     vdata[7] =((unsigned int)vdata[7]&0xff00)  | (tft(s7)&0xff); 
}
//**************************************************
//LED处理函数
void  dis_led()
{ 
     if(msecond_count<5)
     {
        vdata[2] =((unsigned int)vdata[2]&0xff00)  | (0x00&0xff);
        vdata[5] =((unsigned int)vdata[5]&0xff00)  | (0x00&0xff);
     }
     else
     {
        vdata[2] =((unsigned int)vdata[2]&0xff00)  | (0x04&0xff);
        vdata[5] =((unsigned int)vdata[5]&0xff00)  | (0x04&0xff);  
     }
}

//****************************************
////闪烁显示子程序
void  dis_san(uchar s0,uchar s1,uchar s3,uchar s4,uchar s6,uchar s7,uchar san)
{    
     if(san==1)
     {
        if(msecond_count<5)
        {
           vdata[0] =((unsigned int)vdata[0]&0xff00)  | (0x00&0xff);
        }
 	    else
        {
         vdata[0] =((unsigned int)vdata[0]&0xff00)  | (tft(s0)&0xff);
          }
     }
     else
     {
         vdata[0] =((unsigned int)vdata[0]&0xff00)  | (tft(s0)&0xff);
     }
     if(san==2)
     {
        if(msecond_count<5)
        {
          vdata[1] =((unsigned int)vdata[1]&0xff00)  | (0x00&0xff);
        } 
		else
        {
         vdata[1] =((unsigned int)vdata[1]&0xff00)  | (tft(s1)&0xff);
         }
     }
     else
     {
         vdata[1] =((unsigned int)vdata[1]&0xff00)  | (tft(s1)&0xff);
     }
     if(san==3)
     {
        if(msecond_count<5)
        {
          vdata[3] =((unsigned int)vdata[3]&0xff00)  | (0x00&0xff);
        } 
		else
        {
         vdata[3] =((unsigned int)vdata[3]&0xff00)  | (tft(s3)&0xff);
        }
     }
     else
     {
         vdata[3] =((unsigned int)vdata[3]&0xff00)  | (tft(s3)&0xff);
     }
     if(san==4)
     {
        if(msecond_count<5)
        {
           vdata[4] =((unsigned int)vdata[4]&0xff00)  | (0x00&0xff);
        } 
		else
        {
         vdata[4] =((unsigned int)vdata[4]&0xff00)  | (tft(s4)&0xff);
        }
     }
     else
     {
         vdata[4] =((unsigned int)vdata[4]&0xff00)  | (tft(s4)&0xff);
     }
     if(san==5)
     {
        if(msecond_count<5)
        {
           vdata[6] =((unsigned int)vdata[6]&0xff00)  | (0x00&0xff);
        } 
	 	else
        {
         vdata[6] =((unsigned int)vdata[6]&0xff00)  | (tft(s6)&0xff);
        }
     }
     else
     {
         vdata[6] =((unsigned int)vdata[6]&0xff00)  | (tft(s6)&0xff);
     }
     if(san==6)
     {
        if(msecond_count<5)
        {
          vdata[7] =((unsigned int)vdata[7]&0xff00)  | (0x00&0xff);
        } 
		else
        {
         vdata[7] =((unsigned int)vdata[7]&0xff00)  | (tft(s7)&0xff);
         }
     }
     else
     {
         vdata[7] =((unsigned int)vdata[7]&0xff00)  | (tft(s7)&0xff);
     }
}
//**************************************************
//显示处理函数
void  judge_dis()
{     
    if(flag1==0)
    {
         zancun4=hour_count&0xf0;
         zancun4>>=4;
         zancun5=minute_count&0xf0;
         zancun5>>=4;
         zancun6=second_count&0xf0;
         zancun6>>=4;
         dis(zancun4,hour_count&0x0f,zancun5,minute_count&0x0f,zancun6,second_count&0x0f);
         dis_led();
         if(zancun1==5)zancun1=0;
    }
    if(flag1!=0)
    {
       switch(flag1)
       {
           case 1:
                dis(5,10,11,1,12,12);//显示SET1
                vdata[2] =((unsigned int)vdata[2]&0xff00)  | (0x00&0xff);
                vdata[5] =((unsigned int)vdata[5]&0xff00)  | (0x00&0xff);
                break;
           case 2:
                dis(5,10,11,2,12,12);//显示SET2
                break;
           case 3:
                dis(5,10,11,3,12,12);//显示SET3
                break;
           case 4:
                dis(5,10,11,4,12,12);//显示SET4
                break;
           case 5:
                dis(5,10,11,5,12,12);//显示SET5
                break;
           case 6:
                dis_san(zancun1/10,zancun1%10,zancun2/10,zancun2%10,12,12,1);
                break;
           case 7:
                dis_san(zancun1/10,zancun1%10,zancun2/10,zancun2%10,12,12,2);
                break;
           case 8:
                dis_san(zancun1/10,zancun1%10,zancun2/10,zancun2%10,12,12,3);
                break;
           case 9://进入修改时间，时间分位个位闪烁
                dis_san(zancun1/10,zancun1%10,zancun2/10,zancun2%10,12,12,4);
                break;
           case 10://进入修改闹钟，闹钟小时十位闪烁
                dis_san(zancun1/10,zancun1%10,zancun2/10,zancun2%10,12,zancun3,1);
                break;
           case 11://进入修改闹钟，闹钟小时个位闪烁
                dis_san(zancun1/10,zancun1%10,zancun2/10,zancun2%10,12,zancun3,2);
                break;
           case 12://进入修改闹钟，闹钟小时十位闪烁
                dis_san(zancun1/10,zancun1%10,zancun2/10,zancun2%10,12,zancun3,3);
                break;
           case 13://进入修改闹钟，闹钟小时个位闪烁
                dis_san(zancun1/10,zancun1%10,zancun2/10,zancun2%10,12,zancun3,4);
                break;
           case 14://进入修改闹钟的开关
                dis_san(zancun1/10,zancun1%10,zancun2/10,zancun2%10,12,zancun3,6);
                break;
           case 15:
                dis_san(zancun1/10,zancun1%10,zancun2/10,zancun2%10,zancun3/10,zancun3%10,1);
                break;
           case 16:
                dis_san(zancun1/10,zancun1%10,zancun2/10,zancun2%10,zancun3/10,zancun3%10,2);
                break;
           case 17:
                dis_san(zancun1/10,zancun1%10,zancun2/10,zancun2%10,zancun3/10,zancun3%10,3);
                break;
           case 18:
                dis_san(zancun1/10,zancun1%10,zancun2/10,zancun2%10,zancun3/10,zancun3%10,4);
                break;
           case 19:
                dis_san(zancun1/10,zancun1%10,zancun2/10,zancun2%10,zancun3/10,zancun3%10,5);
                break;
           case 20:
                dis_san(zancun1/10,zancun1%10,zancun2/10,zancun2%10,zancun3/10,zancun3%10,6);
                break;
           case 21:
                if(second_flag==1)
                {
                    second_flag=0;
                    countdown_second--;
                    if(countdown_second==255)
                    {
                        countdown_second=59;
                        countdown_minute--;
                        if(countdown_minute==255)
                        {
                            countdown_minute=59;
                            countdown_hour--;
                            if(countdown_hour==255)
                            {
                               flag1=22;
                               countdown_minute=0;
                               countdown_hour=0;
                               countdown_second=0;
                               countdown_flag=1;
                            }
                        }
                    }
                    
                }
                dis(countdown_hour/10,countdown_hour%10,countdown_minute/10,countdown_minute%10,countdown_second/10,countdown_second%10);//
                break;
           case 22:
                if(countdown_flag>0 && countdown_flag<7)
                {
                   speak=0;
                   if(second_flag==1)
                   {
                       second_flag=0;
                       countdown_flag++;
                   }
                }
                else 
                {
                   speak=1;
                }
                 dis(countdown_hour/10,countdown_hour%10,countdown_minute/10,countdown_minute%10,countdown_second/10,countdown_second%10);//
                break;
            case 23:
                dis(msecond_minute/10,msecond_minute%10,msecond_second/10,msecond_second%10,msecond_msecond%10,12);
                break;
            case 24:
                if(msecond_flag==1)
                {
                     msecond_flag=0;
                     msecond_msecond++;
                     if(msecond_msecond==10)
                     {
                         msecond_msecond=0; 
                         msecond_second++;
                         if(msecond_second==60)
                         {
                           msecond_second=0;
                           msecond_minute++;
                           if(msecond_minute==100)
                           {
                             msecond_minute=99;
                             flag1=23;
                           }
                         }
                     }
                }
                dis(msecond_minute/10,msecond_minute%10,msecond_second/10,msecond_second%10,msecond_msecond%10,12);
                break;
            case 25:
                dis(zancun3/10,zancun3%10,zancun2/10,zancun2%10,zancun1/10,zancun1%10);
                break;
            default:
                break;
        }
    }
}
//****************************************
//显示处理函数
void  judge_clock()
{
    zancun4=hour_count&0xf0;
    zancun4>>=4;
    zancun6=hour_count&0x0f;
    zancun4*=10;
    zancun4+=zancun6;

    zancun5=minute_count&0xf0;
    zancun5>>=4;
    zancun6=minute_count&0x0f;
    zancun5*=10;
    zancun5+=zancun6;

    if(clock_hour==zancun4 && clock_minute==zancun5)
    {
         if(clock_en==1 && clock_flag==0)
         {          
              speak_count=0;//开启蜂鸣器
              clock_flag=1;
              speak_count=0;
         }
    }
    else
    {
     clock_flag=0;
    }
    if(clock_flag==1 && speak_count<400)
    {
       
        if(msecond_count<=5)
        {
           speak=0;
           speak_count++;
        }
        else
        {
          speak=1;
    
        }
    }
    else
    {
        speak=1;
    }
}


//;########################################################################
//;子程序名：r_1byte_ds1302()
//;功能：    从DS1302读一个字节的数据
uchar r_1byte_ds1302()
{ 
  uchar i,temp11=0;
  io_ds1302=1;//置IO为1，准备读入数据
  for(i=0;i<8;i++)
  {
    temp11>>=1;
    if(io_ds1302) temp11 |= 0x80;
    clk_ds1302=1;
    delay_3us();
    delay_3us();
    clk_ds1302=0;
    delay_3us();
  } 
  return(temp11);
}  
//;##############################################################################
//;子程序名：w_1byte_ds1302
//;功能：   向DS1302写一个字节的数据
void w_1byte_ds1302(uchar t)
{
  uchar i;
  for(i=0;i<8;i++)
  {
    if(t & 0x01)
     {io_ds1302=1;}
    else
     {io_ds1302=0;}
    clk_ds1302=1;
    delay_3us();
    delay_3us();
    clk_ds1302=0;
    delay_3us();
    delay_3us();
    t>>=1;
  }  
}

//;#################################################################################
//;子程序名：setbds1302
//;功能：   设置DS1302初始时间,并启动计时
void set_ds1302()
{ 
  uchar i,j;
  rest_ds1302=0;
  delay_3us();
  clk_ds1302=0;
  delay_3us();
  rest_ds1302=1;
  delay_3us();
  w_1byte_ds1302(0x8e);//写控制命令字
  delay_3us();
  w_1byte_ds1302(0x00);//写保护关闭
  clk_ds1302=1;
  delay_3us();
  rest_ds1302=0;
  for(i=0,j=0x80;i<7;i++,j+=2)
  {
    rest_ds1302=0;
    delay_3us();
    clk_ds1302=0;
    delay_3us();
    rest_ds1302=1;
    delay_3us();
    w_1byte_ds1302(j);
    delay_3us();
    w_1byte_ds1302(tab23[i]);
    delay_3us();
    delay_3us();
    clk_ds1302=1;
    delay_3us();
    rest_ds1302=0;
    delay_3us();
    delay_3us();
  }
  rest_ds1302=0;
  delay_3us();
  clk_ds1302=0;
  delay_3us();
  rest_ds1302=1;
  delay_3us();
  w_1byte_ds1302(0x8e);
  delay_3us();
  w_1byte_ds1302(0x80);
  clk_ds1302=1;
  delay_3us();
  rest_ds1302=0;
  delay_3us();
}  

//;#################################################################-------
//;子程序名?y延迟3us
void delay_3us()
{ 
 _nop_();_nop_();_nop_();_nop_();
 }
//**************************************************************************************************
//函数名称：void delay_50US(unsigned int t)
//功能： 延时50*t(us)
void delay_50us(uint t)
{
  unsigned char j; 
  for(;t>0;t--) 
  {
    for(j=46;j>0;j--);
  }
}

//;#################################################################-------
//;子程序名：get1302
void get_ds1302()
{ 
   uchar temp11[7],i,j;
   for(i=0;i<7;i++)
   {temp11[i]=0;}
   for(i=0,j=0x81;i<7;i++,j+=2)
   {  
    rest_ds1302=0;
    delay_3us();
    clk_ds1302=0;
    delay_3us();
    rest_ds1302=1;
    delay_3us();
    w_1byte_ds1302(j);
    temp11[i]=r_1byte_ds1302();
    delay_3us();
    clk_ds1302=1;
    delay_3us(); 
    rest_ds1302=0;
    delay_3us();
  }
  if(temp11[0]!=0xff)
  {second_count=temp11[0];}
  if(temp11[1]!=0xff)// 数据验证
  {minute_count=temp11[1];}
  if(temp11[2]!=0xff)//数据验证
  {hour_count=temp11[2];}
 // date=temp[3];
  //month=temp[4];
 // week=temp[5];
  //year=temp[6];
}

//;#################################################################-------
//;子程序名   初始化T0定时
void Init_Timer0(void)  //
{                     //
 TMOD |= 0x02;	  //使用模式2，8位定时器，使用"|"符号可以在使用多个定时器时不受影响		     
 TH0=0x00;	      //重载值
 TL0=0x00;         //
 EA=1;            //总中断打开
 ET0=1;          //定时器中断打开
 TR0=1;          //定时器开关打开
}
//**********************************************************************
// ;子程序名   初始化T1定时 
void Init_Timer1(void)     
{
 TMOD |= 0x10;	  //使用模式1，16位定时器，使用"|"符号可以在使用多个定时器时不受影响 
 TH1=0x4c;	      //给定初值   定时19456  25ms
 TL1=0x00;
 EA=1;            //总中断打开
 ET1=1;           //定时器中断打开
 TR1=1;           //定时器开关打开
 t0_crycle=0;//定时器中断次数计数单元
}
//;#################################################################-------
//;子程序名：TO 定时中断子程序  interrupt 1 (T0 中断向量号为 1)
void Timer0_isr(void) interrupt 1
{
 uchar i;
 TR0=0;
 
 for(idx=0;idx<8;idx++)
   {  TMP=vdata[idx];
     
     for(i=17;i>0;i--)
     {DIN=TMP&0x0001;
      CLK=1;_nop_();
      CLK=0;_nop_();
      TMP>>=1;
      }
    
      LOAD=1;_nop_();
      LOAD=0;_nop_();
    }
  TR0=1;
  }
//;#################################################################-------
//;子程序名：T1 定时中断子程序  (T1 中断向量号为 3)
  void Timer1_isr(void) interrupt 3
{
	TH1=0x4c;
	TL1=0x00;
	t0_crycle++;
	if(t0_crycle==4)// 0.1秒
	{
	  t0_crycle=0;
      msecond_flag=1;
	  msecond_count++;
      if(msecond_count==10)//1秒
      { 
        msecond_count=0;
        second_flag=1;
      }    
	}
}

//#################################################################-------
//S1按键处理函数
void  judge_s1()
{
    s1_bit=1;//置IO为1，准备读入收据
    if(s1_bit==0)//判断是否有按键按下
    {
        delay_50us(1);// 延时，去除机械抖动
        if(s1_bit==0)
        {
           switch(flag1)
           {
               case 0:
               case 1:
               case 2:
               case 3:
               case 4:
               case 6:
               case 7:
               case 8:
               case 10:
               case 11:
               case 12:
               case 13:
               case 15:
               case 16:
               case 17:
               case 18:
               case 19:
                    flag1++;
                    break;
               case 9:
                    flag1=6;
                    break;
               case 14:
                    flag1=10;
                    break;
               case 20:
                    flag1=15;
                    break;
               case 5:
               case 21:
               case 22:
               case 23://系统从秒表状态复位
               case 24://系统从秒表状态复位
               case 25://系统从计数器复位
                    flag1=0;
                    zancun1=0;
                    zancun2=0;
                    zancun3=0;
                    break;
                default:
                    break;
           }           
           while(s1_bit==0)
           {
              judge_dis();
           }//等待按键释放 
        }
    }
}
//**************************************************
//S2按键处理函数
void  judge_s2()
{
    s2_bit=1;//置IO为1，准备读入收据
    if(s2_bit==0)//判断是否有按键按下
    {
        delay_50us(1);// 延时，去除机械抖动
        if(s2_bit==0)
        {
           switch (flag1)
           {
              case 1: //在显示SET1状态下按S2件，进入修改时间
                 flag1=6;
                 zancun4=hour_count&0xf0;
                 zancun4>>=4;
                 zancun6=hour_count&0x0f;
                 zancun1=zancun4*10+zancun6;
                 //zancun1=hour_count;
                 zancun5=minute_count&0xf0;
                 zancun5>>=4;
                 zancun6=minute_count&0x0f;
                 zancun2=zancun5*10+zancun6;
                 //  zancun2=minute_count;
                 break;
              case 2://在显示SET2状态下按S2，进入设置闹钟
                 zancun1=clock_hour;
                 zancun2=clock_minute;
                 flag1=10;
                 break; 
              case 6://修改时钟小时十位状态下按S2件
              case 7://修改时钟小时个位状态下按S2件
              case 8://修改时钟分钟十位状态下按S2件
              case 9://修改时钟分钟个位状态下按S2件
                 //zancun4=zancun1/10;

                 tab23[2]=zancun1/10*16+zancun1%10;
                  
                 //zancun5=zancun2&0xf0;
                 //zancun5>>=4;
                 tab23[1]=zancun2/10*16+zancun2%10;
                 hour_count=tab23[2];
                 minute_count=tab23[1];
                 second_count=0;
                 tab23[0]=0;
                 set_ds1302();//设置DS1302的初始时间
                 flag1=0;
                 break;
              case 10://修改闹钟小时十位状态下按S2
              case 11://修改闹钟小时个位状态下按S2
              case 12://修改闹钟分钟十位状态下按S2
              case 13://修改闹钟分钟个位状态下按S2
              case 14://修改闹钟使能状态下按S2
                 clock_hour=zancun1;
                 clock_minute=zancun2;
                 clock_en=zancun3;
                 flag1=0;
                 break;
              case 3:
                 flag1=15;
                 zancun1=countdown_hour;
                 zancun2=countdown_minute;
                 zancun3=countdown_second;
                 break;
              case 15:
              case 16:
              case 17:
              case 18:
              case 19:
              case 20:
                 countdown_hour=zancun1;
                 countdown_minute=zancun2;
                 countdown_second=zancun3;
                 flag1=21;
                 countdown_flag=0;
                 break;
              case 22:
                 flag1=21;
                 break;
              case 21:
                 flag1=22;
                 break;
              case 4:
                 flag1=23;//秒表暂停
                 msecond_minute=0;
                 msecond_second=0;
                 msecond_msecond=0;
                 break;
              case 23:
                 flag1=24;
                 break;
              case 24:
                 flag1=23;
                 break;
              case 5:
                 flag1=25;//进入计数器模式
                 break;
              default:
                 break;
                 
           }   
           while(s2_bit==0)
           {
              judge_dis();
           }//等待按键释放 
        }
    }
}
//**************************************************
//S3按键处理函数
void  judge_s3()
{
    s3_bit=1;//置IO为1，准备读入收据
    if(s3_bit==0)//判断是否有按键按下
    {
        delay_50us(1);// 延时，去除机械抖动
        if(s3_bit==0)
        {
           switch (flag1)
           {
             
              case 6://修改时间小时的十位数
                 zancun1+=10;
                 if(zancun1>=24)zancun1=zancun1%10;
                 break;
              case 7://修改时间小时的个位数
                 zancun1=zancun1/10*10+(zancun1%10+1)%10;
                 if(zancun1>=24)zancun1=20;
                 break;
              case 8://修改时间分钟的十位数
                 zancun2+=10;
                 if(zancun2>=60)zancun2-=60;
                 break;
              case 9://修改时间分钟的个位数
                 zancun2=zancun2/10*10+(zancun2%10+1)%10;
                 break;
              case 10://修改闹钟小时的十位数
                 zancun1+=10;
                 if(zancun1>=24)zancun1=zancun1%10;
                 break;
              case 11://修改闹钟小时的个位数
                 zancun1=zancun1/10*10+(zancun1%10+1)%10;
                 if(zancun1>=24)zancun1=20;
                 break;
              case 12://修改闹钟分钟的十位数
                 zancun2+=10;
                 if(zancun2>=60)zancun2-=60;
                 break;
              case 13://修改闹钟分钟的个位数
                 zancun2=zancun2/10*10+(zancun2%10+1)%10;
                 break;
              case 14:
                 zancun3^=1;
                 break;
              case 15://修改倒计时小时的十位数
                 zancun1+=10;
                 if(zancun1>=100)zancun1-=100;
                 break;
              case 16: //修改倒计时小时的个位数
                 zancun1=zancun1/10*10+(zancun1%10+1)%10;
                 break;
              case 17://修改倒计时分钟的十位数
                 zancun2+=10;
                 if(zancun2>=60)zancun2-=60;
                 break;
              case 18: //修改倒计时分钟的个位数
                 zancun2=zancun2/10*10+(zancun2%10+1)%10;
                 break;
              case 19://修改倒计时秒的十位数
                 zancun3+=10;
                 if(zancun3>=60)zancun3-=60;
                 break;
              case 20: //修改倒计时秒的个位数
                 zancun3=zancun3/10*10+(zancun3%10+1)%10;
                 break;
              case 21:
              case 22://
                 countdown_hour=zancun1;
                 countdown_minute=zancun2;
                 countdown_second=zancun3;
                 flag1=21;
                 break;
              case 23:
              case 24://秒表复位
                 flag1=24;
                 msecond_minute=0;
                 msecond_second=0; 
                 msecond_msecond=0;
                 break;
              case 25:
                 zancun1++;
                 if(zancun1==100)
                 {
                     zancun1=0;
                     zancun2++;
                     if(zancun2==100)
                     {
                       zancun2=0;
                       zancun3++;
                     }
                 }
                 break;
              default:
                 break;
           }   
           while(s3_bit==0)
           {
              judge_dis();
           }//等待按键释放 
        }
    }
}
 
 

