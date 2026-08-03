#include "flmaza.h"

void ext_main(void *r)
{
	t_class *c = class_new("flmaza", (method)fl_maza_new, (method)fl_maza_free, sizeof(t_fl_maza), (method)NULL, A_GIMME, 0);

	class_addmethod(c, (method)fl_maza_float, "float", A_FLOAT, 0);
	class_addmethod(c, (method)fl_maza_int, "int", A_LONG, 0);
	class_addmethod(c, (method)fl_maza_bang, "bang", 0);
	class_addmethod(c, (method)fl_maza_assist, "assist", A_CANT, 0);

	class_addmethod(c, (method)fl_maza_bar, "bar", A_GIMME, 0);
	class_addmethod(c, (method)fl_maza_loop, "loop", A_GIMME, 0);
	class_addmethod(c, (method)fl_maza_beatms, "ms_beat", A_GIMME, 0);
	class_addmethod(c, (method)fl_maza_wrapmode, "wrapmode", A_GIMME, 0);

	class_register(CLASS_BOX, c);
	fl_maza_class = c;
}

void *fl_maza_new(t_symbol *s, short argc, t_atom *argv)
{
	t_fl_maza *x = (t_fl_maza *)object_alloc(fl_maza_class);

	x->m_outlet4 = outlet_new((t_object *)x, "bang");	//final bang
	x->m_outlet3 = outlet_new((t_object *)x, "float");	//pitchbend
	x->m_outlet2 = outlet_new((t_object *)x, "int");	//dur
	x->m_outlet1 = outlet_new((t_object *)x, "list");	//note

	x->total_old_hits = 0;
	x->total_new_hits = 0;
	x->old_timesig = 0.;
	x->new_timesig = 0.;
	x->index_old_hits = 0;

	x->total_old_notes = 0;
	x->total_new_notes = 0;
	
	x->total_old_chords = 0;
	x->total_new_chords = 0;
	x->index_old_chords = 0;

	x->beat_ms = DFLT_BEATMS;
	x->m_clock = clock_new(x, (method)fl_maza_tick);
	x->time = (long)gettime_forobject((t_object *)x);
	x->interval = DFLT_TIMEINTERVAL; //interval < 5 ? 5 : interval;

	x->loop_mode = DFLT_LOOPMODE;

	x->index_old_notes = 0;

	x->wrap_mode = WM_CLAMP;

	x->curve_type = -1;
	x->curve_amp = 0.0;

	x->old_chords = (fl_chord *)sysmem_newptr(MAX_CHORDS * sizeof(fl_chord));
	if (!x->old_chords) { object_error((t_object *)x, "fl_maza_new: no memory space for old_chords list"); return x; }
	for (long i = 0; i < MAX_CHORDS; i++) {
		x->old_chords[i].voices = 0;
		x->old_chords[i].notes = (t_atom *)sysmem_newptr(MAX_VOICES * sizeof(t_atom));
		if (!x->old_chords[i].notes) { object_error((t_object *)x, "fl_maza_new: memory alloc for old_chords[i].notes list failed"); return x; }
		for (long j = 0; j < MAX_VOICES; j++) {
			atom_setfloat(x->old_chords[i].notes + j, 0.0f);
		}
	}
	x->new_chords = (fl_chord *)sysmem_newptr(MAX_CHORDS * sizeof(fl_chord));
	if (!x->new_chords) { object_error((t_object *)x, "fl_maza_new: no memory space for new_chords list"); return x; }
	for (long i = 0; i < MAX_CHORDS; i++) {
		x->new_chords[i].voices = 0;
		x->new_chords[i].notes = (t_atom *)sysmem_newptr(MAX_VOICES * sizeof(t_atom));
		if (!x->new_chords[i].notes) { object_error((t_object *)x, "fl_maza_new: memory alloc for old_chords[i].notes list failed"); return x; }
		for (long j = 0; j < MAX_VOICES; j++) {
			atom_setfloat(x->new_chords[i].notes + j, 0.0f);
		}
	}

	x->old_notes = (fl_note *)sysmem_newptr(MAX_NOTES * sizeof(fl_note));
	if (!x->old_notes) { object_error((t_object *)x, "fl_maza_new: no memory space for old_notes list"); return x; }
	for (long i = 0; i < MAX_NOTES; i++) {
		atom_setfloat(&x->old_notes[i].note, 0.0f);
		x->old_notes[i].chord_idx = -1;
		x->old_notes[i].curve_idx = -1;

	}
	x->new_notes = (fl_note *)sysmem_newptr(MAX_NOTES * sizeof(fl_note));
	if (!x->new_notes) { object_error((t_object *)x, "fl_maza_new: no memory space for new_notes list"); return x; }
	for (long i = 0; i < MAX_NOTES; i++) {
		atom_setfloat(&x->new_notes[i].note, 0.0f);
		x->new_notes[i].chord_idx = -1;
		x->new_notes[i].curve_idx = -1;
	}

	x->old_curves = (fl_curve *)sysmem_newptr(MAX_CURVES * sizeof(fl_curve));
	if (!x->old_curves) { object_error((t_object *)x, "fl_maza_new: no memory space for old_curves list"); return x; }
	for (long i = 0; i < MAX_CURVES; i++) {
		x->old_curves[i].type = NC_LIN;
		x->old_curves[i].ampl = 0.0;
	}
	x->new_curves = (fl_curve *)sysmem_newptr(MAX_CURVES * sizeof(fl_curve));
	if (!x->new_curves) { object_error((t_object *)x, "fl_maza_new: no memory space for new_curves list"); return x; }
	for (long i = 0; i < MAX_CURVES; i++) {
		x->new_curves[i].type = NC_LIN;
		x->new_curves[i].ampl = 0.0;
	}

	x->old_hits = (fl_beat *)sysmem_newptr(MAX_HITS * sizeof(fl_beat));
	if (!x->old_hits) { object_error((t_object *)x, "fl_maza_new: no memory space for old_hits list"); return x; }
	for (long i = 0; i < MAX_HITS; i++) {
		x->old_hits[i].dur_beat = 0.0;
		x->old_hits[i].start_beat = 0.0;
	}
	x->new_hits = (fl_beat *)sysmem_newptr(MAX_HITS * sizeof(fl_beat));
	if (!x->new_hits) { object_error((t_object *)x, "fl_maza_new: no memory space for new_hits list"); return x; }
	for (long i = 0; i < MAX_HITS; i++) {
		x->new_hits[i].dur_beat = 0.0;
		x->new_hits[i].start_beat = 0.0;
	}

	return x;
}

