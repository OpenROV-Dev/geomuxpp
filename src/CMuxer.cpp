// Includes
#include "CMuxer.h"
#include <iostream>
#include <unistd.h>

using namespace std;
using namespace CpperoMQ;

CMuxer::CMuxer( CpperoMQ::Context *contextIn, const std::string &endpointIn, EVideoFormat formatIn )
	: m_format( formatIn )
	, m_pContext( contextIn )
	, m_dataPub( m_pContext->createPublishSocket() )
	, m_thread()
	, m_killThread( false )
{
	// Bind the data publisher
	m_dataPub.bind( endpointIn.c_str() );

	// Start the muxer thread
	m_thread = std::thread( &CMuxer::ThreadLoop, this );
}

CMuxer::~CMuxer()
{
	cout << "Cleaning up CMuxer" << endl;
	
	m_killThread = true;
	
	try
	{
		// TODO: Rework data signaling
		// Send a notification to make sure the thread gets killed
		m_inputBuffer.m_dataAvailableCondition.notify_one();
		m_thread.join();
	}
	catch( const std::exception &e )
	{
		cerr << "Error cleaning up CMuxer: " << e.what() << endl;
	}
	
	// TODO: Probably need to clean up more things
	// Clean up libav structures
	avformat_close_input( &m_pInputFormatContext );
	avformat_close_input( &m_pOutputFormatContext );
	
    if( m_pInputAvioContext ) 
	{
        av_freep( &m_pInputAvioContext) ;
    }
	
	if( m_pOutputAvioContext ) 
	{
        av_freep( &m_pOutputAvioContext) ;
    }
}

void CMuxer::Initialize()
{	
	//////////////////////////////////////////////////////////////
	// Input setup
	//////////////////////////////////////////////////////////////
	
	// Turn on all traces
	av_log_set_level( AV_LOG_TRACE );
	
	// Allocate format context
	if( !( m_pInputFormatContext = avformat_alloc_context() ) ) 
	{
        cerr << "Failed to allocate format context!" << endl;
		return;
    }
	
	// Allocate the buffer for the input AVIO context
	m_pInputAvioContextBuffer = (uint8_t*)av_malloc( m_avioContextBufferSize );
	
	// Associate the input AVIO context with the allocated input buffer and ReadPacket function
    m_pInputAvioContext = avio_alloc_context( m_pInputAvioContextBuffer, m_avioContextBufferSize, 0, this, &ReadPacket, NULL, NULL );
	if( !m_pInputAvioContext ) 
	{
       	cerr << "Failed to allocate avio context!" << endl;
		return;
    }
	
	// Set the IO context for the input format context
    m_pInputFormatContext->pb 		= m_pInputAvioContext;
	
	// TODO: Is NOBUFFER needed?
	m_pInputFormatContext->flags 	= AVFMT_FLAG_CUSTOM_IO | AVFMT_FLAG_NOBUFFER;
	
	
	
	//////////////////////////////////////////////////////////////
	// Output setup
	//////////////////////////////////////////////////////////////
	
	// Allocate output format context for muxing to MP4
	avformat_alloc_output_context2( &m_pOutputFormatContext, NULL, "mp4", NULL );
    if( !m_pOutputFormatContext ) 
	{
       	cerr << "Could not create output context!" << endl;
        return;
    }
	
	// Allocate the buffer for the output AVIO context
	m_pOutputAvioContextBuffer 	= (uint8_t*)av_malloc(m_avioContextBufferSize);
	
	// Associate the output AVIO context with the allocated output buffer and WritePacket function
    m_pOutputAvioContext = avio_alloc_context( m_pOutputAvioContextBuffer, m_avioContextBufferSize, 1, this, NULL, &WritePacket, NULL );
	if( !m_pOutputAvioContext ) 
	{
       	cerr << "Failed to allocate output avio context!" << endl;
		return;
    }
	
	// Set the IO context for the output format context
    m_pOutputFormatContext->pb 		= m_pOutputAvioContext;
	
	// TODO: Are NOBUFFER and FLUSH_PACKETS needed?
	m_pOutputFormatContext->flags 	= AVFMT_FLAG_CUSTOM_IO | AVFMT_FLAG_NOBUFFER | AVFMT_FLAG_FLUSH_PACKETS;
	
	cout << "Init success" << endl;
}

