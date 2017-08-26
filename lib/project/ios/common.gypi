{
	'target_defaults': {
		'conditions': [
			['OS=="ios"', {
			'defines': [
			'VR_IOS',
			'VR_GLES'
			],
			}]
		],

		'configurations': {
			'Debug': {
				'conditions': [

				['OS=="ios"', {
				"defines": [
					"DEBUG=1"
                		],

				'xcode_settings': {
					'ALWAYS_SEARCH_USER_PATHS': 'NO',
					'SDKROOT': 'iphoneos',
					'TARGETED_DEVICE_FAMILY': '1,2',
					'CODE_SIGN_IDENTITY': 'iPhone Developer',
					'IPHONEOS_DEPLOYMENT_TARGET': '8.0',
					'ARCHS': '$(ARCHS_STANDARD)',
					'VALID_ARCHS': 'arm64 armv7 armv7s',
					'CLANG_ENABLE_OBJC_ARC': 'YES',
					'CLANG_CXX_LANGUAGE_STANDARD' : 'c++11',
					'GCC_C_LANGUAGE_STANDARD': 'c11',
					"GCC_OPTIMIZATION_LEVEL": "0",
					"GCC_GENERATE_DEBUGGING_SYMBOLS": "YES"				
				}
				}]				
				
				]
			},

			'Release': {
				'conditions': [

				['OS=="ios"', {
				"defines": [
				],
					
				'xcode_settings': {
					'ALWAYS_SEARCH_USER_PATHS': 'NO',
					'SDKROOT': 'iphoneos',
					'TARGETED_DEVICE_FAMILY': '1,2',
					'CODE_SIGN_IDENTITY': 'iPhone Developer',
					'IPHONEOS_DEPLOYMENT_TARGET': '8.0',
					'ARCHS': '$(ARCHS_STANDARD)',
					'VALID_ARCHS': 'arm64 armv7 armv7s',
					'CLANG_ENABLE_OBJC_ARC': 'YES',
					'CLANG_CXX_LANGUAGE_STANDARD' : 'c++11',				
					'GCC_C_LANGUAGE_STANDARD': 'c11',
					"GCC_OPTIMIZATION_LEVEL": "3",
					"GCC_GENERATE_DEBUGGING_SYMBOLS": "NO",
					"DEAD_CODE_STRIPPING": "YES",
					"GCC_INLINES_ARE_PRIVATE_EXTERN": "YES"				
				}
				}]
				
				]
			}
		}
	},
}
