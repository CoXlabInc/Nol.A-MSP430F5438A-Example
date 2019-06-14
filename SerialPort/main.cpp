#include <cox.h>

Timer timerHello;

void setup() {
  Serial.begin(115200);
  Serial.println("*** [TI MSP430F5438A] ***");

  timerHello.onFired([](void *) {
    struct timeval t;
    System.getTimeSinceBoot(&t);

    printf("[%lu.%06lu] Hello World!\n", t.tv_sec, t.tv_usec);

    static float x = 1.1;
    printf("- float x:%f\n", x);
    x += 0.01;
  }, nullptr);
  timerHello.startPeriodic(1000);
}
