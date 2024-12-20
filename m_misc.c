// M_misc.c

#include <sys/stat.h>
#include <sys/types.h>
#include <direct.h>
#include <fcntl.h>
#include <stdlib.h>
#include <ctype.h>


#include "doomdef.h"
#include "z_zone.h"
#include "m_swap.h"
#include "m_argv.h"
#include "w_wad.h"
#include "i_system.h"
#include "i_video.h"
#include "v_video.h"
#include "sounds.h" // FS
#include "hu_stuff.h"
#include "doomstat.h" // State.
#include "dstrings.h" // Data.
#include "m_misc.h"

//
// M_DrawText
// Returns the final X coordinate
// HU_Init must have been called to init the font
//
extern patch_t*		hu_font[HU_FONTSIZE];

int
M_DrawText
( int		x,
  int		y,
  boolean	direct,
  char*		string )
{
    int 	c;
    int		w;

    while (*string)
    {
	c = toupper(*string) - HU_FONTSTART;
	string++;
	if (c < 0 || c> HU_FONTSIZE)
	{
	    x += 4;
	    continue;
	}
		
	w = SHORT (hu_font[c]->width);
	if (x+w > SCREENWIDTH)
	    break;
	if (direct)
	    V_DrawPatchDirect(x, y, 0, hu_font[c]);
	else
	    V_DrawPatch(x, y, 0, hu_font[c]);
	x+=w;
    }

    return x;
}




//
// M_WriteFile
//
#ifndef O_BINARY
#define O_BINARY 0
#endif

boolean
M_WriteFile
( char const*	name,
  void*		source,
  int		length )
{
    int		handle;
    int		count;
	
    handle = open ( name, O_WRONLY | O_CREAT | O_TRUNC | O_BINARY, 0666);

    if (handle == -1)
	return false;

    count = write (handle, source, length);
    close (handle);
	
    if (count < length)
	return false;
		
    return true;
}


//
// M_ReadFile
//
int M_ReadFile (char const*	name, byte**	buffer)
{
    int	handle, count, length;
    struct stat	fileinfo;
    byte		*buf;
	
    handle = open (name, O_RDONLY | O_BINARY, 0666);
    if (handle == -1)
		I_Error ("Couldn't read file %s", name);
    if (fstat (handle,&fileinfo) == -1)
		I_Error ("Couldn't read file %s", name);
    length = fileinfo.st_size;
    buf = Z_Malloc (length, PU_STATIC, NULL);
    count = read (handle, buf, length);
    close (handle);
	
    if (count < length)
		I_Error ("Couldn't read file %s", name);
		
    *buffer = buf;
    return length;
}


//
// DEFAULTS
//
int		usemouse;
int		usejoystick;

extern int	key_right;
extern int	key_left;
extern int	key_up;
extern int	key_down;

extern int	key_strafeleft;
extern int	key_straferight;

extern int	key_fire;
extern int	key_use;
extern int	key_strafe;
extern int	key_speed;

extern int	mousebfire;
extern int	mousebstrafe;
extern int	mousebforward;

extern int	joybfire;
extern int	joybstrafe;
extern int	joybuse;
extern int	joybspeed;

extern int	viewwidth;
extern int	viewheight;

extern int	mouseSensitivity;
extern int	showMessages;

/* FS: Extended.cfg stuff */
boolean headBob;
int	drawTime; /* FS: Draw Time on Automap */
boolean	useIntGus; /* FS: Use internal GUS1*.WADs */
boolean novert; /* FS: No vertical mouse movement */
boolean noprecache;
boolean noquitsound;
boolean nowipe;
boolean usePalFlash;
boolean alwaysRun; // 2024/11/10 toggle always run
boolean secretCount; // 2024/11/10 toggle secret counter
boolean bugFix; // 2024/11/10 toggle vanilla bug fixes
boolean newCheats; // 2024/11/10 make new cheats optional
boolean varPitch; // 2024/11/12 variable sound pitch
/* FS: For custom weapon binds */
boolean	use_wpnbinds;
int	wpn_shotgun;
int	wpn_chaingun;
int	wpn_rocket;
int	wpn_plasma; 

