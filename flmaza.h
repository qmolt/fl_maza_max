#ifndef fl_maza_h
#define fl_maza_h

#include "ext.h"
#include "ext_obex.h"
#include "ext_time.h"
#include <math.h>

#define DFLT_BEAT 1.0f

#define MAX_NOTES_SIZE 127
#define MAX_HITS_SIZE 127
#define MIN_BEATMS 50
#define DFLT_BEATMS 500 //120bpm

#define DFLT_OCT_DIV 12
#define DFLT_TIMEINTERVAL 20
#define DFLT_POWEXP 3.0

#define MATH_PI 3.14159265358979323846

enum WRAPMODES {WM_CLAMP, WM_REPEAT, WM_MIRROR, WM_TOTAL};
enum FILTERMODES {FM_REAL, FM_NAT};// /f0 /f1
enum NOTECURVES {
	NC_SET,		// 
	NC_FLAT,	// y = a			// /a	1: constant 
	NC_LIN,		// y = x			// /x	2: lineal
	NC_EI_COS,	// y = cos(x)		// /it	3: ease in ("speeding-up" function)
	NC_EI_POWO,	// y = x^a (a>1)	// /ip
	NC_EI_POWU,	// y = x^a (a<1)	// /ir
	NC_EI_CIRC,	// y^2 - x^2		// /ic
	NC_EO_SIN,	// y = sin(x)		// /ot	7: ease out ("slowing-down" function)
	NC_EO_POWU,	// y = x^a (a<1)	// /or
	NC_EO_POWO,	// y = x^a (a>1)	// /op
	NC_EO_CIRC,	// y^2 - x^2		// /oc
	NC_EIO_COS,	// y = cos			// /st	11: ease in-out (sigmoid-shaped function)
	NC_EIO_POW,	// y = x^a			// /sp 
	NC_EIO_CIRC,// y^2 - x^2		// /sc
	NC_EOI_ACOS,// y = acos			// /lt	14: ease out-in (logit-shaped function)
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
	float start;
	float end;
	short curve_type;
	short filter_mode;
}fl_note;

typedef struct _fl_maza {
	t_object obj;

	long oct_div;
	
	short curve_task;
	short dur_task;
	short end_task;
	short curve_type;
	float curve_start;
	float curve_end;
	short filter_mode;
	float dur_beat;
	float start_beat;

	fl_note *old_notes;
	fl_note *new_notes;
	long total_old_notes;
	long total_new_notes;
	long index_old_notes;
	short wrap_mode;
	short filter_toggle;

	fl_beat *old_hits;
	fl_beat *new_hits;
	long total_old_hits;
	long total_new_hits;
	float old_cifra; 
	float new_cifra;
	long index_old_hits;

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
enum OUTLETS { O_DUR, O_NOTE, O_FINALFLAG, NUM_OUTLETS };

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
#endif
