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

	x->m_outlet3 = outlet_new((t_object *)x, "bang");	//final bang
	x->m_outlet2 = outlet_new((t_object *)x, "float");	//dur
	x->m_outlet1 = outlet_new((t_object *)x, "float");	//note

	x->curve_task = 0;
	x->dur_task = 0;
	x->end_task = 0;

	x->total_old_hits = 0;
	x->total_new_hits = 0;
	x->old_cifra = 0.;
	x->new_cifra = 0.;
	x->index_old_hits = 0;
	x->total_old_notes = 0;
	x->total_new_notes = 0;

	x->beat_ms = DFLT_BEATMS;
	x->m_clock = clock_new(x, (method)fl_maza_tick);
	x->time = (long)gettime_forobject((t_object *)x);
	x->interval = DFLT_TIMEINTERVAL; //interval < 5 ? 5 : interval;

	x->loop_mode = DFLT_LOOPMODE;

	x->index_old_notes = 0;
	
	x->oct_div = DFLT_OCT_DIV;

	x->wrap_mode = WM_CLAMP;
	x->filter_toggle = FM_NAT;

	x->curve_type = NC_FLAT;
	x->curve_start = (float)(x->oct_div * 5);
	x->curve_end = (float)(x->oct_div * 5);
	x->filter_mode = FM_NAT;
	x->dur_beat = 0.0f;
	x->start_beat = 0.0f;

	x->old_notes = (fl_note *)sysmem_newptr(MAX_NOTES_SIZE * sizeof(fl_note));
	if (!x->old_notes) { object_error((t_object *)x, "out of memory: no space for notes list"); return x; }
	x->new_notes = (fl_note *)sysmem_newptr(MAX_NOTES_SIZE * sizeof(fl_note));
	if (!x->new_notes) { object_error((t_object *)x, "out of memory: no space for notes list"); return x; }
	for (long i = 0; i < MAX_NOTES_SIZE; i++) {
		x->old_notes[i].start = 0;
		x->old_notes[i].end = 0;
		x->old_notes[i].curve_type = NC_SET;
		x->old_notes[i].filter_mode = FM_NAT;

		x->new_notes[i].start = 0;
		x->new_notes[i].end = 0;
		x->new_notes[i].curve_type = NC_SET;
		x->new_notes[i].filter_mode = FM_NAT;
	}

	x->old_hits = (fl_beat *)sysmem_newptr(MAX_HITS_SIZE * sizeof(fl_beat));
	if (!x->old_hits) { object_error((t_object *)x, "out of memory: no space for beats list"); return x; }
	x->new_hits = (fl_beat *)sysmem_newptr(MAX_HITS_SIZE * sizeof(fl_beat));
	if (!x->new_hits) { object_error((t_object *)x, "out of memory: no space for beats list"); return x; }
	for (long i = 0; i < MAX_HITS_SIZE; i++) {
		x->old_hits[i].dur_beat = 0.0;
		x->old_hits[i].start_beat = 0.0;

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
		case O_NOTE:sprintf(dst, "(float) note"); break;
		case O_DUR: sprintf(dst, "(float) note duration in milliseconds"); break;
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
	//format: mel bar <div...
		ie: /s/32/x/35/a/30 4.5 '<10010'

		mel:	/{a, x, it, ip, ...}
		bar:	(int/float)
		div[]:	{0, 1, -}
	*/
	t_atom *ap = argv;
	long ac = argc;

	char *bar_string;
	long idx_string;

	float acum_bar;
	long acum_hits;
	long acum_notes;
	long total_hits;
	long total_notes;
	float beat = DFLT_BEAT;
	long subdiv;
	
	long legatura;
	long idx_null;
	short legatura_flag;

	char *str;
	char *token = NULL;
	char *next_token = NULL;
	long barmemlen;

	char *endptr;
	float start_curve = DFLT_OCT_DIV * 5;
	float end_curve = DFLT_OCT_DIV * 5;

	short curve_type = NC_SET;

	acum_hits = -1;
	acum_notes = 0;
	acum_bar = 0.;

	x->filter_toggle = FM_NAT;

	for (long i = 0; i < ac; i++) {
		if (atom_gettype(ap + i) == A_FLOAT || atom_gettype(ap + i) == A_LONG) {
			beat = (float)fabs((double)atom_getfloat(ap + i));
		}
		else if (atom_gettype(ap + i) == A_SYM) {
			bar_string = atom_getsym(ap + i)->s_name;

			if (bar_string[0] == '<') { 
				idx_string = 1;
				while (bar_string[idx_string] != '\0') {idx_string++;}
				
				subdiv = idx_string - 1;
				idx_null = idx_string + 1;
				legatura = 0;
				legatura_flag = 0;
				for (long j = 1; j < idx_null; j++) {
					if (acum_hits >= MAX_HITS_SIZE) {break;}

					if (bar_string[j] == '1') {
						acum_hits++;
						x->new_hits[acum_hits].dur_beat = beat / subdiv;
						x->new_hits[acum_hits].start_beat = acum_bar + beat * (j - 1) / subdiv;
						legatura = 0;
						legatura_flag = 1;
					}
					else if (bar_string[j] == '0') {
						legatura_flag = 0;
					}
					else if (bar_string[j] == '-') {
						legatura++;
						if (legatura_flag) {
							x->new_hits[acum_hits].dur_beat = beat * (legatura + 1) / subdiv;
						}
					}
					
				}

				acum_bar += beat;
			}
			else if (bar_string[0] == '/') {
				
				barmemlen = (long)strlen(bar_string) + 1;

				str = (char *)sysmem_newptr(barmemlen * sizeof(char));
				if (!str) { object_error((t_object *)x, "no memory for str"); return; }
				else {
					sysmem_copyptr(bar_string, str, barmemlen * sizeof(char));
					
					token = strtok_s(str, "/", &next_token);
					while (token) {

						end_curve = strtof(token, &endptr);
						if (token == endptr) { 
							end_curve = DFLT_OCT_DIV * 5;
						
							if (!strcmp(token, "s")) { curve_type = NC_SET; }
							else if (!strcmp(token, "a")) { curve_type = NC_FLAT; }
							else if (!strcmp(token, "x")) { curve_type = NC_LIN; }
							else if (!strcmp(token, "it")) { curve_type = NC_EI_COS; }
							else if (!strcmp(token, "ip")) { curve_type = NC_EI_POWO; }
							else if (!strcmp(token, "ir")) { curve_type = NC_EI_POWU; }
							else if (!strcmp(token, "ic")) { curve_type = NC_EI_CIRC; }
							else if (!strcmp(token, "ot")) { curve_type = NC_EO_SIN; }
							else if (!strcmp(token, "or")) { curve_type = NC_EO_POWU; }
							else if (!strcmp(token, "op")) { curve_type = NC_EO_POWO; }
							else if (!strcmp(token, "oc")) { curve_type = NC_EO_CIRC; }
							else if (!strcmp(token, "st")) { curve_type = NC_EIO_COS; }
							else if (!strcmp(token, "sp")) { curve_type = NC_EIO_POW; }
							else if (!strcmp(token, "sc")) { curve_type = NC_EIO_CIRC; }
							else if (!strcmp(token, "lt")) { curve_type = NC_EOI_ACOS; }
							else if (!strcmp(token, "lr")) { curve_type = NC_EOI_POW; }
							else if (!strcmp(token, "lc")) { curve_type = NC_EOI_CIRC; }
							else if (!strcmp(token, "f0")) { x->filter_toggle = FM_REAL; }
							else if (!strcmp(token, "f1")) { x->filter_toggle = FM_NAT; }
						}
						else {
							if (curve_type == 0) {
								start_curve = end_curve;
							}
							else if (curve_type > 0 && acum_notes < MAX_NOTES_SIZE) {

								x->new_notes[acum_notes].curve_type = curve_type;
								x->new_notes[acum_notes].start = start_curve;
								x->new_notes[acum_notes].end = end_curve;
								x->new_notes[acum_notes].filter_mode = x->filter_toggle;
								acum_notes++;

								start_curve = end_curve;
							}
						}

						token = strtok_s(NULL, "/", &next_token);
					}
				}
				sysmem_freeptr(str);
			}
		}
		else {
			object_error((t_object *)x, "bar: wrong input"); return;
		}
	}
	x->new_cifra = acum_bar;
	x->total_new_hits = total_hits = acum_hits + 1;
	x->total_new_notes = total_notes = acum_notes;

	clock_unset(x->m_clock);
	for (long i = 0; i < total_hits; i++) {
		x->old_hits[i].dur_beat = x->new_hits[i].dur_beat;
		x->old_hits[i].start_beat = x->new_hits[i].start_beat;
	}
	for (long i = 0; i < total_notes; i++) {
		x->old_notes[i].curve_type = x->new_notes[i].curve_type;
		x->old_notes[i].start = x->new_notes[i].start;
		x->old_notes[i].end = x->new_notes[i].end;
		x->old_notes[i].filter_mode = x->new_notes[i].filter_mode;
	}
	x->old_cifra = x->new_cifra;
	x->total_old_hits = x->total_new_hits;
	x->total_old_notes = x->total_new_notes;

	fl_maza_bang(x);
}

