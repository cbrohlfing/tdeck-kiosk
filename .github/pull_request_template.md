---

### ✅ **Final `.github/pull_request_template.md`**

```md
## Summary
Brief description of what this PR adds or changes.

- Feature / Fix:
- Board(s) affected:
  - [ ] Heltec V3
  - [ ] T-Deck (future)
  - [ ] Board-agnostic

---

## Type of change
(check all that apply)

- [ ] New feature
- [ ] Bug fix
- [ ] Refactor (no functional change)
- [ ] Hardware abstraction / board support
- [ ] Docs / comments only
- [ ] Breaking change

---

## Testing

### Smoke Test
I ran **SMOKE_TEST.md** on real hardware:

- [ ] `pio run -t clean`
- [ ] `pio run -t upload`
- [ ] Device boots successfully
- [ ] OLED displays content (if applicable)
- [ ] Battery voltage behaves correctly (if applicable)
- [ ] Charging detection behaves correctly (if applicable)
- [ ] CLI works
- [ ] Power / sleep behavior works (if applicable)

**Board tested:**
- [ ] Heltec WiFi LoRa 32 V3

**Power modes tested:**
- [ ] USB only
- [ ] Battery only
- [ ] USB + Battery

---

## Behavior Verification
- [ ] No boot loops
- [ ] No crashes or watchdog resets
- [ ] No regression in previously working features

---

## Code Quality
- [ ] Implemented on a feature branch
- [ ] Board-specific code isolated under `hw/`
- [ ] No debug-only logging left behind
- [ ] No commented-out code unless intentional
- [ ] Naming follows project conventions

---

## Docs
- [ ] SMOKE_TEST.md updated (if needed)
- [ ] README or comments updated (if needed)

---

## Merge Plan
- [ ] Squash & merge into `main`
- [ ] Feature branch can be deleted after merge
- [ ] Release tag will be created after merge (if applicable)

---

## Notes / Follow-ups
(Optional)

- Known limitations:
- Future cleanup:
- Related issues: