#pragma once

#include <json.hpp>

namespace gc6500
{
	
	nlohmann::json api = R"(
	{
		"version": "7.7.13",
		
		"publicAPI":
		{
			"report_settings":
			{
				"formats": [ "all" ],
				"params": {},
				"alias": "Publish Settings",
				"description": "Publishes current settings of the channel."
			},
			
			"report_health":
			{
				"formats": [ "all" ],
				"params": {},
				"alias": "Publish Health",
				"description": "Publishes current health stats of the channel."
			},
			
			"report_api":
			{
				"formats": [ "all" ],
				"params": {},
				"alias": "Publish API",
				"description": "Publishes the API of the channel."
			},
			
			"video_start":
			{
				"formats": [ "all" ],
				"params": {},
				"alias": "Start Video",
				"description": "Starts the video stream."
			},
			
			"video_stop":
			{
				"formats": [ "all" ],
				"params": {},
				"alias": "Stop Video",
				"description": "Stops the video stream."
			},
			
			"force_iframe":
			{
				"formats": [ "h264" ],
				"params": {},
				"alias": "Force I-Frame",
				"description": "Forces the channel to output an I-frame."
			},
			
			"apply_settings":
			{
				"formats": [ "all" ],
				"params": 
				{
					"settings":
					{
						"type": "object",
						"alias": "Settings",
						"description": "Collection of objects describing specific settings. See individual settings API for more info."
					}
				},
				"alias": "Apply Settings",
				"description": "Allows the user to set multiple settings at once."
			}
		},
		
		"settingsAPI":
		{
			"format":
			{
				"formats": [ "all" ],
				"params": 
				{
					"value":
					{
						"type": "string",
						"unit": "",
						"options": [ "h264", "mjpeg" ],
						"description": "Video stream format."
					}
				},
				"alias": "Video Format"
			},
			
			"width":
			{
				"formats": [ "all" ],
				"params": 
				{
					"value":
					{
						"type": "uint16",
						"unit": "px",
						"min": 800,
						"max": 1920,
						"description": "Width of image in pixels."
					}
				},
				"alias": "Width"
			},
			
			"height":
			{
				"formats": [ "all" ],
				"params": 
				{
					"value":
					{
						"type": "uint16",
						"unit": "px",
						"min": 600,
						"max": 1080,
						"description": "Height of image in pixels."
					}
				},
				"alias": "Height"
			},
			
			"framerate":
			{
				"formats": [ "all" ],
				"params": 
				{
					"value":
					{
						"type": "uint32",
						"unit": "fps",
						"min": 1,
						"max": 60,
						"description": "Sets max framerate of the channel. Does not guarantee the true framerate."
					}
				},
				"alias": "Framerate"
			},
			
			"bitrate":
			{
				"formats": [ "all" ],
				"params": 
				{
					"value":
					{
						"type": "uint32",
						"unit": "bps",
						"min": 1000,
						"max": 10000000,
						"description": "Sets max bitrate of the channel. Does not guarantee the true bitrate."
					}
				},
				"alias": "Bitrate"
			},
			
			"goplen":
			{
				"formats": [ "h264" ],
				"params": 
				{
					"value":
					{
						"type": "uint32",
						"unit": "frames",
						"min": 1,
						"max": 300,
						"description": "Number of P-frames between each I-frame."
					}
				},
				"alias": "GOP Length"
			},
			
			"gop_hierarchy_level":
			{
				"formats": [ "h264" ],
				"params": 
				{
					"value":
					{
						"type": "uint32",
						"unit": "",
						"min": 0,
						"max": 4,
						"description": "GOP Hierarchy Level."
					}
				},
				"alias": "GOP Hierarchy Level"
			},
			
			"avc_profile":
			{
				"formats": [ "h264" ],
				"params": 
				{
					"value":
					{
						"type": "string",
						"unit": "",
						"options": [ "baseline", "main", "high" ],
						"description": "AVC Encoding Profile."
					}
				},
				"alias": "AVC Encoding Profile"
			},
			
			"avc_level":
			{
				"formats": [ "h264" ],
				"params": 
				{
					"value":
					{
						"type": "uint32",
						"unit": "level",
						"min": 10,
						"max": 52,
						"description": "AVC Encoding Level."
					}
				},
				"alias": "AVC Encoding Level"
			},
			
			"maxnal":
			{
				"formats": [ "h264" ],
				"params": 
				{
					"enabled":
					{
						"type": "bool",
						"description": "When enabled, caps the max size of a given NAL unit to the specified value. If a frame is larger than this value, it will be broken up into multiple NAL units. If false, value will be implicitly set to 0."
					},
					
					"value":
					{
						"type": "uint32",
						"unit": "bytes",
						"min": 1,
						"max": 500000,
						"alias": "Size",
						"description": "Max NAL size in bytes."
					}
				},
				"alias": "Max NAL Unit Size"
			},
			
			"vui":
			{
				"formats": [ "h264" ],
				"params": 
				{
					"enabled":
					{
						"type": "bool",
						"description": "When enabled, includes VUI information in the h264 SPS."
					}
				},
				"alias": "VUI Information"
			},
			
			"pict_timing":
			{
				"formats": [ "h264" ],
				"params": 
				{
					"enabled":
					{
						"type": "bool",
						"description": "When enabled, includes picture timing information in the h264 SPS."
					}
				},
				"alias": "Picture Timing Information"
			},
			
			"max_framesize":
			{
				"formats": [ "h264" ],
				"params": 
				{
					"value":
					{
						"type": "uint32",
						"unit": "bytes",
						"min": 1,
						"max": 500000,
						"description": "Max frame size in bytes."
					}
				},
				"alias": "Max Frame Size"
			},
			
			"compression_quality":
			{
				"formats": [ "mjpeg" ],
				"params": 
				{
					"value":
					{
						"type": "uint32",
						"unit": "bytes",
						"min": 0,
						"max": 10000,
						"description": "Sets MJPEG compression quality in terms of image quantization parameter (QP)."
					}
				},
				"alias": "Compression Quality"
			},
			
			"flip_vertical":
			{
				"formats": [ "all" ],
				"params": 
				{
					"enabled":
					{
						"type": "bool",
						"description": "Flips image vertically."
					}
				},
				"alias": "Flip Vertical"
			},
			
			"flip_horizontal":
			{
				"formats": [ "all" ],
				"params": 
				{
					"enabled":
					{
						"type": "bool",
						"description": "Flips image horizontally."
					}
				},
				"alias": "Flip Horizontal"
			},
			
			"contrast":
			{
				"formats": [ "all" ],
				"params": 
				{
					"value":
					{
						"type": "uint16",
						"unit": "none",
						"min": 0,
						"max": 200,
						"description": "Image contrast."
					}
				},
				"alias": "Contrast"
			},
			
			"zoom":
			{
				"formats": [ "all" ],
				"params": 
				{
					"value":
					{
						"type": "uint16",
						"unit": "none",
						"min": 0,
						"max": 100,
						"description": "Image zoom."
					}
				},
				"alias": "Zoom"
			},
			
			"pan":
			{
				"formats": [ "all" ],
				"params": 
				{
					"value":
					{
						"type": "int32",
						"unit": "none",
						"min": -648000,
						"max": 648000,
						"description": "Image pan."
					}
				},
				"alias": "Pan"
			},
			
			"tilt":
			{
				"formats": [ "all" ],
				"params": 
				{
					"value":
					{
						"type": "int32",
						"unit": "none",
						"min": -648000,
						"max": 648000,
						"description": "Image tilt."
					}
				},
				"alias": "Tilt"
			},
			
			"pantilt":
			{
				"formats": [ "all" ],
				"params": 
				{
					"pan":
					{
						"type": "int32",
						"unit": "none",
						"min": -648000,
						"max": 648000,
						"alias": "Pan",
						"description": "Image pan."
					},
					
					"tilt":
					{
						"type": "int32",
						"unit": "none",
						"min": -648000,
						"max": 648000,
						"alias": "Tilt",
						"description": "Image tilt."
					}
				},
				"alias": "Pan-Tilt"
			},
			
			"brightness":
			{
				"formats": [ "all" ],
				"params": 
				{
					"value":
					{
						"type": "int16",
						"unit": "none",
						"min": -255,
						"max": 255,
						"description": "Image brightness."
					}
				},
				"alias": "Brightness"
			},
			
			"hue":
			{
				"formats": [ "all" ],
				"params": 
				{
					"value":
					{
						"type": "int16",
						"unit": "none",
						"min": -18000,
						"max": 18000,
						"description": "Image hue."
					}
				},
				"alias": "Hue"
			},
			
			"gamma":
			{
				"formats": [ "all" ],
				"params": 
				{
					"value":
					{
						"type": "uint16",
						"unit": "none",
						"min": 100,
						"max": 300,
						"description": "Image gamma."
					}
				},
				"alias": "Gamma"
			},
			
			"saturation":
			{
				"formats": [ "all" ],
				"params": 
				{
					"value":
					{
						"type": "uint16",
						"unit": "none",
						"min": 0,
						"max": 200,
						"description": "Image saturation."
					}
				},
				"alias": "Saturation"
			},
			
			"gain":
			{
				"formats": [ "all" ],
				"params": 
				{
					"value":
					{
						"type": "uint16",
						"unit": "none",
						"min": 1,
						"max": 100,
						"description": "Image gain."
					}
				},
				"alias": "Gain"
			},
			
			"sharpness":
			{
				"formats": [ "all" ],
				"params": 
				{
					"value":
					{
						"type": "uint16",
						"unit": "none",
						"min": 1,
						"max": 100,
						"description": "Image sharpness."
					}
				},
				"alias": "Sharpness"
			},
			
			"max_analog_gain":
			{
				"formats": [ "all" ],
				"params": 
				{
					"value":
					{
						"type": "uint32",
						"unit": "none",
						"min": 0,
						"max": 15,
						"description": "Maximum sensor analog gain in auto exposure algorithm."
					}
				},
				"alias": "Max Analog Gain"
			},
			
			"histogram_eq":
			{
				"formats": [ "all" ],
				"params": 
				{
					"enabled":
					{
						"type": "bool",
						"description": "Enable/disable histogram equalization, which gives more contrast to the image."
					}
				},
				"alias": "Histogram Equalization"
			},
			
			"sharpen_filter":
			{
				"formats": [ "all" ],
				"params": 
				{
					"value":
					{
						"type": "uint32",
						"unit": "none",
						"min": 0,
						"max": 2,
						"description": "Strength of the sharpening filter, higher being stronger."
					}
				},
				"alias": "Sharpen Filter"
			},
			
			"min_exp_framerate":
			{
				"formats": [ "all" ],
				"params": 
				{
					"value":
					{
						"type": "uint32",
						"unit": "none",
						"min": 0,
						"max": 30,
						"description": "Minimum framerate that the autoexposure algorithm can drop to."
					}
				},
				"alias": "Minimum Autoexposure Framerate"
			},
			
			"tf_strength":
			{
				"formats": [ "all" ],
				"params": 
				{
					"enabled":
					{
						"type": "bool",
						"description": "Enable/disable the temporal filter."
					},
					
					"value":
					{
						"type": "uint32",
						"unit": "none",
						"min": 1,
						"max": 7,
						"alias": "Strength",
						"description": "Strength of the temporal filter."
					}
				},
				"alias": "Temporal Filter"
			},
			
			"gain_multiplier":
			{
				"formats": [ "all" ],
				"params": 
				{
					"value":
					{
						"type": "uint32",
						"unit": "none",
						"min": 0,
						"max": 256,
						"description": "Controls the autoexposure algorithm to adjust the sensor analog gain and exposure based on different lighting conditions."
					}
				},
				"alias": "Autoexposure Gain Multiplier"
			},
			
			"exp":
			{
				"formats": [ "all" ],
				"params": 
				{
					"mode":
					{
						"type": "string",
						"unit": "",
						"options": [ "auto", "manual" ],
						"alias": "Mode",
						"description": "Autoexposure mode."
					},
					
					"value":
					{
						"type": "uint16",
						"unit": "none",
						"min": 0,
						"max": 255,
						"alias": "Exposure Time",
						"description": "Exposure time."
					}
				},
				"alias": "Autoexposure Mode"
			},
			
			"nf":
			{
				"formats": [ "all" ],
				"params": 
				{
					"mode":
					{
						"type": "string",
						"unit": "",
						"options": [ "auto", "manual" ],
						"alias": "Mode",
						"description": "Noise filter mode."
					},
					
					"value":
					{
						"type": "uint16",
						"unit": "none",
						"min": 0,
						"max": 100,
						"alias": "Strength",
						"description": "Noise filter strength."
					}
				},
				"alias": "Noise Filter Mode"
			},
			
			"wb":
			{
				"formats": [ "all" ],
				"params": 
				{
					"mode":
					{
						"type": "string",
						"unit": "",
						"options": [ "auto", "manual" ],
						"alias": "Mode",
						"description": "White balance mode."
					},
					
					"value":
					{
						"type": "uint16",
						"unit": "none",
						"min": 2800,
						"max": 6500,
						"alias": "Temperature",
						"description": "White balance temperature."
					}
				},
				"alias": "White Balance Mode"
			},
			
			"wdr":
			{
				"formats": [ "all" ],
				"params": 
				{
					"enabled":
					{
						"type": "bool",
						"description": "Enable/disable Wide Dynamic Range mode."
					},
					
					"mode":
					{
						"type": "string",
						"unit": "",
						"options": [ "auto", "manual", "disabled" ],
						"alias": "Mode",
						"description": "WDR mode."
					},
					
					"value":
					{
						"type": "uint8",
						"unit": "none",
						"min": 0,
						"max": 255,
						"alias": "Intensity",
						"description": "WDR intensity."
					}
				},
				"alias": "Wide Dynamic Range Mode"
			},
			
			"zone_exp":
			{
				"formats": [ "all" ],
				"params": 
				{
					"enabled":
					{
						"type": "bool",
						"description": "Enable/disable zonal exposure."
					},
					
					"value":
					{
						"type": "uint16",
						"unit": "none",
						"min": 0,
						"max": 62,
						"alias": "Zone",
						"description": "Zone value."
					}
				},
				"alias": "Zonal Exposure"
			},
			
			"zone_wb":
			{
				"formats": [ "all" ],
				"params": 
				{
					"enabled":
					{
						"type": "bool",
						"description": "Enable/disable zonal white balance."
					},
					
					"value":
					{
						"type": "uint16",
						"unit": "none",
						"min": 0,
						"max": 63,
						"alias": "Zone",
						"description": "Zone value."
					}
				},
				"alias": "Zonal White Balance"
			},
			
			"pwr_line_freq":
			{
				"formats": [ "all" ],
				"params": 
				{
					"enabled":
					{
						"type": "bool",
						"description": "Enable/disable power line frequency compensation."
					},
					
					"mode":
					{
						"type": "string",
						"unit": "",
						"options": [ "50HZ", "60HZ", "disabled" ],
						"alias": "Frequency",
						"description": "Power line frequency of the operating region. Sensor exposure value under the auto-exposure algorithm will be adjusted to avoid flickering caused by power level oscillation."
					}
				},
				"alias": "Power Line Frequency Compensation"
			}
		}
	}
	)"_json;
}