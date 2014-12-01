#ifndef _IVECTOR_H_
#define _IVECTOR_H_

#include <memory.h>
#include <assert.h>

#define DEF_CHUNK_SIZE 1024

template<class T>
class ibuffer
{
	T* mem;
	unsigned int sz;
	unsigned int chunk_sz;
	int* ref_count_;

	void destroy()
	{
		(*ref_count_)--;
		if (*ref_count_==0)
		{
			if (mem) delete [] mem;
			delete ref_count_;
		}
		ref_count_ = 0;
		mem = 0;
		sz = 0;
	}
	T* alloc(unsigned int &n)
	{
		if (n==0) return 0;
		else
		{
			unsigned int m = (n/chunk_sz)*chunk_sz;
			n = m<n ? m+chunk_sz : m;
			return new T[n];
		}
	}
public:
	ibuffer(unsigned int asz = 0, unsigned int acsz = DEF_CHUNK_SIZE/sizeof(T)) : mem(0), sz(asz), chunk_sz(acsz)
	{
		mem = alloc(sz);
		ref_count_ =  new int;
		*ref_count_ = 1;
	}
	ibuffer(const ibuffer& rhs) : mem(rhs.mem), sz(rhs.sz), chunk_sz(rhs.chunk_sz), ref_count_(rhs.ref_count_)
	{
		(*ref_count_)++;
	}
	~ibuffer()
	{ 
		destroy();
	}

	ibuffer& operator=(const ibuffer& rhs)
	{
		destroy();
		mem = rhs.mem;
		sz = rhs.sz;
		ref_count_ = rhs.ref_count_;
		(*ref_count_)++;
		return *this;
	}

	unsigned int size() const { return sz; }
	void setSize(unsigned int nsz)
	{
		if (
			( nsz<sz && (sz-nsz>chunk_sz) ) ||
			( nsz>sz )
			)
		{
			T* temp = alloc(nsz);
			memcpy(temp, mem, (nsz>sz ? sz : nsz)*sizeof(T));
			destroy();
			mem = temp;
			sz = nsz;
			ref_count_ =  new int;
			*ref_count_ = 1;
		}
	}
	unsigned int chunkSize() const { return chunk_sz; }
	void setChunkSize(unsigned int nsz) { chunk_sz = nsz; } 

	T* begin() { return mem; }
	const T* begin() const { return mem; }
};

template<class T>
class ivector
{
public:
	enum iVectorType { Open, Fixed, Circular };
private:
	/// buffer
	ibuffer<T> buff;
	/// vector capacity
	unsigned int cp;
	/// vector size
	unsigned int sz;
	/// vector type
	iVectorType type_;
	/// pointer to next position for circular vectors
	unsigned int tail;

	void set_(unsigned int i, const T& v)
	{
		*(buff.begin()+i) = v;
	}

	void normalize_()
	{
		if (type_!=Circular || sz==0) return;

		if (sz==cp && tail>0)
		{
			T* temp = new T[sz];
			const T* head = buff.begin() + tail;
			unsigned int sz1 = sz-tail;
			memcpy(temp, head, sz1*sizeof(T));
			memcpy(temp+sz1, buff.begin(), (sz-sz1)*sizeof(T));
			memcpy(buff.begin(), temp, sz*sizeof(T));
			delete temp;
			tail = 0;
		}
	}
public:
	ivector(unsigned int acap = 0) : buff(acap), sz(0), cp(acap), type_(Fixed), tail(0)
	{
	}
	ivector(const ivector& rhs) : buff(rhs.buff), sz(rhs.sz), cp(rhs.cp), type_(rhs.type_), tail(rhs.tail)
	{
	}
	~ivector(void)
	{
	}

	ivector& operator=(const ivector& rhs)
	{
		buff = rhs.buff;
		sz = rhs.sz;
		cp = rhs.cp;
		type_ = rhs.type_;
		tail = rhs.tail;
	}

	void setCapacity(unsigned int c)
	{
		if (c==cp) return;
		normalize_();
		buff.setSize(c);
		switch (type_)
		{
		case Fixed:
		case Open:
			if (sz>c) sz = c;
			break;
		case Circular:
			if (c>cp)
			{
				if (sz==cp) tail = sz;
			}
			else // (c<cp)
			{
				if (sz>c)
				{
					sz = c; tail = 0;
				}
			}
			break;
		}
		cp = c;
	}
	void clear()
	{
		sz = 0;
		tail = 0;
	}
	void setType(iVectorType newt)
	{
		if (newt==type_) return;
		normalize_();
		type_ = newt;
	}


	T get(unsigned int i) const
	{
		assert(i<sz);
		if (type_==Circular && sz==cap()) i = (tail+i) % sz;
		return *(buff.begin() + i);
	}
	T operator[](unsigned int i) const
	{
		return get(i);
	}

	unsigned int cap() const { return cp; }
	unsigned int size() const { return sz; }
	iVectorType type() const { return type_; }
	unsigned int mem() const { return buff.size()*sizeof(T); }

	void push(const T& v)
	{
		switch (type_)
		{
		case Open:
			if (sz==cp) buff.setSize(++cp);
			set_(sz++,v);
			break;
		case Fixed:
			if (sz<cp) set_(sz++,v);
			break;
		case Circular:
			set_(tail++,v);
			if (sz<cp) sz++;
			if (sz==cp) tail %= sz;
			break;
		}
	}

	const T* data() const
	{
		const_cast< ivector<T> * >( this )->normalize_();
		return buff.begin();
	}
};

#endif


