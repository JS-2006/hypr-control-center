project(
  'hypr-control-center', 'c',
        version : '0.1.0',
        license : 'GPL2+',
  meson_version : '>= 0.60.0'
)

control_center_prefix = get_option('prefix')
control_center_bindir = join_paths(control_center_prefix, get_option('bindir'))
control_center_datadir = join_paths(control_center_prefix, get_option('datadir'))
control_center_libexecdir = join_paths(control_center_prefix, get_option('libexecdir'))
control_center_localedir = join_paths(control_center_prefix, get_option('localedir'))
control_center_mandir = join_paths(control_center_prefix, get_option('mandir'))
control_center_sysconfdir = join_paths(control_center_prefix, get_option('sysconfdir'))

control_center_pkgdatadir = join_paths(control_center_datadir, meson.project_name())
control_center_desktopdir = join_paths(control_center_datadir, 'applications')
control_center_icondir = join_paths(control_center_datadir, 'icons')
control_center_schemadir = join_paths (control_center_datadir, 'glib-2.0', 'schemas')

control_center_gettext = meson.project_name() + '-2.0'

host_is_linux = host_machine.system().contains('linux')
host_is_linux_not_s390 = host_is_linux and not host_machine.cpu().contains('s390')

cc = meson.get_compiler('c')

config_h = configuration_data()

py = import('python')
python = py.find_installation('python3')
blueprint_compiler = find_program('blueprint-compiler', version: '>= 0.19')

config_h.set_quoted('TEST_NM_PYTHON', python.full_path())

application_id = 'org.hypr.Settings'

version_split = meson.project_version().split('.')

# defines
set_defines = [
  # package
  ['APPLICATION_ID', application_id],
  ['PACKAGE', meson.project_name()],
  ['PACKAGE_VERSION', meson.project_version()],
  ['VERSION', meson.project_version()],
  ['MAJOR_VERSION', version_split[0]],
  # i18n
  ['GETTEXT_PACKAGE', control_center_gettext]
]

foreach define: set_defines
  config_h.set_quoted(define[0], define[1])
endforeach

distributor_logo = get_option('distributor_logo')
if (distributor_logo != '')
  config_h.set_quoted('DISTRIBUTOR_LOGO', distributor_logo,
                      description: 'Define to absolute path of distributor logo')
  dark_mode_distributor_logo = get_option('dark_mode_distributor_logo')
  if (dark_mode_distributor_logo != '')
    config_h.set_quoted('DARK_MODE_DISTRIBUTOR_LOGO', dark_mode_distributor_logo,
                        description: 'Define to absolute path of distributor logo for use in dark mode')
  endif
endif

# meson does not support octal values, so it must be handled as a
# string. See: https://github.com/mesonbuild/meson/issues/2047
config_h.set('USER_DIR_MODE', '0700',
             description: 'Permissions for creating the user\'s config, cache and data directories')

# compiler flags
common_flags = ['-DHAVE_CONFIG_H']

# We have so many deprecation warnings that it becomes impossible to notice any other useful
# warning, thus making the compiler output completely useless. Ideally, we should fix all
# of those but, until then, it's better to add this flag so that we avoid adding even more
# warnings by accident. If you want to fix some of those, just build Settings with:
# meson _build -Ddeprecated-declarions=enabled
if get_option('deprecated-declarations').disabled()
  common_flags += '-Wno-deprecated-declarations'
endif

# Only add this when optimizing is enabled (default)
optimized_src = '''
  #if __OPTIMIZE__ == 0
  #error No optimization
  #endif
'''

control_center_optimized = get_option('buildtype').contains('optimized') and cc.compiles(optimized_src)

if control_center_optimized
  common_flags += '-Wp,-D_FORTIFY_SOURCE=2'
endif

if get_option('buildtype').contains('debug')
  test_cflags = [
    '-Wcast-align',
    '-Wmissing-field-initializers',
    '-Wmissing-declarations',
    '-Wmissing-prototypes',
    '-Wnested-externs',
    '-Wno-strict-aliasing',
    '-Wno-sign-compare'
  ]

  common_flags += cc.get_supported_arguments(test_cflags)
endif

# Warnings which are not useful for our coding style, regardless of the build type
common_flags += cc.get_supported_arguments([
    '-Wno-unused-parameter',
])

add_project_arguments(common_flags, language: 'c')

# Check that we meet the dependencies
gtk_version = '>= 4.22.0'
pulse_req_version = '>= 2.0'

gtk_dep = dependency(
  'gtk4',
  version: gtk_version,
  fallback: 'gtk',
  default_options: [
    'introspection=disabled',
    'build-demos=false',
    'build-testsuite=false',
    'build-tests=false',
    'build-examples=false',
  ]
)

gtk_wayland_dep = dependency(
  'gtk4-wayland',
  version: gtk_version,
  fallback: 'gtk',
  default_options: [
    'introspection=disabled',
    'build-demos=false',
    'build-testsuite=false',
    'build-tests=false',
    'build-examples=false',
  ]
)

libadwaita_dep = dependency('libadwaita-1', version: '>= 1.4')

