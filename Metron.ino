#include <avr/sleep.h>
#include <Wire.h>
#include <Tiny4kOLED.h>
#include <SimpleTimer.h>

const int trigPin = 10;
const int echoPin = 9;
const int buttonPin = 8;
const int onTime = 10;
const int batteryPin = A7;

SimpleTimer timer;
volatile unsigned secs;
volatile bool flip;

ISR(INT0_vect)
{
	if (secs > 5)
		secs = 0;
	else
		flip = !flip;
}

void tick() {
	secs++;
	if (secs == onTime) {
		oled.off();
		set_sleep_mode(SLEEP_MODE_PWR_DOWN);
		sleep_enable();
		interrupts();
		sleep_cpu();
		sleep_disable();
		oled.on();
		secs = 0;
	}
}

void setup() {
	pinMode(buttonPin, INPUT_PULLUP);
	pinMode(trigPin, OUTPUT);
	pinMode(echoPin, INPUT);	

	noInterrupts();
	MCUCR &= ~(bit(ISC01) | bit(ISC00));
	GIMSK |= bit(INT0);
	interrupts();
	
	oled.begin();
	oled.setRotation(0);
	oled.setFont(FONT8X16);

	oled.clear();
	oled.on();
	oled.switchRenderFrame();

	timer.setInterval(1000, tick);
}

long update(long mm) {
	const int buflen = 12;
	static long buf[buflen];
	static long t;
	static int i;

	t += mm - buf[i];
	buf[i++] = mm;
	if (i == buflen) i = 0;
	return t / buflen;
}

void loop() {
	timer.run();
	
	digitalWrite(trigPin, LOW);
	delayMicroseconds(2);

	digitalWrite(trigPin, HIGH);
	delayMicroseconds(10);
	digitalWrite(trigPin, LOW);

	long duration = pulseIn(echoPin, HIGH);
	long mm = (duration * 343) / 1000;
	long av = update(mm);
	
	long a = analogRead(batteryPin);
	long bv = (5000 * a) / 1024;

	oled.setRotation(flip);
	oled.clear();
	oled.setCursor(0, 0);
	oled.setFont(FONT8X16);
	oled.print(av);
	oled.print(F("mm"));

	oled.setCursor(100, 0);
	oled.setFont(FONT6X8);
	oled.print(secs);

	oled.setCursor(80, 3);
	oled.print(bv);
	oled.print(F("mV"));

	oled.switchFrame();
}
