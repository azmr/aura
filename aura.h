/* Aura - A single header C library of music theory tools */
/* TODO:
 *  - basic audio output (e.g. sine/square/triangle/sawtooth)
 *  - accenting? - volume?
 *  - demo different rhythms with different tempos
 *  - common melody patterns available for sketching/modifying
 *  	- scales up/down
 *  	- ...
 *  - scale melodies
 *  - primitives
 *  	- Semitone (1D)
 *  	- Tones + Semitones (2D)
 *  	- Harmonic _? (3D)
 *  - mapping point/vector to note/interval
 *  - rhythm
 *  	- claves?
 *  - use bit vector to control composition
 *  	- not sure if better treated as flags or opaque seed
 *  	- incrementable for subsequent measures?
 *  	- small variations in number -> small output variations
 *  	- higher bits -> more fundamental options
 *  - Invert chords to match as much as possible (smoooooth)
 *  	- Common note should be in same position of each (high/middle/low)
 *  	- Sus2/4 where no common notes
 *  - Inversions for sense of movement
 *  - Modes - to make more apparent, repeat chords, make longer, avoid major/minor chords
 *
 * Concepts
 *  - Roman Numeral Analysis (6 diatonic chords per key)
 *  	- Borrowed chords (quarter circle left)
 *  - Harmonic Relativity
 *  	- Reverse extension
 *  - Function exchanges
 *  - Voice exchanges
 *  - Contrapoint/Part construction
 *  - (A)Tonal
 *  - Modes
 *  	- I think: key/scale is offset in the 12 semitones; mode is offset inside 8 notes of key
 *  - Dimensions (ref: https://www.youtube.com/watch?v=9kihzlWsB4k) (conclusiveness - question vs answer)
 *  	- Rhythm
 *  		- Metric placement (on strong vs weak beats)
 *  		- Note duration
 *  	- Melody
 *  		- Relative range/pitch
 *  		- Motivic strength (approached by isolated leap vs halfstep/step/arpeggio)
 *  	- Harmony
 *  		- Harmonic span (distant vs same/close key)
 *  		- Vertical sonority (dissonant vs consonant)
 *  		- Chord function (9/11/13/X vs 1/3/5/7)
 *  	- Expression
 *  		- Dynamics (quiet vs loud)
 *  		- Articulations (legato vs marcato)
 *  	- Texture
 *  		- Instrumentation (sparse vs dense)
 *  		- Homogeneity (disjointed vs unified)
 *  		- Timbre (sine vs saw)
 *  - Suspension   - move 1 note of chord later
 *  - Anticipation - move 1 note of chord earlier
 *  - Multiple ways of voicing a chord
 *  - Genera
 *  	- Diatonic
 *  	- Chromatic - twelve pitches, each an equally spaced (100 cents) semitone apart (i.e. all available pitches)
 *  	- Enharmonic
 *
 *  - Units
 *  	- Cents (acoustic logarithms)
 *  - Consonance
 *  	- Harmonic (simultaneous notes)
 *  		- Imperfect (3/6)
 *  		- Perfect (5/0/8)
 *  		- Dissonance (1/2/7)
 *  	- Melodic (sequential notes)
 *  - Harmonic transformations - https://www.youtube.com/watch?v=Dghtjke4X7I
 *  	- Neo-Riemannian theory
 *  	- Each 1 note different from current chord:
 *  	- Parallel (switch chord quality - e.g. maj to min)
 *  	- Relative (e.g. drop down to 6th chord)
 *  	- Leading tone (up to 3 chord)
 *  	- IV an V chords are 2 transformations
 *  	- Chromatic mediants (e.g. Emaj-C#maj)
 *  	- Proximate harmony e.g. planing (moving chord shape up and down)
 *  	- Harmonic rhythm - speed of chord change mediates 'connectedness' (faster -> more connected -> ensure gaps aren't too big)
 *  	- 
 *  	- Non-tertian chords
 *  - Themes (longer, melodic)
 *  - Motifs (shorter, rhythmic)
 */

