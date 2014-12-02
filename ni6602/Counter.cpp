#include "ni6602.h"

namespace pci6602
{
	
u32 toHz(BaseFrequency f)
{
	u32 hz;
	switch (f)
	{
	case F100kHz: hz = 100000; break;
	case F20MHz: hz = 20000000; break;
	case F80MHz: hz = 80000000; break;
	}
	return hz;
}

/**
	Construction/Destruction
**/
counter::counter(i16 adevice, int acounter):
counter_no(acounter),
device(adevice),
status(0),
buffer(0),
bufflen(0)
{
	assert(counter_no>=0 && counter_no<8);
	counterid = CounterID[counter_no];
	//Reset();
}

counter::~counter()
{
	Reset();
	if (buffer) delete buffer;
}
/**
	Setting SRC
**/
i16 counter::SetSource(int i) 
{
	// set SRC to SRC of counter i
	assert(i>=0 && i<8);
	if (status==0) status = GPCTR_Change_Parameter(device, counterid, ND_SOURCE, SourceID[i]);
	return status;
}
i16 counter::SetSourceToTwin(int Gate)
{
	if (status==0)
	{
		if (Gate) // set to GATE of twin counter GATE
			status = GPCTR_Change_Parameter(device, counterid, ND_SOURCE, ND_OTHER_GPCTR_GATE);
		else // set to GATE of twin counter OUT (TC)
			status = GPCTR_Change_Parameter(device, counterid, ND_GATE, ND_OTHER_GPCTR_TC);
	}
	return status;
}
i16 counter::SetSourceFreq(BaseFrequency freq) 
{
	if (status) return status;
	u32 code;
	switch (freq)
	{
	case F100kHz: code = ND_INTERNAL_100_KHZ; break;
	case F20MHz: code = ND_INTERNAL_20_MHZ; break;
	case F80MHz: code = ND_INTERNAL_MAX_TIMEBASE; break;
	}
	return status = GPCTR_Change_Parameter(device, counterid, ND_SOURCE, code); 
}
i16 counter::SetSourcePolarity(int Positive) 
{
	if (status) return status;
	u32 v = Positive ? ND_POSITIVE : ND_NEGATIVE;
	return status = GPCTR_Change_Parameter(device, counterid, ND_SOURCE_POLARITY, v);
}
/**
	Setting GATE
**/
i16 counter::SetGate(int i) 
{ 
	// set GATE to GATE of counter i
	assert(i>=-1 && i<8);
	if (status==0)
	{
	if (i==-1)
		status = GPCTR_Change_Parameter(device, counterid, ND_GATE, ND_NONE);
	else
		status = GPCTR_Change_Parameter(device, counterid, ND_GATE, GateID[i]);
	}
	return status;
}
i16 counter::SetGateToTwin(int Output)
/* Output(true) or Source of twin counter*/
{
	if (status==0)
	{
	if (Output) 
		status = GPCTR_Change_Parameter(device, counterid, ND_GATE, ND_OTHER_GPCTR_OUTPUT);
	else 
		status = GPCTR_Change_Parameter(device, counterid, ND_GATE, ND_OTHER_GPCTR_SOURCE);
	}
	return status;
}
i16 counter::SetGatePolarity(int Positive) 
{ 
	if (status) return status;
	u32 v = Positive ? ND_POSITIVE : ND_NEGATIVE;
	return status = GPCTR_Change_Parameter(device, counterid, ND_GATE_POLARITY, v);
}
/**
	Setting AUX
**/
i16 counter::SetAux(int i) 
{ 
	// set AUX to AUX of counter i
	assert(i>=0 && i<8);
	if (status) return status;
	return status = GPCTR_Change_Parameter(device, counterid, ND_AUX_LINE, AuxID[i]);
}
i16 counter::SetAuxToTwin(int Output)
{
	if (status) return status;
	if (Output) 
		return status = GPCTR_Change_Parameter(device, counterid, ND_AUX_LINE, ND_OTHER_GPCTR_OUTPUT);
	else 
		return status = GPCTR_Change_Parameter(device, counterid, ND_AUX_LINE, ND_OTHER_GPCTR_SOURCE);
}
/**
	Setting OUT
**/
i16 counter::EnableOutput(int enable)
{
	if (status) return status;
	if (enable)
		return status = Select_Signal(device,OutID[counter_no],CounterOutID[counter_no],ND_DONT_CARE);
	else
		return status = Select_Signal(device,OutID[counter_no],ND_NONE,ND_DONT_CARE);
}
i16 counter::SetOutputMode(int toggle)
{
	if (status) return status;
	if (toggle)
		return status = GPCTR_Change_Parameter(device,counterid,ND_OUTPUT_MODE,ND_TOGGLE);
	else
		return status = GPCTR_Change_Parameter(device,counterid,ND_OUTPUT_MODE,ND_PULSE);
}
i16 counter::PreloadCount(u32 count)
{
	if (status==0) status = GPCTR_Change_Parameter(device,counterid,ND_INITIAL_COUNT,count);
	return status;
}
/**
	Programing the counter
**/
i16 counter::ConfEventCount()
{
	if (status) return status;
	return status = GPCTR_Set_Application (device, counterid, ND_SIMPLE_EVENT_CNT);
}
i16 counter::ConfSinglePulse(u32 tlow, u32 thigh)
{
	if (status) return status;

	status = GPCTR_Set_Application (device, counterid, ND_SINGLE_PULSE_GNR);
	if (status) return status;

	status = GPCTR_Change_Parameter(device, counterid, ND_COUNT_1, tlow);
	if (status) return status;

	status = GPCTR_Change_Parameter(device, counterid, ND_COUNT_2, thigh);
	if (status) return status;

	status = Select_Signal(device, OutID[counter_no], CounterOutID[counter_no], ND_LOW_TO_HIGH);

	return status;
}
i16 counter::ConfRetrigPulse(u32 tlow, u32 thigh)
{
	if (status) return status;

	status = GPCTR_Set_Application (device, counterid, ND_RETRIG_PULSE_GNR);
	if (status) return status;

	status = GPCTR_Change_Parameter(device, counterid, ND_COUNT_1, tlow);
	if (status) return status;

	status = GPCTR_Change_Parameter(device, counterid, ND_COUNT_2, thigh);
	if (status) return status;

	status = Select_Signal(device, OutID[counter_no], CounterOutID[counter_no], ND_LOW_TO_HIGH);

	return status;
}
i16 counter::ConfSingleTrigPulse(u32 tlow, u32 thigh)
{
	if (status) return status;

	status = GPCTR_Set_Application (device, counterid, ND_SINGLE_TRIG_PULSE_GNR);
	if (status) return status;

	status = GPCTR_Change_Parameter(device, counterid, ND_COUNT_1, tlow);
	if (status) return status;

	status = GPCTR_Change_Parameter(device, counterid, ND_COUNT_2, thigh);
	if (status) return status;

	status = Select_Signal(device, OutID[counter_no], CounterOutID[counter_no], ND_LOW_TO_HIGH);

	return status;
}
i16 counter::ConfFSK(u32 tlow0, u32 thigh0, u32 tlow1, u32 thigh1, u32 delay)
{
	if (status) return status;

	status = GPCTR_Set_Application (device, counterid, ND_FSK);
	if (status) return status;

	status = GPCTR_Change_Parameter(device, counterid, ND_COUNT_1, tlow0);
	if (status) return status;

	status = GPCTR_Change_Parameter(device, counterid, ND_COUNT_2, thigh0);
	if (status) return status;

	status = GPCTR_Change_Parameter(device, counterid, ND_COUNT_3, tlow1);
	if (status) return status;

	status = GPCTR_Change_Parameter(device, counterid, ND_COUNT_4, thigh1);
	if (status) return status;

	status = GPCTR_Change_Parameter(device, counterid, ND_INITIAL_COUNT, delay);
	if (status) return status;

	status = Select_Signal(device, OutID[counter_no], CounterOutID[counter_no], ND_LOW_TO_HIGH);
	return status;
}
i16 counter::ConfSquareGenerator(u32 tlow, u32 thigh)
{
	if (status) return status;

	status = GPCTR_Set_Application (device, counterid, ND_PULSE_TRAIN_GNR);
	if (status) return status;

	status = GPCTR_Change_Parameter(device, counterid, ND_COUNT_1, tlow);
	if (status) return status;

	status = GPCTR_Change_Parameter(device, counterid, ND_COUNT_2, thigh);
	if (status) return status;

	status = Select_Signal(device, OutID[counter_no], CounterOutID[counter_no], ND_LOW_TO_HIGH);
	return status;
}
i16 counter::ConfDoubleBuffCount(u32 abufflen, bool GateFasterThanSource)
{
	if (status) return status;

	status = GPCTR_Set_Application (device, counterid, ND_BUFFERED_EVENT_CNT);
	if (status) return status;

	status = GPCTR_Change_Parameter(device, counterid, ND_INITIAL_COUNT, 0);
	if (status) return status;

	status = GPCTR_Change_Parameter(device, counterid, ND_BUFFER_MODE, ND_CONTINUOUS);
	if (status) return status;

	if (GateFasterThanSource) {
        status = GPCTR_Change_Parameter(device, counterid, ND_COUNTING_SYNCHRONOUS, ND_YES);
		if (status) return status;
	}

	if (buffer) {
		if (abufflen!=bufflen) {
			if (buffer) delete buffer;
			bufflen = abufflen;
			buffer = new u32[2*bufflen];
		}
	}
	else {
		bufflen = abufflen;
		buffer = new u32[2*bufflen];
	}

    status = GPCTR_Config_Buffer(device, counterid, 0, 2*bufflen, buffer);
	return status;
}
i16 counter::ConfDoubleBuffPeriod(u32 abufflen, bool GateFasterThanSource)
{
	if (status) return status;

	status = GPCTR_Set_Application (device, counterid, ND_BUFFERED_PERIOD_MSR);
	if (status) return status;

	status = GPCTR_Change_Parameter(device, counterid, ND_INITIAL_COUNT, 0);
	if (status) return status;

	status = GPCTR_Change_Parameter(device, counterid, ND_BUFFER_MODE, ND_CONTINUOUS);
	if (status) return status;

	if (GateFasterThanSource) {
        status = GPCTR_Change_Parameter(device, counterid, ND_COUNTING_SYNCHRONOUS, ND_YES);
		if (status) return status;
	}

	if (buffer) {
		if (abufflen!=bufflen) {
			if (buffer) delete buffer;
			bufflen = abufflen;
			buffer = new u32[2*bufflen];
		}
	}
	else {
		bufflen = abufflen;
		buffer = new u32[2*bufflen];
	}

    status = GPCTR_Config_Buffer(device, counterid, 0, 2*bufflen, buffer);
	return status;
}

i16 counter::ConfDoubleBuffEdgeSep(u32 abufflen)
{
	if (status) return status;

	status = GPCTR_Set_Application (device, counterid, ND_BUFFERED_TWO_SIGNAL_EDGE_SEPARATION_MSR);
	if (status) return status;

	status = GPCTR_Change_Parameter(device, counterid, ND_INITIAL_COUNT, 0);
	if (status) return status;

	status = GPCTR_Change_Parameter(device, counterid, ND_BUFFER_MODE, ND_CONTINUOUS);
	if (status) return status;

	if (buffer) {
		if (abufflen!=bufflen) {
			if (buffer) delete buffer;
			bufflen = abufflen;
			buffer = new u32[2*bufflen];
		}
	}
	else {
		bufflen = abufflen;
		buffer = new u32[2*bufflen];
	}

    status = GPCTR_Config_Buffer(device, counterid, 0, 2*bufflen, buffer);
	return status;
}
/**
	Select Trigger mode
**/
i16 counter::SetTriggerMode(TriggerMode mode) 
{
	if (status) return status;
	if (mode == SoftwareTrigger) 
		return status = GPCTR_Change_Parameter(device, counterid, ND_START_TRIGGER, ND_AUTOMATIC);
	else 
		return status = GPCTR_Change_Parameter(device, counterid, ND_START_TRIGGER, ND_ENABLED);
}
/**
	Error reporting
**/
const char* counter::getErrorMessage() const
{
	return NIDAQ_Error_Message(status);
}

} // namespace pci6602


