#ifndef _sensor_buttons_h
#define _sensor_buttons_h

#include <circle/gpiomanager.h>
#include <circle/gpiopin.h>
#include <circle/timer.h>
#include <circle/types.h>

class BUTTONS
{
public:
    enum TEvent
    {
        EventSwitchDown,
        EventSwitchUp,
        EventSwitchClick,
        EventSwitchDoubleClick,
        EventSwitchTripleClick,
        EventSwitchQuadrupleClick,  // New event for 4 clicks
        EventSwitchQuintupleClick,  // New event for 5 clicks
        EventSwitchHold,
        EventUnknown
    };
	    enum TSwitchState
    {
        SwitchStateStart,
        SwitchStateDown,
        SwitchStateClick,
        SwitchStateDown2,
        SwitchStateClick2,
        SwitchStateDown3,
        SwitchStateClick3,
        SwitchStateDown4,    // New state for 4th click
        SwitchStateClick4,   // New state for 4th click
        SwitchStateDown5,    // New state for 5th click
        SwitchStateClick5,   // New state for 5th click
        SwitchStateHold,
        SwitchStateInvalid,
        SwitchStateUnknown
    };

    typedef void TEventHandler (TEvent Event, void *pParam);

public:
    BUTTONS (unsigned nSWPin, CGPIOManager *pGPIOManager = 0);
    ~BUTTONS (void);

    boolean Initialize (void);
    void RegisterEventHandler (TEventHandler *pHandler, void *pParam = 0);
    unsigned GetHoldQuarterSeconds (void) const;
    void Update (void);
    
    // New method to get the last event time
    unsigned long GetLastEventTime (void) const;
    unsigned long GetInactiveTime (void) const;  // Matches GetLastEventTime declaration  
	
	TSwitchState GetCurrentState() const { return m_SwitchState; }


private:
    enum TSwitchEvent
    {
        SwitchEventDown,
        SwitchEventUp,
        SwitchEventTick,
        SwitchEventUnknown
    };

private:
    void HandleSwitchEvent (TSwitchEvent SwitchEvent);
    static void SwitchInterruptHandler (void *pParam);
    static void SwitchDebounceHandler (TKernelTimerHandle hTimer, void *pParam, void *pContext);
    static void SwitchTickHandler (TKernelTimerHandle hTimer, void *pParam, void *pContext);

private:
    CGPIOPin m_SWPin;
    boolean m_bPollingMode;
    boolean m_bInterruptConnected;
    TEventHandler *m_pEventHandler;
    void *m_pEventParam;
    TKernelTimerHandle m_hDebounceTimer;
    TKernelTimerHandle m_hTickTimer;
    unsigned m_nLastSWLevel;
    boolean m_bDebounceActive;
    unsigned m_nDebounceLastTicks;
    TSwitchState m_SwitchState;
    unsigned m_nSwitchLastTicks;
    unsigned m_nHoldCounter;
    unsigned long m_nLastEventTime;     // New member to store last event time

    static TSwitchState s_NextSwitchState[SwitchStateUnknown][SwitchEventUnknown];
    static TEvent s_SwitchOutput[SwitchStateUnknown][SwitchEventUnknown];
};

#endif