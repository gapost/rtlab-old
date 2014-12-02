#include "ni6602.h"

namespace pci6602
{
	
digital_io::digital_io(i16 adevice, i16 aline) :
device(adevice), 
status(0), 
line(aline), 
dir(Out), 
state(0) 
{
	//setDirection(dir);
	//setState(state);	
}
	
digital_io::~digital_io() 
{
}

i16 digital_io::setDirection(Direction adir)
{
	i16 dircode = adir==Out ? 1 : 0;
	if (status==0) status = DIG_Line_Config(device, 0, line, dircode);
	if (status==0) dir = adir;
	return status;
}

const char* digital_io::getErrorMessage() const
{
	return NIDAQ_Error_Message(status);
}
	
} // namespace pci6602
