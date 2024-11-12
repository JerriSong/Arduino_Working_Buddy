#include <LiquidCrystal.h>
#include <Adafruit_NeoPixel.h>

// Define all notes
#define NOTE_B0  31
#define NOTE_C1  33
#define NOTE_CS1 35
#define NOTE_D1  37
#define NOTE_DS1 39
#define NOTE_E1  41
#define NOTE_F1  44
#define NOTE_FS1 46
#define NOTE_G1  49
#define NOTE_GS1 52
#define NOTE_A1  55
#define NOTE_AS1 58
#define NOTE_B1  62
#define NOTE_C2  65
#define NOTE_CS2 69
#define NOTE_D2  73
#define NOTE_DS2 78
#define NOTE_E2  82
#define NOTE_F2  87
#define NOTE_FS2 93
#define NOTE_G2  98
#define NOTE_GS2 104
#define NOTE_A2  110
#define NOTE_AS2 117
#define NOTE_B2  123
#define NOTE_C3  131
#define NOTE_CS3 139
#define NOTE_D3  147
#define NOTE_DS3 156
#define NOTE_E3  165
#define NOTE_F3  175
#define NOTE_FS3 185
#define NOTE_G3  196
#define NOTE_GS3 208
#define NOTE_A3  220
#define NOTE_AS3 233
#define NOTE_B3  247
#define NOTE_C4  262
#define NOTE_CS4 277
#define NOTE_D4  294
#define NOTE_DS4 311
#define NOTE_E4  330
#define NOTE_F4  349
#define NOTE_FS4 370
#define NOTE_G4  392
#define NOTE_GS4 415
#define NOTE_A4  440
#define NOTE_AS4 466
#define NOTE_B4  494
#define NOTE_C5  523
#define NOTE_CS5 554
#define NOTE_D5  587
#define NOTE_DS5 622
#define NOTE_E5  659
#define NOTE_F5  698
#define NOTE_FS5 740
#define NOTE_G5  784
#define NOTE_GS5 831
#define NOTE_A5  880
#define NOTE_AS5 932
#define NOTE_B5  988
#define NOTE_C6  1047
#define NOTE_CS6 1109
#define NOTE_D6  1175
#define NOTE_DS6 1245
#define NOTE_E6  1319
#define NOTE_F6  1397
#define NOTE_FS6 1480
#define NOTE_G6  1568
#define NOTE_GS6 1661
#define NOTE_A6  1760
#define NOTE_AS6 1865
#define NOTE_B6  1976
#define NOTE_C7  2093
#define NOTE_CS7 2217
#define NOTE_D7  2349
#define NOTE_DS7 2489
#define NOTE_E7  2637
#define NOTE_F7  2794
#define NOTE_FS7 2960
#define NOTE_G7  3136
#define NOTE_GS7 3322
#define NOTE_A7  3520
#define NOTE_AS7 3729
#define NOTE_B7  3951
#define NOTE_C8  4186
#define NOTE_CS8 4435
#define NOTE_D8  4699
#define NOTE_DS8 4978
#define REST      0

// LED Ring Setting
#define LED_PIN     13
#define NUM_LEDS    24
#define BRIGHTNESS  100
#define PROBLEM_LED 10

// Initialize LED ring
Adafruit_NeoPixel ring = Adafruit_NeoPixel(NUM_LEDS, LED_PIN, NEO_GRB + NEO_KHZ800);

// Initialize LCD pins
LiquidCrystal lcd(12, 11, 5, 4, 3, 2);

// Defining Pins
const int START_BUTTON_PIN = 6;    
const int CUP_BUTTON_PIN = 7;      
const int HUMAN_TRIG_PIN = 8;      
const int HUMAN_ECHO_PIN = 9;
const int PIEZO_PIN = 10;          

// LED Color Definition
const uint32_t COLOR_BLUE = ring.Color(0, 0, 255);
const uint32_t COLOR_GREEN = ring.Color(0, 255, 0);
const uint32_t COLOR_RED = ring.Color(255, 0, 0);   
const uint32_t COLOR_OFF = ring.Color(0, 0, 0);

