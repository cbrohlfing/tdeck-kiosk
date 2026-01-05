# SMOKE_TEST.md

Fast manual validation checklist to run **before merging any feature branch** into `main`.

**If any required check fails, do not merge.**
If a check is not applicable, explain why in the PR.

Goal: catch regressions in core behaviors (boot, display, input, power) in under ~2 minutes.

---

## Preconditions

- Board: **Heltec WiFi LoRa 32 (V3 / ESP32-S3)**
- PlatformIO environment: `heltec_v3`
- USB connected for flashing and serial monitor
- For battery tests:
  - LiPo connected to the Heltec battery connector
  - USB cable available to test charging behavior

---

## Build + Flash (Required)

From repo root, run in order:

```bash
pio run -t clean
pio run
pio run -t upload
pio device monitor