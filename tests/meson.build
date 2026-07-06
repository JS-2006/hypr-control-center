setxkbmap = find_program('setxkbmap', required: false)
Xvfb = find_program('Xvfb', required: false)

subdir('common')
#subdir('datetime')

# FIXME: network tests are disabled due to CI failure
#if host_is_linux
#  subdir('network')
#endif

subdir('interactive-panels')

subdir('printers')
subdir('keyboard')
