// <TASK 정의>
// spell 별로 LED 추가
// 리무스트로아부터 LED 색 지정
// 그리모어 디자인
// 기구학 설계
// 기구 설계 끝나면 배터리 비롯해서 전선길이 확보해서 납땜. 추가기능: 전원 스위치, 배터리 부드럽게 뽁뽁이로 수납, 각각 모듈 흔들리지 않도록 수납.

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include "RedMP3.h"
#include <Adafruit_NeoPixel.h>
#include <Wire.h>
#include <SparkFun_APDS9960.h>

// Pin Definitions
#define ESP_RX 16
#define ESP_TX 17
#define TOUCH_PIN 2
#define NEOPIXEL_PIN1 18
#define NEOPIXEL_PIN2 25
#define NUMPIXELS1 28
#define NUMPIXELS2 9

#define MAGIC_SOUND_1 1
#define MAGIC_SOUND_2 2
#define MAGIC_SOUND_3 3
#define MAGIC_SOUND_4 4
#define MAGIC_SOUND_5 5
#define MAGIC_SOUND_6 6
#define MAGIC_SOUND_7 7
#define MAGIC_SOUND_8 8
#define MAGIC_SOUND_9 9
#define MAGIC_SOUND_10 10
#define MAGIC_SOUND_11 11
#define MAGIC_SOUND_12 12
#define MAGIC_SOUND_13 13
#define MAGIC_SOUND_14 14
#define MAGIC_SOUND_15 15
#define MAGIC_SOUND_16 16
#define MAGIC_SOUND_17 17
#define MAGIC_SOUND_18 18
#define MAGIC_SOUND_19 19
#define MAGIC_SOUND_20 20
#define MAGIC_SOUND_21 21
#define MAGIC_SOUND_22 22
#define MAGIC_SOUND_23 23

bool inputMode = false;

// MP3 Variables
HardwareSerial MP3Serial(2);
MP3 mp3(MP3Serial);
bool isPlaying = false;
const int8_t songIndex = 0x01;
int8_t volume = 7;
int8_t volume2 = 11;
int8_t volume3 = 14;
int8_t volume4 = 17;
int8_t volume5 = 30;
// Neopixel Variables
Adafruit_NeoPixel strip1(NUMPIXELS1, NEOPIXEL_PIN1, NEO_GRB + NEO_KHZ800);
Adafruit_NeoPixel strip2(NUMPIXELS2, NEOPIXEL_PIN2, NEO_GRB + NEO_KHZ800);
const int color1 = strip1.Color(64, 224, 208);
const int color2 = strip1.Color(255, 105, 180);
const int color3 = strip1.Color(255, 250, 180);
const int black = strip1.Color(0, 0, 0);
const int delay1 = 50;
const int delay2 = 1500;
const int brightness1 = 2;
const int brightness2 = 10;

// Gesture Variables
SparkFun_APDS9960 apds;
uint8_t Touch_flag = 0;

uint16_t readTouchSensor() {
  uint16_t touchValue = touchRead(TOUCH_PIN);
  //Serial.println(touchValue);

  if (touchValue >= 40) {
    if (Touch_flag == 0) return touchValue;
    delay(10);
    Touch_flag = 0;
    return touchValue;
  } else {
    if (Touch_flag != 0) return 80;
    Touch_flag = 1;
    delay(10);
    return touchValue;
  }
}

void touchISR() {
  if (readTouchSensor() < 40) {
    if (isPlaying) {
      mp3.pause();
    } else {
      mp3.play();
    }
    isPlaying = !isPlaying;
  }
}

void setStripBrightnessAndColor(Adafruit_NeoPixel& strip, int cell, int brightness, uint32_t color, bool diagonal = false) {
  strip.setBrightness(brightness);
  strip.setPixelColor(cell, color);
  if (diagonal == true && &strip == &strip1 && 16 <= cell && cell <= 27) strip.setPixelColor(cell + 1, color);
  strip.show();
}

void ledAnimationTask(void* param) {
  while (1) {
    for (int i = 0; i < NUMPIXELS2; i++) {
      strip2.setPixelColor(i, strip2.Color(0, 255, 255));
      strip2.show();
      vTaskDelay(pdMS_TO_TICKS(1000));
    }
    vTaskDelay(pdMS_TO_TICKS(3000));
    for (int i = 0; i < NUMPIXELS2; i++) {
      strip2.setPixelColor(i, strip2.Color(0, 0, 0));
    }
    strip2.show();
  }
}

