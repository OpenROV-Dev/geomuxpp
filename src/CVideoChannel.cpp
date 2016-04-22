// Includes
#include <utility>
#include "CVideoChannel.h"

CVideoChannel::CVideoChannel( video_channel_t channelIn )
	: m_channel( channelIn )
{
	
}

CVideoChannel::~CVideoChannel(){}

///////////////////////////////////////
// Private Channel API
///////////////////////////////////////
void CVideoChannel::Initialize()
{
	RegisterAPIFunctions();
}

void CVideoChannel::Cleanup()
{
	
}

void CVideoChannel::RegisterAPIFunctions()
{
	// Register callbacks in our handler map
	m_apiMap.insert( std::make_pair( std::string("StartVideo"), 					[this]( const nlohmann::json &commandIn ){ this->StartVideo( commandIn ); } ) );
	m_apiMap.insert( std::make_pair( std::string("StopVideo"), 						[this]( const nlohmann::json &commandIn ){ this->StopVideo( commandIn ); } ) );
	m_apiMap.insert( std::make_pair( std::string("SetMultipleSettings"), 			[this]( const nlohmann::json &commandIn ){ this->SetMultipleSettings( commandIn ); } ) );
	m_apiMap.insert( std::make_pair( std::string("SetFramerate"), 					[this]( const nlohmann::json &commandIn ){ this->SetFramerate( commandIn ); } ) );
	m_apiMap.insert( std::make_pair( std::string("SetBitrate"), 					[this]( const nlohmann::json &commandIn ){ this->SetBitrate( commandIn ); } ) );
	m_apiMap.insert( std::make_pair( std::string("ForceIFrame"), 					[this]( const nlohmann::json &commandIn ){ this->ForceIFrame( commandIn ); } ) );
	m_apiMap.insert( std::make_pair( std::string("SetGOPLength"), 					[this]( const nlohmann::json &commandIn ){ this->SetGOPLength( commandIn ); } ) );
	m_apiMap.insert( std::make_pair( std::string("SetGOPHierarchy"), 				[this]( const nlohmann::json &commandIn ){ this->SetGOPHierarchy( commandIn ); } ) );
	m_apiMap.insert( std::make_pair( std::string("SetAVCProfile"), 					[this]( const nlohmann::json &commandIn ){ this->SetAVCProfile( commandIn ); } ) );
	m_apiMap.insert( std::make_pair( std::string("SetAVCLevel"), 					[this]( const nlohmann::json &commandIn ){ this->SetAVCLevel( commandIn ); } ) );
	m_apiMap.insert( std::make_pair( std::string("SetMaxNALSize"), 					[this]( const nlohmann::json &commandIn ){ this->SetMaxNALSize( commandIn ); } ) );
	m_apiMap.insert( std::make_pair( std::string("EnableVUI"), 						[this]( const nlohmann::json &commandIn ){ this->EnableVUI( commandIn ); } ) );
	m_apiMap.insert( std::make_pair( std::string("EnablePictTiming"), 				[this]( const nlohmann::json &commandIn ){ this->EnablePictTiming( commandIn ); } ) );
	m_apiMap.insert( std::make_pair( std::string("SetMaxIFrameSize"), 				[this]( const nlohmann::json &commandIn ){ this->SetMaxIFrameSize( commandIn ); } ) );
	m_apiMap.insert( std::make_pair( std::string("SetCompressionQuality"), 			[this]( const nlohmann::json &commandIn ){ this->SetCompressionQuality( commandIn ); } ) );
	m_apiMap.insert( std::make_pair( std::string("EnableFlipVertical"), 			[this]( const nlohmann::json &commandIn ){ this->EnableFlipVertical( commandIn ); } ) );
	m_apiMap.insert( std::make_pair( std::string("EnableFlipHorizontal"), 			[this]( const nlohmann::json &commandIn ){ this->EnableFlipHorizontal( commandIn ); } ) );
	m_apiMap.insert( std::make_pair( std::string("SetContrast"), 					[this]( const nlohmann::json &commandIn ){ this->SetContrast( commandIn ); } ) );
	m_apiMap.insert( std::make_pair( std::string("SetZoom"), 						[this]( const nlohmann::json &commandIn ){ this->SetZoom( commandIn ); } ) );
	m_apiMap.insert( std::make_pair( std::string("SetPan"), 						[this]( const nlohmann::json &commandIn ){ this->SetPan( commandIn ); } ) );
	m_apiMap.insert( std::make_pair( std::string("SetTilt"), 						[this]( const nlohmann::json &commandIn ){ this->SetTilt( commandIn ); } ) );
	m_apiMap.insert( std::make_pair( std::string("SetPantilt"), 					[this]( const nlohmann::json &commandIn ){ this->SetPantilt( commandIn ); } ) );
	m_apiMap.insert( std::make_pair( std::string("SetBrightness"), 					[this]( const nlohmann::json &commandIn ){ this->SetBrightness( commandIn ); } ) );
	m_apiMap.insert( std::make_pair( std::string("SetHue"), 						[this]( const nlohmann::json &commandIn ){ this->SetHue( commandIn ); } ) );
	m_apiMap.insert( std::make_pair( std::string("SetGamma"), 						[this]( const nlohmann::json &commandIn ){ this->SetGamma( commandIn ); } ) );
	m_apiMap.insert( std::make_pair( std::string("SetSaturation"), 					[this]( const nlohmann::json &commandIn ){ this->SetSaturation( commandIn ); } ) );
	m_apiMap.insert( std::make_pair( std::string("SetGain"), 						[this]( const nlohmann::json &commandIn ){ this->SetGain( commandIn ); } ) );
	m_apiMap.insert( std::make_pair( std::string("SetSharpness"), 					[this]( const nlohmann::json &commandIn ){ this->SetSharpness( commandIn ); } ) );
	m_apiMap.insert( std::make_pair( std::string("SetMaxAnalogGain"), 				[this]( const nlohmann::json &commandIn ){ this->SetMaxAnalogGain( commandIn ); } ) );
	m_apiMap.insert( std::make_pair( std::string("SetHistogramEQ"), 				[this]( const nlohmann::json &commandIn ){ this->SetHistogramEQ( commandIn ); } ) );
	m_apiMap.insert( std::make_pair( std::string("SetSharpenFilter"), 				[this]( const nlohmann::json &commandIn ){ this->SetSharpenFilter( commandIn ); } ) );
	m_apiMap.insert( std::make_pair( std::string("SetMinAutoExposureFramerate"), 	[this]( const nlohmann::json &commandIn ){ this->SetMinAutoExposureFramerate( commandIn ); } ) );
	m_apiMap.insert( std::make_pair( std::string("SetTemporalFilterStrength"), 		[this]( const nlohmann::json &commandIn ){ this->SetTemporalFilterStrength( commandIn ); } ) );
	m_apiMap.insert( std::make_pair( std::string("SetGainMultiplier"), 				[this]( const nlohmann::json &commandIn ){ this->SetGainMultiplier( commandIn ); } ) );
	m_apiMap.insert( std::make_pair( std::string("SetExposureMode"), 				[this]( const nlohmann::json &commandIn ){ this->SetExposureMode( commandIn ); } ) );
	m_apiMap.insert( std::make_pair( std::string("SetNoiseFilterMode"), 			[this]( const nlohmann::json &commandIn ){ this->SetNoiseFilterMode( commandIn ); } ) );
	m_apiMap.insert( std::make_pair( std::string("SetWhiteBalanceMode"), 			[this]( const nlohmann::json &commandIn ){ this->SetWhiteBalanceMode( commandIn ); } ) );
	m_apiMap.insert( std::make_pair( std::string("SetWideDynamicRangeMode"), 		[this]( const nlohmann::json &commandIn ){ this->SetWideDynamicRangeMode( commandIn ); } ) );
	m_apiMap.insert( std::make_pair( std::string("SetZoneExposure"), 				[this]( const nlohmann::json &commandIn ){ this->SetZoneExposure( commandIn ); } ) );
	m_apiMap.insert( std::make_pair( std::string("SetZoneWhiteBalance"), 			[this]( const nlohmann::json &commandIn ){ this->SetZoneWhiteBalance( commandIn ); } ) );
	m_apiMap.insert( std::make_pair( std::string("SetPowerLineFrequency"), 			[this]( const nlohmann::json &commandIn ){ this->SetPowerLineFrequency( commandIn ); } ) );
}