/* NOTE: these are simply identifiers, and should not be used numerically... or maybe as semitones? */
/* TODO: I may want to conflate sharps and flats... - enharmonics */
#define AURA_NOTES      \
	AURA_NOTE(C,  "C")  \
	AURA_NOTE(Cs, "C#") \
	AURA_NOTE(Db, "Db") \
	AURA_NOTE(D,  "D")  \
	AURA_NOTE(Ds, "D#") \
	AURA_NOTE(Eb, "Eb") \
	AURA_NOTE(E,  "E")  \
	AURA_NOTE(Es, "E#") \
	AURA_NOTE(Fb, "Fb") \
	AURA_NOTE(F,  "F")  \
	AURA_NOTE(Fs, "F#") \
	AURA_NOTE(Gb, "Gb") \
	AURA_NOTE(G,  "G")  \
	AURA_NOTE(Gs, "G#") \
	AURA_NOTE(Ab, "Ab") \
	AURA_NOTE(A,  "A")  \
	AURA_NOTE(As, "A#") \
	AURA_NOTE(Bb, "Bb") \
	AURA_NOTE(B,  "B")  \
	AURA_NOTE(Bs, "B#") \
	AURA_NOTE(Cb, "Cb") \

#define AURA_NOTE_DECORATE(x) aura_## x
#define AURA_NOTE(x, v, s) AURA_NOTE_DECORATE(x),
typedef enum aura_note { AURA_NOTES AURA_NOTE_DECORATE(Error) = -1 } aura_note;
#undef AURA_NOTE

#define AURA_NOTE(x, s) AU_5ths_## s,
char *auraNotesStrings[] = { AURA_NOTES };
#undef AURA_NOTE

/* TODO: could represent as bit flags? */
typedef int[7] key;

/* CHORDS
 * - Any 2 adjacent notes will typically be either a major or minor 3rd apart
 */
typedef struct aura_chord {
	/* NOTE (reference): https://www.youtube.com/watch?v=5Y01jIorpeA */
	/* NOTE (learning): Point of confusion: Major != interval major (although related) */
	/* NOTE (learning): this interval representation is much easier to grok than the names,
	 * even more so if translated to semitones (3/4 per interval, thirds -> "tertian harmony") */
	/* TODO(opt): could use 2-bit pattern for consecutive intervals
	 * either 0 for major 3rd as good default:
	 * 0 for major 3rd, 1 for minor 3rd, 2 for perfect 4th, 3 for major 2nd (for sus),
	 * or add 2 and treat it as num semitones (possibly better for calculation?)
	 * & an indication of how many intervals - put this on lower bits for easy masking
	 * might be able to fit inversions into 32-bit int as well
	 * */
	enum {          /*  intervals: consecutive  | from root   */
		AURA_CHORD_Major,       /*  M3, m3      |  M3, P5     */
		AURA_CHORD_Minor,       /*  m3, M3      |  m3, P5     */
                                                      
		AURA_CHORD_Diminished,  /*  m3, m3      |  m3, d5     */
		AURA_CHORD_Augmented,   /*  M3, M3      |  M3, A5     */

		AURA_CHORD_Major7,      /*  M3, m3, M3  |  M3, P5, M7 */
		AURA_CHORD_Minor7,      /*  m3, M3, m3  |  m3, P5, m7 */

		AURA_CHORD_7,           /*  M3, m3, m3  |  M3, P5, m7 */ /* Acts as dominant chord */
		AURA_CHORD_MinMaj7,     /*  m3, M3, M3  |  m3, P5, M7 */

		AURA_CHORD_HalfDim7,    /*  m3, m3, M3  |  m3, d5, m7 */ /* aka Min7 Flat5 */
		AURA_CHORD_Diminished7, /*  m3, m3, m3  |  m3, d5, d7 */

		AURA_CHORD_AugMaj7,     /*  M3, M3, m3  |  M3, A5, M7 */

		/* commonly used for tension; ambiguous what key you're in */
		AURA_CHORD_Suspended4,  /*  P4, M2      |  P4, P5     */ /* more common */
		AURA_CHORD_Suspended2,  /*  M2, P4      |  M2, P5     */
	} Kind;

	aura_note Root;

	/* NOTE (reference): https://www.youtube.com/watch?v=Nr2XBoanNJY */
	int Inversion; /* { 0, 1, 2, 3 (for 7ths) } - allows smoother transitions between chords - they overlap (more) */

	/* Extended chords
	 * NOTE (ref): https://www.youtube.com/watch?v=rLDRWDI-m3w
	 * Basically, just keep stacking 3rds
	 * 9th, 11th and 13th make sense (beyond that, you loop back to the root node)
	 * last num generally tells you farthest point
	 *
	 * Missing notes (for convenience):
	 * for 7ths, the 5th can kind of be dropped, as it doesn't distinguish maj/min
	 * and adds fairly little. This is not true for HalfDim7
	 *
	 * for 11th, 9th can be dropped without too much lost
	 * never drop root/3rd/7th (except maybe 3rd for 11th?)
	 *
	 */

	/* Altered chords (*this term is also used for other things)
	 * NOTE (ref): https://www.youtube.com/watch?v=7OmqeihOXD4
	 * modify extended chords by sticking flats and sharps in superscript:
	 * e.g. G13^(#9 #11)
	 * This is sometimes done for smaller chords as well (particularly jazz):
	 * Emin7^(b5) == E^HalfDim7
	 * It isn't written like this for the end note, which is instead:
	 * F7^(b9) (a modified F9); D11 -> D9^(#11) -> D7^(#9 #11)
	 */


	/* NOTE (ref): https://www.youtube.com/watch?v=16S2F--yxqI */
	/* Slash chord is a way of describing inversions (if the lower note is in the chord),
	 * sometimes they modify the chord (as well) (with new notes added)
	 * e.g. C/E  => C (major) chord over E note (first inversion of C major)
	 * can obscure harmony
	 * */

	/* Polychords are Chord1 / Chord2 (with horizontal line fraction)
	 * Generally describing an extended chord.
	 * In other contexts, may be when there are 2 separate harmonies - "polytonality"
	 */

	/* 6th chords - equiv to min7 chord in 1st inversion (C6 -> Amin7, not sure how general) - can be obfuscatory
	 * Can be intuitively more reasonable if shifing from 7th to '6th' - still *sounds* like root is maintained
	 */

	/* https://www.youtube.com/watch?v=keY3Dk3RC70
	 *
	 * Dominants lead to a particular note, regardless of key
	 */

} aura_chord;

