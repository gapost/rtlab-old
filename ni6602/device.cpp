#include "ni6602.h"

namespace pci6602
{

device::device(i16 adevice) : 
deviceno(adevice), 
status(0), 
usedCounters(0), 
usedIoPorts(0),
usedDmaChannels(0)
{
	//reset();
	counters.reserve(8);
	ioports.reserve(8);
	dmachannels.reserve(3);
	for(int i=0; i<8; i++)
	{
		counters.push_back(new counter(deviceno,i));
		ioports.push_back(new digital_io(deviceno,i));
	}
	for(int i=0; i<3; i++)
	{
		dmachannels.push_back(new dma_channel(i));
	}
}

device::~device()
{
	for(int i=0; i<8; i++)
	{
		delete counters[i];
		delete ioports[i];
	}
	for(int i=0; i<3; i++)
	{
		delete dmachannels[i];
	}

}

i16 device::selectTrigger(u32 PfiLineID)
{
	if (status==0)
	{
		if (PfiLineID==0)
			status = Select_Signal (deviceno, ND_START_TRIGGER, ND_LOW, ND_DONT_CARE);
		else
			status = Select_Signal (deviceno, ND_START_TRIGGER, PfiLineID, ND_LOW_TO_HIGH);
	}
	return status;
}

i16 device::syncLine(u32 PfiLineID, int enable)
{
	u32 enableid = enable ? ND_SYNCHRONIZATION : ND_NONE;
	if (status==0) status = Line_Change_Attribute (deviceno, PfiLineID, ND_LINE_FILTER, enableid);
	return status;
}

i16 device::filterLine(u32 PfiLineID, LineFilter code)
{
	if (status==0) status = Line_Change_Attribute (deviceno, PfiLineID, ND_LINE_FILTER, FilterID[code]);
	return status;
}


i16 device::setTransferMode(counter* const ctr, dma_channel* const dma) 
{
	static u32 CounterDataXferCodes[] =
	{
		ND_DATA_XFER_MODE_GPCTR0,
		ND_DATA_XFER_MODE_GPCTR1,
		ND_DATA_XFER_MODE_GPCTR2,
		ND_DATA_XFER_MODE_GPCTR3,
		ND_DATA_XFER_MODE_GPCTR4,
		ND_DATA_XFER_MODE_GPCTR5,
		ND_DATA_XFER_MODE_GPCTR6,
		ND_DATA_XFER_MODE_GPCTR7
	};

	if (status==0) 
	{
		u32 modecode = dma != NULL ? ND_UP_TO_1_DMA_CHANNEL : ND_INTERRUPTS;
		status = Set_DAQ_Device_Info (deviceno, 
			CounterDataXferCodes[ctr->getCounterNo()], modecode);
	}
	return status;
}

i16 device::reset()
{
	return status = Init_DA_Brds (deviceno, &deviceid);
}

const char* device::getErrorMessage() const
{
	return NIDAQ_Error_Message(status);
}

} // namespace pci6602



