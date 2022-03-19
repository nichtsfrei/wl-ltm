project(
  'wl-tlm',
  'c',
  version : '0.0.1',
  license : 'MIT',
  meson_version : '>=0.47.0',
  default_options : ['c_std=c99', 'warning_level=3', 'werror=true']
)

cc = meson.get_compiler('c')

add_project_arguments(cc.get_supported_arguments([
   '-Wsequence-point',
   '-Wstrict-prototypes',
   '-Wshadow',
   '-Wmissing-prototypes',
   '-Wpedantic',
]), language: 'c')

wayland_client = dependency('wayland-client')

syslog_journal = dependency('libsystemd', required: false)
subdir('protocol')

werror = true
warning_level = 3

wlr_randr_exe = executable(
	meson.project_name(),
	files(['main.c']),
	dependencies: [wayland_client, client_protos, syslog_journal],
	install: true,
)
