#include "Keyboard.h"
#include <Adafruit_NeoPixel.h>

#define _countof(x) (sizeof(x) / sizeof (x[0]))

#define LEDS_PER_BOOSTER 15
// full white, both boosters, under 500mA
#define LED_BRIGHTNESS 100

// they start at about the 2 o' clock position
struct {
  uint16_t start;
  uint16_t len;
} vectors[] = {
  {0, 4},
  {2, 4},
  {4, 3},
  {6, 4},
  {8, 3},
  {9, 4},
  {11, 4},
  {13, 3},
};

enum vector_indexes {
  RIGHT,
  DOWN_RIGHT,
  DOWN,
  DOWN_LEFT,
  LEFT,
  UP_LEFT,
  UP,
  UP_RIGHT,
};

enum booster {
  BOOST_LEFT = 0,
  BOOST_RIGHT = 1,
};

Adafruit_NeoPixel boost_left(LEDS_PER_BOOSTER, 17, NEO_GRB + NEO_KHZ800);
Adafruit_NeoPixel boost_right(LEDS_PER_BOOSTER, 16, NEO_GRB + NEO_KHZ800);

#define DEBOUNCE 50 // millis

struct {
  uint8_t pin;
  uint8_t key;
  uint8_t debounce;
} key_map[] = {
  // left booster
  {0, 'w'}, // up
  {1, 's'}, // down
  {2, 'a'}, // left
  {3, 'd'}, // right
  {4, ' '}, // click
  // start, use as coin
  {6, KEY_F3},
  // right booster
  {9, KEY_RIGHT_CTRL}, // click
  {10, KEY_RIGHT_ARROW}, // right
  {11, KEY_LEFT_ARROW}, // left
  {14, KEY_DOWN_ARROW}, // down
  {15, KEY_UP_ARROW}, // up
};

enum light_indexes {
  LEFT_UP = 0,
  LEFT_DOWN,
  LEFT_LEFT,
  LEFT_RIGHT,
  LEFT_TAP,
  START,
  RIGHT_TAP,
  RIGHT_RIGHT,
  RIGHT_LEFT,
  RIGHT_DOWN,
  RIGHT_UP
};

#define PRESSED(index) (key_map[index].debounce > 0)

// switch LEDs not done at the moment, maybe come back later

void setup() {
  boost_left.begin();
  boost_right.begin();
  boost_left.setBrightness(LED_BRIGHTNESS);
  boost_right.setBrightness(LED_BRIGHTNESS);

  //boost_left.fill(boost_left.Color(255,255,255));
  //boost_right.fill(boost_left.Color(255,255,255));
  //boost_left.show();
  //boost_right.show();
  
  for(size_t i = 0; i < _countof(key_map); i++) {
    pinMode(key_map[i].pin, INPUT_PULLUP);
  }


  Serial.begin(9600);
  Keyboard.begin();
}

