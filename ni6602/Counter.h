#ifndef COUNTER_H_
#define COUNTER_H_

namespace pci6602
{
	
enum BaseFrequency {F100kHz, F20MHz, F80MHz };

enum TriggerMode { SoftwareTrigger, HardwareTrigger };

u32 toHz(BaseFrequency f);

class device;

class counter 
{
private:
	int counter_no; // counter no = 0..7
	i16 device; // 6602 device no, default = 1
	u32 counterid; 
	i16 status; // status returned by NI driver

	//buffer
	u32* buffer;
	u32 bufflen;

	// ctor - dtor private
	// only a device may create/delete counters
	counter(i16 adevice, int acounter);
	~counter();

	friend class device;

public:
	
	int getCounterNo() const { return counter_no; }

	// Counter Control
	i16 Reset()		
	{ 
		return status = GPCTR_Control(device,counterid,ND_RESET); 
	}
	i16 Prepare()	
	{
		if (status==0) status = GPCTR_Control(device,counterid,ND_PREPARE);
		return status; 
	}
	i16 Arm()		
	{ 
		if (status==0) status = GPCTR_Control(device,counterid,ND_ARM); 
		return status;
	}
	i16 Disarm()	
	{ 
		if (status==0) status = GPCTR_Control(device,counterid,ND_DISARM);
		return status;
	}

	// Select Source
	i16 SetSource(int i); // select the i'th counter source
	i16 SetSourceToTwin(int Gate = 1); // Gate(1) or TC(0) of twin counter 
	i16 SetSourceFreq(BaseFrequency freq);
	i16 SetSourcePolarity(int Positive = 1);

	// Select Gate
	i16 SetGate(int i); // select the i'th counter gate
	i16 SetGateToTwin(int Output = 1); // Output(1) or Source(0) of twin counter
	i16 SetGatePolarity(int Positive = 1);

	// Select Aux line
	i16 SetAux(int i); // select the i'th counter aux line
	i16 SetAuxToTwin(int Output = 1); // Output(1) or Source(0) of twin counter

	// Output
	i16 EnableOutput(int enable);
	i16 SetOutputMode(int toggle);

	i16 PreloadCount(u32 count);

	// Programing the counter
	i16 ConfEventCount();
	i16 ConfSinglePulse(u32 tlow, u32 thigh);
	i16 ConfRetrigPulse(u32 tlow, u32 thigh);
	i16 ConfSingleTrigPulse(u32 tlow, u32 thigh);
	i16 ConfFSK(u32 tlow0, u32 thigh0, u32 tlow1, u32 thigh1, u32 delay);
	i16 ConfSquareGenerator(u32 tlow, u32 thigh);
	i16 ConfDoubleBuffCount(u32 abufflen, bool GateFasterThanSource = false);
	i16 ConfDoubleBuffPeriod(u32 abufflen, bool GateFasterThanSource = false);
	i16 ConfDoubleBuffEdgeSep(u32 abufflen);

	// Select Triggering Mode
	i16 SetTriggerMode(TriggerMode mode);

	// Buffer operation
	i16 ReadAvailPts(u32* readbuff, u32 readbufflen, u32& pts_read)
	{
		if (status!=0) return status;
		u32 avail;
		GetAvailPts(avail);
		if (avail && status==0) {
			avail = min(avail,readbufflen);
			status = GPCTR_Read_Buffer(device, counterid, ND_READ_MARK,
				0, avail, 0., &pts_read, readbuff);
		}
		else pts_read = 0;
		return status;
	}
	i16 ReadPts(u32* readbuff, u32 readbufflen, u32& pts_read)
	{
		if (status==0) status = GPCTR_Read_Buffer(device, counterid, ND_READ_MARK,
				0, readbufflen, 0., &pts_read, readbuff);
		return status;
	}
	u32 GetBufferLen() const { return bufflen; }
	i16 GetReadMark(u32& mark)	
	{ 
		if (status==0) status = GPCTR_Watch(device, counterid, ND_READ_MARK, &mark);
		return status;
	}
	i16 GetWriteMark(u32& mark)	
	{ 
		if (status==0) status = GPCTR_Watch(device, counterid, ND_WRITE_MARK, &mark);
		return status;
	}
	i16 GetAvailPts(u32& count)	
	{ 
		if (status==0) status = GPCTR_Watch(device, counterid, ND_AVAILABLE_POINTS, &count);
		return status;
	}
	i16 SnapShot()				
	{ 
		if (status==0) status = GPCTR_Control(device,counterid,ND_SNAPSHOT);
		return status;
	}

	typedef 
    struct {
        DWORD LowPart; 
        DWORD HighPart; 
	} UINT64; 

	i16 GetReadMarkSnapshot(unsigned __int64& mark)
	{
		if (status) return status;
		UINT64* P = (UINT64*)&mark; 
		status = GPCTR_Watch(device, counterid, ND_READ_MARK_L_SNAPSHOT, &(P->LowPart));
		if (status) return status;
		return status = GPCTR_Watch(device, counterid, ND_READ_MARK_H_SNAPSHOT, &(P->HighPart));
	}
	i16 GetWriteMarkSnapshot(unsigned __int64& mark)
	{
		if (status) return status;
		UINT64* P = (UINT64*)&mark; 
		status = GPCTR_Watch(device, counterid, ND_WRITE_MARK_L_SNAPSHOT, &(P->LowPart));
		if (status) return status;
		return status = GPCTR_Watch(device, counterid, ND_WRITE_MARK_H_SNAPSHOT, &(P->HighPart));
	}
	i16 GetBacklogSnapshot(unsigned __int64& mark)
	{
		if (status) return status;
		UINT64* P = (UINT64*)&mark; 
		status = GPCTR_Watch(device, counterid, ND_BACKLOG_L_SNAPSHOT, &(P->LowPart));
		if (status) return status;
		return status = GPCTR_Watch(device, counterid, ND_BACKLOG_H_SNAPSHOT, &(P->HighPart));
	}

	// Getting Info
	i16 GetCount(u32& count)	
	{ 
		if (status==0) status = GPCTR_Watch(device, counterid, ND_COUNT, &count); 
		return status;
	}
	i16 GetTC(int& tc)	
	{
		if (status==0)
		{
			u32 f; 
			status = GPCTR_Watch(device, counterid, ND_TC_REACHED, &f); 
			tc = f==ND_YES; 
		}
		return status; 
	}
	i16 GetOutput(u32& out) 
	{ 
		if (status==0)
		{
			status = GPCTR_Watch(device, counterid, ND_OUTPUT_STATE, &out);	
			out = out==ND_HIGH;	
		}
		return status; 
	}

	// Error reporting
	i16 getStatus() const { return status; }
	const char* getErrorMessage() const;
};

} // namespace pci6602

#endif 
