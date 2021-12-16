#include "Ticker.h"
Ticker showLedTicker;
Ticker showSigTicker;
const char *lightStyle = "00000000000000000000000000000000000000000";
int count = 0;
const char *sigStyle = "00000000000000000000000000000000000000000";
int sigCount = 0;
/**
   灯光闪烁模式
*/
void showLight(long type) {
  light = true;
  Serial.println("闪烁模式");
  Serial.println(type);
  if (type == 1) { //快闪
    lightStyle = "1010101010101010101010101010101010101010";
  } else if (type == 2) { //双闪
    lightStyle = "100100000000000000000000000000000000000";
  } else if (type == 3) { //慢闪
    lightStyle = "1111111111111111110000000000000000000000";
  } else if (type == 4) { //常亮
    lightStyle = "1111111111111111111111111111111111111111";
  } else if (type == 5) { //6闪
    lightStyle = "110000000000010101010101000000000000000";
  }

}

/**
   关闭灯光
*/
void closeLight() {
  lightStyle = "00000000000000000000000000000000000000000";
}

/**
   led灯闪烁控制
*/
void LED_Task()
{
  // 启动灯光检查前先默认将灯关掉
  digitalWrite(relayInput, 1);
  //每隔50毫秒翻转一次
  showLedTicker.attach_ms(50, []() {

    if (count >= 40) {
      count = 0;
    }

    if (String(lightStyle[count]) == "1") {
      // 开灯
      digitalWrite(relayInput, 0);
    } else {
      // 关灯
      digitalWrite(relayInput, 1);
    }
    ++count;
  });

}
