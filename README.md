
# About

An external for Max 8 (Max/MSP). 'maza' is the spanish translation of the word mallet, an object used to play percussion instruments. 'fl_maza' is an external that allows you to create easily any sequence of rhythms/melodies/chords using a specific list format for notes, beats and sequences of notes played. The external will translate a formatted list from beats to a sequence that will be played inmediately. 


# Features

- The beat period (in milliseconds) can be set with a message 'ms_beat'. Default is 500 ms (120 BPM).
- A rhythm entered will be played even if no list of notes was provided.
- Has a 'loop' toggle option.
- Wrap modes: If there are less notes than beat subdivisions listed the notes will repeat in different patterns:

```
wrap modes: 
- 0: [default] clamp to latest note
- 1: repeat notes from the start
- 2: mirror notes from the end
```

- Chords won't be wrapped if there are less chords defined than scheduled in subdivision list.
- No fail proof for previously defined chords if chords are listed but not defined.
- A melody can be created by using midi values and different commands to set and target notes:

```
bar commands:

- duration beat: int or float 
X.X

- beat subdivision: [list of 1/0/-]
<XXX...
ej: <0011-1

- melody: commands, numbers
/a/X/X/...
ej: /a/60/62/64/c/c/r/67/

- chords
/v/X/X/...
ej: /v/60/64/67/71/v/63/66/70/

commands:
----------melody
/a melody list
/c play new chord
/r repeat last chord
/x pitchbend commands
----------chord
/v chord list
----------pitchbend
/x 	lineal 		y = x
/it ease in 	y = cos(x)
/ip ease in 	y = x^a (a>1)
/ir ease in 	y = x^a (a<1)
/ic ease in 	y^2 - x^2
/ot ease out 	y = sin(x)
/or ease out 	y = x^a (a<1)
/op ease out 	y = x^a (a>1)
/oc ease out 	y^2 - x^2
/st ease in-out y = cos
/sp ease in-out y = x^a
/sc ease in-out y^2 - x^2
/lt ease out-in y = acos
/lr ease out-in y = x^a
/lc ease out-in y^2 - x^2

```


# Example

``bar 2. <1111 3.5 <001`` means that the external will play 4 notes in a 4th division of 2.0 beats and the last note in a 3rd division of 3.5 beats. 
This allows you to easily make very complex rhythms if you want to.

After a '<' character '0' is a silence, '1' is a note, '-' is a tie that extends the duration of a previous note. The output will be the subdivision duration in milliseconds of its respective subdivision as a float.

Melodies and timing for chords are defined using commands that start with /a/ and chords voices with commands starting with /v/   


# Notes

- This external is a later version of [fl_ritmo~](https://github.com/qmolt/fl_ritmo-max) (archived). In comparison, fl_maza includes compatibility to play melodies but the major difference is that fl_ritmo~ was a real-time external that counted samples to time the output events, and fl_maza uses the same clock efficiently to avoid linking a method to the DSP chain so it works independiently from dac state, also outputs float values instead of signal values.


# TO DO

- grain output interval as a variable/attribute.
- max notes, max chords, max voices per chord, and max hits length as parameters 