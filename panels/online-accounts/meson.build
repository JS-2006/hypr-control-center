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

cflags += [
  '-DLIBEXECDIR="@0@"'.format(control_center_libexecdir),
]

ui_sources = files(
  'cc-online-account-provider-row.blp',
  'cc-online-account-row.blp',
  'cc-online-accounts-panel.blp',
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
  'cc-online-account-provider-row.c',
  'cc-online-account-row.c',
  'cc-online-accounts-panel.c',
)

sources += gnome.compile_resources(
  'cc-' + cappletname + '-resources',
  cappletname + '.gresource.xml',
  c_name: 'cc_' + cappletname.underscorify(),
  export: true,
  dependencies: blueprints,
)

deps = common_deps + [
  goa_dep,
  goa_backend_dep,
]

panels_libs += static_library(
  cappletname,
  sources: sources,
  include_directories: [ top_inc, common_inc ],
  dependencies: deps,
  c_args: cflags
)

subdir('icons')