// Soft relaxing music
const int PROGMEM relaxMelody[] = {
    NOTE_G4, 4, NOTE_E4, 4, NOTE_G4, 4, NOTE_E4, 4,    
    NOTE_D4, 2, REST, 4,                              
    NOTE_E4, 4, NOTE_G4, 4, NOTE_E4, 4, NOTE_D4, 4,    
    NOTE_C4, 2, REST, 4,                              
    
    NOTE_E4, 4, NOTE_G4, 4, NOTE_E4, 4, NOTE_C4, 4,    
    NOTE_D4, 2, REST, 4,
    NOTE_C4, 4, NOTE_E4, 4, NOTE_D4, 4, NOTE_B3, 4,
    NOTE_C4, 2, REST, 4,
    
    NOTE_G3, 4, NOTE_B3, 4, NOTE_C4, 4, NOTE_D4, 4,
    NOTE_E4, 2, REST, 4,
    NOTE_D4, 4, NOTE_C4, 4, NOTE_B3, 4, NOTE_G3, 4,
    NOTE_C4, 2, REST, 4
};

// Warning Music
const int PROGMEM warningMelody[] = {
  NOTE_E7, 8, NOTE_E7, 8, REST, 8, NOTE_E7, 8,
  REST, 8, NOTE_C7, 8, NOTE_E7, 8, REST, 8,
  NOTE_G7, 4, REST, 4,
  NOTE_G6, 4, REST, 4
};

// State enum
enum State {
  IDLE,          
  WORKING,       
  PAUSED,        
  BREAK          
};

// Double click variables
unsigned long lastButtonPress = 0;
const unsigned long DOUBLE_CLICK_TIME = 500;  // 双击间隔时间500ms
int clickCount = 0;

// Variables
State currentState = IDLE;
unsigned long startTime = 0;
unsigned long pausedTime = 0;
bool isCupPresent = true;

// Distance Detection
unsigned long lastDistanceCheck = 0;
const long DISTANCE_CHECK_INTERVAL = 100;  
const long DELAY_TIME = 1000;            
unsigned long distanceTimeCheck = 0;
bool isDistanceWarning = false;

// Music variables
unsigned long lastMelodyTime = 0;
unsigned long lastRelaxMelodyTime = 0;
int currentNote = 0;
int relaxNoteIndex = 0;
bool isPlayingRelaxMusic = false;

// LED control functions
void setAllLEDs(uint32_t color) {
  for(int i = 0; i < NUM_LEDS; i++) {
    ring.setPixelColor(i, color);
  }
  ring.show();
}

void showCountdown(int remainingSeconds) {
  // 映射剩余时间到LED数量
  int ledsToLight = map(remainingSeconds, 0, 40, 0, NUM_LEDS);
  
  // 先全部熄灭
  setAllLEDs(COLOR_OFF);
  
  // 从LED环的起始位置开始，顺时针点亮LED
  for(int i = NUM_LEDS - 1; i >= (NUM_LEDS - ledsToLight); i--) {
    ring.setPixelColor(i, COLOR_GREEN);
  }
  
  ring.show();
}

void flashRed() {
  static bool ledState = false;
  static unsigned long lastFlash = 0;
  
  if(millis() - lastFlash > 500) {  
    ledState = !ledState;
    setAllLEDs(ledState ? COLOR_RED : COLOR_OFF);
    lastFlash = millis();
  }
}

void breathingBlue() {
  static unsigned long lastRotate = 0;
  static int currentLed = 0;
  
  if(millis() - lastRotate > 50) {  // 保持50ms的速度
    // 先清除所有LED
    setAllLEDs(COLOR_OFF);
    
    // 点亮12个LED形成半圆光带
    for(int i = 0; i < 12; i++) {
      int ledPos = (currentLed + i) % NUM_LEDS;
      // 添加渐变效果
      if(i < 3) {  // 头部渐变
        ring.setPixelColor(ledPos, ring.Color(0, 0, 50 + (i * 68)));
      } else if(i > 8) {  // 尾部渐变
        ring.setPixelColor(ledPos, ring.Color(0, 0, 50 + ((11-i) * 68)));
      } else {  // 中间部分全亮
        ring.setPixelColor(ledPos, COLOR_BLUE);
      }
    }
    
    ring.show();
    
    // 移动到下一个LED位置
    currentLed = (currentLed + 1) % NUM_LEDS;
    lastRotate = millis();
  }
}
// Music functions
void playRelaxMusic() {
  unsigned long currentTime = millis();
  if (currentTime - lastRelaxMelodyTime > 300) {
    pinMode(PIEZO_PIN, OUTPUT);
    int noteDuration = 1200 / pgm_read_word_near(relaxMelody + relaxNoteIndex + 1);
    int noteToPlay = pgm_read_word_near(relaxMelody + relaxNoteIndex);
    
    if (noteToPlay != REST) {
      tone(PIEZO_PIN, noteToPlay, noteDuration * 0.8);
    }
    
    lastRelaxMelodyTime = currentTime;
    relaxNoteIndex = (relaxNoteIndex + 2) % (sizeof(relaxMelody) / sizeof(relaxMelody[0]));
    
    if (relaxNoteIndex == 0) {
      delay(3000);
    }
  }
}

