### Record serial port
```
screen -L /dev/cu.SLAB-USBtoUART 115200
```
Note: output is `screenlog.0`, will overwrite a previous one...

Terminate session with: `Ctrl-a + \`

### Procedure

The vcnl4010_test program was flashed on the ESP8266 to record data using
screen.

### Files

`misty_entry_and_stay.log` : log with one entry (kind of messed up because
Misty stop right under the sensor after entering since the treat was there)

`misty_2_entries.log` : log with two entries in the same file

`misty_entry_?.log` : short version (5 seconds around the event) of the
previous logs

`misty_only_?.log` : door closed, but Misty under the sensor (playing with
my hand)

`door_signatures.ods` : analysis of the data (signature, timing,
background)

`door_entry_signatures.svg` : graph of the door opening signatures

`cat_only_returns.svg` : sensor with Misty playing under it (door closed)


