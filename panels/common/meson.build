common_inc = include_directories('.')

common_sources = []

common_sources += gnome.mkenums_simple('cc-number-row-enums', sources: files('cc-number-row.h'))
ui_sources = files(
  'cc-content-row.blp',
  'cc-duration-editor.blp',
  'cc-duration-row.blp',
  'cc-illustrated-row.blp',
  'cc-list-row-info-button.blp',
  'cc-list-row.blp',
  'cc-permission-infobar.blp',
  'cc-split-row.blp',
  'cc-time-editor.blp',
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

common_sources += gnome.compile_resources(
  'cc-common-resources',
  'common.gresource.xml',
  c_name: 'cc_common',
  export: true,
  dependencies: blueprints,
)

generates_sources_dep = declare_dependency(
  sources: common_sources,
)

sources = files(
  'cc-hostname-entry.c',
  'cc-number-row.c',
  'cc-timelike-entry.c',
  'cc-util.c',
  'hostname-helper.c'
)

libwidgets = static_library(
  'widgets',
  sources: sources,
  include_directories: top_inc,
  dependencies: common_deps + [ generates_sources_dep, polkit_gobject_dep ]
)
libwidgets_dep = declare_dependency(
  include_directories: common_inc,
  link_with: libwidgets
)

sources = common_sources + files(
  'cc-duration-editor.c',
  'cc-duration-row.c',
  'cc-hostname.c',
  'cc-illustrated-row.c',
  'cc-list-row.c',
  'cc-list-row-info-button.c',
  'cc-mask-paintable.c',
  'cc-time-editor.c',
  'cc-permission-infobar.c',
  'cc-split-row.c',
  'cc-texture-utils.c',
  'cc-content-row.c',
  'cc-ui-util.c',
  'cc-util.c',
  'cc-scaler.c',
  'gsd-common-enums.c',
  'gsd-device-manager.c',
  'gsd-input-helper.c',
  'stub-languages.c',
  'stub-wall-clock.c',
  'stub-xkb-info.c',
)

deps = common_deps + [
  generates_sources_dep,

  dependency('fontconfig'),
  dependency('gudev-1.0'),
]

liblanguage = static_library(
  'language',
  sources: sources,
  include_directories: top_inc,
  dependencies: deps
)

liblanguage_dep = declare_dependency(
  include_directories: common_inc,
  link_with: liblanguage
)

polkit_conf = configuration_data()
polkit_conf.set('PRIVILEGED_GROUP', get_option('privileged_group'))
configure_file(
  input: 'hypr-control-center.rules.in',
  output: 'hypr-control-center.rules',
  configuration: polkit_conf,
  install_dir: join_paths(control_center_datadir, 'polkit-1', 'rules.d')
)