void playWarningMusic() {
  unsigned long currentTime = millis();
  if (currentTime - lastMelodyTime > 100) {
    pinMode(PIEZO_PIN, OUTPUT);
    int noteDuration = 500 / pgm_read_word_near(warningMelody + currentNote + 1);
    tone(PIEZO_PIN, pgm_read_word_near(warningMelody + currentNote), noteDuration * 0.9);
    lastMelodyTime = currentTime;
    currentNote = (currentNote + 2) % (sizeof(warningMelody) / sizeof(warningMelody[0]));
  }
}

// Helper functions
bool isButtonPressed(int pin) {
  static unsigned long lastDebounceTime = 0;
  static int lastButtonState = HIGH;
  static int buttonState = HIGH;
  
  int reading = digitalRead(pin);

  if (reading != lastButtonState) {
    lastDebounceTime = millis();
  }

  if ((millis() - lastDebounceTime) > 50) {
    if (reading != buttonState) {
      buttonState = reading;
      if (buttonState == LOW) {
        lastButtonState = reading;
        return true;
      }
    }
  }
  
  lastButtonState = reading;
  return false;
}

long getDistance() {
  long durations[3];
  
  for(int i = 0; i < 3; i++) {
    digitalWrite(HUMAN_TRIG_PIN, LOW);
    delayMicroseconds(2);
    digitalWrite(HUMAN_TRIG_PIN, HIGH);
    delayMicroseconds(10);
    digitalWrite(HUMAN_TRIG_PIN, LOW);
    
    durations[i] = pulseIn(HUMAN_ECHO_PIN, HIGH);
    delay(10);
  }
  
  if(durations[0] > durations[1]) swap(durations[0], durations[1]);
  if(durations[1] > durations[2]) swap(durations[1], durations[2]);
  if(durations[0] > durations[1]) swap(durations[0], durations[1]);
  
  return durations[1] * 0.034 / 2;
}

void swap(long &a, long &b) {
  long temp = a;
  a = b;
  b = temp;
}

// State handling functions
void handleIdleState() {
  if(isButtonPressed(START_BUTTON_PIN)) {
    startWorking();
  }
  lcd.setCursor(0, 0);
  lcd.print("Press to Start   ");
}

void startWorking() {
  currentState = WORKING;
  startTime = millis();
  pausedTime = 0;
  lcd.clear();
  lcd.print("Working: 40s");
  pinMode(PIEZO_PIN, INPUT);
  
  // 确保所有LED先清零,然后全部点亮绿色
  setAllLEDs(COLOR_OFF);
  delay(50); // 短暂延时确保清零命令执行完成
  setAllLEDs(COLOR_GREEN);
  ring.show(); // 确保显示更新
}

void pauseByButton() {
  currentState = PAUSED;
  pausedTime = millis() - startTime;
  lcd.clear();
  lcd.print("Timer Paused");
  lastMelodyTime = 0;
  currentNote = 0;
  isDistanceWarning = false;
  pinMode(PIEZO_PIN, INPUT);
  setAllLEDs(COLOR_OFF);
}

void pauseByDistance() {
  currentState = PAUSED;
  pausedTime = millis() - startTime;
  lcd.clear();
  lcd.print("Keep Focus");
  lcd.setCursor(0, 1);
  lcd.print(40 - (pausedTime / 1000));
  lcd.print("s remaining");
  lastMelodyTime = 0;
  currentNote = 0;
  isDistanceWarning = true;
}

void resumeTimer() {
  currentState = WORKING;
  startTime = millis() - pausedTime;
  lcd.clear();
  lcd.print("Working: 40s");
  pinMode(PIEZO_PIN, INPUT);
  
  // 恢复LED显示
  int remaining = 40 - ((millis() - startTime) / 1000);
  showCountdown(remaining);
}

