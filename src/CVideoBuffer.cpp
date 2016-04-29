// Includes
#include "CVideoBuffer.h"
#include <cstring>
#include <iostream>

// Namespace
using namespace std;

CVideoBuffer::CVideoBuffer( size_t defaultReservedBytesIn )
	: k_containerSize( defaultReservedBytesIn )
	, m_data( new uint8_t[ k_containerSize ] )
	, m_endIndex( 0 )
	, m_remainingCapacity( k_containerSize )
{
}

uint8_t* CVideoBuffer::Begin()
{
	// Return the pointer to the beginning of the buffer 
	return m_data.get();
}

uint8_t* CVideoBuffer::End()
{
	// Return the pointer to the past-the-end element of the buffer. This is where the theoretical next element would go.
	return m_data.get() + m_endIndex;
}

size_t CVideoBuffer::GetSize()
{
	return m_endIndex;
}

size_t CVideoBuffer::GetRemainingCapacity()
{
	return m_remainingCapacity;
}

size_t CVideoBuffer::GetReservedSize()
{
	return k_containerSize;
}
	
void CVideoBuffer::Clear()
{
	m_framesStored		= 0;
	m_endIndex 			= 0;
	m_remainingCapacity = k_containerSize;	
}

uint8_t CVideoBuffer::GetByte( size_t indexIn )
{
	if( indexIn >= k_containerSize )
	{
		return 0;
	}
	else
	{
		return m_data[ indexIn ];
	}
}

void CVideoBuffer::ClearWriteCounts()
{
	std::lock_guard<std::mutex> lock( m_mutex );
	
	m_framesStored 	= 0;
	m_frameStats.Clear();
}

bool CVideoBuffer::Write( uint8_t *rawBufferIn, size_t bufferSizeIn, bool shouldSignalConditionIn )
{	
	auto now = std::chrono::high_resolution_clock::now();
	
	// Lock
	std::lock_guard<std::mutex> lock( m_mutex );
	
	// Calculate framerate (rough diagnostic)
	m_frameStats.m_fps = ( 1000000.0f / (float)std::chrono::duration_cast<std::chrono::microseconds>( now - m_frameStats.m_lastWriteTime ).count() );
	
	// Set last write time
	m_frameStats.m_lastWriteTime = std::move( now );
	
	// Increment framecounter
	m_frameStats.m_frameAttempts++;
	
	// Check capacity
	if( bufferSizeIn > m_remainingCapacity )
	{
		m_frameStats.m_frameFails += m_framesStored;
		std::cerr << "Video buffer full. Dropped frames: " << m_framesStored << std::endl;
		Clear();
	}
	
	// Copy the source buffer to the end of the buffer
	memcpy( End(), rawBufferIn, bufferSizeIn );
	
	// Update index and capacity
	m_endIndex 			+= bufferSizeIn;
	m_remainingCapacity -= bufferSizeIn;
	
	if( shouldSignalConditionIn )
	{
		// Signal to the consumer that data is available
		m_dataAvailableCondition.notify_one();
	}
	
	m_framesStored++;
	m_frameStats.m_frameWrites++;
		
	return true;
}

