#pragma once

#include <shell/cc-panel.h>

G_BEGIN_DECLS

#define CC_TYPE_SOUND_PANEL (cc_sound_panel_get_type ())
G_DECLARE_FINAL_TYPE (CcSoundPanel, cc_sound_panel, CC, SOUND_PANEL, CcPanel);

G_END_DECLS
