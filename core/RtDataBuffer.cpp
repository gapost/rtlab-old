#include "RtDataBuffer.h"
#include "RtDataChannel.h"
#include <QScriptEngine>

RtDataBuffer::RtDataBuffer(const QString& name, RtObject* parent) :
	RtJob(name,"Channel data buffering",parent), depth_(0), numPackets_(0)
{
}

RtDataBuffer::~RtDataBuffer(void)
{
}

void RtDataBuffer::run()
{
	if (wrtBuffer.isNull())
	{
		pushError("Memory full");
		wrtBuffer = PacketWriter<double>(packetManager.getPacket());
		if (wrtBuffer.isNull()) return;
	}

	double* p = wrtBuffer.getNext();
	bool ok;
	foreach(channel_t ch, channels) 
	{
		ok = !(ch.isNull()) && ch->dataReady();
		if (!ok) break;
		*p = ch->value();
		p += wrtBuffer.stride();
	}
	if (!ok) wrtBuffer.putBack();

	// on write buffer full 
	if (wrtBuffer.isFull())
	{
		// send current buffer to listeners
		PacketReader<double> readPacket(wrtBuffer);
		foreach(DataListener* l, listeners)
		{
			l->push(readPacket);
		}

		// get new packet
		wrtBuffer = PacketWriter<double>(packetManager.getPacket());
	}

	RtJob::run();

}

bool RtDataBuffer::resize()
{
	// remove empty channels
	QList<channel_t> temp(channels); 
	channels.clear();
	channel_t ch;
	foreach(ch, temp) 
		if (!ch.isNull()) channels.push_back(ch);

	int nchannels = channels.size();
	packetManager.resize(depth_,nchannels,numPackets_);
	wrtBuffer = PacketWriter<double>(packetManager.getPacket());
	return true;
}

bool RtDataBuffer::addChannel(RtDataChannel* ch)
{
	os::auto_lock L(comm_lock);
	channels.push_back(ch);
	if (armed()) resize();
	return true;
}

bool RtDataBuffer::removeChannel(int i)
{
	os::auto_lock L(comm_lock);
	if (i>=0 && i<channels.size())
	{
		channels.removeAt(i);
		if (armed()) resize();
		return true;
	}
	else return false;
}

QString RtDataBuffer::printChannels() const
{
	QString S;
	if (channels.isEmpty()) S = "No channels buffered.";
	else
	{
		int i = 1;
		foreach(channel_t ch, channels)
		{
			if (i>1) S += '\n';
			if (ch.isNull()) S += QString("%1. Invalid channel").arg(i);
			else S += QString("%1. %2\t%3").arg(i).arg(ch->fullName()).arg(ch->signalName());
			i++;
		}
	}
	return S;

}

void RtDataBuffer::add_listener(DataListener* l)
{
	os::auto_lock L(comm_lock);
	listeners.insert(l);
}
void RtDataBuffer::remove_listener(DataListener* l)
{
	os::auto_lock L(comm_lock);
    if (listeners.contains(l))
        listeners.remove(l);
}
void RtDataBuffer::setPacketDepth(unsigned int d)
{
	os::auto_lock L(comm_lock);
	depth_ = d;
	if (armed()) resize();
}
void RtDataBuffer::setMaxPackets(unsigned int d)
{
	os::auto_lock L(comm_lock);
	numPackets_ = d;
	if (armed()) resize();
}


