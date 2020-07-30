#include <avr/sleep.h>
#include <Wire.h>
#include <Tiny4kOLED.h>
#include <SimpleTimer.h>

const int trigPin = 10;
const int echoPin = 9;
const int buttonPin = 8;
const int onTime = 10;
const int batteryPin = A7;
const int thermistorPin = A5;
const int rb = 14780;

SimpleTimer timer;
volatile unsigned secs;
volatile bool flip;

// index is temperature in degree C: for NTC thermistors!
const PROGMEM uint16_t resistances[] = {
	28704, 27417, 26197, 25039, 23940, 22897, 21906, 20964, 20070, 19219,
	18410, 17641, 16909, 16212, 15548, 14916, 14313, 13739, 13192, 12669,
	12171, 11696, 11242, 10809, 10395, 10000, 9622, 9261, 8916, 8585,
	8269, 7967, 7678, 7400, 7135, 6881, 6637, 6403, 6179, 5965,
	5759, 5561, 5372, 5189, 5015, 4847, 4686, 4531, 4382, 4239
};

int temperature(uint16_t r) {
	const int tmax = sizeof(resistances) / sizeof(uint16_t);
	for (int i = 0; i < tmax; i++)
		if (r > pgm_read_word_near(resistances + i))
			return i-1;
	return tmax;
}

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

long distance(int tc) {
	digitalWrite(trigPin, LOW);
	delayMicroseconds(2);

	digitalWrite(trigPin, HIGH);
	delayMicroseconds(10);
	digitalWrite(trigPin, LOW);

	long usec = pulseIn(echoPin, HIGH);

	// https://en.wikipedia.org/wiki/Speed_of_sound#Practical_formula_for_dry_air
	long cair = (331300 + 606 * tc) / 1000;
	return (usec * cair) / 1000;
}

void loop() {
	timer.run();

	long th = analogRead(thermistorPin);
	uint16_t r = uint16_t((rb * 1023L) / th - rb);
	int tc = temperature(r);

	long mm = distance(tc);
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

	oled.setCursor(0, 3);
	oled.print(tc);
	oled.print('C');

	oled.switchFrame();
}
