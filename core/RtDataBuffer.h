#ifndef _RTDATABUFFER_H_
#define _RTDATABUFFER_H_

#include "RtDataChannel.h"
#include "Packet.h"

#include <QPointer>
#include <QMap>
#include <QSet>

class RtDataChannel;

class RtDataBuffer : public RtJob
{
	Q_OBJECT
	Q_PROPERTY(uint packetDepth READ packetDepth WRITE setPacketDepth)
	Q_PROPERTY(uint maxPackets READ maxPackets WRITE setMaxPackets)

public:
	typedef PacketListener<double> DataListener;

protected:
	QSet<DataListener*> listeners;

public:
	void add_listener(DataListener* l);
	void remove_listener(DataListener* l);

protected:
	PacketManager<double> packetManager;
	PacketWriter<double> wrtBuffer;
	unsigned int depth_;
	unsigned int numPackets_;


	typedef QPointer<RtDataChannel> channel_t;
	typedef QList<channel_t> channel_list;

	channel_list channels;

	virtual void run();

	bool resize();

	virtual bool arm_() 
	{ 
		os::auto_lock L(comm_lock);
		disarm_();
		return armed_ = resize();
	}

public:
	RtDataBuffer(const QString& name, RtObject* parent);
	virtual ~RtDataBuffer(void);

	unsigned int packetDepth() const { return depth_; }
	void setPacketDepth(unsigned int d);

	unsigned int maxPackets() const { return numPackets_; }
	void setMaxPackets(unsigned int d);

public: // slots:
	Q_SLOT bool addChannel(RtDataChannel*);
	Q_SLOT bool removeChannel(int i);
	Q_SLOT QString printChannels() const;
};

#endif