void LED_BLACK() {
  for (int j = 0; j < NUMPIXELS1; j++) {
    strip1.setPixelColor(j, black);
  }
  strip1.show();

  for (int j = 0; j < NUMPIXELS2; j++) {
    strip2.setPixelColor(j, black);
  }
  strip2.show();
}

void COLOR_TEST(uint32_t color) {

  for (int i = 1; i <= 2; i++) {
    strip1.setBrightness(i);
    strip2.setBrightness(i);
    for (int j = 0; j < NUMPIXELS1; j++) {
      strip1.setPixelColor(j, color);
    }
    strip1.show();

    for (int j = 0; j < NUMPIXELS2; j++) {
      strip2.setPixelColor(j, color);
    }
    strip2.show();
    delay(100);
  }
}



void LED_EFFECT(int strip_n, int cell_n, int brightness, uint32_t color) {
  Adafruit_NeoPixel& strip = (strip_n == 1) ? strip1 : strip2;
  setStripBrightnessAndColor(strip, cell_n, brightness, color);
  vTaskDelay(pdMS_TO_TICKS(delay1));
  setStripBrightnessAndColor(strip, cell_n, brightness, black);
}

void LED_TWO_CELLS(int prev_strip_n, int next_strip_n, int prev_cell_n, int next_cell_n, int brightness, uint32_t color, bool diagonal = false) {
  Adafruit_NeoPixel& prev_strip = (prev_strip_n == 1) ? strip1 : strip2;
  Adafruit_NeoPixel& next_strip = (next_strip_n == 1) ? strip1 : strip2;

  setStripBrightnessAndColor(prev_strip, prev_cell_n, brightness, color, diagonal);
  setStripBrightnessAndColor(next_strip, next_cell_n, brightness, color, diagonal);
  vTaskDelay(pdMS_TO_TICKS(delay1));
  setStripBrightnessAndColor(prev_strip, prev_cell_n, brightness, black, diagonal);
  setStripBrightnessAndColor(next_strip, next_cell_n, brightness, black, diagonal);
}

void LED_MOVEMENT(const int* circles, const int* cells, uint32_t color, int brightness, bool diagonal = false) {
  LED_EFFECT(circles[0], cells[0], brightness2, color);
  for (int i = 1; i < 7; i++) {
    LED_TWO_CELLS(circles[i - 1], circles[i], cells[i - 1], cells[i], brightness, color, diagonal);
  }
}




void performRightGesture() {
  LED_BLACK();
  const int circles[7] = { 1, 1, 2, 2, 2, 1, 1 };
  const int cells[7] = { 8, 22, 4, 8, 0, 16, 0 };
  LED_MOVEMENT(circles, cells, color1, brightness1);
}

void performLeftGesture() {
  LED_BLACK();
  const int circles[7] = { 1, 1, 2, 2, 2, 1, 1 };
  const int cells[7] = { 0, 16, 0, 8, 4, 22, 8 };
  LED_MOVEMENT(circles, cells, color1, brightness1);
}

void performUpGesture() {
  LED_BLACK();
  const int circles[7] = { 1, 1, 2, 2, 2, 1, 1 };
  const int cells[7] = { 4, 19, 2, 8, 6, 25, 12 };
  LED_MOVEMENT(circles, cells, color1, brightness1);
}

void performDownGesture() {
  LED_BLACK();
  const int circles[7] = { 1, 1, 2, 2, 2, 1, 1 };
  const int cells[7] = { 12, 25, 6, 8, 2, 19, 4 };
  LED_MOVEMENT(circles, cells, color1, brightness1);
}

void performDownLeftGesture() {
  LED_BLACK();
  const int circles[7] = { 1, 1, 2, 2, 2, 1, 1 };
  const int cells[7] = { 14, 26, 7, 8, 3, 20, 6 };
  LED_MOVEMENT(circles, cells, color2, brightness1, 1);
}

void performDownRightGesture() {
  LED_BLACK();
  const int circles[7] = { 1, 1, 2, 2, 2, 1, 1 };
  const int cells[7] = { 10, 23, 5, 8, 1, 17, 2 };
  LED_MOVEMENT(circles, cells, color2, brightness1, 1);
}

void performUpLeftGesture() {
  LED_BLACK();
  const int circles[7] = { 1, 1, 2, 2, 2, 1, 1 };
  const int cells[7] = { 2, 17, 1, 8, 5, 23, 10 };
  LED_MOVEMENT(circles, cells, color2, brightness1, 1);
}

void performUpRightGesture() {
  LED_BLACK();
  const int circles[7] = { 1, 1, 2, 2, 2, 1, 1 };
  const int cells[7] = { 6, 20, 3, 8, 7, 26, 14 };
  LED_MOVEMENT(circles, cells, color2, brightness1, 1);
}