extern int	detailLevel;

extern int	screenblocks;

extern int	showMessages;
extern int	grmode; // FS: Disable disk flash
            
extern char*	chat_macros[];



typedef struct
{
    char*	name;
    int*	location;
    int		defaultvalue;
    int		scantranslate;		// PC scan code hack
    int		untranslated;		// lousy hack
} default_t;

extern int	snd_Channels;
extern int	snd_DesiredMusicDevice, snd_DesiredSfxDevice;
extern int	snd_MusicDevice; // current music card # (index to dmxCodes)
extern int	snd_SfxDevice; // current sfx card # (index to dmxCodes)

extern int	snd_SBport, snd_SBirq, snd_SBdma;       // sound blaster variables
extern int	snd_Mport;                              // midi variables

extern boolean deh_apply_cheats; // FS
extern boolean deh_internal; // FS
int	dndebug; // FS

default_t defaults[] =
{
    {"mouse_sensitivity",&mouseSensitivity, 5},
    {"sfx_volume",&snd_SfxVolume, 8},
    {"music_volume",&snd_MusicVolume, 8},
    {"show_messages",&showMessages, 1},
    
#ifdef __WATCOMC__
#define SC_UPARROW              0x48
#define SC_DOWNARROW            0x50
#define SC_LEFTARROW            0x4b
#define SC_RIGHTARROW           0x4d
#define SC_RCTRL                0x1d
#define SC_RALT                 0x38
#define SC_RSHIFT               0x36
#define SC_SPACE                0x39
#define SC_COMMA                0x33
#define SC_PERIOD               0x34
#define SC_PAGEUP				0x49
#define SC_INSERT				0x52
#define SC_HOME					0x47
#define SC_PAGEDOWN				0x51
#define SC_DELETE				0x53
#define SC_END					0x4f
#define SC_ENTER				0x1c

	{ "key_right", &key_right, SC_RIGHTARROW, 1 },
	{ "key_left", &key_left, SC_LEFTARROW, 1 },
	{ "key_up", &key_up, SC_UPARROW, 1 },
	{ "key_down", &key_down, SC_DOWNARROW, 1 },
	{ "key_strafeleft", &key_strafeleft, SC_COMMA, 1 },
	{ "key_straferight", &key_straferight, SC_PERIOD, 1 },

	{ "key_fire", &key_fire, SC_RCTRL, 1 },
	{ "key_use", &key_use, SC_SPACE, 1 },
	{ "key_strafe", &key_strafe, SC_RALT, 1 },
	{ "key_speed", &key_speed, SC_RSHIFT, 1 },
#endif

    {"use_mouse",&usemouse, 1},
    {"mouseb_fire",&mousebfire,0},
    {"mouseb_strafe",&mousebstrafe,1},
    {"mouseb_forward",&mousebforward,2},

    {"use_joystick",&usejoystick, 0},
    {"joyb_fire",&joybfire,0},
    {"joyb_strafe",&joybstrafe,1},
    {"joyb_use",&joybuse,3},
    {"joyb_speed",&joybspeed,2},

    {"screenblocks",&screenblocks, 9},
    {"detaillevel",&detailLevel, 0},

	{ "snd_channels", &snd_Channels, 3 },
	{ "snd_musicdevice", &snd_DesiredMusicDevice, 0 },
	{ "snd_sfxdevice", &snd_DesiredSfxDevice, 0 },
	{ "snd_sbport", &snd_SBport, 544 },
	{ "snd_sbirq", &snd_SBirq, -1 },
	{ "snd_sbdma", &snd_SBdma, -1 },
	{ "snd_mport", &snd_Mport, -1 },

	{ "usegamma", &usegamma, 0 },
 
    {"chatmacro0", (int *) &chat_macros[0], (int) HUSTR_CHATMACRO0 },
    {"chatmacro1", (int *) &chat_macros[1], (int) HUSTR_CHATMACRO1 },
    {"chatmacro2", (int *) &chat_macros[2], (int) HUSTR_CHATMACRO2 },
    {"chatmacro3", (int *) &chat_macros[3], (int) HUSTR_CHATMACRO3 },
    {"chatmacro4", (int *) &chat_macros[4], (int) HUSTR_CHATMACRO4 },
    {"chatmacro5", (int *) &chat_macros[5], (int) HUSTR_CHATMACRO5 },
    {"chatmacro6", (int *) &chat_macros[6], (int) HUSTR_CHATMACRO6 },
    {"chatmacro7", (int *) &chat_macros[7], (int) HUSTR_CHATMACRO7 },
    {"chatmacro8", (int *) &chat_macros[8], (int) HUSTR_CHATMACRO8 },
    {"chatmacro9", (int *) &chat_macros[9], (int) HUSTR_CHATMACRO9 }

};

