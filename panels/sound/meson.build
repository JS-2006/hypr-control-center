panels_list += cappletname
desktop = 'hypr-@0@-panel.desktop'.format(cappletname)

i18n.merge_file(
  type: 'desktop',
  input: desktop + '.in',
  output: desktop,
  po_dir: po_dir,
  install: true,
  install_dir: control_center_desktopdir
)

deps = common_deps + [
  pulse_dep,
  pulse_mainloop_dep,
]

cflags += [
  '-DSOUND_DATA_DIR="@0@"'.format(join_paths(control_center_datadir, 'sounds')),
]

ui_sources = files(
  'cc-sound-panel.blp',
)

ui_compiled = []
foreach ui_source : ui_sources
  ui_compiled += fs.name(fs.replace_suffix(ui_source, '.ui'))
endforeach

blueprints = custom_target(
  input: ui_sources,
  output: ui_compiled,
  command: [blueprint_compiler, 'batch-compile', '@OUTDIR@', '@CURRENT_SOURCE_DIR@', '@INPUT@']
)

sources = files(
  'cc-pa-helpers.c',
  'cc-sound-panel.c',
)

sources += gnome.compile_resources(
  'cc-' + cappletname + '-resources',
  cappletname + '.gresource.xml',
  c_name: 'cc_' + cappletname.underscorify (),
  export: true,
  dependencies: blueprints,
)

panels_libs += static_library(
  cappletname,
  sources: sources,
  include_directories: [top_inc, common_inc],
  dependencies: deps,
  c_args: cflags,
)

sound_data = files(
  'sounds/click.ogg',
  'sounds/hum.ogg',
  'sounds/string.ogg',
  'sounds/swing.ogg'
)

install_data(
  sound_data,
  install_dir: join_paths(control_center_datadir, 'sounds', 'gnome', 'default', 'alerts')
)

install_data(
  'icons/org.gnome.Settings-sound-symbolic.svg',
  install_dir: join_paths(control_center_icondir, 'hicolor', 'scalable', 'apps')
)
