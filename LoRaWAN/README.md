# LoRaWAN using TI-MSP430F5438A + SX1276

## Pin Map

|    MSP430    | SX1276 |   Console  |
|--------------|--------|------------|
| P3.4 (J2-39) | MOSI   |            |
| P3.5 (J2-40) | MISO   |            |
| P3.0 (J2-33) | SCK    |            |
| P3.1 (J2-34) | NSS    |            |
| P1.1 (J1-18) | DIO0   |            |
| P1.2 (J1-19) | DIO1   |            |
| P1.3 (J1-20) | DIO2   |            |
| P1.4 (J1-21) | DIO3   |            |
| P1.5 (J1-22) | DIO4   |            |
| P1.6 (J1-23) | DIO5   |            |
| P7.1 (J1-14) | RxTx   |            |
| P7.0 (J1-13) | NRESET |            |
| P9.4 (J3-72) |        | Serial3 Tx |
| P9.5 (J3-73) |        | Serial3 Rx |

* Wiring DIO5 is not required.
* RxTx may be optional for some SX1276 boards such as RFM95W that controls internally.
