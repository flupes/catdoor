# Design of the CatDoor v2

## Requirements

- System based on the [SureFlap Microchip Cat Door](https://sureflap.com/en-us/pet-doors/microchip-cat-flap)
  - right size
  - seems reliable (tested one for 1 year + reviews)
- The RFID entry (from outside) mechanism is left untouched
  - keep the same IN reliability
- Allow exit only during daylight
  - need very high reliability in locking (OK to lock too often, not OK to not lock once) to avoid having the cat becoming a Mountain Lion snack
  - ideally, timer follow seasonal sunset - surise times
- Notify of entrance / exit pattern to get some peace of mind when away, and be able to re-act quicly if cat did not come in before dark.

## Locking Solutions

### Servo + feedback

The servo arm would have two position at 90 deg to lock/unlock the door. The servo can be turned off all the time but when switching mode because there is enough friction to leave the arm in place.

Since the servo would not provide a default locked position (loss of power or other issue), a servo with analog feedback would be used: each time the mode it switched we could get confirmation that the actual position has been reached.

#### Pro
- Simple solution and micro servo available (e.g. https://www.adafruit.com/product/1449)


#### Cons
- This is not a totatlly safe proof solution
- If power is loss, we are operating from the LiPo 3.7V, thus probably requiring a voltage booster to power the servo

### Linear solenoid + proximity sensor

A small pull solenoid (default plunger position out) would be retracted when the cat approach the door.

#### Pro
- Safest solution: when eveything fails, the door is locked and the cat cannot go outside

#### Con
- Requires a an extra sensor ang good signal to trigger the opening
- Not easy to source miniature solenoids operating at low voltages
s
## Notes

There are two ohter products that do already the right in/out control with timer:
- [Cat Mate Elite](https://www.amazon.com/Cat-Mate-Elite-Timer-Control/dp/B000XPSH34). However it only seems to work with RFID chips you put on a colar. And multiple reviews mentioned it is not reliable. Since we cannot afford to have the cat locked out a single time, this product does not seems an option for our usage.

- [SureFlap Microchip Pet Door](https://www.amazon.com/SureFlap-Microchip-Pet-Door-White/dp/B009NH6NR0/). However this product is much larger (Pet vs Cat) than a normal cat door.
  - Pet Door Dimensions:
    - Flap Opening: 6 11⁄16" (W) x 7" (H)
    - Exterior Frame: 10 5⁄16" (W) x 11 1⁄16" (H)
  - Cat Door Dimensions:
    - Flap Opening: 5 5⁄8" x 4 3⁄4"
    - Exterior Frame: 8 1⁄4" (W) x 8 1⁄4" (H)