/* MELODY *****************************
 *  - Chord Tones (CT) (playing notes also being played in current chords)
 *  	- Fits very nicely
 *  - Non-Chord Tones (NCT) (playing notes also being played in current chords)
 *  	- More tension
 *  - NCT->CT: Tension->Resolution
 *  - 7th is somewhere between CT and NCT - can work either
 *  - Careful with 4th
 *  - Devices
 *  	- Movement size:
 *  		- tone/semitone - "step" - "Conjunct motion"
 *  		- anything larger than a tone - "leap" - "Disjunct motion"
 *  		- normally want a combination, or to use rhythm to make interesting
 *  	- Variability:
 *  		- Repetition - identical
 *  		- Contrast   - dissimilar
 *  		- Variation  - similar
 */
/*************************************/

/* RHYTHM *****************************
 *  - Can measure timing in 'beats' (e.g. 0.5s)
 *  - For convenience, counting resets periodically (e.g. every 4 beats)
 *  - Period of beat counts is called a "measure"
 *  - on avg, 4 measures = 1 standard phrase
 *  - 8 measures = 1 standard period
 *  - Speed of beats - "tempo", in bpm (e.g 120bpm)
 *  - Length of a 'whole' note depends on time signature
 *  - Time sig given as 2 numbers, that can be read similar to fraction:
 *  	- Bottom: the subdivision of note size (e.g. 4 -> quarter note == 1 beat)
 *  	- Top: the number of those subdivisions fitting into a measure (sometimes)
 *  - Notes that fall on the beat tend to be accented more than those that fall between beats
 *  - Time sigs get more complex as you require dotted notes (150% of the original time)
 *  	- 6/8 comes from 2 dotted 1/4 notes -> 6 1/8th notes. 2 beats per measure...
 *  	- Normally times are subdivided into 2 ("simple time"); when into 3, it's called "compound time"
 *  	- 3/8 is 3 1/8th notes with 3 beats
 *  	- 7/8 can be divided in multiple ways, with beats of different lengths (3,4; 3,2,2...)
 *  	- can have compound time for single triplet - beat stays same length, internal quarters are faster
 *  	- vice versa for duplet in e.g. 6/8
 *  - Can have multiple 'voices' simultaneously (on some instruments)
 *  - "Polyrhythms" - multiple different rhythms simultaneously (e.g. 3 against 2)
 *  	- doesn't count if they divide evenly (e.g. 8 against 16)
 *  	- can count by using common multiple
 */
/*************************************/

/*****************************************************************************/
#if defined(AURA_IMPLEMENTATION) || defined(AURA_SELFTEST)
/*****************************************************************************/



