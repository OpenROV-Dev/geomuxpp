// Includes
#include "CVideoChannel.h"

CVideoChannel::CVideoChannel()
{
	
}

CVideoChannel::~CVideoChannel(){}

///////////////////////////////////////
// Private Channel API
///////////////////////////////////////
void CVideoChannel::Init( int channelNumIn )
{
	
}

void CVideoChannel::Deinit( int channelNumIn )
{
	
}

void CVideoChannel::RegisterVideoCallback( int channelNumIn )
{
	
}

void CVideoChannel::GetVideoSettings( int channelNumIn )
{
	
}

void CVideoChannel::GetSensorSettings( int channelNumIn )
{ 
	
}

bool CVideoChannel::IsAlive( int channelNumIn )
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