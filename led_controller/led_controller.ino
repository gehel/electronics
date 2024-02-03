#include <Encoder.h>
#include <WS2812FX.h>

#define LED_COUNT 9

#define LED_PIN 5

#define ENCODER_SWITCH_PIN 2
#define ENCODER_PIN_A 4
#define ENCODER_PIN_B 3

#define BRIGHTNESS_SWITCH_PIN 9
#define HUE_SWITCH_PIN 8
#define SPEED_SWITCH_PIN 7
#define MODE_SWITCH_PIN 6

#define BRIGHTNESS_POT_PIN A0
#define HUE_POT_PIN A1
#define SPEED_POT_PIN A2

#define MODE_CHANGE_PERIOD 5000

// Parameter 1 = number of pixels in strip
// Parameter 2 = Arduino pin number (most are valid)
// Parameter 3 = pixel type flags, add together as needed:
//   NEO_KHZ800  800 KHz bitstream (most NeoPixel products w/WS2812 LEDs)
//   NEO_KHZ400  400 KHz (classic 'v1' (not v2) FLORA pixels, WS2811 drivers)
//   NEO_GRB     Pixels are wired for GRB bitstream (most NeoPixel products)
//   NEO_RGB     Pixels are wired for RGB bitstream (v1 FLORA pixels, not v2)
//   NEO_RGBW    Pixels are wired for RGBW bitstream (NeoPixel RGBW products)
WS2812FX ws2812fx = WS2812FX(LED_COUNT, LED_PIN, NEO_RGB + NEO_KHZ800);

Encoder modeEncoder(ENCODER_PIN_A, ENCODER_PIN_B);

long oldPosition  = -999;

long lastBrightnessChange = 0;
long lastHueChange = 0;
long lastSpeedChange = 0;

long lastModeChange = 0;
long now;

void setup() {
  Serial.begin(57600);

  Serial.println("Initializing LEDs...");
  ws2812fx.init();
  ws2812fx.setBrightness(255);
  ws2812fx.setSpeed(1000);
  ws2812fx.setColor(0x007BFF);
  ws2812fx.setMode(FX_MODE_STATIC);
  ws2812fx.start();
  Serial.println("LEDs initialized.");

  Serial.println("Initializing I/O...");
  pinMode(BRIGHTNESS_POT_PIN, INPUT);
  pinMode(BRIGHTNESS_SWITCH_PIN, INPUT_PULLUP);
  pinMode(HUE_POT_PIN, INPUT);
  pinMode(HUE_SWITCH_PIN, INPUT_PULLUP);
  pinMode(SPEED_POT_PIN, INPUT);
  pinMode(SPEED_SWITCH_PIN, INPUT_PULLUP);
  pinMode(ENCODER_SWITCH_PIN, INPUT_PULLUP);
  pinMode(MODE_SWITCH_PIN, INPUT_PULLUP);
  Serial.println("I/O initialized.");

  Serial.println("Start loop.");
}

void loop() {
  now = millis();

  brightness();
  hue();
  speed();
  updateMode();

  long end = millis() - now;

  ws2812fx.service();
}

void brightness() {
  uint8_t brightness;
  if (digitalRead(BRIGHTNESS_SWITCH_PIN)) {
    long period = map(analogRead(BRIGHTNESS_POT_PIN), 1023, 0, 5000, 0);
    brightness = map(now % period, 0, period, 0, 255);
  } else {
    brightness = map(analogRead(BRIGHTNESS_POT_PIN), 1023, 0, 0, 255);
  }
  if (ws2812fx.getBrightness() != brightness) {
    ws2812fx.setBrightness(brightness);
    ws2812fx.show();
  }
}

void hue() {
  uint16_t hue;
  if (digitalRead(HUE_SWITCH_PIN)) {
    long period = map(analogRead(HUE_POT_PIN), 1023, 0, 0, 5000);
    hue = map(now % period, 0, period, 0, 65535);
  } else {
    hue = map(analogRead(HUE_POT_PIN), 1023, 0, 0, 65535);
  }
  uint32_t color = Adafruit_NeoPixel::ColorHSV(hue);
  if (ws2812fx.getColor() != color) {
    ws2812fx.setColor(Adafruit_NeoPixel::ColorHSV(hue));
    ws2812fx.show();
  }
}

void speed() {
  uint16_t speed;
  if (digitalRead(SPEED_SWITCH_PIN)) {
    long period = map(analogRead(SPEED_POT_PIN), 1023, 0, 0, 10000);
    speed = map(now % period, 0, period, 0, 2000);
  } else {
    speed = map(analogRead(SPEED_POT_PIN), 1023, 0, 2000, 0);
  }
  if (ws2812fx.getSpeed() != speed) {
    ws2812fx.setSpeed(speed);
    ws2812fx.show();
  }
}

void updateMode() {
  if (digitalRead(MODE_SWITCH_PIN)) {
    updateModeManual();
  } else {
    updateModeAuto();
  }
}

void updateModeAuto() {
  long newPosition = modeEncoder.read();
  if (newPosition != oldPosition) {
    if (newPosition < 0) {
      newPosition = 0;
      modeEncoder.write(newPosition);
    }
    if (newPosition > ws2812fx.getModeCount() * 4) {
      newPosition = ws2812fx.getModeCount() * 4;
      modeEncoder.write(newPosition);
    }
    oldPosition = newPosition;
  
    ws2812fx.setMode(newPosition / 4);
  }
}

void updateModeManual() {
  if (now - lastModeChange > MODE_CHANGE_PERIOD) {
    ws2812fx.setMode((ws2812fx.getMode() + 1) % ws2812fx.getModeCount());
    lastModeChange = now;
  }
}
