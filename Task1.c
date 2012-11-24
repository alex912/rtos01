#define F_CPU 8000000UL

#include <avr/io.h>
#include <avr/interrupt.h>

#define LED_ON	0
#define LED_OFF	1

#define LED_TIME 1000
#define BUTTON_TIME 360

#define MsToFreq(x) ((x / 1000) * (F_CPU / 1024) + ((x % 1000) * F_CPU) / 1024000)

enum state_t 
{
	BUTTON_OFF = 1,
	BUTTON_ON_NOTFIRED = 2,
	BUTTON_ON_FIRED = 4,
	BUTTON_ON_LAST = 8,	
};

static volatile enum state_t state;

int IsButtonPressed()
{
	return (PINB & 0b00010000);
}

ISR(TIMER1_COMPA_vect)
{
	StopTimer();

	state = BUTTON_ON_LAST;
	PORTB = LED_OFF;
}


ISR(TIMER1_COMPB_vect)
{
	if (state & BUTTON_ON_NOTFIRED)
	{
		state = BUTTON_ON_FIRED;
		
		StartTimer();
		TIMSK = (1 << OCIE1A);

		PORTB = LED_ON;
	}
}

void ButtonPressed()
{
	if (state & BUTTON_OFF)
	{
		state = BUTTON_ON_NOTFIRED;

		StartTimer();
		TIMSK = (1 << OCIE1B);		
	} 

}

void ButtonNotPressed()
{
	if (!(state & BUTTON_OFF))
	{
		StopTimer();

		state = BUTTON_OFF;
		PORTB = LED_OFF;
		
		return;
	}
}

void StartTimer()
{
	TCCR1B = (1<<CS12)|(0<<CS11)|(1<<CS10);	// 8
	TCNT1H = 0;
	TCNT1L = 0;
	TIMSK = 0;
	TIFR = 0xFF;
}

void StopTimer()
{
	TCCR1B = 0;	
	TCNT1H = 0;
	TCNT1L = 0;
	TIMSK = 0;
	TIFR = 0xFF;
}

void Initialize()
{
	DDRB = 0b00000001;	// Led - output, button - input

	state = BUTTON_OFF;

	PORTB = LED_OFF;
	TCCR1B = 0;
	
	OCR1AH = MsToFreq(LED_TIME) >> 8;
	OCR1AL = MsToFreq(LED_TIME);

	OCR1BH = MsToFreq(BUTTON_TIME) >> 8;
	OCR1BL = MsToFreq(BUTTON_TIME);

	sei();
}

int main()
{
	Initialize();
	while (1)
	{
		if (IsButtonPressed())
		{
			ButtonPressed();
		}
		else
		{
			ButtonNotPressed();
		}
	}
	return 0;
}