// FS
default_t	extendeddefaults[] =
{
	{ "usePalFlash", &usePalFlash, 1 }, // FS: Palette Flashing
	{ "headBob", &headBob, 1 }, // FS: Head bob
	{ "drawTime", &drawTime, 0}, // FS: Draw time on Automap
	{ "useIntGus", &useIntGus, 0}, // FS: Use internal GUS WADs
	{ "novert", &novert, 0}, // FS: No vertical mouse movement
	{ "noprecache", &noprecache, 0}, // FS: No graphics precaching
	{ "noquitsound", &noquitsound, 0}, // FS: No quit sound
	{ "nowipe", &nowipe, 0}, // FS: No Wipes
	{ "alwaysRun", &alwaysRun, 0}, // 2024/11/10
	{ "secretCount", &secretCount, 0}, // 2024/11/10
	{ "bugFix", &bugFix, 0}, // 2024/11/10
	{ "newCheats", &newCheats, 0}, // 2024/11/10
	{ "varPitch", &varPitch, 0}, // 2024/11/12

	// FS: Use custom weapon binds
	{ "use_wpnbinds", &use_wpnbinds, 0},
	{ "wpn_shotgun", &wpn_shotgun, 44, 1 }, // FS: Z
	{ "wpn_chaingun", &wpn_chaingun, 45, 1 }, // FS: X
	{ "wpn_rocket", &wpn_rocket, 16, 1 }, // FS: Q
	{ "wpn_plasma", &wpn_plasma, 46, 1 }, // FS: C
	{ "disk_flash_icon", &grmode, 1 }, // FS: Disk Flashing
	{ "deh_apply_cheats", &deh_apply_cheats, 1}, // FS: Apply DEHacked Cheats
	{ "deh_internal", &deh_internal, 1}, // FS: Load DEHs from WADs
	{ "dndebug", &dndebug, 0} // FS: Verbose debugging for my sanity
};

int numextendeddefaults; // FS
int	numdefaults;
char*	defaultfile;


//
// M_SaveDefaults
//
void M_SaveDefaults (void)
{
    int		i;
    int		v;
    FILE*	f;
	
    f = fopen (defaultfile, "w");
    if (!f)
	return; // can't write the file, but don't complain
		
    for (i=0 ; i<numdefaults ; i++)
    {

#ifdef __WATCOMC__
		if (defaults[i].scantranslate)
			defaults[i].location = &defaults[i].untranslated;
#endif

	if (defaults[i].defaultvalue > -0xfff
	    && defaults[i].defaultvalue < 0xfff)
	{
	    v = *defaults[i].location;
	    fprintf (f,"%s\t\t%i\n",defaults[i].name,v);
	} else {
	    fprintf (f,"%s\t\t\"%s\"\n",defaults[i].name,
		     * (char **) (defaults[i].location));
	}
    }
	
    fclose (f);
}