/* MACRO DEFINITIONS *********************************************************/
#define AURA_ENUM_MAP(enum, a, b) AURA_## enum ##_DECORATE(a) = AURA_## enum ##_DECORATE(b)

/* Makes tables mapping between canonical and local forms of notes.
 * The error value for each is put at the start.
 */
#define AURA_NOTE_MAP(t, x, M, start) \
	aura_note aura## x ##sToNote_[] = { AURA_NOTE_Error, AURA_## M ##S }; \
	aura_note *aura## x ##sToNote = aura## x ##sToNote_ + 1; \
	t auraNoteTo## x ##_[] = { AURA_## M ##_DECORATE(Error), AURA_NOTES }; \
	t auraNoteTo## x ## = auraNoteTo## x ##_ + 1 + start
/*/MACRO DEFINITIONS *********************************************************/



aura_note auraNote(char *NoteName) {
	/* TODO: ensure 2 chars exactly */
	short NoteVal = *(short *)NoteName;
	aura_note Result = AURA_NOTE_DECORATE(Error);
	switch(NoteVal)
	{
		/* TODO: would these be better as a table of values? */
#define AURA_NOTE(x, s) \
		case *(short *)s: Result = x; break;

		AURA_NOTES
#undef AURA_NOTE
	}
	return Result;
}




/*************************************/
typedef enum aura_interval {
	/* given as semitone difference */
	aura_Unison            =  0,
	aura_PerfectUnison     =  0,
	aura_P1                =  0,
	aura_DiminishedSecond  =  0,
	aura_d2                =  0,

	aura_AugmentedUnison   =  1,
	aura_A1                =  1,
	aura_MinorSecond       =  1,
	aura_m2                =  1,
	aura_Semitone          =  1,
	aura_S                 =  1,

	aura_MajorSecond       =  2,
	aura_M2                =  2,
	aura_Tone              =  2,
	aura_T                 =  2,
	aura_DiminishedThird   =  2,
	aura_d3                =  2,

	aura_AugmentedSecond   =  3,
	aura_A2                =  3,
	aura_MinorThird        =  3,
	aura_m3                =  3,

	aura_MajorThird        =  4,
	aura_M3                =  4,
	aura_DiminishedFourth  =  4,
	aura_d4                =  4,

	aura_AugmentedThird    =  5,
	aura_A3                =  5,
	aura_PerfectFourth     =  5,
	aura_Fourth            =  5,
	aura_P4                =  5,

	aura_AugmentedFourth   =  6,
	aura_A4                =  6,
	aura_DiminishedFifth   =  6,
	aura_d5                =  6,
	aura_Tritone           =  6,
	aura_TT                =  6,

	aura_PerfectFifth      =  7,
	aura_Fifth             =  7,
	aura_P5                =  7,
	aura_DiminishedSixth   =  7,
	aura_d6                =  7,

	aura_AugmentedFifth    =  8,
	aura_A5                =  8,
	aura_MinorSixth        =  8,
	aura_m6                =  8,

	aura_MajorSixth        =  9,
	aura_M6                =  9,
	aura_DiminishedSeventh =  9,
	aura_d7                =  9,

	aura_AugmentedSixth    = 10,
	aura_A6                = 10,
	aura_MinorSeventh      = 10,
	aura_m7                = 10,

	aura_MajorSeventh      = 11,
	aura_M7                = 11,
	aura_DiminishedOctave  = 11,
	aura_d8                = 11,

	aura_AugmentedSeventh  = 12,
	aura_A7                = 12,
	aura_PerfectOctave     = 12,
	aura_Octave            = 12,
	aura_P8                = 12,
} aura_interval;
/*************************************/




/* Circle of Fifths ******************
 *  - Can be used for:
 *  	- Notes (e.g. for (major) chord creation: Note N, N + 2 whole steps, N + 5th)
 *  	- Chords (e.g. for finding related dominant chord)
 *  	- Keys (e.g. transition similarity)
 */
