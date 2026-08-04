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
#define MAX_VOICES 10
#define MAX_CHORDS 32
#define MAX_CURVES 32

#define MIN_BEATMS 50
#define DFLT_BEATMS 500 //120bpm

#define DFLT_TIMEINTERVAL 20

#define DFLT_POWEXP 3.0
#define MATH_PI 3.14159265358979323846

enum CURVE_TASK { CV_NOTHING, CV_CURVE, CV_END };
enum WRAPMODES { WM_CLAMP, WM_REPEAT, WM_MIRROR, WM_TOTAL };
enum NOTEINPUT {
	F_DEFINEMELODY,	// a
	F_DEFINECHORD	// v
};
enum NOTECURVES {
	NC_NONE,
	NC_LIN,		// y = x			// /x	lineal
	NC_EI_COS,	// y = cos(x)		// /it	ease in ("speeding-up" function)
	NC_EI_POWO,	// y = x^a (a>1)	// /ip
	NC_EI_POWU,	// y = x^a (a<1)	// /ir
	NC_EI_CIRC,	// y^2 - x^2		// /ic
	NC_EO_SIN,	// y = sin(x)		// /ot	ease out ("slowing-down" function)
	NC_EO_POWU,	// y = x^a (a<1)	// /or
	NC_EO_POWO,	// y = x^a (a>1)	// /op
	NC_EO_CIRC,	// y^2 - x^2		// /oc
	NC_EIO_COS,	// y = cos			// /st	ease in-out (sigmoid-shaped function)
	NC_EIO_POW,	// y = x^a			// /sp 
	NC_EIO_CIRC,// y^2 - x^2		// /sc
	NC_EOI_ACOS,// y = acos			// /lt	ease out-in (logit-shaped function)
	NC_EOI_POW,	// y = x^a			// /lr 
	NC_EOI_CIRC	// y^2 - x^2		// /lc
};
/*
i:ease in; o:ease out; s:sigmoid; l:logit
t:trigonom; c:circle; p:power; r:root
*/

typedef struct _fl_beat {
	float dur_beat;
	float start_beat;
}fl_beat;
typedef struct _fl_curve {
	short type;
	double ampl;
}fl_curve;
typedef struct _fl_note {
	t_atom note;
	short chord_idx; //-1 flag: has no chord
	short curve_idx; //-1 flag: has no curve 
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
	
	fl_curve *old_curves;
	fl_curve *new_curves;
	short total_old_curves;
	short total_new_curves;
	short index_old_curves;

	short curve_type;
	double curve_amp;
	short curve_task;
	long start_curve;
	t_atom_long hit_ms;

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
	void *m_outlet4;
	void *m_clock;

} t_fl_maza;

enum INLETS { I_INPUT, NUM_INLETS };
enum OUTLETS { O_NOTE, O_DUR, O_PITCHBEND, O_FINALFLAG, NUM_OUTLETS };

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
long idx_wrap(short mode, long boundary, long n);
int is_pure_float(const char *token);
void parse_flagged_token(const char *token, char *prefix_out, float *val_out);
double easing_curves(short curve_type, double norm_hit);
#endif
