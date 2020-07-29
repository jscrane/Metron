# Metron

Digital "measuring tape" with OLED display and HCSR04 ultrasonic 
distance sensor.

## Hardware

- ATtiny84
- Push-button
- 10k resistor
- 22k resistor
- 2x 100nF ceramic capacitor
- 128x32 I2C OLED display
- HCSR-04 ultrasonic distance sensor module
- 10k thermistor (B = 3435, see below)

## Software

- Arduino 1.8.9 (or later) with [ATTinyCore](https://github.com/SpenceKonde/ATTinyCore)
- [Tiny4kOLED](https://github.com/datacute/Tiny4kOLED) library
- [SimpleTimer](https://github.com/schinken/SimpleTimer) library
- [uC-Makefile](https://github.com/jscrane/uC-Makefile) (optional)

### Thermistor

Assuming operating temperature is in the range [0, 50) degrees C, we generate an array of
resistances as follows:

```
(defn steinhart-hart [R0 B]
  (let [T0 298.15
        rinf (/ R0 (Math/exp (/ B T0)))]
    (fn [T] (Math/round (* rinf (Math/exp (/ B (+ T 273.15)))))))

(map (steinhart-hart 10000 3435) (range 0 50))
```

This is for a part with R0=10k and B=3435, see the description of the 
[Steinhart-Hart equation](https://en.wikipedia.org/wiki/Thermistor#Steinhart%E2%80%93Hart_equation).

(Conveniently all of the values in this range fit in 16-bits so the entire array takes up
only 100 bytes.)