/* REF: https://www.youtube.com/watch?v=50CpDZvTWks */
/*  If you want to know what notes are in a key. Look at the Key note and move anti-clockwise (counter clockwise) one note and, including that note, count all seven going clockwise from that note? So from C move back 1 to F and all 7 from F inclusive are in the key of C.*/
/* TODO: AKA notes/chords/keys ? */
/****************************************************************
 *             MAJOR:                         MINOR: (K -> K + 3 CW)
 *
 *                C                             Am
 *          F           G                 Dm          Em
 *
 *      Bb                  D         Gm                  Bm
 *
 *     Eb                    A       Cm                    F#m
 *
 *      Ab                  E         Fm                  Dbm
 *
 *          Db          B                 Bbm         Abm
 *                F#                            Ebm
 *
 * ALT (https://www.youtube.com/watch?v=VixRcGf74kQ):
 *          Flats                  Sharps
 *    0                    C
 *    1                  F   G
 *    2                Bb      D
 *    3              Eb          A
 *    4            Ab              E
 *    5          Db                  B            (5 <-> 7)
 *    6        Gb                      F#         (6 <-> 6)
 *    7      Cb                          C#       (7 <-> 5)

 ***************************************************************/

/*************************************/
#define AURA_FIFTHS \
	AURA_FIFTH(C,  "C") \
	AURA_FIFTH(G,  "G") \
	AURA_FIFTH(D,  "D") \
	AURA_FIFTH(A,  "A") \
	AURA_FIFTH(E,  "E") \
	AURA_FIFTH(B,  "B") \
	AURA_FIFTH(Fs, "F#") \
	AURA_FIFTH(Db, "Db") \
	AURA_FIFTH(Ab, "Ab") \
	AURA_FIFTH(Eb, "Eb") \
	AURA_FIFTH(Bb, "Bb") \
	AURA_FIFTH(F,  "F") \

#define AURA_FIFTTH_DECORATE(x) AURA_5th_## x
#define AURA_FIFTH(x, s) AURA_FIFTH_DECORATE(x),
typedef enum aura_circle_of_5ths {
	AURA_FIFTHS

	AURA_FIFTH_DECORATE(Count),
	AURA_FIFTH_DECORATE(Error) = -1,

	/* for table mapping */
	/* TODO: should these map to enharmonics? */
	AURA_ENUM_MAP(5TH, Cb, Error),
	AURA_ENUM_MAP(5TH, Cs, Error),
	AURA_ENUM_MAP(5TH, Gb, Error),
	AURA_ENUM_MAP(5TH, Gs, Error),
	AURA_ENUM_MAP(5TH, Ds, Error),
	AURA_ENUM_MAP(5TH, As, Error),
	AURA_ENUM_MAP(5TH, Es, Error),
	AURA_ENUM_MAP(5TH, Bs, Error),
	AURA_ENUM_MAP(5TH, Fb, Error),
} aura_circle_of_5ths;
#undef AURA_FIFTH

#define AURA_FIFTH(x, s) AU_5th_## s,
char *aura5thsStrings[] = { AURA_FIFTHS "12" };
#undef AURA_FIFTH

#define AURA_FIFTH(x, s) AURA_NOTE_DECORATE(x),
#define AURA_NOTE(x, s) AURA_FIFTH_DECORATE(x),
AURA_NOTE_MAP(aura_circle_of_5ths, 5th, FIFTH, 0);
#undef AURA_FIFTH
#undef AURA_NOTE

/* e.g. by interval, inver */
aura_circle_of_5ths
aura_5thsOffset(aura_circle_of_5ths Fifth, int Offset) {
	aura_circle_of_5ths Result = (Fifth < AU_5ths_Count)
		? (Fifth + Offset) % AU_5ths_Count
		: AU_5ths_Error;
	return Result;
}

aura_circle_of_5ths
aura_5thNext(aura_circle_of_5ths Fifth) {
	aura_circle_of_5ths Result = (Fifth < AU_5ths_Count)
		? (Fifth + 1) % AU_5ths_Count
		: AU_5ths_Error;
	return Result;
}

typedef enum aura_major_minor {
	AURA_KEY_Minor = -1,
	AURA_KEY_Major =  1,
} aura_major_minor;

aura_circle_of_5ths
aura_AssociatedMajorMinorKey(aura_circle_of_5ths Key, aura_major_minor Current)
{ return aura_5thsOffset(Key, Current * 3); }

aura_circle_of_5ths aura_MajorThirdOf(aura_circle_of_5ths Note) { return aura_5thsOffset(Note,  4); }
aura_circle_of_5ths aura_MinorThirdOf(aura_circle_of_5ths Note) { return aura_5thsOffset(Note, -3); }

