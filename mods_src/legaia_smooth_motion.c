/*
 * Legaia Recomp: Smooth Motion (game-owned mod, default off).
 *
 * Presents the game above its own frame rate by blending completed guest
 * frames in the OpenGL presenter. Guest VBlank, game logic, timers and audio
 * keep their stock cadence, so game speed never changes. This is a crossfade,
 * not true motion: fast objects can ghost slightly. See
 * psxrecomp/docs/FRAME_RATE.md.
 *
 * Activation only calls the framework's presentation setters
 * (psx_mod_set_frame_interpolation*); it never touches guest memory.
 */
#include "mod_plugins.h"

#include <stdlib.h>
#include <string.h>

#define PKG_SMOOTH "legaia.enhancement.smooth-motion"
#define FEAT_SMOOTH "smooth-motion"

static void smooth_option(const char* id, char* out, unsigned size) {
    out[0] = '\0';
    (void)psx_mod_option_value(PKG_SMOOTH, FEAT_SMOOTH, id, out, size);
}

static void legaia_smooth_motion_activate(void) {
    char rate[16], blend[24];
    smooth_option("rate", rate, sizeof rate);
    smooth_option("blend", blend, sizeof blend);

    /* "display" (or anything unreadable) follows the monitor refresh rate. */
    uint32_t fps = 0;
    if (rate[0] && strcmp(rate, "display") != 0) {
        const long v = strtol(rate, NULL, 10);
        if (v >= 60 && v <= 1000) fps = (uint32_t)v;
    }

    uint32_t mode = PSX_MOD_FRAME_INTERPOLATION_LINEAR;
    if (strcmp(blend, "motion_adaptive") == 0)
        mode = PSX_MOD_FRAME_INTERPOLATION_MOTION_ADAPTIVE;

    /* The game flips at its own (often 30 Hz) rate; FLIP spreads each
     * crossfade over the whole game frame instead of one VBlank. */
    (void)psx_mod_set_frame_interpolation_source(PSX_MOD_FRAME_SOURCE_FLIP);
    (void)psx_mod_set_frame_interpolation_blend(mode);
    (void)psx_mod_set_frame_interpolation(fps);
}

PSX_MOD_CONSTRUCTOR(psx_register_legaia_smooth_motion_plugin) {
    (void)psx_mod_register_activation_plugin("legaia.smooth-motion",
                                             legaia_smooth_motion_activate);
}

/*
 * Legaia Recomp: Widescreen (EXPERIMENTAL, default off).
 *
 * The framework's "widescreen hack": the 3D projection is squashed and the
 * picture presented stretched to a wider aspect, so more of the scene is
 * visible. Nothing here is tuned for Legaia: expect objects popping at the
 * screen edges and stretched or misplaced HUD/menus in some scenes. BIOS logos
 * and movies stay 4:3. Presentation-only; saves are unaffected.
 */
#define PKG_WIDE "legaia.enhancement.widescreen"
#define FEAT_WIDE "widescreen"

static void wide_option(const char* id, char* out, unsigned size) {
    out[0] = '\0';
    (void)psx_mod_option_value(PKG_WIDE, FEAT_WIDE, id, out, size);
}

static void legaia_widescreen_activate(void) {
    char aspect[16], hud[16];
    wide_option("aspect", aspect, sizeof aspect);
    wide_option("hud_size", hud, sizeof hud);

    uint32_t num = 16, den = 9;
    int fit = 0;
    if (strcmp(aspect, "21:9") == 0) { num = 21; den = 9; }
    else if (strcmp(aspect, "fit") == 0) { fit = 1; }

    (void)psx_mod_set_fixed_display_aspect(num, den);
    if (fit) (void)psx_mod_set_adaptive_display_aspect(0u, 0u);
    (void)psx_mod_set_widescreen_hud_size(strcmp(hud, "proportional") == 0 ? 1 : 0);
}

PSX_MOD_CONSTRUCTOR(psx_register_legaia_widescreen_plugin) {
    (void)psx_mod_register_activation_plugin("legaia.widescreen",
                                             legaia_widescreen_activate);
}