void performFarGesture() {
  LED_BLACK();
  const int circles[7] = { 1, 1, 2, 2, 2, 1, 1 };
  const int cells[7] = { 24, 20, 16, 12, 8, 4, 0 };
  LED_MOVEMENT(circles, cells, color3, brightness1);
}

uint8_t handleGesture() {
  if (apds.isGestureAvailable()) {
    uint8_t gesture = apds.readGesture();
    switch (gesture) {
      case DIR_UP:
        mp3.setVolume(volume);
        mp3.pause();
        mp3.playWithIndex(MAGIC_SOUND_2);
        performUpGesture();
        break;
      case DIR_DOWN:
        mp3.setVolume(volume);
        mp3.pause();
        mp3.playWithIndex(MAGIC_SOUND_3);
        performDownGesture();
        break;
      case DIR_LEFT:
        mp3.setVolume(volume);
        mp3.pause();
        mp3.playWithIndex(MAGIC_SOUND_4);
        performLeftGesture();
        break;
      case DIR_RIGHT:
        mp3.setVolume(volume);
        mp3.pause();
        mp3.playWithIndex(MAGIC_SOUND_5);
        performRightGesture();
        break;
      case DIR_NEAR:
        mp3.setVolume(volume3);
        mp3.pause();
        mp3.playWithIndex(MAGIC_SOUND_23);
        performNearGesture();
        break;
      case DIR_FAR:
        mp3.setVolume(volume);
        mp3.pause();
        mp3.playWithIndex(MAGIC_SOUND_10);
        performFarGesture();
        break;
      case DIR_DOWN_LEFT:
        mp3.setVolume(volume);
        mp3.pause();
        mp3.playWithIndex(MAGIC_SOUND_6);
        performDownLeftGesture();
        break;
      case DIR_DOWN_RIGHT:
        mp3.setVolume(volume);
        mp3.pause();
        mp3.playWithIndex(MAGIC_SOUND_7);
        performDownRightGesture();
        break;
      case DIR_UP_LEFT:
        mp3.setVolume(volume);
        mp3.pause();
        mp3.playWithIndex(MAGIC_SOUND_8);
        performUpLeftGesture();
        break;
      case DIR_UP_RIGHT:
        mp3.setVolume(volume);
        mp3.pause();
        mp3.playWithIndex(MAGIC_SOUND_9);
        performUpRightGesture();
        break;
      default:
        //Serial.println("NONE");
        break;
    }
    return gesture;
  }
}

void CheckPlayAndBlack() {
  vTaskDelay(pdMS_TO_TICKS(1000));
  int data;
  while (1) {
    data = mp3.getStatus();
    delay(20);
    Serial.println(data);
    if (data == 0) break;
  }
  LED_BLACK();
  delay(500);
  LED_BLACK();
}