void fl_maza_assist(t_fl_maza *x, void *b, long msg, long arg, char *dst)
{
	if (msg == ASSIST_INLET) {
		switch (arg) {
		case I_INPUT: sprintf(dst, "(bang)on; (int,float)on/off; (messages)bar, ms_beat, wrapmode"); break;
		}
	}
	else if (msg == ASSIST_OUTLET) {
		switch (arg) {
		case O_NOTE:sprintf(dst, "(list) notes"); break;
		case O_DUR: sprintf(dst, "(long) note duration in milliseconds"); break;
		case O_PITCHBEND: sprintf(dst, "(float) pitchbend"); break;
		case O_FINALFLAG: sprintf(dst, "(bang) end flag"); break;
		}
	}
}

void fl_maza_loop(t_fl_maza *x, t_symbol *msg, short argc, t_atom *argv)
{ 
	t_atom *ap = argv;
	long ac = argc;
	short loop;

	if (ac != 1) { object_error((t_object *)x, "loop: only 1 argument"); return; }
	if (atom_gettype(ap) != A_LONG) { object_error((t_object *)x, "loop: argument must be an integer (0/1)"); return; }
	
	loop = (short)atom_getlong(ap);
	x->loop_mode = loop ? 1 : 0;
}

void fl_maza_beatms(t_fl_maza *x, t_symbol *msg, short argc, t_atom *argv)
{
	t_atom *ap = argv;
	long ac = argc;
	long beat_ms;

	if (ac != 1) { object_error((t_object *)x, "ms_beat: only 1 argument"); return; }
	if (atom_gettype(ap) != A_FLOAT && atom_gettype(ap) != A_LONG) { object_error((t_object *)x, "ms_beat: argument must be a number"); return; }

	beat_ms = (long)atom_getlong(ap);
	if (beat_ms < 10.0) { object_error((t_object *)x, "ms_beat: argument must be a positive number greater than 10"); return; }

	x->beat_ms = beat_ms;
}