void CMuxer::Update()
{
	if( !m_formatAcquired )
	{
		// Attempt to acquire the input stream format
		if( !m_pInputFormatContext->iformat )
		{			
			std::lock_guard<std::mutex> lock( m_inputBuffer.m_mutex );
			
			AVProbeData probeData;
			
			// Create point a probe data buffer to our input buffer
			probeData.buf 		= m_inputBuffer.Begin();
			probeData.buf_size 	= m_inputBuffer.GetSize();
			probeData.filename 	= "";
	
			int score = AVPROBE_SCORE_MAX / 4;
	
			// Let the input buffer grow to a decent size before attempting to probe it
			if( probeData.buf_size > 2000000 )
			{
				// Probe data
				cout << "Probing input buffer for format info..." << endl;
				m_pInputFormatContext->iformat = av_probe_input_format2( &probeData, 1, &score );
			}
			else
			{
				return;
			}
			
			// If we failed to get the input format, clear the input buffer and let it build up again for a second try
		 	if( !m_pInputFormatContext->iformat )
			{
				m_inputBuffer.Clear();
			}
		}
		
		if( m_pInputFormatContext->iformat )
		{	
			// Successfully for the input format
			cout << "Successfully acquired input format." << endl;
			
			// Open the format context. Codec type was already detected in prior step.
			int ret = avformat_open_input( &m_pInputFormatContext, NULL, NULL, NULL );
			if (ret < 0)
			{
				cerr << "Could not open input!" << endl;
				return;
			}
			else
			{
				cout << "Input opened successfully!" << endl;
				
				m_formatAcquired = true;
				
				cout << "Finding stream info..." << endl;
				
				// Read some packets in the AVIO context buffer to get stream information
				if( avformat_find_stream_info( m_pInputFormatContext, NULL ) < 0 )
				{
					cerr << "Unable to find stream info!" << endl;
					return;
				}
				
				// DEBUG
				av_dump_format( m_pInputFormatContext, 0, 0, 0 );
				
				cout << "Stream info acquired." << endl;
				
				// Find the codec info in the stream info
				m_pInputCodecContext = m_pInputFormatContext->streams[0]->codec;
				
				cout << "Finding codec..." << endl;
				
				// Find the codec needed for the stream
				AVCodec *pCodec = avcodec_find_decoder( m_pInputCodecContext->codec_id );
				if( pCodec == NULL ) 
				{
					cerr << "Failed to find decoder" << endl;
					return;
				}				
				
				cout << "Opening codec..." << endl;
				
				// Open codec
				if( avcodec_open2( m_pInputCodecContext, pCodec, NULL ) < 0 ) 
				{
					cerr << "Failed to open decoder" << endl;
					return;
				}
			
				cout << "Codec opened." << endl;
			
				// Set some flags on the output format context
				// TODO: Is this needed?
				m_pOutputFormatContext->oformat->flags |= AVFMT_ALLOW_FLUSH;
				
				// TODO: Loop through each input stream and create a corresponding output stream. Right now, we really only ever create one, since there is only one input stream.
				for( size_t i = 0; i < m_pInputFormatContext->nb_streams; ++i ) 
				{
					// Get the input stream
					AVStream *in_stream = m_pInputFormatContext->streams[i];
					
					// Create the output stream
					AVStream *out_stream = avformat_new_stream( m_pOutputFormatContext, pCodec );
					if(!out_stream) 
					{
						cerr << "Failed allocating output stream" << endl;
						return;
					}
					
					cout << "Created output stream." << endl;
			
					// Copy the codec context from the input stream to the output stream, since we are just muxing to mp4.
					ret = avcodec_copy_context( out_stream->codec, in_stream->codec );
					if (ret < 0) 
					{
						cerr << "Failed to copy context from input to output stream codec context" << endl;
						return;
					}			
					
					cout << "Copied input codec context to output codec context." << endl;	
					
					// TODO: Is this necessary?
					if (m_pOutputFormatContext->oformat->flags & AVFMT_GLOBALHEADER)
					{
						m_pInputCodecContext->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;
					}
				}
						
				// Set flags that will produce fragmented mp4 for livestreaming
				av_dict_set( &m_pMuxerOptions, "movflags", "empty_moov+default_base_moof+frag_keyframe", 0 );
				
				cout << "Writing FTYP+MOOV..." << endl;
				
				// Write Header Frame (ftyp+moov)
				// The header writes twice. This forces it to buffer the two writes together before sending the async signal
				m_holdBuffer = true;
				ret = avformat_write_header( m_pOutputFormatContext, &m_pMuxerOptions );
				if (ret < 0) 
				{
					cerr << "Error occurred when writing header!";
					return;
				}
				
				cout << "Ready to mux!" << endl;
				
				// We can now proceed to mux and send all subsequent frames
				m_canMux = true;
			}
		}
	}
	else
	{
		if( m_canMux )
		{
			AVPacket packet;
			
			// Read a frame from the input buffer into a packet structure
			int ret = av_read_frame( m_pInputFormatContext, &packet );
			if (ret < 0)
			{
				return;
			}
	
			// Write moof+dat with one frame in it
			// Call the second time with a null packet to flush the buffer and trigger a write_packet call
			ret = av_write_frame(m_pOutputFormatContext, &packet);
			ret |= av_write_frame(m_pOutputFormatContext, NULL );
			if (ret < 0) 
			{
				cerr << "Error writing packet." << endl;
				return;
			}
			
			// Cleanup
			av_packet_unref( &packet );
		}
	}
}