void M_SaveExtendedDefaults (void)
{
    int		i;
    int		v;
    FILE*	f;
	
    f = fopen ("extend.cfg", "w");
    if (!f)
		return; // can't write the file, but don't complain
	
    for (i=0 ; i<numextendeddefaults ; i++)
    {

#ifdef __WATCOMC__
		if (extendeddefaults[i].scantranslate)
			extendeddefaults[i].location = &extendeddefaults[i].untranslated;
#endif

		if (extendeddefaults[i].defaultvalue > -0xfff && extendeddefaults[i].defaultvalue < 0xfff)
		{
		    v = *extendeddefaults[i].location;
		    fprintf (f,"%s\t\t%i\n",extendeddefaults[i].name,v);
		}
		else
		{
		    fprintf (f,"%s\t\t\"%s\"\n",extendeddefaults[i].name, * (char **) (extendeddefaults[i].location));
		}
    }
	
    fclose (f);

}


//
// M_LoadDefaults
//
extern byte	scantokey[128];

void M_LoadDefaults (void)
{
    int		i;
    int		len;
    FILE*	f;
    char	def[80];
    char	strparm[100];
    char*	newstring;
    int		parm;
    boolean	isstring;
    
    // set everything to base values
    numdefaults = sizeof(defaults)/sizeof(defaults[0]);
    for (i=0 ; i<numdefaults ; i++)
	*defaults[i].location = defaults[i].defaultvalue;
    
    // check for a custom default file
    i = M_CheckParm ("-config");
    if (i && i<myargc-1)
    {
	defaultfile = myargv[i+1];
	printf ("	default file: %s\n",defaultfile);
    }
    else
	defaultfile = basedefault;
    
    // read the file in, overriding any set defaults
    f = fopen (defaultfile, "r");
    if (f)
    {
	while (!feof(f))
	{
	    isstring = false;
	    if (fscanf (f, "%79s %[^\n]\n", def, strparm) == 2)
	    {
		if (strparm[0] == '"')
		{
		    // get a string default
		    isstring = true;
		    len = strlen(strparm);
		    newstring = (char *) malloc(len);
		    strparm[len-1] = 0;
		    strcpy(newstring, strparm+1);
		}
		else if (strparm[0] == '0' && strparm[1] == 'x')
		    sscanf(strparm+2, "%x", &parm);
		else
		    sscanf(strparm, "%i", &parm);
		for (i=0 ; i<numdefaults ; i++)
		    if (!strcmp(def, defaults[i].name))
		    {
			if (!isstring)
			    *defaults[i].location = parm;
			else
			    *defaults[i].location =
				(int) newstring;
			break;
		    }
	    }
	}
		
	fclose (f);
    }

#ifdef __WATCOMC__
	for(i = 0; i < numdefaults; i++)
	{
		if(defaults[i].scantranslate)
		{
			parm = *defaults[i].location;
			defaults[i].untranslated = parm;
			*defaults[i].location = scantokey[parm];
		}
	}
#endif

}

void M_LoadExtendedDefaults (void)
{
    int		i;
    int		len;
    FILE*	f;
    char	def[80];
    char	strparm[100];
    char*	newstring;
    int		parm;
    boolean	isstring;
    
    // set everything to base values
    numextendeddefaults = sizeof(extendeddefaults)/sizeof(extendeddefaults[0]);
    for (i=0 ; i<numextendeddefaults ; i++)
		*extendeddefaults[i].location = extendeddefaults[i].defaultvalue;
    
     // read the file in, overriding any set defaults
    f = fopen ("extend.cfg", "r");
    if (f)
    {
		while (!feof(f))
		{
		    isstring = false;
		    if (fscanf (f, "%79s %[^\n]\n", def, strparm) == 2)
		    {
				if (strparm[0] == '"')
				{
				    // get a string default
				    isstring = true;
				    len = strlen(strparm);
				    newstring = (char *) malloc(len);
				    strparm[len-1] = 0;
				    strcpy(newstring, strparm+1);
				}
				else if (strparm[0] == '0' && strparm[1] == 'x')
				    sscanf(strparm+2, "%x", &parm);
				else
				    sscanf(strparm, "%i", &parm);
				for (i=0 ; i<numextendeddefaults ; i++)
				    if (!strcmp(def, extendeddefaults[i].name))
				    {
						if (!isstring)
						    *extendeddefaults[i].location = parm;
						else
						    *extendeddefaults[i].location = (int) newstring;
						break;
					}
			}          
		}
		fclose (f);
	}

#ifdef __WATCOMC__
	for(i = 0; i < numextendeddefaults; i++)
	{
		if(extendeddefaults[i].scantranslate)
		{
			parm = *extendeddefaults[i].location;
			extendeddefaults[i].untranslated = parm;
			*extendeddefaults[i].location = scantokey[parm];
		}
	}
#endif
}


