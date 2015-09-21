{
  "targets": [
    {
      "target_name": "dahua",
      "sources": [ "addon/dahua.cc", "addon/DaHuaClient.cc", "addon/H264Decoder.cc" ],
      "include_dirs": [
      'tmp/dahua-lib/include',
	  'tmp/ffmpeg-lib/include',
	  'addon'
      ],
      'link_settings': {
        'libraries': [
          '-ldhnetsdk',
		  '-lavcodec',
		  '-lavutil',
		  '-lswscale'
        ],
        'library_dirs': [
          'tmp/dahua-lib/lib',
		  'tmp/ffmpeg-lib/lib',
        ]
      },
	  "msbuild_settings": {
			"Link": {
				"ImageHasSafeExceptionHandlers": "false"
			}
		}
    }
  ]
}
