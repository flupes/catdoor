# Design of the CatDoor v2

## Requirements

- System based on the [SureFlap Microchip Cat Door](https://sureflap.com/en-us/pet-doors/microchip-cat-flap)
  - right size
  - seems reliable (tested one for 1 year + reviews)
- The RFID entry (from outside) mechanism is left untouched
  - keep the same IN reliability
- Allow exit only during daylight
  - need very high reliability in locking (OK to lock too often, not OK to not lock once) to avoid having the cat becoming a Mountain Lion snack
  - ideally, timer follow seasonal sunset - surise time
- Notify of entrance / exit pattern to get some peace of mind when away, and be able to re-act quicly if cat did not come in before dark.

## Locking Solutions

### Linear solenoid + proximity sensor

### Servo + feedback

## Notes

There are two ohter products that do already the right in/out control with timer:
- [Cat Mate Elite](https://www.amazon.com/Cat-Mate-Elite-Timer-Control/dp/B000XPSH34). However it only seems to work with RFID chips you put on a colar. And multiple reviews mentioned it is not reliable. Since we cannot afford to have the cat locked out a single time, this product does not seems an option for our usage.

- [SureFlag Microchip Pet Door](https://www.amazon.com/SureFlap-Microchip-Pet-Door-White/dp/B009NH6NR0/). However this product is much larger (Pet vs Cat) than a normal cat door.
  - Pet Door Dimensions:
    - Flap Opening: 6 11⁄16" (W) x 7" (H)
    - Exterior Frame: 10 5⁄16" (W) x 11 1⁄16" (H)
  - Cat Door Dimensions:
    - Flap Opening: 5 5⁄8" x 4 3⁄4"
    - Exterior Frame: 8 1⁄4" (W) x 8 1⁄4" (H)
