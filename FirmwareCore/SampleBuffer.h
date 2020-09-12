#ifndef __SAMPLEBUFFER_H__
#define __SAMPLEBUFFER_H__

#include "stdint.h"
#include "../Magnetometer/Vector3.h"
template <class T>
class SampleBuffer
{
  public:
	uint8_t position, size;
	T* Samples;

	SampleBuffer(const uint8_t size);

	void add(T);
	T get();

}; //SampleBuffer

template <class T>
SampleBuffer<T>::SampleBuffer(const uint8_t _size)
{
	position = 1;
	size = _size;
	Samples = new T[_size];
}

template <class T>
void SampleBuffer<T>::add(T sample)
{
	Samples[--position] = sample;
	if (!position)
	position = size;
}

template <class T>
T SampleBuffer<T>::get()
{
	long out = 0;
	for (uint8_t i = 0; i < size; i++)
	out += Samples[i];
	return out / size;
}
#endif //__SAMPLEBUFFER_H__
