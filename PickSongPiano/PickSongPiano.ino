/*****************************************
 * 点歌台应用程序
 ****************************************/

#include <Microduino_ColorLED.h>  //彩灯库
#include <Microduino_Key.h>       //按键库
#include <Microduino_AudioPro.h>  //audioPro库
#include "Microduino_Music.h"     //音乐库

#define LED_NUM 7                 //定义彩灯数量
#define PIN_LED SDA               //定义彩灯端口号

/***********定义彩灯颜色*/
const uint32_t colorArray[10] = { 
  COLOR_NONE,       //0x000000, 彩灯关闭，colorLED OFF
  COLOR_RED,        //0xff0000, 红色，Red
  COLOR_ORANGE,     //0xea8b17, 橙色，Orange
  COLOR_YELLOW,     //0xffff00, 黄色，Yellow
  COLOR_GREEN,      //0x00ff00, 绿色，Green
  COLOR_CYAN,       //0x00ffff, 青色，Cyan
  COLOR_BLUE,       //0x0000ff, 蓝色，Blue
  COLOR_PURPLE,     //0xff00ff, 紫色，Purple
  COLOR_WARM,       //0x9b6400, 暖色，Warm
  COLOR_COLD,       //0x648264, 冷色，Cold
};

ColorLED strip = ColorLED(LED_NUM, PIN_LED);  //ColorLED类实例化

AudioPro midiPlayer;            //AudioPro类实例化

DigitalKey touchButton[7] = {   //touchButton引脚分配
  DigitalKey(4), DigitalKey(5), DigitalKey(6), DigitalKey(8), DigitalKey(9), DigitalKey(A0), DigitalKey(A1)
};

int playNum = 7;  //初始定义曲目编号变量为7

void setup() {
  /***********串口初始化*/
  Serial.begin(115200);

  /***********触摸按键初始化*/
  for (int a = 0; a < 7; a++) {
    touchButton[a].begin();
  }

  /*************彩灯初始化*/
  strip.begin();
  strip.setBrightness(80);  //设置彩灯亮度
  strip.show(); 

  /************audioPro初始化*/
  if (! midiPlayer.begin()) { // initialise the music player
    Serial.println(F("Couldn't find VS1053, do you have the right pins defined?"));
    while (1);
  }
  Serial.println(F("VS1053 found"));
  midiPlayer.applyPatch(MIDIPatch, sizeof(MIDIPatch) / sizeof(uint16_t));
  midiPlayer.midiSetVolume(0, 127);
  midiPlayer.midiSetBank(0, VS1053_BANK_DEFAULT);
  midiPlayer.midiSetInstrument(0, VS1053_GM1_ELECTRIC_GRAND_PIANO);
  delay(200);

  /**********彩灯与音乐逐渐点亮与响起*/
  for (int a = 0; a < 7; a++) {
    midiPlayer.noteOn(0, toneNum[a], 120);     //播放相应音级
    strip.setPixelColor(a, colorArray[a + 1]); //设置相应灯的颜色
    strip.show();                              //将灯点亮成新的颜色
    delay(100);
    midiPlayer.noteOff(0, toneNum[a], 120);    //关闭相应音级
    strip.setPixelColor(a, colorArray[0]);     //设置相应灯的颜色
    strip.show();                              //将灯点亮成新的颜色
  }
  delay(500);

  /***********彩灯与音乐同时点亮与响起*/
  for (int a = 0; a < 7; a++) {
    midiPlayer.noteOn(0, toneNum[a], 120);      //播放相应音级
    strip.setPixelColor(a, colorArray[a + 1]);  //设置相应灯的颜色
    strip.show();                               //将灯点亮成新设置的颜色
  }
  delay(1000);
  
  /************关闭音乐*/
  for (int a = 0; a < 7; a++) {
    midiPlayer.noteOff(0, toneNum[a], 120);     //关闭相应音级
    delay(20);
  }

}

void loop() {   /**********播放选择的曲目，彩灯随音乐亮灭*/

  /**********通过判断触摸按键状态，选择曲目号*/
  for (int i = 0; i < 7; i++) {
    if (touchButton[i].readVal() == KEY_PRESSED) { //如果触摸按键i被按下，则表示选择了曲目i
      playNum = i;
      Serial.print("playNum: ");                  //串口打印曲目号
      Serial.println(i);
    }
  }

  /**********曲目选择完成后播放相应曲目*/
  int playSta = 0;     //定义临时变量playSta，用来计算彩灯号
  if (playNum != 7) {  //如果选择了曲目，执行此函数
    /*把所有灯熄灭*/
    strip.clear();
    strip.show();

    /*播放音乐，亮相应彩灯，延节拍时间，关闭音乐，关闭彩灯，然后循环*/
    for (int i = 0; i < notationNum[playNum]; i++) {                     //循环次数为对应曲目的音阶数
      playSta = (notationName[playNum][i] - 1) % 7;                      //计算音阶所对应的彩灯号
      midiPlayer.noteOn(0, toneNum[notationName[playNum][i] - 1], 120);  //播放相应音级
      strip.setPixelColor(playSta, colorArray[playSta + 1]);             //设置相应灯的颜色
      strip.show();                                                      //将灯点亮成新设置的颜色
      delay(300 * (beatsName[playNum][i]));                              //延时节拍相对应时间
      midiPlayer.noteOff(0, toneNum[notationName[playNum][i] - 1], 120); //关闭相应音级
      strip.setPixelColor(playSta, colorArray[0]);                       //设置相应灯的颜色
      strip.show();                                                      //将灯点亮成新设置的颜色
      delay(100);
    }
    /*播放结束，将曲目号设置为7，并把所有灯点亮*/
    playNum = 7;   //将曲目号设置为7，因为曲目7不存在，所以曲目号为7时代表未选择曲目
    for (int a = 0; a < 7; a++) {
      strip.setPixelColor(a, colorArray[a + 1]); //设置相应灯的颜色
      strip.show();
    }
    delay(1);
  }
}

