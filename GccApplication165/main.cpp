#define F_CPU 8000000UL
#define BAUD 9600
#define UBRR_VAL F_CPU/16/BAUD-1

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <avr/sfr_defs.h>

void usart_init(unsigned int speed)
{
	UBRRH=(unsigned char)(speed>>8);
	UBRRL=(unsigned char) speed;
	UCSRA=0x00;
	UCSRB|=(1<<RXEN)|(1<<TXEN);
	UCSRB|=(1<<RXCIE);
	UCSRC=(1<<URSEL)|(1<<UCSZ1)|(1<<UCSZ0);
}

char Data;
bool Base = false;
bool _ISR_BASE = false;
bool Collector = false;
bool _ISR_COLLECTOR = false;
bool Vcc = false;
bool _ISR_Vcc = false;
bool NewState = false;
bool State = false;
bool StateVcc = false;
bool _ISR_proccess_VT = false;
bool _ISR_State = false;
bool _ISR_NEW_State = false;

void CheckBaseState(void)
{
	if (Base)
	{
		PORTB |= (1<<PB2);
		UDR = 'B';
	}
	else
	{
		PORTB &= ~(1<<PB2);
		UDR = 'b';
	}
}
void ChekVccState(void)
{
	if(Vcc)
	{
		PORTA |= (1<<PA0);
		PORTB |= (1<<PB0);
	}
	else
	{
		PORTA &= ~(1<<PA0);
		PORTB &= ~(1<<PB0);
	}
}

void ChekCollector(void)
{
        if (Base & Vcc)
        {
            Collector = true;
            PORTB |= (1<<PB3);
            UDR = 'C';
        } 
        else
        {
            Collector = false;
            PORTB &= ~(1<<PB3);
            UDR = 'c';
        }
}

ISR(USART_RXC_vect)
{
    Data = UDR;
    switch(Data)
    {
        case 'B':
            Base = true;
            CheckBaseState();
            ChekCollector();
            break;
        case 'b':
            Base = false;
            CheckBaseState();
            ChekCollector();
            break;
        case 'C':
            Collector = true;
            _ISR_COLLECTOR = true;
            break;
        case 'c':
            Collector = false;
            _ISR_COLLECTOR = true;
            break;
        case 'V':
            Vcc = true;
            ChekVccState();
            ChekCollector();
            break;
        case 'v':
            Vcc = false;
            ChekVccState();
            ChekCollector();
            break; 
    }   
}

void ChekVccIn(void)
{
        if(bit_is_set(PINC, PINC0))
        {
	        NewState = true;
	        StateVcc = true;
        }
        if(bit_is_clear(PINC, PINC0))
        {
	        NewState = false;
	        StateVcc = false;
        }
        if(NewState != State)
        {
	        if (StateVcc)
	        {
		        UDR = 'V';
		        PORTB |= (1<<PB1);
	        }
	        else
	        {
		        UDR = 'v';
		        PORTB &= ~(1<<PB1);
	        }
	        State = NewState;        
        }
}

int main(void)
{
	DDRA = 0xFF; 
    DDRB = 0xFF;
    DDRC = 0x00;
    
	usart_init(UBRR_VAL);
	sei();
	
	while (1)
	{
        ChekVccIn();
    }
}


