// Includes
#include "CMuxer.h"
#include <iostream>
#include <unistd.h>

using namespace std;

CMuxer::CMuxer( CpperoMQ::Context *contextIn, uint32_t channelIn, EVideoFormat formatIn )
	: m_format( formatIn )
	, m_pContext( contextIn )
	, m_dataPub( m_pContext->createPublishSocket() )
	, m_thread()
	, m_killThread( false )
{
	// Create a pub binding for this muxer's video data based on the channel number
	
	std::string temp( "ipc:///tmp/geomux_video" + std::to_string( channelIn ) + ".ipc" );
	// LOG INFO ) << "Muxer: " << temp;
	m_dataPub.bind( temp.c_str() );
	
	m_killThread = false;

	m_thread = std::thread( &CMuxer::ThreadLoop, this );
}

CMuxer::~CMuxer()
{
	m_killThread = true;
	
	// Send a notification 
	m_inputBuffer.m_dataAvailableCondition.notify_one();
	m_thread.join();
}

void CMuxer::Initialize()
{	
	////////////// INPUT 
	
	av_log_set_level(AV_LOG_TRACE);
	
	if( !( m_pInputFormatContext = avformat_alloc_context() ) ) 
	{
        cerr << "Failed to allocate format context!" << endl;
		return;
    }
	
	m_pInputAvioContextBuffer = (uint8_t*)av_malloc( m_avioContextBufferSize );
	
	// Set buffer to put data in, set buffer size, set data source, and packet function
    m_pInputAvioContext = avio_alloc_context( m_pInputAvioContextBuffer, m_avioContextBufferSize, 0, this, &ReadPacket, NULL, NULL );
	
	if( !m_pInputAvioContext ) 
	{
       	cerr << "Failed to allocate avio context!" << endl;
		return;
    }
	
    m_pInputFormatContext->pb 		= m_pInputAvioContext;
	
	// TODO: Check if these are needed
	m_pInputFormatContext->flags 	= AVFMT_FLAG_CUSTOM_IO | AVFMT_FLAG_NOBUFFER;
	
	//////////// OUTPUT
		
	avformat_alloc_output_context2( &m_pOutputFormatContext, NULL, "mp4", NULL );
	
    if( !m_pOutputFormatContext ) 
	{
       	cerr << "Could not create output context!" << endl;
        return;
    }
	
	m_pOutputFormat 			= m_pOutputFormatContext->oformat;
	m_pOutputAvioContextBuffer 	= (uint8_t*)av_malloc(m_avioContextBufferSize);
	
	// Set buffer to put data in, set buffer size, set data source, and packet function
    m_pOutputAvioContext = avio_alloc_context( m_pOutputAvioContextBuffer, m_avioContextBufferSize, 1, this, NULL, &WritePacket, NULL );
	
	if( !m_pOutputAvioContext ) 
	{
       	cerr << "Failed to allocate output avio context!" << endl;
		return;
    }
	
    m_pOutputFormatContext->pb 		= m_pOutputAvioContext;
	m_pOutputFormatContext->flags 	= AVFMT_FLAG_CUSTOM_IO | AVFMT_FLAG_NOBUFFER | AVFMT_FLAG_FLUSH_PACKETS;
	
	cout << "Init success" << endl;
}

void CMuxer::Cleanup()
{
	avformat_close_input( &m_pInputFormatContext );
	
    /* note: the internal buffer could have changed, and be != avio_ctx_buffer */
    if( m_pInputAvioContext ) 
	{
        //av_freep( &m_pInputAvioContext->buffer );
        av_freep( &m_pInputAvioContext) ;
    }
}

