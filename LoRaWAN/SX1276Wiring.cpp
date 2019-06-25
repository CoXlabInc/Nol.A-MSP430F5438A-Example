#include "SX1276Wiring.hpp"

SX1276Wiring::SX1276Wiring(
  SPI &spi,
  int8_t pinReset,
  int8_t pinCS,
  int8_t pinRxTx,
  int8_t pinDIO0,
  int8_t pinDIO1,
  int8_t pinDIO2,
  int8_t pinDIO3,
  int8_t pinDIO4
)
: SX1276Chip(
  spi,
  pinReset,
  pinCS,
  pinRxTx,
  pinDIO0,
  pinDIO1,
  pinDIO2,
  pinDIO3,
  pinDIO4) {
}

bool SX1276Wiring::usingPaBoost(uint32_t channel) {
  if (channel > 525000000) {
    return true;
  } else {
    return false;
  }
}