/* Always returns positive number shortest distance */
int
aura_5thDist(aura_circle_of_5ths A, aura_circle_of_5ths B) {
	int Result = AU_5ths_Error;
	if(A < AU_5ths_Count && B < AU_5ths_Count) {
	 	Result = A-B;
		if (Result < 0) { Result = -Result; }
		if (Result > 6) { Result = AU_5ths_Count - Result; }
	}
	return Result;
}
/* TODO (api fn): aura_5thDistCW ? */

/* TODO (consistency): when should I use 1-based numbering? */
typedef enum aura_scale_degree {
	AURA_Tonic = 1,     /* Restful */
	AURA_SuperTonic,    /* Medium Tension */
	AURA_Mediant,       /* Medium Tension */
	AURA_Subdominant,   /* High Tension */
	AURA_Dominant,      /* Medium Tension */
	AURA_LeadingNote,   /* Highest Tension */
	AURA_LeadingTone = AURA_LeadingNote,
} aura_scale_degree;

/* TODO: not sure if this currently only works for major chords
 * may need to do some official 'key' struct */
/* MajorMinor should be 1 for major or -1 for minor */
aura_circle_of_5ths
aura_RNADiatonicChord(aura_circle_of_5ths Chord, int Numeral, int MajorMinor)
{
	int Offsets[] = {/* if starting major: */
		 0, /* major */
		 2, /* minor */
		 4, /* minor */
		-1, /* major */
		 1, /* major */
		 3  /* minor */
	};
	int Offset = MajorMinor * Offsets[Numeral - 1]; /* account for 1-based counting */
	aura_circle_of_5ths Result = aura_5thsOffset(Chord, Offset);
	return Result;
}
/*************************************/

/* MODES *****************************/
typedef enum aura_mode {
/* Start point:
 *  I  D  P L  M  A  L
 *  0  1  2 3  4  5  6 0 
 *  |--|--|=|--|--|--|=|  */

	AURA_MODE_Ionian = 0, /*  |--|--|=|--|--|--|=  */
	AURA_MODE_Dorian,     /*  |--|=|--|--|--|=|--  */
	AURA_MODE_Phrygian,   /*  |=|--|--|--|=|--|--  */
	AURA_MODE_Lydian,     /*  |--|--|--|=|--|--|=  */
	AURA_MODE_Mixolydian, /*  |--|--|=|--|--|=|--  */
	AURA_MODE_Aeolian,    /*  |--|=|--|--|=|--|--  */
	AURA_MODE_Locrian,    /*  |=|--|--|=|--|--|--  */

	AURA_MODE_COUNT,
	
	AURA_MODE_Major = AURA_MODE_Ionian,
	AURA_MODE_Minor = AURA_MODE_Aeolian,

/* NOTES:
 * Negative harmony:
 * instead of going up by the pattern (2212221), go down by it.
 * e.g. Ionian  <-> Phrygian
 * 		Dorian  <-> Dorian
 *		Lydian  <-> Locrian
 *		Aeolian <-> Mixolydian
 */
} aura_mode

/*************************************/

/* SCALES *****************************
 * Scale ~= The notes of a key (i.e. not chords with the same names)
 *
 *  Major:
 *  |--|--|=|--|--|--|=
 *
 *  - End note leads you into the beginning
 *
 *
 *  Natural minor:
 *  |--|=|--|--|=|--|--
 *
 *  - No longer leading
 *
 *
 *  Harmonic minor:
 *  |--|=|--|--|=|---|=
 *                  >
 *  - Get lead back (better for tension->resolution)
 *  - Distinctive gap
 *
 *
 *  Melodic minor:
 *  |--|=|--|--|--|--|=
 *               >
 *  - resembles major
 *
 * In Classical music:
 *  - Go up with melodic minor
 *  - Go down with natural minor (leading not needed in that direction)
 *
 *************************************/

/* HARMONY ****************************
 * Common associations (triads) (REF: https://www.youtube.com/watch?v=YSKAt3pmYBs) :
 * (can be inverted)
 * M2M: protagonism
 * M6M: outer space
 * M8M: fantastical
 * M4m: Sadness, loss
 * M5m: Romantic, middle-eastern, quiet hope
 * m5M/M7m: Wonder, transcendence
 * m2M: mystery, dark comedy
 * m11M: dramatic sound
 * m6m: antagonism, danger
 * m8m: antagonism, evil (more character-based)
 * Trailers: m8M7M7M2m (repeated) (aka: i VI III VII)
 */
