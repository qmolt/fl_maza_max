#ifndef fl_maza_h
#define fl_maza_h

#include "ext.h"
#include "ext_obex.h"
#include "ext_time.h"
#include <math.h>

#define DFLT_BEAT 1.0f

#define DFLT_LOOPMODE 0

#define MAX_NOTES 127
#define MAX_HITS 127
#define MIN_BEATMS 50
#define DFLT_BEATMS 500 //120bpm

#define DFLT_TIMEINTERVAL 20

#define MAX_VOICES 10
#define MAX_CHORDS 32

enum WRAPMODES { WM_CLAMP, WM_REPEAT, WM_MIRROR, WM_TOTAL };
enum NOTETYPES { N_NOTE, N_CHORDIDX };
enum NOTEINPUT {
	F_DEFINEMELODY,	// a
	F_DEFINECHORD	// v
};

typedef struct _fl_beat {
	float dur_beat;
	float start_beat;
}fl_beat;
typedef struct _fl_note {
	short type;
	t_atom note;
	short chord_idx;
}fl_note;

typedef struct _fl_chord {
	t_atom *notes;
	short voices;
}fl_chord;

typedef struct _fl_maza {
	t_object obj;
	
	fl_chord *old_chords;
	fl_chord *new_chords;
	short total_old_chords;
	short total_new_chords;
	short index_old_chords;

	fl_note *old_notes;
	fl_note *new_notes;
	long total_old_notes;
	long total_new_notes;
	long index_old_notes;

	short wrap_mode;
	
	fl_beat *old_hits;
	fl_beat *new_hits;
	long total_old_hits;
	long total_new_hits;
	long index_old_hits;

	float old_timesig;
	float new_timesig;

	long beat_ms;
	long time;
	long interval;

	short loop_mode;

	void *m_outlet1;
	void *m_outlet2;
	void *m_outlet3;
	void *m_clock;

} t_fl_maza;

enum INLETS { I_INPUT, NUM_INLETS };
enum OUTLETS { O_NOTE, O_DUR, O_FINALFLAG, NUM_OUTLETS };

static t_class *fl_maza_class;

void *fl_maza_new(t_symbol *s, short argc, t_atom *argv);
void fl_maza_float(t_fl_maza *x, double f);
void fl_maza_bang(t_fl_maza *x);
void fl_maza_int(t_fl_maza *x, long n);
void fl_maza_assist(t_fl_maza *x, void *b, long msg, long arg, char *dst);

void fl_maza_bar(t_fl_maza *x, t_symbol *msg, short argc, t_atom *argv);
void fl_maza_loop(t_fl_maza *x, t_symbol *msg, short argc, t_atom *argv);
void fl_maza_beatms(t_fl_maza *x, t_symbol *msg, short argc, t_atom *argv);
void fl_maza_wrapmode(t_fl_maza *x, t_symbol *msg, short argc, t_atom *argv);
void fl_maza_tick(t_fl_maza *x);

void fl_maza_free(t_fl_maza *x);

long z_mod(long x, long base);
int is_pure_float(const char *token);
long idx_wrap(short mode, long boundary, long n);
#endif
