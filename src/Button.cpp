#include "Button.h"

/*----------------------------------------------------------------------*
/ initialize a Button object and the pin it's connected to.             *
/-----------------------------------------------------------------------*/
void Button::begin()
{
    pinMode(m_pin, INPUT_PULLUP);
    m_state = (digitalRead(m_pin) == LOW); //pressed == 0
    m_time = millis();
    m_lastState = m_state;
    m_changed = false;
}

/*----------------------------------------------------------------------*
/ returns the state of the button, true if pressed, false if released.  *
/ does debouncing, captures and maintains times, previous state, etc.   *
/-----------------------------------------------------------------------*/
void Button::read()
{
    uint32_t ms = millis();
    bool pinVal = (digitalRead(m_pin) == LOW);

    if (ms - m_lastChange < 50)
    {
        m_changed = false;
    }
    else
    {
        m_lastState = m_state;
        m_state = pinVal;
        m_changed = (m_state != m_lastState);
        if (m_changed) 
	{
	   m_lastChange = ms;
	}
    }
    m_time = ms;
}

/*----------------------------------------------------------------------*
 * isPressed() and isReleased() check the button state when it was last *
 * read, and return false (0) or true (!=0) accordingly.                *
 * These functions do not cause the button to be read.                  *
 *----------------------------------------------------------------------*/
bool Button::isPressed()
{
    return m_state;
}

bool Button::isReleased()
{
    return !m_state;
}

bool Button::wasPressed()
{
    return m_state && m_changed;
}

bool Button::wasReleased()
{
    return !m_state && m_changed;
}

/*----------------------------------------------------------------------*
 * pressedFor(ms) and releasedFor(ms) check to see if the button is     *
 * pressed (or released), and has been in that state for the specified  *
 * time in milliseconds. Returns false (0) or true (!=0) accordingly.   *
 * These functions do not cause the button to be read.                  *
 *----------------------------------------------------------------------*/
bool Button::pressedFor(uint32_t ms)
{
    return m_state && m_time - m_lastChange >= ms;
}

bool Button::releasedFor(uint32_t ms)
{
    return !m_state && m_time - m_lastChange >= ms;
}

bool Button::changed()
{
    return m_changed;
}