//
// SCREEN SHOTS
//


typedef struct
{
    char		manufacturer;
    char		version;
    char		encoding;
    char		bits_per_pixel;

    unsigned short	xmin;
    unsigned short	ymin;
    unsigned short	xmax;
    unsigned short	ymax;
    
    unsigned short	hres;
    unsigned short	vres;

    unsigned char	palette[48];
    
    char		reserved;
    char		color_planes;
    unsigned short	bytes_per_line;
    unsigned short	palette_type;
    
    char		filler[58];
    unsigned char	data;		// unbounded
} pcx_t;


//
// WritePCXfile
//
void
WritePCXfile
( char*		filename,
  byte*		data,
  int		width,
  int		height,
  byte*		palette )
{
    int		i;
    int		length;
    pcx_t*	pcx;
    byte*	pack;
	
    pcx = Z_Malloc (width*height*2+1000, PU_STATIC, NULL);

    pcx->manufacturer = 0x0a;		// PCX id
    pcx->version = 5;			// 256 color
    pcx->encoding = 1;			// uncompressed
    pcx->bits_per_pixel = 8;		// 256 color
    pcx->xmin = 0;
    pcx->ymin = 0;
    pcx->xmax = SHORT(width-1);
    pcx->ymax = SHORT(height-1);
    pcx->hres = SHORT(width);
    pcx->vres = SHORT(height);
    memset (pcx->palette,0,sizeof(pcx->palette));
    pcx->color_planes = 1;		// chunky image
    pcx->bytes_per_line = SHORT(width);
    pcx->palette_type = SHORT(2);	// not a grey scale
    memset (pcx->filler,0,sizeof(pcx->filler));


    // pack the image
    pack = &pcx->data;
	
    for (i=0 ; i<width*height ; i++)
    {
	if ( (*data & 0xc0) != 0xc0)
	    *pack++ = *data++;
	else
	{
	    *pack++ = 0xc1;
	    *pack++ = *data++;
	}
    }
    
    // write the palette
    *pack++ = 0x0c;	// palette ID byte
    for (i=0 ; i<768 ; i++)
	*pack++ = *palette++;
    
    // write output file
    length = pack - (byte *)pcx;
    M_WriteFile (filename, pcx, length);

    Z_Free (pcx);
}


//
// M_ScreenShot
//
void M_ScreenShot (void)
{
	int		i;
	byte*	linear;
	char	lbmname[12];

	// munge planar buffer to linear
	linear = screens[2];
	I_ReadScreen (linear);

    // find a file name to save it to
	strcpy(lbmname,DEH_String("DOOM00.pcx"));
		
	for (i=0 ; i<=99 ; i++)
	{
		lbmname[4] = i/10 + '0';
		lbmname[5] = i%10 + '0';
		if (access(lbmname,0) == -1)
		    break;	// file doesn't exist
	}
    if (i==100)
		I_Error ("M_ScreenShot: Couldn't create a PCX");

	// save the pcx file
	WritePCXfile (lbmname, linear, SCREENWIDTH, SCREENHEIGHT, W_CacheLumpName (DEH_String("PLAYPAL"),PU_CACHE));

	players[consoleplayer].message = DEH_String("screen shot");
}
