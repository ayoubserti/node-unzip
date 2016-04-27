{
	"targets":
	[
		{
            "target_name": "unzip",
            "sources": ["unzip_node.cpp", "unzip_object.cpp","zlib-1.2.3/contrib/minizip/unzip.c","zlib-1.2.3/contrib/minizip/ioapi.c"],
            'defines': [
              
            ],
            "cflags" : [ "-std=c++11" ],
            "include_dirs":[
                "./zlib-1.2.3/contrib/minizip",
                "./zlib-1.2.3"
            ],
            	
            "conditions": [
              [ 'OS!="win"', {
                "cflags+": [ "-std=c++11" ],
                "cflags_c+": [ "-std=c++11" ],
                "cflags_cc+": [ "-std=c++11" ],
                
                "link_settings": {
                
                },
              }],
              [ 'OS=="win"', {
                
                "link_settings": {
                
                },
              }],
              [ 'OS=="mac"', {
                "xcode_settings": {
                  "OTHER_CPLUSPLUSFLAGS" : [ "-std=c++11", "-stdlib=libc++" ],
                  "OTHER_LDFLAGS": [ "-stdlib=libc++" ],
                  "MACOSX_DEPLOYMENT_TARGET": "10.7"
                },
              }],
              ['OS=="linux"',{
                  "libraries": ["-lz"]
              }]
            ],
		},
	],
}