#ifndef _SERIALPORT_H_
#define _SERIALPORT_H_

#include <windows.h>

namespace os {


class serialport
{
	HANDLE hfile;

public:
	enum BaudRate
	{
		bps110     = CBR_110,		// 110 bits/sec
		bps300     = CBR_300,		// 300 bits/sec
		bps600     = CBR_600,		// 600 bits/sec
		bps1200    = CBR_1200,		// 1200 bits/sec
		bps2400    = CBR_2400,		// 2400 bits/sec
		bps4800    = CBR_4800,		// 4800 bits/sec
		bps9600    = CBR_9600,		// 9600 bits/sec
		bps14400   = CBR_14400,		// 14400 bits/sec
		bps19200   = CBR_19200,		// 19200 bits/sec (default)
		bps38400   = CBR_38400,		// 38400 bits/sec
		bps56000   = CBR_56000,		// 56000 bits/sec
		bps57600   = CBR_57600,		// 57600 bits/sec
		bps115200  = CBR_115200,	// 115200 bits/sec
		bps128000  = CBR_128000,	// 128000 bits/sec
		bps256000  = CBR_256000,	// 256000 bits/sec
	};

	// Data bits (5-8)
	enum DataBits
	{
		DataBits5       =  5,			// 5 bits per byte
		DataBits6       =  6,			// 6 bits per byte
		DataBits7       =  7,			// 7 bits per byte
		DataBits8       =  8			// 8 bits per byte (default)
	};

	// Parity scheme
	enum Parity
	{
		None    = NOPARITY,		// No parity (default)
		Odd     = ODDPARITY,	// Odd parity
		Even    = EVENPARITY,	// Even parity
		Mark    = MARKPARITY,	// Mark parity
		Space   = SPACEPARITY	// Space parity
	};

	// Stop bits
	enum StopBits
	{
		StopBits1       = ONESTOPBIT,	// 1 stopbit (default)
		StopBits1_5     = ONE5STOPBITS,// 1.5 stopbit
		StopBits2       = TWOSTOPBITS	// 2 stopbits
	};

	// Handshaking
	enum Handshake
	{
		No				=  0,	// No handshaking
		Hardware		=  1,	// Hardware handshaking (RTS/CTS)
		Software		=  2	// Software handshaking (XON/XOFF)
	};

public:
	serialport() : hfile(0)
	{}
	virtual ~serialport()
	{
		close();
	}
	bool isopen() const { return hfile!=0; }
	bool open(const char* port, BaudRate baud, Parity parity,
		DataBits data, StopBits stop, Handshake handshake, int timeout)
	{
		close();

		// Open the device
		hfile = ::CreateFileA(port,
			GENERIC_READ|GENERIC_WRITE,
			0,
			NULL,
			OPEN_EXISTING,
			0,
			NULL);

		if (hfile == INVALID_HANDLE_VALUE)
		{
			hfile = 0;
			return false;
		}


		if (!::SetupComm(hfile,4096,4096))
		{
			// Close the port
			close();
			return false;
		}

		// Obtain the DCB structure for the device
		DCB dcb;
		if (!::GetCommState(hfile,&dcb))
		{
			close();
			return false;
		}

		// Set the new data
		dcb.BaudRate = DWORD(baud);
		dcb.ByteSize = BYTE(data);
		dcb.Parity   = BYTE(parity);
		dcb.StopBits = BYTE(stop);

		// Determine if parity is used
		dcb.fParity  = (parity != None);

		// Set the handshaking flags
		switch (handshake)
		{
		case No:
			dcb.fOutxCtsFlow = false;					// Disable CTS monitoring
			dcb.fOutxDsrFlow = false;					// Disable DSR monitoring
			dcb.fDtrControl = DTR_CONTROL_DISABLE;		// Disable DTR monitoring
			dcb.fOutX = false;							// Disable XON/XOFF for transmission
			dcb.fInX = false;							// Disable XON/XOFF for receiving
			dcb.fRtsControl = RTS_CONTROL_DISABLE;		// Disable RTS (Ready To Send)
			break;

		case Hardware:
			dcb.fOutxCtsFlow = true;					// Enable CTS monitoring
			dcb.fOutxDsrFlow = true;					// Enable DSR monitoring
			dcb.fDtrControl = DTR_CONTROL_HANDSHAKE;	// Enable DTR handshaking
			dcb.fOutX = false;							// Disable XON/XOFF for transmission
			dcb.fInX = false;							// Disable XON/XOFF for receiving
			dcb.fRtsControl = RTS_CONTROL_HANDSHAKE;	// Enable RTS handshaking
			break;

		case Software:
			dcb.fOutxCtsFlow = false;					// Disable CTS (Clear To Send)
			dcb.fOutxDsrFlow = false;					// Disable DSR (Data Set Ready)
			dcb.fDtrControl = DTR_CONTROL_DISABLE;		// Disable DTR (Data Terminal Ready)
			dcb.fOutX = true;							// Enable XON/XOFF for transmission
			dcb.fInX = true;							// Enable XON/XOFF for receiving
			dcb.fRtsControl = RTS_CONTROL_DISABLE;		// Disable RTS (Ready To Send)
			break;
		}

		// Set the new DCB structure
		if (!::SetCommState(hfile,&dcb))
		{
			close();
			return false;
		}

		COMMTIMEOUTS commTimeout;

		if(!GetCommTimeouts(hfile, &commTimeout)) /* Configuring Read & Write Time Outs */
		{
			close();
			return false;
		}
		commTimeout.ReadIntervalTimeout = timeout;
		commTimeout.ReadTotalTimeoutConstant = timeout;
		commTimeout.ReadTotalTimeoutMultiplier = 0;
		commTimeout.WriteTotalTimeoutConstant = timeout;
		commTimeout.WriteTotalTimeoutMultiplier = 0;

		if(!SetCommTimeouts(hfile, &commTimeout))
		{
			close();
			return false;
		}

		return purge();
	}
	void close()
	{
		if (hfile!=0)
		{
			CloseHandle(hfile);
			hfile=0;
		}
	}
	bool purge()
	{
		return PurgeComm(hfile, PURGE_TXCLEAR | PURGE_RXCLEAR);
	}
	bool read(void* buff, DWORD len, DWORD& read)
	{
		return ReadFile(hfile,buff,len,&read,0);
	}
	bool write(const void* buff, DWORD len, DWORD& written)
	{
		return WriteFile(hfile,buff,len,&written,0);
	}
	bool getch(char& ch)
	{
		DWORD rd;
		return read(&ch,1,rd) && rd==1;
	}
	bool putch(char ch)
	{
		DWORD wr;
		return write(&ch,1,wr) && wr==1;
	}

	int bytesAvailable() 
	{
		int ret = 0;
		DWORD Errors;
		COMSTAT Status;
		if (isopen() && ClearCommError(hfile, &Errors, &Status))
			ret = Status.cbInQue;
		return ret;
	}
};

} // os

#endif


