# SMOKE_TEST.md

Fast manual validation checklist to run **before merging any feature branch** into `main`.

Goal: catch regressions in core behaviors (boot, display, input, power) in under ~2 minutes.

---

## Preconditions

- Board: **Heltec WiFi LoRa 32 (V3 / ESP32-S3)**
- Build env: PlatformIO env `heltec_v3`
- USB connected for serial monitor unless explicitly testing battery-only
- If testing battery features, have:
  - LiPo connected to the Heltec battery connector
  - USB cable available to test charging behavior

---

## Build + Flash

From repo root:

```bash
pio run -t clean
pio run
pio run -t upload