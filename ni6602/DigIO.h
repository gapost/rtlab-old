#ifndef DIGIO_H_
#define DIGIO_H_

namespace pci6602
{
	
class digital_io  
{
public:
	enum Direction { In, Out };
protected:

	i16 device;
	i16 status;
	i16 line;
	Direction dir;
	i16 state;

	digital_io(i16 adevice, i16 aline);
	~digital_io();

	friend class device;

public:

	i16 getPortNo() const { return line; }
	i16 setDirection(Direction adir);
	Direction getDirection() const { return dir; }
	i16 getState(i16& astate) 
	{
		if (status==0)
		{
			if (dir==In) 
				status = DIG_In_Line(device,0,line,&astate);
			else
			{
				astate = state;
			}
		}
		return status;
	}
	i16 setState(i16 astate) 
	{
		//assert(dir==Out);
		if (status==0)
		{
			if (dir==Out) // output
			{
				status = DIG_Out_Line(device,0,line,astate);
				if (status==0) state = astate;
			}
		}
		return status;
	}

	i16 getStatus() const { return status; }
	const char* getErrorMessage() const;

};

} //namespace pci6602

#endif 