/*************************************/


/*************************************/
/* Harmonic Heptagon
 **************************************
 *        A===============F           *
 *       / \             / \          *
 *      /   ->E, D    C<-   \         *
 *     /                     \        *
 *    C->F,G                  \       *
 *    #                        \      *
 *    #                 A, G<---D     *
 *    #                        /      *
 *    E-->A,B                 /       *
 *     \                     /        *
 *      \   ->C,D     E<-   /         *
 *       \ /             \ /          *
 *        G===============B           *
 **************************************
 *   =, #   -   Major Third           *
 *   \, /   -   Minor Third           *
 *   -->    -   Perfect Fifth         *
 **************************************
 * if odd, move 1 away from 0 for m3
 **************************************/
#define AURA_HARMONIC_HEPTS \
	AURA_HARMONIC_HEPT(C, -3) \
	AURA_HARMONIC_HEPT(A, -2) \
	AURA_HARMONIC_HEPT(F, -1) \
	AURA_HARMONIC_HEPT(D,  0) \
	AURA_HARMONIC_HEPT(B,  1) \
	AURA_HARMONIC_HEPT(G,  2) \
	AURA_HARMONIC_HEPT(E,  3) \

#define AURA_HARMONIC_HEPT_DECORATE(x) AURA_HHEPT_## x
#define AURA_HARMONIC_HEPT(x, v) AURA_HARMONIC_HEPT_DECORATE(x) = v,
typedef enum aura_harmonic_heptagon {
	AURA_HARMONIC_HEPTS

	AURA_HARMONIC_HEPT_DECORATE(Min)   = -3,
	AURA_HARMONIC_HEPT_DECORATE(Max)   =  3,
	AURA_HARMONIC_HEPT_DECORATE(Count) =  7,
	AURA_HARMONIC_HEPT_DECORATE(Error) = -4,

	/* for table mapping */
	AURA_ENUM_MAP(HHEPT, Ab, Error),
    AURA_ENUM_MAP(HHEPT, As, Error),
    AURA_ENUM_MAP(HHEPT, Bb, Error),
    AURA_ENUM_MAP(HHEPT, Bs, Error),
    AURA_ENUM_MAP(HHEPT, Cb, Error),
    AURA_ENUM_MAP(HHEPT, Cs, Error),
    AURA_ENUM_MAP(HHEPT, Db, Error),
    AURA_ENUM_MAP(HHEPT, Ds, Error),
    AURA_ENUM_MAP(HHEPT, Eb, Error),
    AURA_ENUM_MAP(HHEPT, Es, Error),
    AURA_ENUM_MAP(HHEPT, Fb, Error),
    AURA_ENUM_MAP(HHEPT, Fs, Error),
    AURA_ENUM_MAP(HHEPT, Gb, Error),
    AURA_ENUM_MAP(HHEPT, Gs, Error),
} aura_harmonic_heptagon;
#undef AURA_HARMONIC_HEPT

#define AURA_HARMONIC_HEPT(x, s) AURA_NOTE_DECORATE(x),
#define AURA_NOTE(x, s) AURA_HARMONIC_HEPT_DECORATE(x),
AURA_NOTE_MAP(aura_harmonic_heptagon, HHEPT, HARMONIC_HEPT, AURA_HARMONIC_HEPT_DECORATE(Min));
#undef AURA_HARMONIC_HEPT
#undef AURA_NOTE
/*************************************/

int aura_KeySharpsCount(aura_circle_of_5ths Key) { return aura_5thDist(Key, AU_5ths_C); }

int aura_KeySharps(aura_circle_of_5ths Key, signature *Sig) {
	int i, Result = aura_5thDist(Key, AU_5ths_C);
	aura_circle_of_5ths Sharp = AU_5ths_F;
	for(i = 0; i <= Result-1; ++i) {
		Sig[Sharp] = 1;
		/* TODO */
	}
	return Result;
}
#endif/* AURA_IMPLEMENTATION || AURA_SELFTEST */
/*****************************************************************************/

#ifdef AURA_SELFTEST
#include <stdio.h>
#include "../sweet/sweet.h"

int main()
{
	TestGroup("Circle of Fifths");
	{
		TestEq(AU_5ths_Count, 12);
	} EndTestGroup;

	return 0;
}
#endif/*AURA_SELFTEST*/
