#pragma once

// Includes
#include <cstdint>
#include <memory>
#include <mutex>
#include <condition_variable>
#include <chrono>

struct TFrameStats
{
	uint64_t	m_frameAttempts	= 0;
	uint64_t 	m_frameWrites	= 0;
	uint64_t 	m_frameFails 	= 0;
	float		m_fps			= 0.0f;
	
	std::chrono::high_resolution_clock::time_point m_lastWriteTime;
	
	void Clear()
	{
		m_frameAttempts = 0;
		m_frameWrites 	= 0;
		m_frameFails 	= 0;
		m_fps			= 0;
		
		m_lastWriteTime = {};
	}
};

class CVideoBuffer
{
public:
	// Attributes		
	std::mutex 						m_mutex;
	std::condition_variable 		m_dataAvailableCondition;
	
	TFrameStats						m_frameStats;
	size_t							m_framesStored		= 0;
	
	// Methods
	CVideoBuffer( size_t defaultAllocationSizeIn = 5000000 ); 	// Default to ~5MB
	
	size_t GetSize();
	size_t GetRemainingCapacity();
	size_t GetReservedSize();
	
	void ClearWriteCounts();
	
	uint8_t GetByte( size_t indexIn );
	
	uint8_t* Begin();
	uint8_t* End();
	
	void Clear();
	bool Write( uint8_t *rawBufferIn, size_t bufferSizeIn, bool shouldSignalConditionIn = true );
	
private:
	// Attributes
	const size_t					k_containerSize;	
	std::unique_ptr<uint8_t[]>		m_data;
	size_t 							m_endIndex;
	size_t 							m_remainingCapacity;
};