void fl_maza_wrapmode(t_fl_maza *x, t_symbol *msg, short argc, t_atom *argv)
{
	t_atom *ap = argv;
	long ac = argc;
	short wrapmode;

	if (ac != 1) { object_error((t_object *)x, "wrapmode: only 1 argument"); return; }
	if (atom_gettype(ap) != A_LONG && atom_gettype(ap) != A_FLOAT) { object_error((t_object *)x, "wrapmode: argument must be an integer"); return; }

	wrapmode = (short)atom_getlong(ap);
	if(wrapmode < 0 || wrapmode > WM_TOTAL) { object_error((t_object *)x, "wrapmode: argument must be a positive number less than %d", WM_TOTAL); return; }
	x->wrap_mode = wrapmode;
}

void fl_maza_bar(t_fl_maza *x, t_symbol *msg, short argc, t_atom *argv)
{
	/*
	//format: 'bar' beat '/a/51/.../' '<div' '/c/51/.../' ...
		ie: bar 4.5 '<1111-101' /a/32/35/v/r/30/v/ /c/35/32/30/c/29/33/ 
		
		1. beat:	(int/float)
		2. div:		<0: silence
					<1: play note
					 -: tie previous note
		3. mel:		/a: note
					/c: chord
					/r: repeat chord
		3.1 curve:	/x	lineal
					/it	/ip /ir /ic		//ease in ("speeding-up" function)
					/ot	/or /op /oc		//ease out ("slowing-down" function)
					/st	/sp /sc			//ease in-out (sigmoid-shaped function)
					/lt	/lr /lc			//ease out-in (logit-shaped function)
		4. chord:	/v: chord
		----------------------------------
	*/
	t_atom *ap = argv;
	long ac = argc;

	char *bar_string;
	long idx_string;

	float acum_bar = 0.0f;
	long acum_hits = 0;
	long acum_notes = 0;
	short acum_chords = 0;
	short acum_voices = 0;
	short acum_def_chords = 0;
	short first_voice = 0;
	short acum_curves = 0;
	short has_curve = 0;

	float beat = DFLT_BEAT;
	long subdiv;
	
	long legatura;
	long idx_null;
	short legatura_flag;

	char *str;
	char *token = NULL;
	char *next_token = NULL;
	long barmemlen;

	x->total_old_hits = 0;
	x->total_old_notes = 0;
	x->total_old_chords = 0;
	x->total_old_curves = 0;

	float note;
	short flag = 0;
	
	for (long i = 0; i < ac; i++) {
		//1. beat
		if (atom_gettype(ap + i) == A_FLOAT || atom_gettype(ap + i) == A_LONG) {
			beat = (float)fabs((double)atom_getfloat(ap + i));
		}
		else if (atom_gettype(ap + i) == A_SYM) {
			bar_string = atom_getsym(ap + i)->s_name;

			//2. div
			if (bar_string[0] == '<') { 
				idx_string = 1;
				while (bar_string[idx_string] != '\0') {idx_string++;}
				
				subdiv = idx_string - 1;
				idx_null = idx_string + 1;
				legatura = 0;
				legatura_flag = 0;
				for (long j = 1; j < idx_null; j++) {
					if (acum_hits > MAX_HITS) {break;}

					if (bar_string[j] == '1') {
						acum_hits++;
						x->new_hits[acum_hits - 1].dur_beat = beat / subdiv;
						x->new_hits[acum_hits - 1].start_beat = acum_bar + beat * (j - 1) / subdiv;
						legatura = 0;
						legatura_flag = 1;
					}
					else if (bar_string[j] == '0') {
						legatura_flag = 0;
					}
					else if (bar_string[j] == '-') {
						legatura++;
						if (legatura_flag) {
							x->new_hits[acum_hits - 1].dur_beat = beat * (legatura + 1) / subdiv;
						}
					}
				}
				acum_bar += beat; //only accumulate bar if <subdiv defined 
			}
			else if (bar_string[0] == '/') {
			
				barmemlen = (long)strlen(bar_string) + 1;

				str = (char *)sysmem_newptr(barmemlen * sizeof(char));
				if (!str) { object_error((t_object *)x, "fl_maza_bar: no memory for str"); continue; }
				
				sysmem_copyptr(bar_string, str, barmemlen * sizeof(char));
					
				token = strtok_s(str, "/", &next_token);
				while (token) {
					if (is_pure_float(token)) {
						note = strtof(token, NULL); 
						//3. mel
						if (flag == F_DEFINEMELODY && acum_notes < MAX_NOTES - 1) {
							acum_notes++;
							x->new_notes[acum_notes - 1].chord_idx = -1;
							atom_setfloat(&x->new_notes[acum_notes - 1].note, note);
							if (has_curve) { x->new_notes[acum_notes - 1].curve_idx = acum_curves - 1; has_curve = 0; }
							else{ x->new_notes[acum_notes - 1].curve_idx = - 1; }
						}
						//4. chord
						else if (flag == F_DEFINECHORD && acum_def_chords < MAX_CHORDS - 1 && acum_voices < MAX_VOICES - 1) {
							if (first_voice) {
								acum_def_chords++;
								first_voice = 0;
							}
							acum_voices++;
							x->new_chords[acum_def_chords - 1].voices = acum_voices;
							atom_setfloat(x->new_chords[acum_def_chords - 1].notes + (acum_voices - 1), note);
						}
					}
					else {
						//melody or chord starter
						if (!strcmp(token, "a")) { 
							flag = F_DEFINEMELODY; 
						}
						else if (!strcmp(token, "v")) { 
							flag = F_DEFINECHORD;
							first_voice = 1;
							acum_voices = 0;
						}
						//any string after /a/
						else if (flag == F_DEFINEMELODY) {
							if (!strcmp(token, "c") && acum_notes < MAX_NOTES - 1 && acum_chords < MAX_CHORDS - 1) {
								acum_chords++;
								acum_notes++;
								x->new_notes[acum_notes - 1].chord_idx = acum_chords - 1;
								atom_setfloat(&x->new_notes[acum_notes - 1].note, 0.0f);
								if (has_curve) { x->new_notes[acum_notes - 1].curve_idx = acum_curves - 1; has_curve = 0; }
								else { x->new_notes[acum_notes - 1].curve_idx = -1; }
							}
							else if (!strcmp(token, "r") && acum_notes < MAX_NOTES - 1 && acum_chords < MAX_CHORDS) {
								acum_notes++;
								x->new_notes[acum_notes - 1].chord_idx = acum_chords - 1;
								atom_setfloat(&x->new_notes[acum_notes - 1].note, 0.0f);
								if (has_curve) { x->new_notes[acum_notes - 1].curve_idx = acum_curves - 1; has_curve = 0; }
								else { x->new_notes[acum_notes - 1].curve_idx = -1; }
							}
							else {
								char prefix[8];
								float curve_amp;
								short curve_type;
								parse_flagged_token(token, prefix, &curve_amp);

								if (!strcmp(prefix, "x")) { curve_type = NC_LIN; }
								else if (!strcmp(prefix, "it")) { curve_type = NC_EI_COS; }
								else if (!strcmp(prefix, "ip")) { curve_type = NC_EI_POWO; }
								else if (!strcmp(prefix, "ir")) { curve_type = NC_EI_POWU; }
								else if (!strcmp(prefix, "ic")) { curve_type = NC_EI_CIRC; }
								else if (!strcmp(prefix, "ot")) { curve_type = NC_EO_SIN; }
								else if (!strcmp(prefix, "or")) { curve_type = NC_EO_POWU; }
								else if (!strcmp(prefix, "op")) { curve_type = NC_EO_POWO; }
								else if (!strcmp(prefix, "oc")) { curve_type = NC_EO_CIRC; }
								else if (!strcmp(prefix, "st")) { curve_type = NC_EIO_COS; }
								else if (!strcmp(prefix, "sp")) { curve_type = NC_EIO_POW; }
								else if (!strcmp(prefix, "sc")) { curve_type = NC_EIO_CIRC; }
								else if (!strcmp(prefix, "lt")) { curve_type = NC_EOI_ACOS; }
								else if (!strcmp(prefix, "lr")) { curve_type = NC_EOI_POW; }
								else if (!strcmp(prefix, "lc")) { curve_type = NC_EOI_CIRC; }
								else { curve_type = NC_NONE; continue; }

								acum_curves++;
								x->new_curves[acum_curves - 1].type = curve_type;
								x->new_curves[acum_curves - 1].ampl = curve_amp;
								has_curve = 1;
							}
						}
					}
					token = strtok_s(NULL, "/", &next_token);
				}
				sysmem_freeptr(str);
			}
		}
		else {
			object_error((t_object *)x, "bar: wrong input"); continue;
		}
	}
	x->new_timesig = acum_bar;
	x->total_new_hits = acum_hits;
	x->total_new_notes = acum_notes;
	x->total_new_chords = acum_def_chords;
	x->total_new_curves = acum_curves;

	clock_unset(x->m_clock);
	for (long i = 0; i < acum_hits; i++) {
		x->old_hits[i].dur_beat = x->new_hits[i].dur_beat;
		x->old_hits[i].start_beat = x->new_hits[i].start_beat;
	}
	for (long i = 0; i < acum_curves; i++) {
		x->old_curves[i].type = x->new_curves[i].type;
		x->old_curves[i].ampl = x->new_curves[i].ampl;
	}
	for (long i = 0; i < acum_notes; i++) {
		x->old_notes[i].note = x->new_notes[i].note;
		x->old_notes[i].chord_idx = x->new_notes[i].chord_idx;
	}
	for (long i = 0; i < acum_def_chords; i++) {
		x->old_chords[i].voices = x->new_chords[i].voices;
		for (long j = 0; j < MAX_VOICES; j++){ 
			x->old_chords[i].notes[j] = x->new_chords[i].notes[j]; 
		}
	}
	x->old_timesig = x->new_timesig;
	x->total_old_hits = x->total_new_hits;
	x->total_old_notes = x->total_new_notes;
	x->total_old_chords = x->total_new_chords;
	x->total_old_curves = x->total_new_curves;

	fl_maza_bang(x);
}

