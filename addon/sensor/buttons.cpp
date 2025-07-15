//
// ky040.cpp
//
// Circle - A C++ bare metal environment for Raspberry Pi
// Copyright (C) 2022  R. Stange <rsta2@o2online.de>
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.
//
#include "buttons.h"
#include <assert.h>

static const unsigned SwitchDebounceDelayMillis	= 50;
static const unsigned SwitchTickDelayMillis = 200;

BUTTONS::TSwitchState BUTTONS::s_NextSwitchState[SwitchStateUnknown][SwitchEventUnknown] =
{
    // {SwitchEventDown, SwitchEventUp, SwitchEventTick}
    {SwitchStateDown,    SwitchStateStart,  SwitchStateStart},  // SwitchStateStart
    {SwitchStateDown,    SwitchStateClick,  SwitchStateHold},   // SwitchStateDown
    {SwitchStateDown2,   SwitchStateClick,  SwitchStateStart},  // SwitchStateClick
    {SwitchStateDown2,   SwitchStateClick2, SwitchStateInvalid},// SwitchStateDown2
    {SwitchStateDown3,   SwitchStateClick2, SwitchStateStart},  // SwitchStateClick2
    {SwitchStateDown3,   SwitchStateClick3, SwitchStateInvalid},// SwitchStateDown3
    {SwitchStateDown4,   SwitchStateClick3, SwitchStateStart},  // SwitchStateClick3
    {SwitchStateDown4,   SwitchStateClick4, SwitchStateInvalid},// SwitchStateDown4
    {SwitchStateDown5,   SwitchStateClick4, SwitchStateStart},  // SwitchStateClick4
    {SwitchStateDown5,   SwitchStateClick5, SwitchStateInvalid},// SwitchStateDown5
    {SwitchStateInvalid, SwitchStateClick5, SwitchStateStart},  // SwitchStateClick5
    {SwitchStateHold,    SwitchStateStart,  SwitchStateHold},   // SwitchStateHold
    {SwitchStateInvalid, SwitchStateStart,  SwitchStateInvalid} // SwitchStateInvalid
};

BUTTONS::TEvent BUTTONS::s_SwitchOutput[SwitchStateUnknown][SwitchEventUnknown] =
{
    // {SwitchEventDown, SwitchEventUp, SwitchEventTick}
    {EventUnknown, EventUnknown, EventUnknown},             // SwitchStateStart
    {EventUnknown, EventUnknown, EventSwitchHold},          // SwitchStateDown
    {EventUnknown, EventUnknown, EventSwitchClick},         // SwitchStateClick
    {EventUnknown, EventUnknown, EventUnknown},             // SwitchStateDown2
    {EventUnknown, EventUnknown, EventSwitchDoubleClick},   // SwitchStateClick2
    {EventUnknown, EventUnknown, EventUnknown},             // SwitchStateDown3
    {EventUnknown, EventUnknown, EventSwitchTripleClick},   // SwitchStateClick3
    {EventUnknown, EventUnknown, EventUnknown},             // SwitchStateDown4
    {EventUnknown, EventUnknown, EventSwitchQuadrupleClick},// SwitchStateClick4
    {EventUnknown, EventUnknown, EventUnknown},             // SwitchStateDown5
    {EventUnknown, EventUnknown, EventSwitchQuintupleClick},// SwitchStateClick5
    {EventUnknown, EventUnknown, EventSwitchHold},          // SwitchStateHold
    {EventUnknown, EventUnknown, EventUnknown}              // SwitchStateInvalid
};

BUTTONS::BUTTONS (unsigned nSWPin, CGPIOManager *pGPIOManager)
:   m_SWPin (nSWPin, GPIOModeInputPullUp, pGPIOManager),
    m_bPollingMode (!pGPIOManager),
    m_bInterruptConnected (FALSE),
    m_pEventHandler (nullptr),
    m_hDebounceTimer (0),
    m_hTickTimer (0),
    m_nLastSWLevel (HIGH),
    m_bDebounceActive (FALSE),
    m_SwitchState (SwitchStateStart),
    m_nSwitchLastTicks (0),
    m_nLastEventTime (0)  // Initialize the new member
{
}

BUTTONS::~BUTTONS (void)
{
	if (m_bInterruptConnected)
	{
		m_pEventHandler = nullptr;

		m_SWPin.DisableInterrupt2 ();
		m_SWPin.DisableInterrupt ();
		m_SWPin.DisconnectInterrupt ();
	}

	if (m_hDebounceTimer)
	{
		CTimer::Get ()->CancelKernelTimer (m_hDebounceTimer);
	}

	if (m_hTickTimer)
	{
		CTimer::Get ()->CancelKernelTimer (m_hTickTimer);
	}
}

boolean BUTTONS::Initialize (void)
{
	if (!m_bPollingMode)
	{
		assert (!m_bInterruptConnected);
		m_bInterruptConnected = TRUE;

		m_SWPin.ConnectInterrupt (SwitchInterruptHandler, this);

		m_SWPin.EnableInterrupt (GPIOInterruptOnFallingEdge);
		m_SWPin.EnableInterrupt2 (GPIOInterruptOnRisingEdge);
	}

	return TRUE;
}

