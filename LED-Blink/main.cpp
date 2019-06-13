#include <cox.h>

Timer timerBlink;

void setup() {
  pinMode(P1_0, OUTPUT);
  timerBlink.onFired([](void *) {
    digitalToggle(P1_0);
  }, nullptr);
  timerBlink.startPeriodic(2000);
}
