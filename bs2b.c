/*
    Bauer stereophonic-to-binaural plugin for DeaDBeeF
    Copyright (C) 2010 Steven McDonald <steven.mcdonald@libremail.me>
    Uses libbs2b by Boris Mikhaylov <http://bs2b.sourceforge.net/>

    This program is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public License
    as published by the Free Software Foundation; either version 2
    of the License, or (at your option) any later version.
    
    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.
    
    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
*/

#include <deadbeef/deadbeef.h>
#include <bs2b/bs2b.h>

//#define trace(...) { fprintf(stderr, __VA_ARGS__); }
#define trace(fmt,...)

static DB_dsp_t plugin;
DB_functions_t *deadbeef;

static t_bs2bdp dp = 0;
static short enabled = 0;
static uint32_t level = ( (uint32_t)700 | ((uint32_t)45 << 16) );
static int savedrate = 44100;

static void
bs2b_reset (void);

static int
bs2b_on_configchanged (DB_event_t *ev, uintptr_t data) {
    int e = deadbeef->conf_get_int ("bs2b.enable", 0);
    if (e != enabled) {
        if (e) {
            bs2b_reset ();
        }
        enabled = e;
    }

    uint32_t l = ( ((uint32_t)deadbeef->conf_get_int ("bs2b.fcut", 700)) | ((uint32_t)(deadbeef->conf_get_float ("bs2b.feed", 4.5) * 10) << 16) );
    if (l != level) {
        bs2b_set_level (dp, l);
        level = l;
    }
    
    return 0;
}

static int
bs2b_plugin_start (void) {
    enabled = deadbeef->conf_get_int ("bs2b.enable", 0);
    level = ( ((uint32_t)deadbeef->conf_get_int ("bs2b.fcut", 700)) | ((uint32_t)(deadbeef->conf_get_float ("bs2b.feed", 4.5) * 10) << 16) );
    deadbeef->ev_subscribe (DB_PLUGIN (&plugin), DB_EV_CONFIGCHANGED, DB_CALLBACK (bs2b_on_configchanged), 0);
    dp = bs2b_open ();
    bs2b_set_level (dp, level);
}

static int
bs2b_plugin_stop (void) {
    deadbeef->ev_unsubscribe (DB_PLUGIN (&plugin), DB_EV_CONFIGCHANGED, DB_CALLBACK (bs2b_on_configchanged), 0);
    bs2b_close (dp);
}

static int
bs2b_process_int16 (int16_t *samples, int nsamples, int nch, int bps, int srate) {
    if (nch != 2) return nsamples;
    if (srate != savedrate) {
        bs2b_set_srate (dp, (uint32_t)srate);
        savedrate = srate;
    }
    bs2b_cross_feed_s16 (dp, samples, nsamples);
    return nsamples;
}

static void
bs2b_reset (void) {
    bs2b_clear (dp);
    return;
}

static void
bs2b_enable (int e) {
    if (e != enabled) {
        deadbeef->conf_set_int ("bs2b.enable", e);
        if (e && !enabled) {
            bs2b_reset ();
        }
        enabled = e;
    }
    return;
}

static int
bs2b_enabled (void) {
    return enabled;
}

static const char settings_dlg[] =
    "property \"Enable\" checkbox bs2b.enable 0;\n"
    "property \"Crossfeed level (dB)\" hscale[1.0,15.0,0.1] bs2b.feed 4.5;\n"
    "property \"Cutoff filter (Hz)\" hscale[100,2000,1] bs2b.fcut 700;\n"
;

static DB_dsp_t plugin = {
    .plugin.api_vmajor = DB_API_VERSION_MAJOR,
    .plugin.api_vminor = DB_API_VERSION_MINOR,
    .plugin.type = DB_PLUGIN_DSP,
    .plugin.id = "bs2b",
    .plugin.name = "Headphone crossfeed",
    .plugin.descr = "Headphone crossfeed plugin using libbs2b by Boris Mikhaylov",
    .plugin.author = "Steven McDonald",
    .plugin.email = "steven.mcdonald@libremail.me",
    .plugin.website = "http://deadbeef.sf.net",
    .plugin.start = bs2b_plugin_start,
    .plugin.stop = bs2b_plugin_stop,
    .plugin.configdialog = settings_dlg,
    .process_int16 = bs2b_process_int16,
    .reset = bs2b_reset,
    .enable = bs2b_enable,
    .enabled = bs2b_enabled,
};

DB_plugin_t *
bs2b_load (DB_functions_t *api) {
    deadbeef = api;
    return DB_PLUGIN (&plugin);
}