void CMuxer::ThreadLoop()
{
	Initialize();
	
	while( m_killThread == false )
	{
		{
			std::unique_lock<std::mutex> lock( m_inputBuffer.m_mutex );
			while( m_inputBuffer.GetSize() == 0 )
			{
				if( m_killThread )
				{
					break;
				}
				
				// Wait to be signaled that there is data
				m_inputBuffer.m_dataAvailableCondition.wait( lock );
			}
		}
		
		// Run the muxer's update function
		Update();
	}
}

// Custom read function for ffmpeg
int CMuxer::ReadPacket( void *muxerIn, uint8_t *avioBufferOut, int avioBufferSizeAvailableIn )
{
	// Convert opaque data to TSharedData
	CMuxer* muxer = (CMuxer*)muxerIn;
	
	// Figure out how many bytes to copy into 
	int bytesToConsume = muxer->m_inputBuffer.GetSize();
	
	// If the buffer size is bigger than the avioBuffer, we need to flush the avio buffer to make room, dropping old frames
	if( bytesToConsume > avioBufferSizeAvailableIn )
	{
		// TODO: Revisit
		// Flush
		avio_flush( muxer->m_pInputAvioContext );
		
		// Drop existing packets, since we can't fit them in the buffer. This might help the next time around
		{
			std::lock_guard<std::mutex> lock( muxer->m_inputBuffer.m_mutex );
			
			muxer->m_inputBuffer.Clear();
		}
			
		// Read 0 bytes
		return 0; 
	}

	// Copy video data to avio buffer
	{
		std::lock_guard<std::mutex> lock( muxer->m_inputBuffer.m_mutex );
		
		memcpy( avioBufferOut, muxer->m_inputBuffer.Begin(), bytesToConsume );
		muxer->m_inputBuffer.Clear();
	}
	
	return bytesToConsume;
}

// Custom read function for ffmpeg
int CMuxer::WritePacket( void *muxerIn, uint8_t *avioBufferIn, int bytesAvailableIn )
{
	// Convert opaque data to muxer pointer
	CMuxer *muxer = (CMuxer*)muxerIn;

	cout << "Writing muxed packet. Bytes: " << bytesAvailableIn << endl;

	try
	{
		if( muxer->m_holdBuffer )
		{
			
			if( !muxer->m_isComposingInitFrame )
			{
				OutgoingMessage topic( "init" );
				topic.send( muxer->m_dataPub, true );
				
				muxer->m_isComposingInitFrame = true;

				return 0;
			}
			else
			{
				OutgoingMessage payload( bytesAvailableIn, avioBufferIn );
				payload.send( muxer->m_dataPub, false );
				
				muxer->m_isComposingInitFrame = false;
				muxer->m_holdBuffer = false;
			}
		}
		else
		{
			OutgoingMessage topic( "geo" );
			topic.send( muxer->m_dataPub, true );
			
			OutgoingMessage payload( bytesAvailableIn, avioBufferIn );
			payload.send( muxer->m_dataPub, false );
		}
	}	
	catch (const std::exception &e)
	{	
		std::string errStr = e.what();
		cerr << "caught error: " << errStr << endl;
	}
	
	return bytesAvailableIn;	
}