void fl_maza_free(t_fl_maza *x)
{
	sysmem_freeptr(x->old_hits);
	sysmem_freeptr(x->new_hits);

	sysmem_freeptr(x->old_curves);
	sysmem_freeptr(x->new_curves);

	sysmem_freeptr(x->old_chords);
	sysmem_freeptr(x->new_chords);

	sysmem_freeptr(x->old_notes);
	sysmem_freeptr(x->new_notes);
}

void fl_maza_float(t_fl_maza *x, double f)
{
	fl_maza_int(x, (long)f);
}

void fl_maza_int(t_fl_maza *x, long n)
{
	if (n != n) { return; }

	if (n > 0) { fl_maza_bang(x); }
	else { clock_unset(x->m_clock); }
}

void fl_maza_bang(t_fl_maza *x)
{
	x->index_old_hits = 0;
	x->index_old_curves = -1;
	x->index_old_notes = 0;
	x->index_old_chords = -1;

	x->time = (long)gettime_forobject((t_object *)x);
	
	clock_delay(x->m_clock, x->interval);
}

void fl_maza_tick(t_fl_maza *x)
{
	long elap_ms = (long)gettime_forobject((t_object *)x) - x->time;
	long beat_ms = x->beat_ms;
	float timesig = x->old_timesig;
	
	fl_beat *p_hits = x->old_hits;
	long index_hits = x->index_old_hits;
	long total_hits = x->total_old_hits;

	fl_curve *p_curve = x->old_curves;
	short index_curves = x->index_old_curves;
	short total_curves = x->total_old_curves;

	fl_note *p_note = x->old_notes;
	long index_notes = x->index_old_notes;
	long total_notes = x->total_old_notes;

	fl_chord *p_chord = x->old_chords;
	short index_chords = x->index_old_chords;
	short total_chords = x->total_old_chords;

	short loop = x->loop_mode;
	short wrap_mode = x->wrap_mode;

	double norm_hit;
	double curve;
	short curve_type = x->curve_type;
	double curve_amp = x->curve_amp;

	t_atom_long hit_ms;
	long start_ms;
	long bar_ms = (long)(timesig * (double)beat_ms);

	//schedule next tick call
	clock_delay(x->m_clock, x->interval);

	//new index
	if (index_hits < total_hits) {

		//next start
		start_ms = (long)(p_hits[index_hits].start_beat * (double)beat_ms);

		//if hit
		if (elap_ms >= start_ms) {

			//duration
			hit_ms = (t_atom_long)(p_hits[index_hits].dur_beat * (double)beat_ms);
			outlet_int(x->m_outlet2, hit_ms);
			index_hits++;

			//note or chord
			index_notes = idx_wrap(wrap_mode, index_hits, total_notes); 
			index_chords = p_note[index_notes].chord_idx;
			index_curves = p_note[index_notes].curve_idx;

			if (index_chords > -1) {
				if (index_chords < total_chords) {
					outlet_list(x->m_outlet1, 0L, p_chord[index_chords].voices, p_chord[index_chords].notes);
				}
			}
			else {
				outlet_list(x->m_outlet1, 0L, 1, &p_note[index_notes].note);
			}

			//curve
			if (index_curves > -1 && index_curves < total_curves) {
				x->curve_type = curve_type = p_curve[index_curves].type;
				x->curve_amp = curve_amp = p_curve[index_curves].ampl;
			}
		}
	}
	//no hits left, only final flag
	else if (elap_ms >= bar_ms) {
		start_ms = bar_ms;
		clock_unset(x->m_clock);
		outlet_bang(x->m_outlet4);
		if (loop) { fl_maza_bang(x); }
	}
	
	if (index_curves > -1) {
		norm_hit = (double)(elap_ms - start_ms) / (double)hit_ms;
		curve = curve_amp * easing_curves(curve_type, MIN(1.0, norm_hit));
		outlet_float(x->m_outlet3, curve);
	}

	//save state
	x->index_old_hits = index_hits;
	x->index_old_curves = index_curves;
	x->index_old_notes = index_notes;
	x->index_old_chords = index_chords;
}

