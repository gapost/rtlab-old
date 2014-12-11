#ifndef _PACKET_H_
#define _PACKET_H_

#include <QList>
#include <QAtomicInt>
#include <QSharedPointer>
#include "os_utils.h"

template<class T> class Packet;

template<class T> 
class PacketContainer
{
	typedef Packet<T> _P;
	typedef _P* _P_ptr;

	QList<_P_ptr> plist;
	os::critical_section lock;

public:
	PacketContainer()
	{}
	~PacketContainer()
	{
		foreach(_P_ptr p, plist) delete p;
	}
	void push(const _P_ptr& p)
	{
		os::auto_lock L(lock);
		plist.push_back(p);
	}
	_P_ptr pop()
	{
		os::auto_lock L(lock);
		return plist.empty() ? 0 : plist.takeFirst();
	}
};


template<class T>
class Packet
{
	typedef PacketContainer<T> container_t;
	typedef QSharedPointer<container_t> mngr_t;
	T* mem;
	int d, w;
	QAtomicInt ref_count;
	mngr_t mngr;
	int id;

public:
	Packet(T* p, int ad, int aw, mngr_t amngr, int aid);
	~Packet(void);

	int depth() const { return d; }
	int width() const { return w; }

	T* row(int i) { return mem + i*d; }
	const T* row(int i) const { return mem + i*d; }

	void ref();
	void deref();
	int refCount() const { return ref_count; }
	int Id() const { return id; }
};

template<class T>
class PacketReader;

template<class T>
class PacketWriter
{
	Packet<T>* packet;
	int written;
public:
	explicit PacketWriter(Packet<T>* p=0) : packet(p), written(0)
	{
		if (packet) packet->ref();
	}
	PacketWriter(const PacketWriter& other) : packet(other.packet), written(other.written)
	{
		packet->ref();
	}
	~PacketWriter()
	{
		if (packet) packet->deref();
	}

	PacketWriter& operator=(const PacketWriter& other)
	{
		if (packet) packet->deref();
		packet = other.packet;
		written = other.written;
		if (packet) packet->ref();
		return *this;
	}

	bool isNull() const { return packet==0; }
	bool isFull() const { return written == packet->depth(); }
	int width() const { return packet->width(); }
	int stride() const { return packet->depth(); }

	T* getNext() { return packet->row(0) + written++; }
	void putBack() { if (written) written--; }

        //template<class T>
        friend class PacketReader<T>;
};

template<class T>
class PacketReader
{
	Packet<T>* packet;
public:
	explicit PacketReader(Packet<T>* p) : packet(p)
	{
		packet->ref();
	}
	PacketReader(const PacketReader& other) : packet(other.packet)
	{
		packet->ref();
	}
	explicit PacketReader(const PacketWriter<T>& other) : packet(other.packet)
	{
		packet->ref();
	}
	~PacketReader()
	{
		packet->deref();
	}

	int depth() const { return packet->depth(); }
	int width() const { return packet->width(); }

	const T* operator[](int i) const { return packet->row(i); }
};

template<class T>
class PacketManager
{
	typedef Packet<T> packet_t;
	typedef PacketContainer<T> container_t;
	typedef QSharedPointer<container_t> mngr_t;
	T* mem;
	int memsz;
	mngr_t free_packets;

public:
	PacketManager();
	~PacketManager();

	void resize(int depth, int width, int npackets);

	packet_t* getPacket();

};

template<class T>
class PacketListener
{
	typedef PacketReader<T> reader_t;
	QList<reader_t> packets;
	os::critical_section lock;
protected:
	virtual void dataReady_(int) = 0;
public:
	PacketListener()
	{
	}
	virtual ~PacketListener()
	{
	}

	void push(const PacketReader<T>& p)
	{
		int sz;
		{
			os::auto_lock L(lock);
			packets.push_back(p);
			sz = packets.size();
		}
		dataReady_(sz);
	}

	PacketReader<T> pop()
	{
		os::auto_lock L(lock);
		return packets.takeFirst();
	}

	bool isEmpty() const { return packets.isEmpty(); }


};





#endif
