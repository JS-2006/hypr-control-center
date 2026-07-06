hypr_backend_inc = include_directories('.')

hypr_backend_sources = files(
  'hypr-backend.c',
)

hypr_backend_deps = [
  dependency('json-glib-1.0'),
  dependency('gio-unix-2.0'),
  glib_dep,
]

libhypr_backend = static_library(
  'hypr-backend',
  sources: hypr_backend_sources,
  include_directories: [top_inc],
  dependencies: hypr_backend_deps,
)

hypr_backend_dep = declare_dependency(
  include_directories: hypr_backend_inc,
  link_with: libhypr_backend,
  dependencies: hypr_backend_deps,
)
