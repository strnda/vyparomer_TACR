
#define NR_OF_LIGHTS 6
int pins[NR_OF_LIGHTS] = { 3, 5, 6, 9, 10, 11 };

int values[NR_OF_LIGHTS];
int steps[NR_OF_LIGHTS];

#define NR_OF_FADESTEPS 4
int fadesteps[NR_OF_FADESTEPS] = { 192, 128, 64, 0 };
int fade_delay = 20;
int fade_cycles = 1000;

int nr_of_blinks = 3;
int blink_delay = 300;

int effect = 1;

int randomlights[NR_OF_LIGHTS];
bool chosenlights[NR_OF_LIGHTS];

void setup() {
  randomSeed(analogRead(0));
  for (int i = 0; i < NR_OF_LIGHTS; i++) {
    values[i] = (int)random(230) + 13;
    steps[i] = (int)random(4) + 1;
  }
}

void loop() {

  for (int j = 0; j < fade_cycles; j++) {
    for (int i = 0; i < NR_OF_LIGHTS; i++) {
      fadingLight(i);
    }
    delay(fade_delay);
  }

  if (effect == 1) {
    blinkAll();
    effect = 2;
  }
  else if (effect == 2) {
    runningLight(0);
    effect = 3;
  }
  else if (effect == 3) {
    runningLight(255);
    effect = 1;
  }
}

void fadingLight(int i) {

  int minvalue = (NR_OF_FADESTEPS * abs(steps[i])) + 1;
  int maxvalue = 255 - minvalue;

  int fs = NR_OF_FADESTEPS;
  for (int j = 0; fs > 0; fs--, j++) {
    if (values[i] > fadesteps[j]) {
      break;
    }
  }
  values[i] += fs * steps[i];

  if (values[i] > maxvalue  ||  values[i] < minvalue) {
    steps[i] *= -1;
  }

  analogWrite(pins[i], values[i]);
}

void setAllLights(int value) {
  for (int i = 0; i < NR_OF_LIGHTS; i++) {
    analogWrite(pins[i], value);
  }
}

void blinkAll() {
  for (int i = 0; i < nr_of_blinks; i++) {
    setAllLights(255);
    delay(blink_delay);
    setAllLights(0);
    delay(blink_delay);
  }
}

void runningLight(int startvalue) {
  setAllLights(startvalue);
  for (int j = 0; j < 2; j++) {
    randomize();
    for (int i = 0; i < NR_OF_LIGHTS; i++) {
      analogWrite(pins[randomlights[i]], 255 - startvalue);
      delay(200);
      analogWrite(pins[randomlights[i]], startvalue);
    }
  }
}

void randomize() {
  for (int i = 0; i < NR_OF_LIGHTS; i++) {
    chosenlights[i] = false;
  }
  int r = (int)random(NR_OF_LIGHTS);
  randomlights[0] = r;
  chosenlights[r] = true;
  for (int i = 1; i < 5; i++) {
    while (true) {
      r = (int)random(NR_OF_LIGHTS);
      if (chosenlights[r] == false) {
        break;
      }
    }
    randomlights[i] = r;
    chosenlights[r] = true;
  }
  for (int i = 0; i < NR_OF_LIGHTS; i++) {
    if (chosenlights[i] == false) {
      randomlights[5] = i;
      break;
    }
  }
}
