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

keyboard_inc = include_directories('.')

libkeyboard_shortcuts = static_library(
  'keyboard-shortcuts',
  sources: 'keyboard-shortcuts.c',
  include_directories: [top_inc, common_inc],
  dependencies: common_deps + [generates_sources_dep],
)

keyboard_shortcuts_dep = declare_dependency(
  include_directories: [top_inc, keyboard_inc],
  link_with: libkeyboard_shortcuts,
)

sources = files(
  'cc-ibus-utils.c',
  'cc-input-chooser.c',
  'cc-input-list-box.c',
  'cc-input-row.c',
  'cc-input-source-ibus.c',
  'cc-input-source-xkb.c',
  'cc-input-source.c',
  'cc-keyboard-item.c',
  'cc-keyboard-manager.c',
  'cc-keyboard-panel.c',
  'cc-keyboard-shortcut-editor.c',
  'cc-keyboard-shortcut-group.c',
  'cc-keyboard-shortcut-page.c',
  'cc-keyboard-shortcut-row.c',
  'cc-xkb-modifier-page.c',
)

keyboard_previewer = find_program('gkbd-keyboard-display', required: false)
if keyboard_previewer.found()
  keyboard_previewer_path = keyboard_previewer.full_path()
else
  keyboard_previewer_path = ''
endif

common_sources = []

ui_sources = files(
  'cc-input-chooser.blp',
  'cc-input-list-box.blp',
  'cc-input-row.blp',
  'cc-keyboard-panel.blp',
  'cc-keyboard-shortcut-editor.blp',
  'cc-keyboard-shortcut-group.blp',
  'cc-keyboard-shortcut-page.blp',
  'cc-keyboard-shortcut-row.blp',
  'cc-xkb-modifier-page.blp',
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
  'cc-@0@-resources'.format(cappletname),
  cappletname + '.gresource.xml',
  c_name: 'cc_' + cappletname,
  export: true,
  dependencies: blueprints,
)

sources += common_sources

deps = common_deps + [
  keyboard_shortcuts_dep,
  dependency('accountsservice'),
  dependency('polkit-gobject-1'),
  dependency('ibus-1.0'),
  dependency('gtk4-wayland'),
]

cflags += [
  '-DHAVE_IBUS',
  '-DKEYBOARD_PREVIEWER_EXEC="@0@"'.format(keyboard_previewer_path),
]

panels_libs += static_library(
  cappletname,
  sources: sources,
  include_directories: [top_inc, common_inc, keyboard_inc],
  dependencies: deps,
  c_args: cflags,
)

subdir('icons')
