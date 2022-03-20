project(
  'wl-ltm',
  'c',
  version : '0.0.1',
  license : 'MIT',
  meson_version : '>=0.47.0',
  default_options : ['c_std=c99', 'warning_level=3', 'werror=true']
)

cc = meson.get_compiler('c')

pid_location = get_option('pidlocation')
message('PID location: ' + pid_location)
add_global_arguments('-DLTM_PID_LOCATION="@0@"'.format(pid_location), language : 'c')

lidstatelocation = get_option('lidstatelocation')
message('lid state file location: ' + lidstatelocation)
add_global_arguments('-DLTM_LID_STATE_FILE_PATH="@0@"'.format(lidstatelocation), language: 'c')

display = get_option('display')
message('display: ' + display)
add_global_arguments('-DLTM_TOGGLE_MONITOR="@0@"'.format(display), language: 'c')

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

wl_tlm_exe = executable(
	meson.project_name(),
	files(['main.c']),
	dependencies: [wayland_client, client_protos, syslog_journal],
	install: true,
)