void fl_maza_free(t_fl_maza *x)
{
	sysmem_freeptr(x->old_hits);
	sysmem_freeptr(x->new_hits);
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
	x->time = (long)gettime_forobject((t_object *)x);
	x->index_old_hits = 0;
	x->curve_task = 0;
	x->dur_task = 0;
	x->end_task = 0;

	clock_delay(x->m_clock, x->interval);
}

void fl_maza_tick(t_fl_maza *x)
{
	long elap_ms = (long)gettime_forobject((t_object *)x) - x->time;
	long beat_ms = x->beat_ms;
	float cifra = x->old_cifra;
	
	fl_beat *p_hits = x->old_hits;
	long index_hits = x->index_old_hits;
	long total_hits = x->total_old_hits;
	fl_note *p_note = x->old_notes;
	long total_notes = x->total_old_notes;
	long index_notes = x->index_old_notes;

	short loop = x->loop_mode;
	short wrap_mode = x->wrap_mode;
	short filter_mode = x->filter_mode;
	short curve_type = x->curve_type;
	float curve_start = x->curve_start;
	float curve_end = x->curve_end;
	float dur_beat = x->dur_beat;
	float start_beat = x->start_beat;

	float note_final;
	long next_start_ms;

	float note_norm = 1.0;

	float note_ease;
	long mirror_dn, mirror_up;
	short mirror_sw;

	long hit_ms;
	long start_ms;
	long bar_ms = (long)(cifra * (float)beat_ms);

	short curve_task = x->curve_task;

	//new index
	if (index_hits < total_hits) {

		//next start
		next_start_ms = (long)(p_hits[index_hits].start_beat * (float)beat_ms);

		//schedule next tick call
		clock_delay(x->m_clock, x->interval);

		//calculate every tick
		hit_ms = (long)(dur_beat * (float)beat_ms);
		start_ms = (long)(start_beat * (float)beat_ms);

		//if hit
		if (elap_ms >= next_start_ms) {

			//hit index and data
			dur_beat = p_hits[index_hits].dur_beat;
			start_beat = p_hits[index_hits].start_beat;
			x->dur_beat = dur_beat;
			x->start_beat = start_beat;

			hit_ms = (long)(dur_beat * (float)beat_ms);
			start_ms = (long)(start_beat * (float)beat_ms);

			//notes index and data
			if (total_notes > 0) {
				switch (wrap_mode) {
				case WM_CLAMP:
					index_notes = MIN(index_hits, total_notes - 1);
					break;
				case WM_REPEAT:
					index_notes = z_mod(index_hits, total_notes);
					break;
				case WM_MIRROR:
					mirror_dn = z_mod(index_hits, (total_notes - 1) * 2);
					mirror_up = z_mod(-index_hits, total_notes - 1);
					mirror_sw = (short)floor((float)mirror_dn / (float)total_notes);
					index_notes = (mirror_sw) ? mirror_up : mirror_dn;
					break;
				default: //WM_CLAMP
					index_notes = MIN(index_hits, total_notes - 1);
				}
				curve_type = p_note[index_notes].curve_type;
				curve_start = p_note[index_notes].start;
				curve_end = p_note[index_notes].end;
				filter_mode = p_note[index_notes].filter_mode;

				x->curve_type = curve_type;
				x->curve_start = curve_start;
				x->curve_end = curve_end;
				x->filter_mode = filter_mode;

				//curve
				curve_task = 1;
			}

			//output duration
			outlet_float(x->m_outlet2, (float)hit_ms);

			//save index
			x->index_old_hits = ++index_hits;
			x->index_old_notes = index_notes;
		}

		//notes data
		if (total_notes > 0) {
			//curve
			note_norm = (float)MIN(1.0, (elap_ms - start_ms) / (float)hit_ms);

			switch (curve_type) {
			case NC_FLAT:
				note_ease = 1.0;
				break;
			case NC_LIN:
				note_ease = note_norm;
				break;
			case NC_EI_COS:
				note_ease = (float)(1.0 - cos(0.5 * note_norm * MATH_PI));
				break;
			case NC_EI_POWO:
				note_ease = (float)pow(note_norm, DFLT_POWEXP);
				break;
			case NC_EI_POWU:
				note_ease = (float)(1.0 - pow(1.0 - note_norm, 1.0 / DFLT_POWEXP));
				break;
			case NC_EI_CIRC:
				note_ease = (float)(1.0 - sqrt(1.0 - pow(note_norm, 20)));
				break;
			case NC_EO_SIN:
				note_ease = (float)sin(note_norm * MATH_PI / 2.0);
				break;
			case NC_EO_POWU:
				note_ease = (float)pow(note_norm, 1.0 / DFLT_POWEXP);
				break;
			case NC_EO_POWO:
				note_ease = (float)(1.0 - pow(1.0 - note_norm, DFLT_POWEXP));
				break;
			case NC_EO_CIRC:
				note_ease = (float)sqrt(1.0 - pow(note_norm - 1.0, 2.0));
				break;
			case NC_EIO_COS:
				note_ease = (float)(-(cos(MATH_PI * note_norm) - 1.0) / 2.0);
				break;
			case NC_EIO_POW:
				if (note_norm < 0.5) {
					note_ease = (float)(4.0 * pow(note_norm, DFLT_POWEXP));
				}
				else {
					note_ease = (float)(1.0 - (pow(-2.0 * note_norm + 2.0, DFLT_POWEXP)) / 2.0);
				}
				break;
			case NC_EIO_CIRC:
				if (note_norm < 0.5)
					note_ease = (float)((1.0 - sqrt(1.0 - pow(2.0 * note_norm, 2.0))) / 2.0);
				else
					note_ease = (float)((sqrt(1.0 - pow(-2.0 * note_norm + 2.0, 2.0)) + 1.0) / 2.0);
				break;
			case NC_EOI_ACOS:
				note_ease = (float)(acos(-2.0 * note_norm + 1.0) / MATH_PI);
				break;
			case NC_EOI_POW:
				if (note_norm < 0.5) {
					note_ease = (float)pow(0.25 * note_norm, 1.0 / DFLT_POWEXP);
				}
				else {
					note_ease = (float)((2.0 - pow(2.0 - 2.0 * note_norm, 1.0 / DFLT_POWEXP)) / 2.0);
				}
				break;
			case NC_EOI_CIRC:
				if (note_norm < 0.5) {
					note_ease = (float)((sqrt(1.0 - pow(2.0 * note_norm - 1.0, 2.0))) / 2.0);
				}
				else {
					note_ease = (float)(1.0 - (float)sqrt(note_norm - note_norm * note_norm));
				}
				break;
			default: //NC_FLAT
				note_ease = 1.0;
				break;
			}

			//note data
			note_final = curve_start + (curve_end - curve_start) * note_ease;

			switch (filter_mode) {
			case FM_REAL:
				break;
			case FM_NAT:
				note_final = (float)floor(note_final);
				break;
			}

			//output note
			if (curve_task) {
				outlet_float(x->m_outlet1, note_final);
				curve_task = 0;
			}
		}
	}
	else {
		//no hits left, only final flag
		next_start_ms = bar_ms;

		if (elap_ms >= next_start_ms) {
			clock_unset(x->m_clock);
			outlet_bang(x->m_outlet3);
			if (loop) { fl_maza_bang(x); }
		}
	}

	x->curve_task = curve_task;
}

long z_mod(long x, long base)
{
	long y = x;
	long b = labs(base);
	while (y < 0) { y += b; }
	y = y % b;
	return y;
}