void BUTTONS::RegisterEventHandler (TEventHandler *pHandler, void *pParam)
{
	assert (!m_pEventHandler);
	m_pEventHandler = pHandler;
	assert (m_pEventHandler);
	m_pEventParam = pParam;
}

unsigned BUTTONS::GetHoldQuarterSeconds (void) const
{
	return m_nHoldCounter;
}

void BUTTONS::Update (void)
{
	assert (m_bPollingMode);

//	EncoderInterruptHandler (this);

	// handle switch
	unsigned nTicks = CTimer::GetClockTicks ();
	unsigned nSW = m_SWPin.Read ();

	if (nSW != m_nLastSWLevel)
	{
		m_nLastSWLevel = nSW;

		m_bDebounceActive = TRUE;
		m_nDebounceLastTicks = CTimer::GetClockTicks ();
	}
	else
	{
		if (   m_bDebounceActive
		    && nTicks - m_nDebounceLastTicks >= SwitchDebounceDelayMillis * (CLOCKHZ / 1000))
		{
			m_bDebounceActive = FALSE;
			m_nSwitchLastTicks = nTicks;

			if (m_pEventHandler)
			{
				(*m_pEventHandler) (nSW ? EventSwitchUp : EventSwitchDown,
						    m_pEventParam);
			}

			HandleSwitchEvent (nSW ? SwitchEventUp : SwitchEventDown);
		}

		if (nTicks - m_nSwitchLastTicks >= SwitchTickDelayMillis * (CLOCKHZ / 1000))
		{
			m_nSwitchLastTicks = nTicks;

			HandleSwitchEvent (SwitchEventTick);
		}
	}
}

// generates the higher level switch events
void BUTTONS::HandleSwitchEvent (TSwitchEvent SwitchEvent)
{
	assert (SwitchEvent < SwitchEventUnknown);
	TEvent Event = s_SwitchOutput[m_SwitchState][SwitchEvent];
	TSwitchState NextState = s_NextSwitchState[m_SwitchState][SwitchEvent];

    if (SwitchEvent == SwitchEventDown)
    {
        m_nLastEventTime = CTimer::GetClockTicks();
    }

	if (NextState == SwitchStateHold)
	{
		if (m_SwitchState != SwitchStateHold)
		{
			m_nHoldCounter = 0;
		}

		m_nHoldCounter++;
	}

	m_SwitchState = NextState;

	if (   Event != EventUnknown
	    && (Event != EventSwitchHold || !(m_nHoldCounter < 1 )) // was (m_nHoldCounter ) emit hold event each second
	    && m_pEventHandler)
	{
		(*m_pEventHandler) (Event, m_pEventParam);
	}
}

void BUTTONS::SwitchInterruptHandler (void *pParam)
{
	BUTTONS *pThis = static_cast<BUTTONS *> (pParam);
	assert (pThis != 0);

	if (pThis->m_hDebounceTimer)
	{
		CTimer::Get ()->CancelKernelTimer (pThis->m_hDebounceTimer);
	}

	pThis->m_hDebounceTimer =
		CTimer::Get ()->StartKernelTimer (MSEC2HZ (SwitchDebounceDelayMillis),
						  SwitchDebounceHandler, pThis, 0);
}

void BUTTONS::SwitchDebounceHandler (TKernelTimerHandle hTimer, void *pParam, void *pContext)
{
	BUTTONS *pThis = static_cast<BUTTONS *> (pParam);
	assert (pThis != 0);

	pThis->m_hDebounceTimer = 0;

	if (pThis->m_hTickTimer)
	{
		CTimer::Get ()->CancelKernelTimer (pThis->m_hTickTimer);
	}

	pThis->m_hTickTimer = CTimer::Get ()->StartKernelTimer (MSEC2HZ (SwitchTickDelayMillis),
								SwitchTickHandler, pThis, 0);

	unsigned nSW = pThis->m_SWPin.Read ();

	if (pThis->m_pEventHandler)
	{
		(*pThis->m_pEventHandler) (nSW ? EventSwitchUp : EventSwitchDown,
					   pThis->m_pEventParam);
	}

	pThis->HandleSwitchEvent (nSW ? SwitchEventUp : SwitchEventDown);
}

void BUTTONS::SwitchTickHandler (TKernelTimerHandle hTimer, void *pParam, void *pContext)
{
	BUTTONS *pThis = static_cast<BUTTONS *> (pParam);
	assert (pThis != 0);

	pThis->m_hTickTimer = CTimer::Get ()->StartKernelTimer (MSEC2HZ (SwitchTickDelayMillis),
								SwitchTickHandler, pThis, 0);

	pThis->HandleSwitchEvent (SwitchEventTick);
}
unsigned long BUTTONS::GetLastEventTime (void) const
{
    return m_nLastEventTime;
}
unsigned long BUTTONS::GetInactiveTime (void) const  // Matches GetLastEventTime implementation
{
    return CTimer::GetClockTicks() - m_nLastEventTime;
}