void performNearGesture() {

  strip1.setBrightness(brightness1);
  strip2.setBrightness(brightness1);
  // 첫 번째 스트립의 LED를 첫 번째부터 하나씩 1초 간격으로 켜기
  for (int i = 0; i < NUMPIXELS1; i++) {
    strip1.setPixelColor(i, color3);  // 빨간색으로 설정
    strip1.show();                    // 색상을 NeoPixel에 적용
    delay(50 - (i * 1.5));            // 1초 대기
  }

  // 두 번째 스트립의 LED를 첫 번째부터 하나씩 1초 간격으로 켜기
  for (int i = 0; i < NUMPIXELS2; i++) {
    strip2.setPixelColor(i, color3);  // 초록색으로 설정
    strip2.show();                    // 색상을 NeoPixel에 적용
    delay(100 - (28 * 2) - (i * 2));  // 1초 대기
  }

  // 새로운 제스처 시퀀스 초기화
  int sequence[10] = { 0 };
  int sequenceIndex = 0;
  unsigned long startMillis = millis();
  inputMode = true;

  while (inputMode) {

    if (millis() - startMillis > 5000) {  // 5초 동안 입력이 없으면 모드 종료
      LED_BLACK();
      inputMode = false;
      break;
    }
    uint8_t gesture = handleGesture();
    if (gesture == DIR_LEFT || gesture == DIR_RIGHT || gesture == DIR_UP || gesture == DIR_DOWN || gesture == DIR_UP_LEFT || gesture == DIR_UP_RIGHT || gesture == DIR_DOWN_LEFT || gesture == DIR_DOWN_RIGHT) {
      sequence[sequenceIndex++] = gesture;
      startMillis = millis();  // 입력 있을 시 타이머 리셋

      // 조건 1: 왼쪽 -> 오른쪽 -> 왼쪽
      // Nephtear
      /*
      Arrows of Ice: Nephtear ｢氷の矢を放つ魔法ネフティーア Nefutīa?｣
      Produces large spears of ice around the caster that are then fired at the target.
      */
      if (sequenceIndex == 3 && sequence[0] == DIR_LEFT && sequence[1] == DIR_RIGHT && sequence[2] == DIR_LEFT) {
        delay(delay2);
        mp3.setVolume(volume2);
        mp3.playWithIndex(MAGIC_SOUND_11);
        // LED 특수 동작 추가
        // 여기에 LED 특수 동작 코드 작성
        COLOR_TEST(strip2.Color(135, 206, 235));
        CheckPlayAndBlack();
        inputMode = false;
        break;
      }

      // 조건 2: 위 -> 아래 -> 왼쪽 -> 오른쪽
      // Daosdorg
      /*
      Wind becomes Hellfire: Daosdorg ｢風を業火に変える魔法ダオスドルグ Daosudorugu?｣
      Turns a source of wind into a blaze of fire surrounding the target.
      */
      if (sequenceIndex == 4 && sequence[0] == DIR_UP && sequence[1] == DIR_DOWN && sequence[2] == DIR_LEFT && sequence[3] == DIR_RIGHT) {
        delay(delay2);
        mp3.setVolume(volume4);
        mp3.playWithIndex(MAGIC_SOUND_12);
        // LED 특수 동작2 추가
        // 여기에 LED 특수 동작2 코드 작성
        COLOR_TEST(strip2.Color(255, 30, 0));
        CheckPlayAndBlack();
        inputMode = false;
        break;
      }

      // 조건 3: 왼쪽 -> 대각선 아래 왼쪽 -> 오른쪽
      // Doragate
      /*
      Change rocks into bullets: Doragate ｢石を弾丸に変える魔法ドラガーテ Doragāte?｣
      Converts rocks into bullet projectiles that can be directed at a target.
      */
      if (sequenceIndex == 3 && sequence[0] == DIR_LEFT && sequence[1] == DIR_DOWN_LEFT && sequence[2] == DIR_RIGHT) {
        delay(delay2);
        mp3.setVolume(volume4);
        mp3.playWithIndex(MAGIC_SOUND_13);
        // LED 특수 동작3 추가
        // 여기에 LED 특수 동작3 코드 작성
        COLOR_TEST(strip2.Color(184, 134, 11));
        CheckPlayAndBlack();
        inputMode = false;
        break;
      }

      // 조건 4: 위 -> 대각선 위 오른쪽 -> 아래 -> 대각선 아래 오른쪽 -> 위
      // Reelseiden
      /*
      Spell that slashes almost anything: Reelseiden ｢大体なんでも切る魔法レイルザイデン Reiruzaiden?｣
      Slashes through anything with the caster's staff as long as the caster can properly visualize themselves cutting through it. A simple magic that is difficult to follow the trajectory of.
      */
      if (sequenceIndex == 5 && sequence[0] == DIR_UP && sequence[1] == DIR_UP_RIGHT && sequence[2] == DIR_DOWN && sequence[3] == DIR_DOWN_RIGHT && sequence[4] == DIR_UP) {
        delay(delay2);
        mp3.setVolume(volume4);
        mp3.playWithIndex(MAGIC_SOUND_14);
        // LED 특수 동작4 추가
        // 여기에 LED 특수 동작4 코드 작성
        COLOR_TEST(strip2.Color(175, 238, 238));

        delay(1000);

        int data, count;
        while (1) {
          if (count % 2 == 0) performDownRightGesture();
          else if (count % 2 == 1) performDownLeftGesture();
          data = mp3.getStatus();
          delay(1000);
          if (data == 0) break;
          count++;
        }
        CheckPlayAndBlack();
        inputMode = false;
        break;
      }

      // 조건 5: 왼쪽 -> 위 -> 오른쪽 -> 아래 -> 위 -> 왼쪽
      // Reamstroha
      /*
      Water Manipulation: Reamstroha ｢水を操る魔法リームシュトローア Rīmushutorōa?｣
      Surrounding water is collected to form a massive sphere of water directed above the target that then waterfalls down, drenching them. Especially effective when raining. With enough water, the spell is powerful enough to break defensive barriers.
      */
      if (sequenceIndex == 6 && sequence[0] == DIR_LEFT && sequence[1] == DIR_UP && sequence[2] == DIR_RIGHT && sequence[3] == DIR_DOWN && sequence[4] == DIR_UP && sequence[5] == DIR_LEFT) {
        delay(delay2);
        mp3.setVolume(volume3);
        mp3.playWithIndex(MAGIC_SOUND_15);
        // LED 특수 동작5 추가
        // 여기에 LED 특수 동작5 코드 작성
        int reamcolor = strip1.Color(65, 105, 225);
        for (int j = 0; j < NUMPIXELS2; j++) {
          strip2.setPixelColor(j, reamcolor);
          strip2.show();
          delay(100);
        }
        for (int j = 15; j >= 0; j--) {
          strip1.setPixelColor(j, reamcolor);
          strip1.show();
          delay(50);
        }

        CheckPlayAndBlack();
        inputMode = false;
        break;
      }

      // 조건 6: 오른쪽 -> 대각선 아래 오른쪽 -> 왼쪽
      // Shield
      if (sequenceIndex == 3 && sequence[0] == DIR_RIGHT && sequence[1] == DIR_DOWN_RIGHT && sequence[2] == DIR_LEFT) {
        delay(delay2);
        mp3.setVolume(volume3);
        mp3.playWithIndex(MAGIC_SOUND_16);
        // LED 특수 동작6 추가
        // 여기에 LED 특수 동작6 코드 작성
        for (int i = 0; i < 3; i++) {
          strip1.setPixelColor(26, color1);
          strip1.setPixelColor(12, color1);
          strip1.setPixelColor(24, color1);
          strip2.setPixelColor(6, color1);

          strip1.setPixelColor(0, color1);
          strip1.setPixelColor(27, color1);
          strip2.setPixelColor(0, color1);
          strip1.setPixelColor(17, color1);

          strip1.setPixelColor(18, color1);
          strip2.setPixelColor(2, color1);
          strip1.setPixelColor(20, color1);
          strip1.setPixelColor(4, color1);

          strip2.setPixelColor(4, color1);
          strip1.setPixelColor(23, color1);
          strip1.setPixelColor(8, color1);
          strip1.setPixelColor(21, color1);

          strip1.show();
          strip2.show();

          //vTaskDelay(pdMS_TO_TICKS(200));
          delay(i * 50);

          LED_BLACK();

          delay(i * 50);

          strip1.setPixelColor(14, color1);
          strip1.setPixelColor(26, color1);
          strip2.setPixelColor(7, color1);
          strip1.setPixelColor(27, color1);

          strip1.setPixelColor(24, color1);
          strip1.setPixelColor(10, color1);
          strip1.setPixelColor(23, color1);
          strip2.setPixelColor(5, color1);

          strip2.setPixelColor(3, color1);
          strip1.setPixelColor(21, color1);
          strip1.setPixelColor(6, color1);
          strip1.setPixelColor(20, color1);

          strip1.setPixelColor(17, color1);
          strip2.setPixelColor(1, color1);
          strip1.setPixelColor(18, color1);
          strip1.setPixelColor(2, color1);

          strip1.show();
          strip2.show();
          //vTaskDelay(pdMS_TO_TICKS(200));
          delay(i * 100);

          LED_BLACK();


          delay(i * 100);
        }

        strip1.setPixelColor(26, color1);
        strip1.setPixelColor(12, color1);
        strip1.setPixelColor(24, color1);
        strip2.setPixelColor(6, color1);

        strip1.setPixelColor(0, color1);
        strip1.setPixelColor(27, color1);
        strip2.setPixelColor(0, color1);
        strip1.setPixelColor(17, color1);

        strip1.setPixelColor(18, color1);
        strip2.setPixelColor(2, color1);
        strip1.setPixelColor(20, color1);
        strip1.setPixelColor(4, color1);

        strip2.setPixelColor(4, color1);
        strip1.setPixelColor(23, color1);
        strip1.setPixelColor(8, color1);
        strip1.setPixelColor(21, color1);

        strip1.show();
        strip2.show();

        CheckPlayAndBlack();

        inputMode = false;
        break;
      }

      // 조건 7: 위 -> 대각선 아래 오른쪽 -> 아래 -> 대각선 위 왼쪽
      // Waldgose
      /*
      Tornado Winds: Waldgose ｢竜巻を起こす魔法ヴァルドゴーゼ Varudogōze?｣
      Forms a tornado, enveloping the target.
      */
      if (sequenceIndex == 4 && sequence[0] == DIR_UP && sequence[1] == DIR_DOWN_RIGHT && sequence[2] == DIR_DOWN && sequence[3] == DIR_UP_LEFT) {
        delay(delay2);
        mp3.setVolume(volume3);
        mp3.playWithIndex(MAGIC_SOUND_17);
        // LED 특수 동작7 추가
        // 여기에 LED 특수 동작7 코드 작성
        int red = strip1.Color(255, 0, 0);
        for (int i = 0; i < NUMPIXELS1; i++) {
          strip1.setPixelColor(i, red);  // 빨간색으로 설정
          strip1.show();                 // 색상을 NeoPixel에 적용
          delay(50 - (i * 1.5));         // 1초 대기
        }

        // 두 번째 스트립의 LED를 첫 번째부터 하나씩 1초 간격으로 켜기
        for (int i = 0; i < NUMPIXELS2; i++) {
          strip2.setPixelColor(i, red);     // 초록색으로 설정
          strip2.show();                    // 색상을 NeoPixel에 적용
          delay(100 - (28 * 2) - (i * 2));  // 1초 대기
        }
        CheckPlayAndBlack();
        inputMode = false;
        break;
      }

      // 조건 8: 왼쪽 -> 위 -> 오른쪽 -> 아래 -> 대각선 위 왼쪽 -> 대각선 아래 오른쪽 -> 위 -> 아래
      // Sorganeil
      /*
      Spell of Binding: Sorganeil ｢見た者を拘束する魔法ソルガニール Soruganīru?｣
      Completely prevents someone from moving or accessing their mana to cast spells while their full body is within the user’s vision. The spell is visually represented with particles moving in a circular motion that surround the target. The target's body, including hair, must be in full view while casting the spell and maintaining it, otherwise the spell disables itself.
      */
      if (sequenceIndex == 8 && sequence[0] == DIR_LEFT && sequence[1] == DIR_UP && sequence[2] == DIR_RIGHT && sequence[3] == DIR_DOWN && sequence[4] == DIR_UP_LEFT && sequence[5] == DIR_DOWN_RIGHT && sequence[6] == DIR_UP && sequence[7] == DIR_DOWN) {
        delay(delay2);
        mp3.setVolume(volume3);
        mp3.playWithIndex(MAGIC_SOUND_18);
        // LED 특수 동작8 추가
        // 여기에 LED 특수 동작8 코드 작성


        for (int i = 0; i < 3; i++) {
          strip1.setPixelColor(14, strip2.Color(255, 255, 0));
          strip2.setPixelColor(7, strip2.Color(255, 255, 0));
          strip2.setPixelColor(3, strip2.Color(255, 255, 0));
          strip1.setPixelColor(6, strip2.Color(255, 255, 0));

          strip2.setPixelColor(8, strip2.Color(255, 255, 0));

          strip1.setPixelColor(10, strip2.Color(255, 255, 0));
          strip2.setPixelColor(5, strip2.Color(255, 255, 0));
          strip2.setPixelColor(1, strip2.Color(255, 255, 0));
          strip1.setPixelColor(2, strip2.Color(255, 255, 0));

          strip1.show();
          strip2.show();

          //vTaskDelay(pdMS_TO_TICKS(200));
          delay(i * 50);

          LED_BLACK();

          delay(i * 50);

          strip1.setPixelColor(14, strip2.Color(255, 255, 0));
          strip2.setPixelColor(7, strip2.Color(255, 255, 0));
          strip2.setPixelColor(3, strip2.Color(255, 255, 0));
          strip1.setPixelColor(6, strip2.Color(255, 255, 0));

          strip2.setPixelColor(8, strip2.Color(255, 255, 0));

          strip1.setPixelColor(10, strip2.Color(255, 255, 0));
          strip2.setPixelColor(5, strip2.Color(255, 255, 0));
          strip2.setPixelColor(1, strip2.Color(255, 255, 0));
          strip1.setPixelColor(2, strip2.Color(255, 255, 0));

          strip1.show();
          strip2.show();
          //vTaskDelay(pdMS_TO_TICKS(200));
          delay(i * 100);

          LED_BLACK();

          delay(i * 100);
        }

        strip1.setPixelColor(14, strip2.Color(255, 255, 0));
        strip2.setPixelColor(7, strip2.Color(255, 255, 0));
        strip2.setPixelColor(3, strip2.Color(255, 255, 0));
        strip1.setPixelColor(6, strip2.Color(255, 255, 0));

        strip2.setPixelColor(8, strip2.Color(255, 255, 0));

        strip1.setPixelColor(10, strip2.Color(255, 255, 0));
        strip2.setPixelColor(5, strip2.Color(255, 255, 0));
        strip2.setPixelColor(1, strip2.Color(255, 255, 0));
        strip1.setPixelColor(2, strip2.Color(255, 255, 0));

        strip1.show();
        strip2.show();

        CheckPlayAndBlack();
        inputMode = false;
        break;
      }

      // 조건 9: 위 -> 오른쪽 -> 아래
      // Zoltraak
      /*
      Offensive magic: Zoltraak ｢一般攻撃魔法ゾルトラーク Zorutorāku?｣
      A simple spell that shoots either a large blast or multiple smaller blasts of concentrated mana. Originally created by the demon Qual as the first form of piercing magic, but humans have altered it to be effective against demons. Considered the basis of humans' modern offensive magic system.
      */
      if (sequenceIndex == 3 && sequence[0] == DIR_UP && sequence[1] == DIR_RIGHT && sequence[2] == DIR_DOWN) {
        delay(delay2);
        mp3.setVolume(volume3);
        mp3.playWithIndex(MAGIC_SOUND_19);
        // LED 특수 동작9 추가
        // 여기에 LED 특수 동작9 코드 작성
        for (int i = 0; i < 3; i++) {
          for (int j = 0; j < 16; j++) {
            strip1.setPixelColor(j, strip1.Color(0, 191, 255));
            strip1.show();
          }
          for (int j = 0; j < NUMPIXELS2; j++) {
            strip2.setPixelColor(j, strip2.Color(0, 191, 255));
            strip2.show();
          }

          strip1.show();
          strip2.show();

          //vTaskDelay(pdMS_TO_TICKS(200));
          delay(i * 50);

          LED_BLACK();

          delay(i * 50);

          for (int j = 0; j < 16; j++) {
            strip1.setPixelColor(j, strip1.Color(0, 191, 255));
            strip1.show();
          }
          for (int j = 0; j < NUMPIXELS2; j++) {
            strip2.setPixelColor(j, strip2.Color(0, 191, 255));
            strip2.show();
          }

          //vTaskDelay(pdMS_TO_TICKS(200));
          delay(i * 100);

          LED_BLACK();

          delay(i * 100);
        }

        for (int j = 0; j < 16; j++) {
          strip1.setPixelColor(j, strip1.Color(0, 191, 255));
          strip1.show();
        }
        for (int j = 0; j < NUMPIXELS2; j++) {
          strip2.setPixelColor(j, strip2.Color(0, 191, 255));
          strip2.show();
        }

        CheckPlayAndBlack();
        inputMode = false;
        break;
      }

      // 조건 10: 왼쪽 -> 대각선 위 왼쪽 -> 아래 -> 오른쪽
      // Jilwer
      /*
      Spell for High-Speed Movement: Jilwer ｢高速で移動する魔法ジルヴェーア Jiruvēa?｣
      A folk spell originally belonging to a mountain tribe in the Southern Lands.[3] It allows the caster to move at high speeds, even when they are holding objects or individuals.
      */
      if (sequenceIndex == 4 && sequence[0] == DIR_LEFT && sequence[1] == DIR_UP_LEFT && sequence[2] == DIR_DOWN && sequence[3] == DIR_RIGHT) {
        delay(delay2);
        mp3.setVolume(volume3);
        mp3.playWithIndex(MAGIC_SOUND_20);
        // LED 특수 동작10 추가
        // 여기에 LED 특수 동작10 코드 작성
        int JilwerColor = strip1.Color(0, 255, 255);
        
        for (int i = 1; i <= 2; i++) {
          strip1.setBrightness(i);
          strip2.setBrightness(i);
          for (int j = 0; j < 16; j++) {
            strip1.setPixelColor(j, JilwerColor);
            strip1.show();
          }
          for (int j = 0; j < NUMPIXELS2; j++) {
            strip2.setPixelColor(j, JilwerColor);
            strip2.show();
          }
          delay(100);
        }

        CheckPlayAndBlack();
        inputMode = false;
        break;
      }

      // 조건 11: 대각선 아래 오른쪽 -> 위 -> 대각선 위 왼쪽 -> 아래 -> 오른쪽
      // Catastravia
      /*
      Lights of Judgement: Catastravia ｢裁きの光を放つ魔法カタストラーヴィア Katasutorāvia?｣
      Numerous arrows of light are directed at the target. Upon contact, they form a barrage of large explosions.
      */
      if (sequenceIndex == 5 && sequence[0] == DIR_DOWN_RIGHT && sequence[1] == DIR_UP && sequence[2] == DIR_UP_LEFT && sequence[3] == DIR_DOWN && sequence[4] == DIR_RIGHT) {
        delay(delay2);
        mp3.setVolume(volume3);
        mp3.playWithIndex(MAGIC_SOUND_21);
        // LED 특수 동작11 추가
        // 여기에 LED 특수 동작11 코드 작성
        int gold = strip1.Color(255, 215, 0);
        for (int i = 0; i < 3; i++) {
          strip1.setPixelColor(14, gold);
          strip2.setPixelColor(7, gold);
          strip2.setPixelColor(3, gold);
          strip1.setPixelColor(6, gold);

          strip2.setPixelColor(8, gold);

          strip1.setPixelColor(10, gold);
          strip2.setPixelColor(5, gold);
          strip2.setPixelColor(1, gold);
          strip1.setPixelColor(2, gold);

          strip1.show();
          strip2.show();

          //vTaskDelay(pdMS_TO_TICKS(200));
          delay(i * 50);

          LED_BLACK();

          delay(i * 50);

          strip1.setPixelColor(12, gold);
          strip1.setPixelColor(25, gold);
          strip2.setPixelColor(6, gold);

          strip2.setPixelColor(8, gold);

          strip2.setPixelColor(2, gold);
          strip1.setPixelColor(19, gold);
          strip1.setPixelColor(4, gold);
          //
          strip1.setPixelColor(8, gold);
          strip1.setPixelColor(22, gold);
          strip2.setPixelColor(4, gold);

          strip2.setPixelColor(0, gold);
          strip1.setPixelColor(16, gold);
          strip1.setPixelColor(0, gold);



          strip1.show();
          strip2.show();
          //vTaskDelay(pdMS_TO_TICKS(200));
          delay(i * 100);

          LED_BLACK();

          delay(i * 100);
        }

        strip1.setPixelColor(14, gold);
        strip2.setPixelColor(7, gold);
        strip2.setPixelColor(3, gold);
        strip1.setPixelColor(6, gold);

        strip2.setPixelColor(8, gold);

        strip1.setPixelColor(10, gold);
        strip2.setPixelColor(5, gold);
        strip2.setPixelColor(1, gold);
        strip1.setPixelColor(2, gold);

        //
        strip1.setPixelColor(12, gold);
        strip1.setPixelColor(25, gold);
        strip2.setPixelColor(6, gold);

        strip2.setPixelColor(2, gold);
        strip1.setPixelColor(19, gold);
        strip1.setPixelColor(4, gold);
        //
        strip1.setPixelColor(8, gold);
        strip1.setPixelColor(22, gold);
        strip2.setPixelColor(4, gold);

        strip2.setPixelColor(0, gold);
        strip1.setPixelColor(16, gold);
        strip1.setPixelColor(0, gold);

        strip1.show();
        strip2.show();

        CheckPlayAndBlack();
        inputMode = false;
        break;
      }

      // 조건 12: 대각선 아래 왼쪽 -> 오른쪽 -> 위
      // Healing
      if (sequenceIndex == 3 && sequence[0] == DIR_DOWN_LEFT && sequence[1] == DIR_RIGHT && sequence[2] == DIR_UP) {
        delay(delay2);
        mp3.setVolume(volume4);
        mp3.playWithIndex(MAGIC_SOUND_22);
        // LED 특수 동작12 추가
        // 여기에 LED 특수 동작12 코드 작성
        int healingColor = strip1.Color(173, 255, 47);
        for (int i = 1; i <= 2; i++) {
          strip1.setBrightness(i);
          strip2.setBrightness(i);
          for (int j = 0; j < 16; j++) {
            strip1.setPixelColor(j, healingColor);
            strip1.show();
          }
          for (int j = 0; j < NUMPIXELS2; j++) {
            strip2.setPixelColor(j, healingColor);
            strip2.show();
          }
          delay(100);
        }

        CheckPlayAndBlack();
        inputMode = false;
        break;
      }
    }
  }
}





void mp3_setup() {
  MP3Serial.begin(9600, SERIAL_8N1, ESP_RX, ESP_TX);
  mp3.setVolume(volume);
  mp3.playWithIndex(10);
}

void neopixel_setup() {
  strip1.begin();
  strip1.setBrightness(10);
  strip2.begin();
  strip2.setBrightness(10);
}

void gesture_setup() {
  Wire.begin(21, 22);
  Wire.setClock(100000); // 이 줄이 핵심입니다.
  delay(600);           // 센서에게 마음의 준비를 할 시간을 줍니다.
  if (apds.init()) {
    Serial.println(F("APDS-9960 initialization complete"));
  } else {
    Serial.println(F("Something went wrong during APDS-9960 init!"));
  }

  if (apds.enableGestureSensor(false)) {
    Serial.println(F("Gesture sensor is now running"));
  } else {
    Serial.println(F("Something went wrong during gesture sensor init!"));
  }
}

void setup() {
  Serial.begin(115200);
  mp3_setup();
  neopixel_setup();
  gesture_setup();
}

void loop() {
  touchISR();
  if (inputMode == false) handleGesture();
}