long z_mod(long x, long base)
{
	long y = x;
	long b = labs(base);
	while (y < 0) { y += b; }
	y = y % b;
	return y;
}

long idx_wrap(short mode, long boundary, long n) {
	long out;
	long odd_repeat;
	switch(mode){
	case WM_REPEAT:
		out = z_mod(n, boundary);
		break;
	case WM_MIRROR:
		odd_repeat = z_mod(n, 2 * boundary);
		out = (odd_repeat > boundary) ? z_mod(boundary - odd_repeat, boundary) : odd_repeat ;
		break;
	default: //WM_CLAMP
		out = MIN(n, boundary - 1);
		break;
	}
	return out;
}

int is_pure_float(const char *token) {
	char *endptr;
	float val;
	if (!token) return 0;
	val = strtof(token, &endptr);
	return (endptr != token) && (*endptr == '\0') && isfinite(val);
}

void parse_flagged_token(const char *token, char *prefix_out, float *val_out) {
	const char *p = token;
	int prefix_len = 0;

	while (*p && !isdigit(*p) && *p != '-' && *p != '+') {
		p++;
		prefix_len++;
	}

	// copy prefix
	strncpy(prefix_out, token, prefix_len);
	prefix_out[prefix_len] = '\0';

	// parse number
	*val_out = strtof(p, NULL);
}