gdk_pixbuf_dep = dependency('gdk-pixbuf-2.0', version: '>= 2.23.0')
gio_dep = dependency('gio-2.0')
glib_dep = dependency('glib-2.0', version: '>= 2.76.6')
libxml_dep = dependency('libxml-2.0')
pulse_dep = dependency('libpulse', version: pulse_req_version)
pulse_mainloop_dep = dependency('libpulse-mainloop-glib', version: pulse_req_version)
upower_glib_dep = dependency('upower-glib', version: '>= 1.90.6')
gudev_dep = dependency('gudev-1.0', version: '>= 232')
epoxy_dep = dependency('epoxy')
json_glib_dep = dependency('json-glib-1.0')

goa_dep = dependency('goa-1.0', version: '>= 3.45')
goa_backend_dep = dependency('goa-backend-1.0', version: '>= 3.45')

m_dep = cc.find_library('m')

colord_dep = dependency('colord', version: '>= 1.4')
gsettings_desktop_dep = dependency('gsettings-desktop-schemas', version: '>= 50.alpha')

common_deps = [
  gio_dep,
  glib_dep,
  libadwaita_dep,
  dependency('gio-unix-2.0'),
  dependency('gthread-2.0'),
  gtk_dep,
  gsettings_desktop_dep,
]

polkit_gobject_dep = dependency('polkit-gobject-1', version: '>= 0.103')
# Also verify that polkit ITS files exist:
# https://gitlab.gnome.org/GNOME/gnome-control-center/-/issues/491
polkit_files = [ 'gettext/its/polkit.its', 'gettext/its/polkit.loc' ]
foreach polkit_file: polkit_files
  r = run_command('build-aux/meson/find_xdg_file.py', polkit_file, check: true)
  assert(r.returncode() == 0, 'ITS support missing from polkit, please upgrade or contact your distribution')
endforeach

# Localisation API
if cc.has_header_symbol('langinfo.h', '_NL_TIME_FIRST_WEEKDAY')
  config_h.set('HAVE__NL_TIME_FIRST_WEEKDAY', 1,
               description: 'Define if langinfo.h supports _NL_TIME_FIRST_WEEKDAY')
endif

# Check for CUPS 1.4 or newer
cups_dep = dependency('cups', version : '>= 1.4', required: false)
assert(cups_dep.found(), 'CUPS 1.4 or newer not found')

# https://bugzilla.gnome.org/show_bug.cgi?id=696766
cups_cflags = []
if cups_dep.version().version_compare('>= 1.6')
  cups_cflags += '-D_PPD_DEPRECATED='
endif

# cups headers
check_headers = [
  ['HAVE_CUPS_CUPS_H', 'cups/cups.h'],
  ['HAVE_CUPS_PPD_H', 'cups/ppd.h']
]

foreach header: check_headers
  assert(cc.has_header(header[1], args: cups_cflags), 'CUPS headers not found: ' + header[1])
endforeach

if host_is_linux
  mm_dep =  dependency('mm-glib', version: '>= 0.7')
  network_manager_deps = [
    dependency('libnm', version: '>= 1.52.0'),
    dependency('libnma-gtk4', version: '>= 1.10.2'),
    mm_dep,
  ]
endif
config_h.set('BUILD_NETWORK', host_is_linux,
             description: 'Define to 1 to build the Network panel')
config_h.set('BUILD_BLUETOOTH', host_is_linux,
             description: 'Define to 1 to build the Bluetooth panel')
config_h.set('BUILD_WWAN', host_is_linux,
             description: 'Define to 1 to build the WWan panel')

fs = import('fs')
gnome = import('gnome')

# Try to find gdesktop-enums.h from gsettings-desktop-schemas
gdesktop_enums_header = '/usr/include/gsettings-desktop-schemas/gdesktop-enums.h'
if not fs.exists(gdesktop_enums_header)
  gdesktop_enums_header = '/usr/local/include/gsettings-desktop-schemas/gdesktop-enums.h'
endif
gdesktop_enums_types = gnome.mkenums_simple('gdesktop-enums-types',
  sources: gdesktop_enums_header,
  install_header: false,
)
i18n = import('i18n')
pkg = import('pkgconfig')

po_dir = join_paths(meson.project_source_root(), 'po')
its_dir = join_paths(meson.project_source_root(), 'gettext')

install_subdir(
  'gettext',
  install_dir: control_center_datadir
)

top_inc = include_directories('.')
shell_inc = include_directories('shell')

subdir('data/icons')
subdir('libhypr')
subdir('po')
subdir('panels')
subdir('shell')
if get_option('tests')
  subdir('tests')
endif

if get_option('documentation')
  subdir('man')
endif

configure_file(
  output: 'config.h',
  configuration: config_h
)

summary({
    'Documentation': get_option('documentation'),
    'Tests': get_option('tests'),
    'Optimized': control_center_optimized,
  },
  bool_yn: true,
)

summary({
    'NetworkManager': host_is_linux,
  },
  section: 'Dependencies',
  bool_yn: true,
)
