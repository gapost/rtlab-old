#include <QDebug>
#include <assert.h>
#include "Packet.h"

template<class T>
Packet<T>::Packet(T* p, int ad, int aw, mngr_t amngr, int aid):
mem(p), d(ad), w(aw), mngr(amngr), id(aid)
{
}

template<class T>
Packet<T>::~Packet(void)
{
}

template<class T>
void Packet<T>::ref()
{
	ref_count.ref();
	//qDebug() << "Ref packet id=" << id << ", count=" << refCount();
}

template<class T>
void Packet<T>::deref()
{
	bool ret = ref_count.deref();
	if (!ret)
	{
		mngr->push(this);
	}
	//qDebug() << "Deref packet id=" << id << ", count=" << refCount();
}

template<class T>
PacketManager<T>::PacketManager() : mem(0), memsz(0)
{
}
	
template<class T>
PacketManager<T>::~PacketManager()
{
	free_packets.clear();
	if (mem) delete [] mem;
}

template<class T>
void PacketManager<T>::resize(int depth, int width, int npackets)
{
	// delete packets
	free_packets.clear();

	// allocate memory (if needed)
	int packet_sz = depth*width;
	int newsz = packet_sz*npackets;
	if (newsz>memsz)
	{
		if (mem) delete [] mem;
		memsz=0;
		mem = new T[newsz];
		memsz = newsz;
	}

	free_packets = mngr_t(new container_t()); 
	T* p = mem;
	for(int i = 0; i<npackets; ++i)
	{
		packet_t* pckt = new packet_t(p,depth,width,free_packets,i);
		free_packets->push(pckt);
		p += packet_sz;
	}
}

template<class T>
Packet<T>* PacketManager<T>::getPacket()
{
	return free_packets ? free_packets->pop() : 0;
}

template class Packet<double>;
template class PacketReader<double>;
template class PacketWriter<double>;
template class PacketManager<double>;