void CVideoChannel::GetVideoSettings()
{
	
}

bool CVideoChannel::IsAlive()
{ 
	return false;
}


///////////////////////////////////////
// Public channel API
///////////////////////////////////////

// General
void CVideoChannel::StartVideo( const nlohmann::json &commandIn )
{
	
}
void CVideoChannel::StopVideo( const nlohmann::json &commandIn )
{
	
}

void CVideoChannel::SetMultipleSettings( const nlohmann::json &commandIn )
{
	
}

void CVideoChannel::SetFramerate( const nlohmann::json &commandIn )
{
	
}
void CVideoChannel::SetBitrate( const nlohmann::json &commandIn )
{
	
}

// H264
void CVideoChannel::ForceIFrame( const nlohmann::json &commandIn )
{
	
}
void CVideoChannel::SetGOPLength( const nlohmann::json &commandIn )
{
	
}
void CVideoChannel::SetGOPHierarchy( const nlohmann::json &commandIn )
{
	
}
void CVideoChannel::SetAVCProfile( const nlohmann::json &commandIn )
{
	
}
void CVideoChannel::SetAVCLevel( const nlohmann::json &commandIn )
{
	
}
void CVideoChannel::SetMaxNALSize( const nlohmann::json &commandIn )
{
	
}
void CVideoChannel::EnableVUI( const nlohmann::json &commandIn )
{
	
}
void CVideoChannel::EnablePictTiming( const nlohmann::json &commandIn )
{
	
}
void CVideoChannel::SetMaxIFrameSize( const nlohmann::json &commandIn )
{
	
}

