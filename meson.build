project('camlib', 'c')

files = [
  'src/operations.c',
  'src/packet.c',
  'src/enums.c',
  'src/data.c',
  'src/enum_dump.c',
  'src/util.c',
  'src/canon.c',
  'src/liveview.c',
  'src/bind.c',
  'src/ip.c',
  'src/fuji.c',
  'src/ml.c',
  'src/log.c',
  'src/libusb.c',
  'src/backend.c',
  'src/log.c',
  'src/conv.c',
  'src/generic.c',
]

library('camlib', files,
	include_directories: include_directories('src/'),
	dependencies: dependency('libusb-1.0')
)
