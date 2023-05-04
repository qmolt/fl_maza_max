
# About

An external for Max 8 (Max/MSP). 'Maza' is the spanish translation of the word mallet, an object used to play percussion instruments. 'fl_maza' is an external that allows you to create easily any sequence of rhythms/melodies using a specific list format for notes, beats and sequences of notes played. The external will translate a formatted list to a sequence that will be played inmediately. 


# Features

- The beat period (in milliseconds) can be set with a message 'ms_beat'.
- The external will play any rhythm even if no list of notes was provided.
- The external has a 'loop' option
- A filter toggle, when inactive will output float values (linear); when active will output integers approximations (chromatic (dflt)).
- A melody can be created by using midi values and different commands to set and target notes:

```
commands:

----------filter toggle
	/f0 off (linear)  
	/f1 on (chromatic)

----------note
	/s set start

----------note curves
	/a constant 
	/x lineal
ease in ("speeding-up")
	/it trigonom y = cos(x)
	/ip power y = x^a (a>1)
	/ir power y = x^a (a<1)
	/ic circular y^2 - x^2
ease out ("slowing-down") 
	/ot trigonom y = sin(x)
	/or power y = x^a (a<1)
	/op power y = x^a (a>1)
	/oc circular y^2 - x^2
ease in-out (sigmoid-shaped)
	/st trigonom y = cos
	/sp power y = x^a
	/sc circular y^2 - x^2
ease out-in (logit-shaped)
	/lt trigonom y = acos
	/lr power y = x^a
	/lc circular y^2 - x^2
```


# Example

``bar 2. <1111 3.5 <001`` means that the external will play 4 notes in a 4th division of 2.0 beats and the last note in a 3rd division of 3.5 beats. 
This allows you to make very complex rhythms if you want to.

After a '<' character '0' is a silence, '1' is a note, '-' is a tie that extends the duration of a note.

The output will be the subdivision duration in milliseconds of its respective subdivision as a float.


# Notes

This external is a later version of [fl_ritmo~](https://github.com/qumolt/fl_ritmo-max). In comparison, fl_maza includes compatibility to play melodies but the major difference is that fl_ritmo~ was a real-time external that counted samples to time the output events, and fl_maza uses the same clock efficiently to avoid linking a method to the DSP chain so it works independiently from dac state, and also outputs float melody values when needed instead of signal values.


| <img src="media/c54.jpg" width="450" title="fl_ritmo and fl_maza"> |
| :----------------------: |
| *fl\_ritmo and fl\_maza* |

# Coming soon

- grain output interval as a variable/attribute.