void loop() {
  static unsigned long last_millis;
  static unsigned long last_led;

  unsigned long new_millis = millis();
  unsigned long ticks = new_millis - last_millis;
  last_millis = new_millis;

  if((new_millis - last_led) > 16) {
    last_led = new_millis;
    #define SPEED_DIVISOR 80
    uint32_t left_base = Wheel(new_millis / SPEED_DIVISOR);
    uint32_t right_base = Wheel((new_millis / SPEED_DIVISOR) + 128);
    uint32_t left_invert = left_base ^ 0xFFFFFF;
    uint32_t right_invert = right_base ^ 0xFFFFFF;

    boost_left.fill(left_base);
    boost_right.fill(right_base);

    if(PRESSED(LEFT_TAP)) {
      boost_left.fill(0xFFFFFF);
    }
    if(PRESSED(RIGHT_TAP)) {
      boost_right.fill(0xFFFFFF);
    }

    // this is incredibly shit
    if(PRESSED(LEFT_UP)) {
      if(PRESSED(LEFT_LEFT)) {
        draw_vector(UP_LEFT, BOOST_LEFT, left_invert);
      } else if(PRESSED(LEFT_RIGHT)) {
        draw_vector(UP_RIGHT, BOOST_LEFT, left_invert);
      } else {
        draw_vector(UP, BOOST_LEFT, left_invert);
      }
    } else if(PRESSED(LEFT_DOWN)) {
      if(PRESSED(LEFT_LEFT)) {
        draw_vector(DOWN_LEFT, BOOST_LEFT, left_invert);
      } else if(PRESSED(LEFT_RIGHT)) {
        draw_vector(DOWN_RIGHT, BOOST_LEFT, left_invert);
      } else {
        draw_vector(DOWN, BOOST_LEFT, left_invert);
      }
    } else if(PRESSED(LEFT_RIGHT)) {
      draw_vector(RIGHT, BOOST_LEFT, left_invert);
    } else if(PRESSED(LEFT_LEFT)) {
      draw_vector(LEFT, BOOST_LEFT, left_invert);
    }

    if(PRESSED(RIGHT_UP)) {
      if(PRESSED(RIGHT_LEFT)) {
        draw_vector(UP_LEFT, BOOST_RIGHT, right_invert);
      } else if(PRESSED(RIGHT_RIGHT)) {
        draw_vector(UP_RIGHT, BOOST_RIGHT, right_invert);
      } else {
        draw_vector(UP, BOOST_RIGHT, right_invert);
      }
    } else if(PRESSED(RIGHT_DOWN)) {
      if(PRESSED(RIGHT_LEFT)) {
        draw_vector(DOWN_LEFT, BOOST_RIGHT, right_invert);
      } else if(PRESSED(RIGHT_RIGHT)) {
        draw_vector(DOWN_RIGHT, BOOST_RIGHT, right_invert);
      } else {
        draw_vector(DOWN, BOOST_RIGHT, right_invert);
      }
    } else if(PRESSED(RIGHT_RIGHT)) {
      draw_vector(RIGHT, BOOST_RIGHT, right_invert);
    } else if(PRESSED(RIGHT_LEFT)) {
      draw_vector(LEFT, BOOST_RIGHT, right_invert);
    }

    //draw_vector(LEFT, BOOST_LEFT, left_invert);
    //draw_vector(DOWN, BOOST_RIGHT, right_invert);
    
    boost_left.show();
    boost_right.show();
  }
  
  for(size_t i = 0; i < _countof(key_map); i++) {
    if(!digitalRead(key_map[i].pin)) {
      key_map[i].debounce = DEBOUNCE;
      Keyboard.press(key_map[i].key);
      //Serial.print(i);
      //Serial.print(' ');
    } else {
      if(key_map[i].debounce > 0) {
        if(ticks > key_map[i].debounce) {
          key_map[i].debounce = 0;
        } else {
          key_map[i].debounce -= ticks;
        }
      } else {
        Keyboard.release(key_map[i].key);
      }
    }
  }
  //Serial.println(' ');

//  boost_left.fill(boost_left.Color(255,0,0));
//  boost_right.fill(boost_left.Color(255,0,0));
//  boost_left.show();
//  boost_right.show();
//  delay(500);
//  boost_left.fill(boost_left.Color(0,255,0));
//  boost_right.fill(boost_left.Color(0,255,0));
//  boost_left.show();
//  boost_right.show();
//  delay(500);
//  boost_left.fill(boost_left.Color(0,0,255));
//  boost_right.fill(boost_left.Color(0,0,255));
//  boost_left.show();
//  boost_right.show();
//  delay(500);
}

void draw_vector(enum vector_indexes dir, enum booster side, uint32_t colour) {
  uint16_t start = vectors[dir].start;
  uint16_t len = vectors[dir].len;
  for(uint16_t i = 0; i < len; i++) {
    uint16_t ind = (start + i) % LEDS_PER_BOOSTER;
    if(side == BOOST_LEFT) {
      boost_left.setPixelColor(ind, colour);
    } else {
      boost_right.setPixelColor(ind, colour);
    }
  }
}

// Input a value 0 to 255 to get a color value.
// The colours are a transition r - g - b - back to r.
uint32_t Wheel(byte WheelPos) {
  WheelPos = 255 - WheelPos;
  if(WheelPos < 85) {
    return boost_left.Color(255 - WheelPos * 3, 0, WheelPos * 3);
  }
  if(WheelPos < 170) {
    WheelPos -= 85;
    return boost_left.Color(0, WheelPos * 3, 255 - WheelPos * 3);
  }
  WheelPos -= 170;
  return boost_left.Color(WheelPos * 3, 255 - WheelPos * 3, 0);
}