void CMuxer::Update()
{
	if( !m_formatAcquired )
	{
		if( !m_pInputFormatContext->iformat )
		{			
			std::lock_guard<std::mutex> lock( m_inputBuffer.m_mutex );
			
			AVProbeData probeData;
			probeData.buf 		= m_inputBuffer.Begin();
			probeData.buf_size 	= m_inputBuffer.GetSize();
			probeData.filename 	= "";
	
			int score = AVPROBE_SCORE_MAX / 4;
	
			// This seems to help get enough data for proper muxing. write_header is missing some avcC data if you dont
			if( probeData.buf_size > 2000000 )
			{
				m_pInputFormatContext->iformat = av_probe_input_format2( &probeData, 1, &score );
			}
			else
			{
				return;
			}
			
		 	if( !m_pInputFormatContext->iformat )
			{
				m_inputBuffer.Clear();
			}
		}
		
		if( m_pInputFormatContext->iformat )
		{	
			cout << "Got input format." << endl;
						
			int ret = avformat_open_input( &m_pInputFormatContext, NULL, NULL, NULL );
			
			if (ret < 0)
			{
				cerr << "Could not open input!" << endl;
				return;
			}
			else
			{
				cout << "Input opened successfully!" << endl;
				
				av_dump_format(m_pInputFormatContext, 0, 0, 0);
				
				m_formatAcquired = true;
				
				if( avformat_find_stream_info( m_pInputFormatContext, NULL ) < 0 )
				{
					cerr << "Unable to find stream info!" << endl;
				}
				
				// Find the decoder for the video stream
				m_pInputCodecContext = m_pInputFormatContext->streams[0]->codec;
				
				AVCodec *pCodec = avcodec_find_decoder(m_pInputCodecContext->codec_id);
				
				if (pCodec == NULL) 
				{
					cerr << "Failed to find decoder" << endl;
					return;
				}				
				
				// Open codec
				if (avcodec_open2(m_pInputCodecContext, pCodec, NULL ) < 0) 
				{
					cerr << "Failed to open decoder" << endl;
					return;
				}
				
				cout << "Setting up output context..." << endl;
				
				m_pOutputFormatContext->oformat->flags |= AVFMT_ALLOW_FLUSH;
				
				
				
				for (size_t i = 0; i < m_pInputFormatContext->nb_streams; i++) 
				{
					cout << "Adding stream" << endl;
					
					AVStream *in_stream = m_pInputFormatContext->streams[i];
					AVStream *out_stream = avformat_new_stream( m_pOutputFormatContext, pCodec );

					if(!out_stream) 
					{
						cerr << "Failed allocating output stream" << endl;
						return;
					}
			
					ret = avcodec_copy_context(out_stream->codec, in_stream->codec);
					
					if (ret < 0) 
					{
						cerr << "Failed to copy context from input to output stream codec context" << endl;
						return;
					}
					
					// TODO: What was this?
					//out_stream->codec->codec_tag = 0;					
					
					if (m_pOutputFormatContext->oformat->flags & AVFMT_GLOBALHEADER)
					{
						m_pInputCodecContext->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;
					}
					
					//out_stream->time_base = out_stream->codec->time_base;
					
					// LOG INFO ) << in_stream->codecpar->codec_tag;
					// LOG INFO ) << in_stream->codecpar->format;
					// LOG INFO ) << in_stream->codecpar->bit_rate;
					// LOG INFO ) << in_stream->codecpar->profile;
					// LOG INFO ) << in_stream->codecpar->level;
					// LOG INFO ) << in_stream->codecpar->width;
					// LOG INFO ) << in_stream->codecpar->height;
					// LOG INFO ) << in_stream->codecpar->sample_rate;
				}
				
				m_canMux = true;
				
				av_dict_set( &m_pMuxerOptions, "movflags", "empty_moov+default_base_moof+frag_keyframe", 0 );
				
				// Write Header Frame (ftyp+moov)
				// The header writes twice. This forces it to buffer the two writes together before sending the async signal
				m_holdBuffer = true;
				ret = avformat_write_header( m_pOutputFormatContext, &m_pMuxerOptions );
				
				if (ret < 0) 
				{
					cerr << "Error occurred when writing header!";
					return;
				}
			}
		}
	}
	else
	{
		// Mux into something else here
		if( m_canMux )
		{
			AVPacket packet;
	
			// AVStream *in_stream, *out_stream;

			int ret = av_read_frame(m_pInputFormatContext, &packet);
			
			if (ret < 0)
			{
				return;
			}
	
			// AVStream *in_stream  = m_pInputFormatContext->streams[packet.stream_index];
			// AVStream *out_stream = m_pOutputFormatContext->streams[packet.stream_index];
	
			// // LOG INFO ) << "Packet PTS: " << packet.pts;
			// // LOG INFO ) << "Packet DTS: " << packet.dts;
			// // LOG INFO ) << "Packet duration: " << packet.duration;
	
			// // Not sure if any of this is needed
			// packet.pts = AV_NOPTS_VALUE;
			// packet.dts = AV_NOPTS_VALUE;
			// packet.duration = AV_NOPTS_VALUE;
			// packet.pos = -1;
	
			// Write moof+dat with one frame in it
			// Call the second time with a null packet to flush the buffer and trigger a write_packet
			ret = av_write_frame(m_pOutputFormatContext, &packet);
			ret = av_write_frame(m_pOutputFormatContext, NULL );

			if (ret < 0) 
			{
				// LOG INFO ) << "Error writing packet.";
				return;
			}
			
			av_packet_unref(&packet);
		}
	}
}

void CMuxer::StartThread()
{
	
}

void CMuxer::StopThread()
{
	
}
	

void CMuxer::ThreadLoop()
{
	Initialize();
	
	// LOG INFO ) << "thread started";
	
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
		
		// // LOG INFO ) << "Hey";
		// sleep( 1 );
		
		// Run the muxer's update function
		Update();
	}

	Cleanup();
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
	// // Convert opaque data to muxer pointer
	// CMuxer *muxer = (CMuxer*)muxerIn;

	// // LOG INFO ) << "Emitting packet. Size: " << bytesAvailableIn;

	// try
	// {
	// 	if( muxer->m_holdBuffer )
	// 	{
			
	// 		if( !muxer->m_isComposingInitFrame )
	// 		{
	// 			zmq::message_t topic( 4 );
	// 			memcpy( topic.data(), "init", 4);
	// 			muxer->m_zmqPublisher.send( topic, ZMQ_SNDMORE );
				
	// 			muxer->m_isComposingInitFrame = true;

	// 			return 0;
	// 		}
	// 		else
	// 		{
	// 			zmq::message_t payload( bytesAvailableIn );
	// 			memcpy( payload.data(), (void*)avioBufferIn, bytesAvailableIn );
	// 			muxer->m_zmqPublisher.send( payload );
				
	// 			muxer->m_isComposingInitFrame = false;
	// 			muxer->m_holdBuffer = false;
	// 		}
	// 	}
	// 	else
	// 	{
	// 		zmq::message_t topic( 3 );
	// 		memcpy( topic.data(), "geo", 3);
	// 		muxer->m_zmqPublisher.send( topic, ZMQ_SNDMORE );
			
	// 		muxer->m_zmqPublisher.send((void*)avioBufferIn, bytesAvailableIn, 0 );	
	// 	}
	// }	
	// catch (const zmq::error_t& e)
	// {	
	// 	std::string errStr = e.what();
	// 	// LOG ERROR ) << "caught error: " << errStr;
	// }
	
	// return bytesAvailableIn;	
}