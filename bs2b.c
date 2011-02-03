/*
    Bauer stereophonic-to-binaural plugin for DeaDBeeF
    Copyright (C) 2010 Steven McDonald <steven.mcdonald@libremail.me>
    See COPYING file for modification and redistribution conditions.
*/

#include <deadbeef/deadbeef.h>
#include <bs2b/bs2b.h>
#include <stdlib.h>

//#define trace(...) { fprintf(stderr, __VA_ARGS__); }
#define trace(fmt,...)

static DB_dsp_t plugin;
DB_functions_t *deadbeef;

/*
static t_bs2bdp dp = 0;
static short enabled = 0;
static uint32_t level = ( (uint32_t)700 | ((uint32_t)45 << 16) );
static int savedrate = 44100;
*/

enum {
    BS2B_PARAM_XFEED = 0,
    BS2B_PARAM_CUTOFF = 1,
    BS2B_PARAM_COUNT
};

typedef struct {
    ddb_dsp_context_t ctx;

    t_bs2bdp dp;
    uint16_t xfeed;
    uint16_t cutoff;
    int savedrate;
} ddb_bs2b_t;

//void
//ddb_bs2b_reset (void);

/* int
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
} */

ddb_dsp_context_t*
ddb_bs2b_open (void) {
    ddb_bs2b_t *bs2b = malloc (sizeof (ddb_bs2b_t));
    DDB_INIT_DSP_CONTEXT (bs2b, ddb_bs2b_t, &plugin);

    bs2b->xfeed = 700;
    bs2b->cutoff = 45;
    bs2b->savedrate = 44100;
    bs2b->dp = bs2b_open ();
//    bs2b_set_level (dp, level);
    return (ddb_dsp_context_t *)bs2b;
}

void
ddb_bs2b_close (ddb_dsp_context_t *_bs2b) {
    ddb_bs2b_t *bs2b = (ddb_bs2b_t*)_bs2b;
    bs2b_close (bs2b->dp);
    free (bs2b);
}

int
ddb_bs2b_process (ddb_dsp_context_t *_bs2b, float *samples, int frames, int maxframes, ddb_waveformat_t *fmt, float *ratio) {
    ddb_bs2b_t *bs2b = (ddb_bs2b_t*)_bs2b;

    if (fmt->channels != 2) return frames;
    if (fmt->samplerate != bs2b->savedrate) {
        bs2b_set_srate (bs2b->dp, (uint32_t)fmt->samplerate);
        bs2b->savedrate = fmt->samplerate;
    }
    bs2b_cross_feed_f (bs2b->dp, samples, frames);
    return frames;
}

void
ddb_bs2b_reset (ddb_dsp_context_t *ctx) {
    bs2b_clear (((ddb_bs2b_t*)ctx)->dp);
    return;
}

/* void
bs2b_enable (int e) {
    if (e != enabled) {
        deadbeef->conf_set_int ("bs2b.enable", e);
        if (e && !enabled) {
            bs2b_reset ();
        }
        enabled = e;
    }
    return;
}*/

int
ddb_bs2b_num_params (void) {
    return BS2B_PARAM_COUNT;
}

const char *
ddb_bs2b_get_param_name (int p) {
    switch (p) {
    case BS2B_PARAM_XFEED:
        return "Crossfeed level (dB)";
    case BS2B_PARAM_CUTOFF:
        return "Cutoff filter (Hz)";
    default:
        fprintf (stderr, "ddb_bs2b_get_param_name: invalid param index (%d)\n", p);
    }
}

void
ddb_bs2b_set_param (ddb_dsp_context_t *_bs2b, int p, const char *val) {
    ddb_bs2b_t *bs2b = (ddb_bs2b_t*)_bs2b;

    switch (p) {
    case BS2B_PARAM_XFEED:
        bs2b->xfeed = (uint16_t)(atof (val) * 10);
        break;
    case BS2B_PARAM_CUTOFF:
        bs2b->cutoff = atoi (val);
        break;
    default:
        fprintf (stderr, "ddb_bs2b_set_param: invalid param index (%d)\n", p);
        return;
    }

    bs2b_set_level (bs2b->dp, (uint32_t)bs2b->xfeed | ((uint32_t)bs2b->cutoff << 16));
}

void
ddb_bs2b_get_param (ddb_dsp_context_t *_bs2b, int p, char *val, int sz) {
    ddb_bs2b_t *bs2b = (ddb_bs2b_t*)_bs2b;

    switch (p) {
    case BS2B_PARAM_XFEED:
        snprintf (val, sz, "%f", (float)bs2b->xfeed / 10);
        break;
    case BS2B_PARAM_CUTOFF:
        snprintf (val, sz, "%d", bs2b->cutoff);
        break;
    default:
        fprintf (stderr, "ddb_bs2b_get_param: invalid param index (%d)\n", p);
    }
}

static const char ddb_bs2b_dialog[] =
//    "property \"Enable\" checkbox bs2b.enable 0;\n"
    "property \"Crossfeed level (dB)\" hscale[1.0,15.0,0.1] 0 4.5;\n"
    "property \"Cutoff filter (Hz)\" hscale[100,2000,1] 1 700;\n"
;

static DB_dsp_t plugin = {
    DB_PLUGIN_SET_API_VERSION
    .plugin.version_major = 0,
    .plugin.version_minor = 2,
    .plugin.type = DB_PLUGIN_DSP,
    .plugin.id = "bs2b",
    .plugin.name = "Headphone crossfeed",
    .plugin.descr = "Headphone crossfeed plugin using libbs2b by Boris Mikhaylov",
    .plugin.author = "Steven McDonald",
    .plugin.email = "steven.mcdonald@libremail.me",
    .plugin.website = "http://gitorious.org/deadbeef-sm-plugins/pages/Home",
    .num_params = ddb_bs2b_num_params,
    .get_param_name = ddb_bs2b_get_param_name,
    .set_param = ddb_bs2b_set_param,
    .get_param = ddb_bs2b_get_param,
    .configdialog = ddb_bs2b_dialog,
    .open = ddb_bs2b_open,
    .close = ddb_bs2b_close,
    .process = ddb_bs2b_process,
    .reset = ddb_bs2b_reset,
};

DB_plugin_t *
bs2b_load (DB_functions_t *api) {
    deadbeef = api;
    return DB_PLUGIN (&plugin);
}