// MJPEG
void CVideoChannel::SetCompressionQuality( const nlohmann::json &commandIn )
{
	
}

// Sensor
void CVideoChannel::EnableFlipVertical( const nlohmann::json &commandIn )
{
	
}

void CVideoChannel::EnableFlipHorizontal( const nlohmann::json &commandIn )
{
	
}

void CVideoChannel::SetContrast( const nlohmann::json &commandIn )
{
	
}

void CVideoChannel::SetZoom( const nlohmann::json &commandIn )
{
	
}

void CVideoChannel::SetPan( const nlohmann::json &commandIn )
{
	
}

void CVideoChannel::SetTilt( const nlohmann::json &commandIn )
{
	
}

void CVideoChannel::SetPantilt( const nlohmann::json &commandIn )
{
	
}

void CVideoChannel::SetBrightness( const nlohmann::json &commandIn )
{
	
}

void CVideoChannel::SetHue( const nlohmann::json &commandIn )
{
	
}

void CVideoChannel::SetGamma( const nlohmann::json &commandIn )
{
	
}

void CVideoChannel::SetSaturation( const nlohmann::json &commandIn )
{
	
}

void CVideoChannel::SetGain( const nlohmann::json &commandIn )
{
	
}

void CVideoChannel::SetSharpness( const nlohmann::json &commandIn )
{
	
}

void CVideoChannel::SetMaxAnalogGain( const nlohmann::json &commandIn )
{
	
}

void CVideoChannel::SetHistogramEQ( const nlohmann::json &commandIn )
{
	
}

void CVideoChannel::SetSharpenFilter( const nlohmann::json &commandIn )
{
	
}

void CVideoChannel::SetMinAutoExposureFramerate( const nlohmann::json &commandIn )
{
	
}

void CVideoChannel::SetTemporalFilterStrength( const nlohmann::json &commandIn )
{
	
}

void CVideoChannel::SetGainMultiplier( const nlohmann::json &commandIn )
{
	
}

void CVideoChannel::SetExposureMode( const nlohmann::json &commandIn )
{
	
}

void CVideoChannel::SetNoiseFilterMode( const nlohmann::json &commandIn )
{
	
}

void CVideoChannel::SetWhiteBalanceMode( const nlohmann::json &commandIn )
{
	
}

void CVideoChannel::SetWideDynamicRangeMode( const nlohmann::json &commandIn )
{
	
}

void CVideoChannel::SetZoneExposure( const nlohmann::json &commandIn )
{
	
}

void CVideoChannel::SetZoneWhiteBalance( const nlohmann::json &commandIn )
{
	
}

void CVideoChannel::SetPowerLineFrequency( const nlohmann::json &commandIn )
{
	
}