void checkCupStatus() {
  bool currentCupState = !digitalRead(CUP_BUTTON_PIN);
  
  if(currentCupState != isCupPresent) {
    if(!currentCupState) {
      lcd.clear();
      lcd.print("Break Time");
      lcd.setCursor(0, 1);
      lcd.print("Take water!");
      isPlayingRelaxMusic = true;
      relaxNoteIndex = 0;
    } else {
      currentState = WORKING;
      startTime = millis();
      pausedTime = 0;
      lcd.clear();
      lcd.print("Working: 40s");
      setAllLEDs(COLOR_OFF);
      delay(50);
      setAllLEDs(COLOR_GREEN);
      isPlayingRelaxMusic = false;
      pinMode(PIEZO_PIN, INPUT);
    }
    isCupPresent = currentCupState;
  }
}

void checkStartButton() {
  if(isButtonPressed(START_BUTTON_PIN)) {
    unsigned long currentTime = millis();
    
    // 检查是否是双击
    if(currentTime - lastButtonPress < DOUBLE_CLICK_TIME) {
      clickCount++;
      if(clickCount == 2) {  // 双击完成
        // 重置到IDLE状态
        currentState = IDLE;
        clickCount = 0;
        setAllLEDs(COLOR_OFF);
        lcd.clear();
        lcd.print("Press to Start");
        return;
      }
    } else {
      clickCount = 1;  // 重置点击计数
    }
    lastButtonPress = currentTime;
    
    // 单击的正常处理
    if(currentState == WORKING) {
      pauseByButton();
    } else if(currentState == PAUSED && !isDistanceWarning) {
      resumeTimer();
    }
  }
  
  // 重置超时的点击计数
  if(millis() - lastButtonPress > DOUBLE_CLICK_TIME) {
    clickCount = 0;
  }
}

void checkDistance() {
  static int distanceCount = 0;
  
  if (millis() - lastDistanceCheck >= DISTANCE_CHECK_INTERVAL) {
    long distance = getDistance();
    lastDistanceCheck = millis();
    
    if (distance > 100) {
      distanceCount++;
      if (distanceCount >= 3 && currentState == WORKING) {
        pauseByDistance();
      }
    } else {
      distanceCount = 0;
      if (currentState == PAUSED && isDistanceWarning) {
        resumeTimer();
      }
    }
  }
}

void updateTimer() {
  unsigned long elapsed = millis() - startTime;
  int remaining = 40 - (elapsed / 1000);
  
  if(remaining <= 0) {
    currentState = BREAK;
    lcd.clear();
    lcd.print("Break Time");
    lcd.setCursor(0, 1);
    lcd.print("Take water!");
    pinMode(PIEZO_PIN, INPUT);
    setAllLEDs(COLOR_OFF); // 确保时间结束时LED全部熄灭
    ring.show();
    return;
  }
  
  lcd.setCursor(0, 1);
  lcd.print(remaining);
  lcd.print("s remaining   ");
  showCountdown(remaining);
}

void setup() {
  ring.begin();
  ring.setBrightness(BRIGHTNESS);
  ring.show();
  
  lcd.begin(16, 2);
  Serial.begin(9600);  
  pinMode(START_BUTTON_PIN, INPUT_PULLUP);
  pinMode(CUP_BUTTON_PIN, INPUT_PULLUP);
  pinMode(HUMAN_TRIG_PIN, OUTPUT);
  pinMode(HUMAN_ECHO_PIN, INPUT);
  pinMode(PIEZO_PIN, INPUT);
  
  // Initial Display
  setAllLEDs(COLOR_OFF);
  lcd.clear();
  lcd.print("Press to Start");
}

void loop() {
  switch(currentState) {
    case IDLE:
      handleIdleState();
      setAllLEDs(COLOR_OFF);
      pinMode(PIEZO_PIN, INPUT);  
      break;
      
    case WORKING:
      checkDistance();
      checkStartButton();
      updateTimer();
      pinMode(PIEZO_PIN, INPUT);  
      break;
      
    case PAUSED:
      checkDistance();
      checkStartButton();
      if(isDistanceWarning) {
        playWarningMusic();
        flashRed();
      } else {
        setAllLEDs(COLOR_OFF);
        pinMode(PIEZO_PIN, INPUT);  
      }
      break;
      
    case BREAK:
      checkCupStatus();
      if(isCupPresent) {
        setAllLEDs(COLOR_BLUE);
      } else {
        breathingBlue();
        if(isPlayingRelaxMusic) {
          playRelaxMusic();
        }
      }
      break;
  }
}