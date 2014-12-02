#ifndef _DEVICE_H_ 
#define _DEVICE_H_

#include <vector>
#include <cassert>

namespace pci6602
{
	
const char* NIDAQ_Error_Message(i16 code);

extern u32 PfiID[];
extern u32 CounterID[];
extern u32 GateID[];
extern u32 SourceID[];
extern u32 AuxID[];
extern u32 OutID[];
extern u32 CounterOutID[];
extern u32 FilterID[];

enum LineFilter {lf100kHz = 0, lf500kHz = 1, lf1MHz = 2, lf5MHz = 3, lfConfigurable = 4, lfNone = 5};

class dma_channel
{
	int channel_;
	dma_channel(int n) : channel_(n) {}
	~dma_channel() {}
	friend class device;
public:
	int getChannelNo() const { return channel_; }
};

class device  
{
private: 
	i16 deviceno; // 6602 device no, default = 1
	i16 deviceid; // deviceNumberCode
	i16 status;

	std::vector<counter*> counters;
	std::vector<digital_io*> ioports;
	std::vector<dma_channel*> dmachannels;

	unsigned char usedCounters;
	unsigned char usedIoPorts;
	unsigned char usedDmaChannels;

	void clearBit(unsigned char& flags, int nbit, int nmax = 8)
	{
		assert(nbit>=0 && nbit<nmax);
		unsigned char mask = 1 << nbit;
		assert(flags & mask);
		if (mask & flags) flags ^= mask;
	}
	bool checkedSetBit(unsigned char& flags, int nbit, int nmax = 8)
	{
		assert(nbit>=0 && nbit<nmax);
		unsigned char mask = 1 << nbit;
		if (mask & flags) return false;
		else {
			flags |= mask;
			return true;
		}
	}

public:
	device(i16 adeviceno);
	~device();

	i16 reset();
	i16 syncLine(u32 PfiLineID, int enable);
	i16 filterLine(u32 PfiLineID, LineFilter code);
	i16 selectTrigger(u32 PfiLineID);
	i16 setTransferMode(counter* const ctr, dma_channel* const dma);

	i16 getStatus() const { return status; }
	const char* getErrorMessage() const;
	i16 getDeviceId() const { return deviceid; }
	i16 getDeviceNo() const { return deviceno; } 

	counter* const getCounter(int counterno)
	{
		return checkedSetBit(usedCounters,counterno) ? counters[counterno] : NULL;
	}
	void releaseCounter(counter* const ctr)
	{
		clearBit(usedCounters,ctr->getCounterNo());
	}

	digital_io* const getIoPort(int portno)
	{
		return checkedSetBit(usedIoPorts,portno) ? ioports[portno] : NULL;
	}
	void releaseIoPort(digital_io* const port)
	{
		clearBit(usedIoPorts,port->getPortNo());
	}

	dma_channel* const getDMA()
	{
		unsigned char mask(1);
		int i = 0;
		while(mask<8)
		{
			if ( ! (mask & usedDmaChannels) )
			{
				usedDmaChannels |= mask;
				return dmachannels[i];
			}
			i++;
			mask <<= 1;
		}
		return NULL;
	}

	void releaseDMA(dma_channel* const dma)
	{
		clearBit(usedDmaChannels,dma->getChannelNo(),3);
	}
};

} // namespace pci6602

#endif // !defined(AFX_6602_H__C87A23AE_A319_46F5_856F_68366F736D06__INCLUDED_)