double easing_curves(short curve_type, double norm_hit) {
	double note_ease;

	switch (curve_type) {
	case NC_LIN:
		note_ease = norm_hit;
		break;
	case NC_EI_COS:
		note_ease = (float)(1.0 - cos(0.5 * norm_hit * MATH_PI));
		break;
	case NC_EI_POWO:
		note_ease = (float)pow(norm_hit, DFLT_POWEXP);
		break;
	case NC_EI_POWU:
		note_ease = (float)(1.0 - pow(1.0 - norm_hit, 1.0 / DFLT_POWEXP));
		break;
	case NC_EI_CIRC:
		note_ease = (float)(1.0 - sqrt(1.0 - pow(norm_hit, 20)));
		break;
	case NC_EO_SIN:
		note_ease = (float)sin(norm_hit * MATH_PI / 2.0);
		break;
	case NC_EO_POWU:
		note_ease = (float)pow(norm_hit, 1.0 / DFLT_POWEXP);
		break;
	case NC_EO_POWO:
		note_ease = (float)(1.0 - pow(1.0 - norm_hit, DFLT_POWEXP));
		break;
	case NC_EO_CIRC:
		note_ease = (float)sqrt(1.0 - pow(norm_hit - 1.0, 2.0));
		break;
	case NC_EIO_COS:
		note_ease = (float)(-(cos(MATH_PI * norm_hit) - 1.0) / 2.0);
		break;
	case NC_EIO_POW:
		if (norm_hit < 0.5) {
			note_ease = (float)(4.0 * pow(norm_hit, DFLT_POWEXP));
		}
		else {
			note_ease = (float)(1.0 - (pow(-2.0 * norm_hit + 2.0, DFLT_POWEXP)) / 2.0);
		}
		break;
	case NC_EIO_CIRC:
		if (norm_hit < 0.5)
			note_ease = (float)((1.0 - sqrt(1.0 - pow(2.0 * norm_hit, 2.0))) / 2.0);
		else
			note_ease = (float)((sqrt(1.0 - pow(-2.0 * norm_hit + 2.0, 2.0)) + 1.0) / 2.0);
		break;
	case NC_EOI_ACOS:
		note_ease = (float)(acos(-2.0 * norm_hit + 1.0) / MATH_PI);
		break;
	case NC_EOI_POW:
		if (norm_hit < 0.5) {
			note_ease = (float)pow(0.25 * norm_hit, 1.0 / DFLT_POWEXP);
		}
		else {
			note_ease = (float)((2.0 - pow(2.0 - 2.0 * norm_hit, 1.0 / DFLT_POWEXP)) / 2.0);
		}
		break;
	case NC_EOI_CIRC:
		if (norm_hit < 0.5) {
			note_ease = (float)((sqrt(1.0 - pow(2.0 * norm_hit - 1.0, 2.0))) / 2.0);
		}
		else {
			note_ease = (float)(1.0 - (float)sqrt(norm_hit - norm_hit * norm_hit));
		}
		break;
	default:
		note_ease = 0.0;
		break;
	}
}