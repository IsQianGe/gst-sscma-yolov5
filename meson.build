project('gst-sscma-yolov5', 'c', version : '1.0.0.1', license : 'LGPL')

sscmayolov5_install_dir = join_paths(get_option('libdir'), 'gstreamer-1.0')

cc = meson.get_compiler('c')

gst_version = meson.project_version()

api_version = '1.0'

gst_dep = dependency('gstreamer-1.0', version : '>=1.19',
    required : true, fallback : ['gstreamer', 'gst_dep'])
gstbase_dep = dependency('gstreamer-base-1.0', version : '>=1.19',
  fallback : ['gstreamer', 'gst_base_dep'])


# The sscmayolov5 Plugin
 gstsscmayolov5_sources = [
  'src/gstsscmayolov5.c',
  ]

# The sscmayolov5 include directories
gstsscmayolov5_include_dirs = [
  include_directories('src')
]

gstsscmayolov5 = library('gstsscmayolov5',
  gstsscmayolov5_sources,
  include_directories : gstsscmayolov5_include_dirs,
  dependencies : [gst_dep, gstbase_dep],
  install : true,
  install_dir : sscmayolov5_install_dir,
)
