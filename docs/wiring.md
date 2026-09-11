# Wiring

## Pin map

| Arduino | Connects to | Notes |
|---|---|---|
| D2  | Left IR sensor OUT   | `LOW` when over the line |
| D3  | Right IR sensor OUT  | `LOW` when over the line |
| D9  | HC-SR04 TRIG | |
| D10 | HC-SR04 ECHO | |
| D5  | L298N ENA | PWM — left motor speed |
| D6  | L298N IN1 | left direction |
| D7  | L298N IN2 | left direction |
| D11 | L298N ENB | PWM — right motor speed |
| D8  | L298N IN3 | right direction |
| D12 | L298N IN4 | right direction |
| 5V  | IR sensors VCC, HC-SR04 VCC | |
| GND | common ground | **must** be shared with the motor supply |

## Notes

**Power the motors separately.** Running them off the Arduino's 5V rail browns out the board the moment
both motors start under load. Use a separate pack into the L298N's 12V input, and tie the grounds together.

**Sensor height matters.** The IR sensors need to sit close to the floor — roughly 5–10 mm. Too high and
the reflection is too weak to distinguish tape from floor. Too low and they scrape.

**Check sensor polarity before trusting the code.** Some IR modules output `HIGH` over black instead of
`LOW`. If the robot runs away from the line instead of following it, that's why — flip the comparison in
`onLine()`.

**Ultrasonic mounting.** Point it slightly above horizontal. Aimed dead level it picks up the floor as an
obstacle on anything but a perfectly flat surface.
