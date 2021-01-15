#include <cox.h>
#include <algorithm>
#include <LoRaMacKR920.hpp>
#include "SX1276Wiring.hpp"

#define INTERVAL_SEND 5000
#define OVER_THE_AIR_ACTIVATION 1
#define BLINK_TX 0

#if (OVER_THE_AIR_ACTIVATION == 1)
static const uint8_t devEui[] = "\x00\x11\x22\x33\x44\x55\x66\x77";
static const uint8_t appEui[] = "\x00\x00\x00\x00\x00\x00\x00\x00";
static const uint8_t appKey[] = "\xaa\xe1\x20\xe3\x36\x03\xcd\x09\xd9\xda\x4a\x61\x84\xc6\xf5\x19";
#else
static const uint8_t NwkSKey[] = "\xa4\x88\x55\xad\xe9\xf8\xf4\x6f\xa0\x94\xb1\x98\x36\xc3\xc0\x86";
static const uint8_t AppSKey[] = "\x7a\x56\x2a\x75\xd7\xa3\xbd\x89\xa3\xde\x53\xe1\xcf\x7f\x1c\xc7";
static uint32_t DevAddr = 0x06e632e8;
#endif //OVER_THE_AIR_ACTIVATION

SX1276Wiring SX1276(
  SpiA0,
  P8_0,  //Reset
  P3_1,  //CS
  -1,  //RxTx
  P1_1,  //DIO0
  P1_2,  //DIO1
  P1_3,  //DIO2
  P1_4,  //DIO3
  P1_5   //DIO4
);

LoRaMacKR920 LoRaWAN(SX1276, 10);
Timer timerSend;

static void taskPeriodicSend(void *) {
  error_t err = LoRaWAN.requestLinkCheck();
  printf("* Request LinkCheck: %d\n", err);

  err = LoRaWAN.requestDeviceTime();
  printf("* Request DeviceTime: %d\n", err);

  printf(
    "* Max payload length for DR%u: %u - %u = %u\n",
    LoRaWAN.getCurrentDatarateIndex(),
    LoRaWAN.getMaxPayload(LoRaWAN.getCurrentDatarateIndex()),
    LoRaWAN.getPendingMacCommandsLength(),
    LoRaWAN.getMaxPayload(LoRaWAN.getCurrentDatarateIndex()) - LoRaWAN.getPendingMacCommandsLength()
  );

  LoRaMacFrame *f = new LoRaMacFrame(255);
  if (!f) {
    printf("* Out of memory\n");
    return;
  }

  f->port = 1;
  f->type = LoRaMacFrame::CONFIRMED;
  f->len = sprintf((char *) f->buf, "Test");

  /* Uncomment below line to specify frequency. */
  // f->freq = 922500000;

  /* Uncomment below lines to specify parameters manually. */
  // LoRaWAN.useADR = false;
  // f->modulation = Radio::MOD_LORA;
  // f->meta.LoRa.bw = Radio::BW_125kHz;
  // f->meta.LoRa.sf = Radio::SF12;
  // f->power = 1; /* Index 1 => MaxEIRP - 2 dBm */

  /* Uncomment below line to specify number of trials. */
  // f->numTrials = 1;

  err = LoRaWAN.send(f);
  printf("* Sending periodic report (%s (%u byte)): %d\n", f->buf, f->len, err);
  if (err != ERROR_SUCCESS) {
    delete f;
    timerSend.startOneShot(INTERVAL_SEND);
    return;
  }

#if (BLINK_TX == 1)
  digitalWrite(P1_0, HIGH);
#endif
}

void setup() {
  Serial3.begin(115200);
  System.out = &Serial3;
  printf("*** [TI MSP430F5438A] LoRaWAN using SX1276 ***\n");
  printf("- Last reset reason:0x%02X\n", System.getResetReason());

#if (BLINK_TX == 1)
  pinMode(P1_0, OUTPUT);
  digitalWrite(P1_0, LOW);
#endif

  timerSend.onFired(taskPeriodicSend, nullptr);

  LoRaWAN.begin([]() -> uint8_t {
    return map(System.getSupplyVoltage(), 2900, 3300, 1, 254); // measured battery level
  });

  LoRaWAN.onSendDone([](LoRaMac &lw, LoRaMacFrame *frame) {
#if (BLINK_TX == 1)
    digitalWrite(P1_0, LOW);
#endif
    printf("* Send done(%d): ", frame->result);
    frame->printTo(Serial3);
    printf("\n");

    delete frame;

    timerSend.startOneShot(INTERVAL_SEND);
  });

  LoRaWAN.onReceive([](LoRaMac &lw, const LoRaMacFrame *frame) {
    static uint32_t fCntDownPrev = 0;

    printf("* Received a frame:");
    for (uint8_t i = 0; i < frame->len; i++) {
      printf(" %02X", frame->buf[i]);
    }
    printf(" (");
    frame->printTo(Serial3);
    printf(")\n");

    if (
      (frame->type == LoRaMacFrame::CONFIRMED || lw.framePending) &&
      lw.getNumPendingSendFrames() == 0
    ) {
      // If there is no pending send frames, send an empty frame to ack or pull more frames.
      LoRaMacFrame *emptyFrame = new LoRaMacFrame(0);
      if (emptyFrame) {
        error_t err = LoRaWAN.send(emptyFrame);
        if (err != ERROR_SUCCESS) {
          delete emptyFrame;
        }
      }
    }
  });
  LoRaWAN.setPublicNetwork(false);

#if (OVER_THE_AIR_ACTIVATION == 1)
  LoRaWAN.onJoin([](
    LoRaMac &lw,
    bool joined,
    const uint8_t *joinedDevEui,
    const uint8_t *joinedAppEui,
    const uint8_t *joinedAppKey,
    const uint8_t *joinedNwkSKey,
    const uint8_t *joinedAppSKey,
    uint32_t joinedDevAddr,
    const RadioPacket &,
    uint32_t airTime
  ) {
    printf("* Tx time of JoinRequest: %lu usec.\n", airTime);

    if (joined) {
      printf("* Joining done!\n");
      postTask(taskPeriodicSend, nullptr);
    } else {
      printf("* Joining failed. Retry to join.\n");
      lw.beginJoining(devEui, appEui, appKey);
    }
  });
  LoRaWAN.beginJoining(devEui, appEui, appKey);
#else
  LoRaWAN.setABP(NwkSKey, AppSKey, DevAddr);
  LoRaWAN.setNetworkJoined(true);
  LoRaWAN.setCurrentDatarateIndex(5);
  postTask(taskPeriodicSend, nullptr);
#endif

#if (BLINK_TX == 1)
  digitalWrite(P1_0, LOW);
#endif
}
