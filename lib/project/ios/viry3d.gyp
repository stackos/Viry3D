{
  	'target_defaults': 
  	{
		'includes': 
    		[ 
			'common.gypi', 
		],
		'default_configuration': 'Release',
  	},

	'conditions': [
		['OS=="ios"',
		{
			'includes':
			[
				'viry3d.gypi'
        		]
    		}],   
	],
} 
