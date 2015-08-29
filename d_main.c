// D_main.c

#define	BGCOLOR		7
#define	FGCOLOR		8

#ifdef __WATCOMC__
#include <dos.h>
#include <io.h>
#include <stdlib.h>
#endif


#include "doomdef.h"
#include "doomstat.h"

#include "dstrings.h"
#include "sounds.h"


#include "z_zone.h"
#include "w_wad.h"
#include "s_sound.h"
#include "v_video.h"

#include "f_finale.h"
#include "f_wipe.h"

#include "m_argv.h"
#include "m_misc.h"
#include "m_menu.h"

#include "i_system.h"
#include "i_sound.h"
#include "i_video.h"

#include "deh_main.h" // FS: For DEHacked
#include "g_game.h"

#include "hu_stuff.h"
#include "wi_stuff.h"
#include "st_stuff.h"
#include "am_map.h"

#include "p_setup.h"
#include "r_local.h"


#include "d_main.h"

//
// D-DoomLoop()
// Not a globally visible function,
//  just included for source reference,
//  called by D_DoomMain, never exits.
// Manages timing and IO,
//  calls all ?_Responder, ?_Ticker, and ?_Drawer,
//  calls I_GetTime, I_StartFrame, and I_StartTic
//
void D_DoomLoop (void);


char*		wadfiles[MAXWADFILES];


boolean	devparm; // started game with -devparm
boolean	nomonsters;	// checkparm of -nomonsters
boolean	respawnparm; // checkparm of -respawn
boolean	fastparm; // checkparm of -fast
boolean	debugmode; // FS

boolean	drone;

boolean	singletics = false; // debug flag to cancel adaptiveness



//extern int soundVolume;
//extern  int	sfxVolume;
//extern  int	musicVolume;

extern boolean	inhelpscreens;

skill_t	startskill;
int	startepisode;
int	startmap;
boolean	autostart;

FILE*		debugfile;

boolean	advancedemo;
boolean	plutonia = false; // FS
boolean	tnt = false; // FS
boolean	chex = false; // FS: For Chex(R) Quest
boolean	chex2 = false; // FS: For Chex Quest 2

char	wadfile[1024];		// primary wad file
char	mapdir[1024];		   // directory of development maps
char	basedefault[1024];	  // default file
extern int headBob; // FS: Head bob toggle
extern boolean usePalFlash; // FS

// FS: For all the Save, Convert, and Load game stuff
extern int savegamesize; // FS
extern int savestringsize; // FS
extern menu_t SaveDef; // FS
extern menu_t LoadDef; // FS
int	saveconvertslot; // FS: Convert Save
boolean	convertsave = false; // FS: Convert Save

void D_CheckNetGame (void);
void D_ProcessEvents (void);
void G_BuildTiccmd (ticcmd_t* cmd);
void D_DoAdvanceDemo (void);
void mprintf(char *string); // FS: From OG Doom

#ifdef __WATCOMC__
void CleanExit(void) // FS: From Heretic
{
	union REGS regs;

	I_ShutdownKeyboard();
	regs.x.eax = 0x3;
	int386(0x10, &regs, &regs);
	DEH_printf("DOOM Startup Aborted!\n");
	exit(1);
}
#endif

void CheckAbortStartup(void) // FS: From Heretic
{
#ifdef __WATCOMC__
	extern int lastpress;

	if(lastpress == 1)
	{ // Abort if escape pressed
		CleanExit();
	}
#endif
}

//
// EVENT HANDLING
//
// Events are asynchronous inputs generally generated by the game user.
// Events can be discarded if no responder claims them
//
event_t		 events[MAXEVENTS];
int			 eventhead;
int 		eventtail;


//
// D_PostEvent
// Called by the I/O functions when input is detected
//
void D_PostEvent (event_t* ev)
{
	events[eventhead] = *ev;
	eventhead = (++eventhead)&(MAXEVENTS-1);
}


//
// D_ProcessEvents
// Send all the events of the given timestamp down the responder chain
//
void D_ProcessEvents (void)
{
	event_t*	ev;

	for ( ; eventtail != eventhead ; eventtail = (++eventtail)&(MAXEVENTS-1) )
	{
		ev = &events[eventtail];
		if (M_Responder (ev))
			continue;			   // menu ate the event
		G_Responder (ev);
	}
}




//
// D_Display
//  draw current display, possibly wiping it from the previous
//

// wipegamestate can be set to -1 to force a wipe on the next draw
gamestate_t	 wipegamestate = GS_DEMOSCREEN;
extern  boolean setsizeneeded;
extern  int			 showMessages;
void R_ExecuteSetViewSize (void);

void D_Display (void)
{
	static  boolean		viewactivestate = false;
	static  boolean		menuactivestate = false;
	static  boolean		inhelpscreensstate = false;
	static  boolean		fullscreen = false;
	static  gamestate_t		oldgamestate = -1;
	static  int			borderdrawcount;
	int				nowtime;
	int				tics;
	int				wipestart;
	int				y;
	boolean			done;
	boolean			wipe;
	boolean			redrawsbar;

	if (nodrawers)
		return;					// for comparative timing / profiling
		
	redrawsbar = false;
	
	// change the view size if needed
	if (setsizeneeded)
	{
		R_ExecuteSetViewSize ();
		oldgamestate = -1;					  // force background redraw
		borderdrawcount = 3;
	}

	// save the current screen if about to wipe
	if (gamestate != wipegamestate)
	{
		wipe = true;
		wipe_StartScreen(0, 0, SCREENWIDTH, SCREENHEIGHT);
	}
	else
		wipe = false;

	if (gamestate == GS_LEVEL && gametic)
		HU_Erase();
	
	// do buffered drawing
	switch (gamestate)
	{
		case GS_LEVEL:
			if (!gametic)
				break;
			if (automapactive)
				AM_Drawer ();
			if (wipe || (viewheight != 200 && fullscreen) )
				redrawsbar = true;
			if (inhelpscreensstate && !inhelpscreens)
				redrawsbar = true;			  // just put away the help screen

			ST_Drawer (viewheight == 200, redrawsbar );
			fullscreen = viewheight == 200;
			break;

		case GS_INTERMISSION:
			WI_Drawer ();
			break;

		case GS_FINALE:
			F_Drawer ();
			break;

		case GS_DEMOSCREEN:
			D_PageDrawer ();
			break;
	}
	
	// draw buffered stuff to screen
//	I_UpdateNoBlit (); // FS: Not needed.
	
	// draw the view directly
	if (gamestate == GS_LEVEL && !automapactive && gametic)
		R_RenderPlayerView (&players[displayplayer]);

	if (gamestate == GS_LEVEL && gametic)
		HU_Drawer ();
	
	// clean up border stuff
	if (gamestate != oldgamestate && gamestate != GS_LEVEL)
		I_SetPalette (W_CacheLumpName (DEH_String("PLAYPAL"),PU_CACHE));

	// see if the border needs to be initially drawn
	if (gamestate == GS_LEVEL && oldgamestate != GS_LEVEL)
	{
		viewactivestate = false;		// view was not active
		R_FillBackScreen ();	// draw the pattern into the back screen
	}

	// see if the border needs to be updated to the screen
	if (gamestate == GS_LEVEL && !automapactive && scaledviewwidth != 320)
	{
		if (menuactive || menuactivestate || !viewactivestate)
			borderdrawcount = 3;
		if (borderdrawcount)
		{
			R_DrawViewBorder ();	// erase old menu stuff
			borderdrawcount--;
		}
	}

	menuactivestate = menuactive;
	viewactivestate = viewactive;
	inhelpscreensstate = inhelpscreens;
	oldgamestate = wipegamestate = gamestate;
	
	// draw pause pic
	if (paused)
	{
		if (automapactive)
			y = 4;
		else
			y = viewwindowy+4;
		V_DrawPatchDirect(viewwindowx+(scaledviewwidth-68)/2,y,0,W_CacheLumpName (DEH_String("M_PAUSE"), PU_CACHE));
	}


	// menus go directly to the screen
	M_Drawer ();		  // menu is drawn even on top of everything
	NetUpdate ();		 // send out any new accumulation


	// normal update
	if (!wipe)
	{
		if (fullscreen) // FS: Full screen new school
				ST_Drawer (viewheight == 200, redrawsbar );
		I_Update ();			  // page flip or blit buffer
		return;
	}

	if (!inhelpscreensstate) // FS: Don't draw this if we're in the Read This! Menu, makes things a little faster.
	{
		// wipe update
		wipe_EndScreen(0, 0, SCREENWIDTH, SCREENHEIGHT);

		wipestart = I_GetTime () - 1;

		do
		{
			do
			{
				nowtime = I_GetTime ();
				tics = nowtime - wipestart;
			}
			while (!tics);
			wipestart = nowtime;
			done = wipe_ScreenWipe(wipe_Melt, 0, 0, SCREENWIDTH, SCREENHEIGHT, tics);
			//I_UpdateNoBlit ();	// FS: Not needed.
			M_Drawer ();							// menu is drawn even on top of wipes
			I_Update ();					  // page flip or blit buffer
		}
		while (!done);
	}
}



//
//  D_DoomLoop
//
extern  boolean		 demorecording;

void D_DoomLoop (void)
{
	if (demorecording)
		G_BeginRecording ();
		
	if (M_CheckParm ("-debugfile"))
	{
		char	filename[20];
		sprintf (filename,"debug%i.txt",consoleplayer);
		DEH_printf ("debug output to: %s\n",filename);
		debugfile = fopen (filename,"w");
	}
	
	I_InitGraphics ();


	while (1)
	{
		// frame syncronous IO operations
		I_StartFrame ();				
	
		// process one or more tics
		if (singletics)
		{
			I_StartTic ();
			D_ProcessEvents ();
			G_BuildTiccmd (&netcmds[consoleplayer][maketic%BACKUPTICS]);
			if (advancedemo)
				D_DoAdvanceDemo ();
			M_Ticker ();
			G_Ticker ();
			gametic++;
			maketic++;
		}
		else
		{
			TryRunTics (); // will run at least one tic
		}
		
		S_UpdateSounds (players[consoleplayer].mo);// move positional sounds

		// Update display, next frame, with current state.
		D_Display ();
	}
}



//
//  DEMO LOOP
//
int			 demosequence;
int			 pagetic;
char					*pagename;


//
// D_PageTicker
// Handles timing for warped projection
//
void D_PageTicker (void)
{
	if (--pagetic < 0)
		D_AdvanceDemo ();
}



//
// D_PageDrawer
//
void D_PageDrawer (void)
{
	V_DrawPatch (0,0, 0, W_CacheLumpName(pagename, PU_CACHE));
}


//
// D_AdvanceDemo
// Called after each demo or intro demosequence finishes
//
void D_AdvanceDemo (void)
{
	advancedemo = true;
}


//
// This cycles through the demo sequences.
// FIXME - version dependend demo numbers?
//
void D_DoAdvanceDemo (void)
{
	players[consoleplayer].playerstate = PST_LIVE;  // not reborn
	advancedemo = false;
	usergame = false;			   // no save / end game here
	paused = false;
	gameaction = ga_nothing;

	if ( gamemode == retail )
		demosequence = (demosequence+1)%7;
	else
		demosequence = (demosequence+1)%6;
	
	switch (demosequence)
	{
		case 0:
			if ( gamemode == commercial )
				pagetic = 35 * 11;
			else
				pagetic = 170;
			gamestate = GS_DEMOSCREEN;
			pagename = DEH_String("TITLEPIC");
			if ( gamemode == commercial )
				S_StartMusic(mus_dm2ttl);
			else
				S_StartMusic (mus_intro);
			break;
		case 1:
			G_DeferedPlayDemo (DEH_String("demo1"));
			break;
		case 2:
			pagetic = 200;
			gamestate = GS_DEMOSCREEN;
			pagename = DEH_String("CREDIT");
			break;
		case 3:
			G_DeferedPlayDemo (DEH_String("demo2"));
			break;
		case 4:
			gamestate = GS_DEMOSCREEN;
			if ( gamemode == commercial)
			{
				pagetic = 35 * 11;
				pagename = DEH_String("TITLEPIC");
				S_StartMusic(mus_dm2ttl);
			}
			else
			{
				pagetic = 200;
				if ( gamemode == retail || chex) // FS: Chex Quest uses CREDIT
				  pagename = DEH_String("CREDIT");
				else
				  pagename = DEH_String("HELP2");
			}
			break;
		case 5:
			G_DeferedPlayDemo (DEH_String("demo3"));
			break;
		// THE DEFINITIVE DOOM Special Edition demo
		case 6:
			G_DeferedPlayDemo (DEH_String("demo4"));
			break;
	}
}



//
// D_StartTitle
//
void D_StartTitle (void)
{
	gameaction = ga_nothing;
	demosequence = -1;
	D_AdvanceDemo ();
}

#ifdef __WATCOMC__
//====================================================
//
// Print (in color) a string
//
//====================================================
int getx(void)
{
   union REGS r;
   r.h.ah = 3;
   r.h.bh = 0;
   int386(0x10,&r,&r);
   return r.h.dl;
}
int gety(void)
{
   union REGS r;
   r.h.ah = 3;
   r.h.bh = 0;
   int386(0x10,&r,&r);
   return r.h.dh;
}
void setxy(int x,int y)
{
   union REGS r;
	r.h.ah = 2;
	r.h.bh = 0;
	r.h.dh = y;
	r.h.dl = x;
	int386(0x10,&r,&r);
}

void dprint(char *string,int fg,int bg)
{
   union REGS r;
   int   i,x,y;
   char  color;

   color = (bg << 4) | fg;

	x = getx();
	y = gety();

   for (i = 0; i < strlen(string); i++)
   {
	  r.h.ah = 9;
	  r.h.al = string[i];
	  r.h.bh = 0;
	  r.h.bl = color;
	  r.w.cx = 1;
	  int386(0x10,&r,&r);

	  x++;
	  if (x > 79)
	 x = 0;
		setxy(x,y);
   }
}
#endif




//	  print title for every printed line
char			title[128];
void mprintf(char *string)
{
#ifdef __WATCOMC__
	int x;
	int	 y;
#endif
	
	printf(string);
#ifdef __WATCOMC__
	x = getx();
	y = gety();
	setxy(0,0);	 
	dprint (title,FGCOLOR,BGCOLOR);
	setxy(x,y);
#endif
}

//
// D_AddFile
//
void D_AddFile (char *file)
{
	int	 numwadfiles;
	char	*newfile;

	for (numwadfiles = 0 ; wadfiles[numwadfiles] ; numwadfiles++)
	;

	newfile = malloc (strlen(file)+1);
	strcpy (newfile, file);

	wadfiles[numwadfiles] = newfile;
}

//
// IdentifyVersion
// Checks availability of IWAD files by name,
// to determine whether registered/commercial features
// should be executed (notably loading PWAD's).
//

void IdentifyVersion (void)
{

	char	doom1wad[10] = "doom1.wad";
	char	doomwad[9] = "doom.wad";
	char	doomuwad[10] = "doomu.wad";
	char	doom2wad[10] = "doom2.wad";

	char	doom2fwad[10] = "doom2f.wad";
	char	plutoniawad[13] = "plutonia.wad";
	char	tntwad[8] = "tnt.wad";
	char	chexwad[9] = "chex.wad"; // FS: Chex Quest 1
	char	chex2wad[10] = "chex2.wad"; // FS: Chex Quest 2

	sprintf(basedefault, "default.cfg");

	if (M_CheckParm ("-shdev"))
	{
		gamemode = shareware;
		devparm = true;
		D_AddFile (DEVDATA"doom1.wad");
		D_AddFile (DEVMAPS"data_se/texture1.lmp");
		D_AddFile (DEVMAPS"data_se/pnames.lmp");
		strcpy (basedefault,DEVDATA"default.cfg");
		return;
	}

	if (M_CheckParm ("-regdev"))
	{
		gamemode = registered;
		devparm = true;
		D_AddFile (DEVDATA"doom.wad");
		D_AddFile (DEVMAPS"data_se/texture1.lmp");
		D_AddFile (DEVMAPS"data_se/texture2.lmp");
		D_AddFile (DEVMAPS"data_se/pnames.lmp");
		strcpy (basedefault,DEVDATA"default.cfg");
		return;
	}

	if (M_CheckParm ("-comdev"))
	{
		gamemode = commercial;
		devparm = true;
		D_AddFile (DEVDATA"doom2.wad");
		
		D_AddFile (DEVMAPS"cdata/texture1.lmp");
		D_AddFile (DEVMAPS"cdata/pnames.lmp");
		strcpy (basedefault,DEVDATA"default.cfg");
		strcpy(basedefault, "default.cfg");
		return;
	}

	if (M_CheckParm ("-chex")) // FS: Chex Quest
	{
		gamemode = registered;
		devparm = false;
		chex = true;
		strcpy(basedefault, "default.cfg");
		D_AddFile("chex.wad");
		return;
	}

	if (M_CheckParm ("-chex2")) // FS: Chex Quest 2
	{
		gamemode = registered;
		devparm = false;
		chex = true;
		chex2 = true;
		strcpy(basedefault, "default.cfg");

		if ( !access (chexwad, 0) )
			D_AddFile (chexwad);
		else
			I_Error("You need CHEX.WAD to play Chex Quest 2!");

		D_AddFile("chex2.wad");
		return;
	}

	if (M_CheckParm ("-tnt"))
	{
		gamemode = commercial;
		devparm = false;
		tnt = true; // FS
		strcpy(basedefault, "default.cfg");
		D_AddFile("tnt.wad");
		return;
	}

	if (M_CheckParm ("-doom2"))
	{
		gamemode = commercial;
		devparm = false;

		strcpy(basedefault, "default.cfg");
		D_AddFile("doom2.wad");
		return;
	}

	if (M_CheckParm ("-helltopay") || M_CheckParm("-hell2pay")) // FS: Hell to pay commercial WAD
	{
		gamemode = commercial;
		devparm = false;

		strcpy(basedefault, "default.cfg");
		D_AddFile("hell2pay.wad");
		D_AddFile("htpdmo19.wad");
		return;
	}

	if (M_CheckParm ("-perdgate")) // FS: Perdition's Gate commercial WAD
	{
		gamemode = commercial;
		devparm = false;

		strcpy(basedefault, "default.cfg");
		D_AddFile("perdgate.wad");
		D_AddFile("pgdemo19.WAD");
		return;
	}

	if (M_CheckParm ("-doomu"))
	{
		gamemode = retail;
		devparm = false;

		strcpy(basedefault, "default.cfg");
		D_AddFile("doomu.wad");
		return;
	}

	if (M_CheckParm ("-plutonia"))
	{
		gamemode = commercial;
		devparm = false;
		plutonia = true; // FS
		strcpy(basedefault, "default.cfg");
		D_AddFile("plutonia.wad");
		return;
	}

	if (M_CheckParm ("-doom"))
	{
		gamemode = registered;
		devparm = false;

		strcpy(basedefault, "default.cfg");
		if (!access (doomwad,0)) // FS: Hack for 3-episode version from Ultimate WAD
			D_AddFile("doom.wad");
		else
			D_AddFile("doomu.wad");
		return;
	}

	if(M_CheckParm("-shareware"))
	{
		gamemode = shareware;
		devparm = false;
		
		strcpy(basedefault, "default.cfg");
		D_AddFile("doom1.wad");
		return;
	}
	
	if ( !access (doom2wad,0) )
	{
		gamemode = commercial;
		D_AddFile (doom2wad);
		return;
	}

	if ( !access (plutoniawad, 0) )
	{
		gamemode = commercial;
		plutonia = true; // FS
		D_AddFile (plutoniawad);
		return;
	}

	if ( !access ( tntwad, 0) )
	{
		gamemode = commercial;
		tnt = true; // FS
		D_AddFile (tntwad);
		return;
	}

	if ( !access (doomuwad,0) )
	{
		gamemode = retail;
		D_AddFile (doomuwad);
		return;
	}

	if ( !access (doomwad,0) )
	{
		if (W_CheckNumForName("e4m1")<0) // FS: If Episode 4 exists, then we assume it's Ultimate Doom
			gamemode = retail;
		else
			gamemode = registered;
		D_AddFile (doomwad);
		return;
	}

	if ( !access (doom1wad,0) )
	{
		gamemode = shareware;
		D_AddFile (doom1wad);
		return;
	}

	if ( !access (chexwad, 0) ) // FS: Chex Quest check
	{
		gamemode = registered;
		chex = true;
		chex2 = false;
		D_AddFile (chexwad);
		return;
	}

	printf("Game mode indeterminate\n");
	exit(1);
}

//
// Find a Response File
//
void FindResponseFile (void)
{
	int			 i;
	#define MAXARGVS		100
	
	for (i = 1;i < myargc;i++)
		if (myargv[i][0] == '@')
		{
			FILE	*handle;
			int		size;
			int		k;
			int		index;
			int		indexinfile;
			char	*infile;
			char	*file;
			char	*moreargs[20];
			char	*firstargv;

			// READ THE RESPONSE FILE INTO MEMORY
			handle = fopen (&myargv[i][1],"rb");
			if (!handle)
			{
				DEH_printf ("\nNo such response file!");
				exit(1);
			}
			DEH_printf("Found response file %s!\n",&myargv[i][1]);
			fseek (handle,0,SEEK_END);
			size = ftell(handle);
			fseek (handle,0,SEEK_SET);
			file = malloc (size);
			fread (file,size,1,handle);
			fclose (handle);

			// KEEP ALL CMDLINE ARGS FOLLOWING @RESPONSEFILE ARG
			for (index = 0,k = i+1; k < myargc; k++)
				moreargs[index++] = myargv[k];

			firstargv = myargv[0];
			myargv = malloc(sizeof(char *)*MAXARGVS);
			memset(myargv,0,sizeof(char *)*MAXARGVS);
			myargv[0] = firstargv;

			infile = file;
			indexinfile = k = 0;
			indexinfile++;  // SKIP PAST ARGV[0] (KEEP IT)
			do
			{
				myargv[indexinfile++] = infile+k;
				while(k < size && ((*(infile+k)>= ' '+1) && (*(infile+k)<='z')))
					k++;
				*(infile+k) = 0;
				while(k < size && ((*(infile+k)<= ' ') || (*(infile+k)>'z')))
					k++;
			}
			while(k < size);

			for (k = 0;k < index;k++)
				myargv[indexinfile++] = moreargs[k];
				myargc = indexinfile;

			// DISPLAY ARGS
			DEH_printf("%d command-line args:\n",myargc);
			for (k=1;k<myargc;k++)
				DEH_printf("%s\n",myargv[k]);

			break;
		}
}

// Copyright message banners
// Some dehacked mods replace these.  These are only displayed if they are 
// replaced by dehacked.

static char *copyright_banners[] =
{
    "===========================================================================\n"
    "ATTENTION:  This version of DOOM has been modified.  If you would like to\n"
    "get a copy of the original game, call 1-800-IDGAMES or see the readme file.\n"
    "        You will not receive technical support for modified games.\n"
    "                      press enter to continue\n"
    "===========================================================================\n",

    "===========================================================================\n"
    "                 Commercial product - do not distribute!\n"
    "         Please report software piracy to the SPA: 1-800-388-PIR8\n"
    "===========================================================================\n",

    "===========================================================================\n"
    "                                Shareware!\n"
    "===========================================================================\n"
};

// Prints a message only if it has been modified by dehacked.

void PrintDehackedBanners(void)
{
    size_t i;

    for (i=0; i<arrlen(copyright_banners); ++i)
    {
        char *deh_s;

        deh_s = DEH_String(copyright_banners[i]);

        if (deh_s != copyright_banners[i])
        {
            printf("%s", deh_s);

            // Make sure the modified banner always ends in a newline character.
            // If it doesn't, add a newline.  This fixes av.wad.

            if (deh_s[strlen(deh_s) - 1] != '\n')
            {
                printf("\n");
            }
        }
    }
}



//
// D_DoomMain
//
void D_DoomMain (void)
{
	int			 p;
	int e;
	int m;
	char					file[256];
	FILE *fp;
	boolean devMap;
	char *bannerbuffer; // FS
	
	FindResponseFile ();

	devparm = M_CheckParm ("-devparm"); // FS: Needs to be sooner in case you use -regdev, etc.

	// FS: Define a custom main WAD
	p = M_CheckParm("-mainwad");
	if(p)
	{
		sprintf(basedefault, "default.cfg");
		wadfiles[0] = myargv[p+1];
		W_AddFile(wadfiles[0]);

		if(W_CheckNumForName("MAP01") != -1)
			gamemode = commercial;
		else if(W_CheckNumForName("E4M1") != -1)
			gamemode = retail;
		else if(W_CheckNumForName("E3M1") != -1)
			gamemode = registered;
		else if(W_CheckNumForName("E1M1") != -1)
			gamemode = shareware;
		else
			I_Error("Unable to determine game version!");
	}
	else
		IdentifyVersion ();


	setbuf (stdout, NULL);
	modifiedgame = false;
	nomonsters = M_CheckParm ("-nomonsters");
	respawnparm = M_CheckParm ("-respawn");
	fastparm = M_CheckParm ("-fast");

	if (M_CheckParm("-oldsave")) // FS: Use old save if we still have them
	{
		savegamesize = 0x2c000;
		savestringsize = 24;
	}
	if (M_CheckParm ("-altdeath"))
		deathmatch = 2;
	else if (M_CheckParm ("-deathmatch"))
		deathmatch = 1;


	switch ( gamemode )
	{
		case retail:
			sprintf (title,
			"                         "
			"The Ultimate DOOM Startup v%i.%i"
			"                           ",
			VERSION/100,VERSION%100);
			break;
		case shareware:
			sprintf (title,
			"                            "
			"DOOM Shareware Startup v%i.%i"
			"                           ",
			VERSION/100,VERSION%100);
			break;
		case registered:
			if(chex2)
			{
				sprintf (title,
				"                            "
				"Chex(R) Quest 2 Startup      "
				"                           ");
				break;
			}
			else if(chex)
			{
				sprintf (title,
				"                            "
				"Chex(R) Quest Startup        "
				"                           ");
				break;
			}
			else
			{
				sprintf (title,
				"                            "
				"DOOM Registered Startup v%i.%i"
				"                           ",
				VERSION/100,VERSION%100);
			}
			break;
		case commercial:
			if(plutonia)
			{
				sprintf (title,
				"                   "
				"DOOM 2: Plutonia Experiment v%i.%i"
				"                             ",
				VERSION/100,VERSION%100);
			}
			else if(tnt)
			{
				sprintf (title,
				"                     "
				"DOOM 2: TNT - Evilution v%i.%i"
				"                               ",
				VERSION/100,VERSION%100);
			}
			else
			{
				sprintf (title,
				"                         "
				"DOOM 2: Hell on Earth v%i.%i"
				"                             ",
				VERSION/100,VERSION%100);
			}
			break;
		default:
			sprintf (title,
			"                     "
			"Public DOOM - v%i.%i"
			"                           ",
			VERSION/100,VERSION%100);
			break;
	}

#ifdef __WATCOMC__
	{  
		union REGS regs;
		regs.w.ax = 3;
		int386(0x10,&regs,&regs);
		dprint (title,FGCOLOR,BGCOLOR);
		// ADD SPACES TO BUMP EXE SIZE
		DEH_printf("\nP_Init: Checking cmd-line parameters...\n");
	}
#else
	printf ("%s\n",title);
#endif

	fp = fopen(wadfiles[0], "rb");
	if(fp)
	{
		fclose(fp);
	}
	else
	{ // Change to look for shareware wad
		wadfiles[0] = "doom.wad";
	}

	// Check for -CDROM
	if (M_CheckParm("-cdrom"))
	{
		printf(D_CDROM);
		mkdir("c:\\doomdata",0);
		strcpy (basedefault,"c:/doomdata/default.cfg");
	}

	// -FILE [filename] [filename] ...
	// Add files to the wad list.
	p = M_CheckParm("-file");
	if(p)
	{	// the parms after p are wadfile/lump names, until end of parms
		// or another - preceded parm
		while(++p != myargc && myargv[p][0] != '-')
		{
			D_AddFile(myargv[p]);
		}
	}

	if (M_CheckParm("-gus")) // FS: GUS1M patches
	{
		if(gamemode == commercial)
			D_AddFile("GUS1MIID.WAD");
		else
			D_AddFile("GUS1M.WAD");
	}

	// turbo option
	if ( (p=M_CheckParm ("-turbo")) )
	{
		int 		scale = 200;
		extern int	forwardmove[2];
		extern int	sidemove[2];
	
		if (p<myargc-1)
			scale = atoi (myargv[p+1]);
		if (scale < 10)
			scale = 10;
		if (scale > 400)
			scale = 400;
		DEH_printf ("turbo scale: %i%%\n",scale);
		forwardmove[0] = forwardmove[0]*scale/100;
		forwardmove[1] = forwardmove[1]*scale/100;
		sidemove[0] = sidemove[0]*scale/100;
		sidemove[1] = sidemove[1]*scale/100;
	}

	p = M_CheckParm ("-playdemo");

	if (!p)
	{
		p = M_CheckParm("-timedemo");
	}
	if (p && p < myargc-1)
	{
		sprintf (file,"%s.lmp", myargv[p+1]);
		D_AddFile (file);
		DEH_printf("Playing demo %s.lmp.\n",myargv[p+1]);
	}
	
	// get skill / episode / map from parms
	startskill = sk_medium;
	startepisode = 1;
	startmap = 1;
	autostart = false;

		
	p = M_CheckParm ("-skill");
	if (p && p < myargc-1)
	{
		startskill = myargv[p+1][0]-'1';
		if(startskill < sk_baby || startskill > sk_nightmare) // FS: Make sure it's valid.  Was fucking up my Kali gamin'!
			I_Error("Invalid Skill parameter! Valid options are 1 through 5.");		
		autostart = true;
	}

	p = M_CheckParm ("-episode");
	if (p && p < myargc-1)
	{
		startepisode = myargv[p+1][0]-'0';
		startmap = 1;
		autostart = true;
	}
	
	p = M_CheckParm ("-timer");
	if (p && p < myargc-1 && deathmatch)
	{
		int	time;
		time = atoi(myargv[p+1]);
		printf("Levels will end after %d minute",time);
		if (time>1)
			printf("s");
		printf(".\n");
	}

	p = M_CheckParm ("-avg");
	if (p && p < myargc-1 && deathmatch)
		printf("Austin Virtual Gaming: Levels will end after 20 minutes\n");

	p = M_CheckParm ("-warp");
	if (p && p < myargc-1)
	{
		if (gamemode == commercial)
			startmap = atoi (myargv[p+1]);
		else
		{
			startepisode = myargv[p+1][0]-'0';
			startmap = myargv[p+2][0]-'0';
		}
		autostart = true;
	}
	
	// init subsystems
	printf ("V_Init: allocate screens.\n");
	V_Init ();
	CheckAbortStartup(); // FS: Check if ESC key is held during startup

	printf ("M_LoadDefaults: Load system defaults.\n");
	M_LoadDefaults ();			  // load before initing other systems
	CheckAbortStartup(); // FS: Check if ESC key is held during startup

	printf ("Z_Init: Init zone memory allocation daemon. \n");
	Z_Init ();
	CheckAbortStartup(); // FS: Check if ESC key is held during startup

	printf ("W_Init: Init WADfiles.\n");
	W_InitMultipleFiles (wadfiles);
	CheckAbortStartup(); // FS: Check if ESC key is held during startup

#ifdef FEATURE_DEHACKED // FS: DEH
    printf("DEH_Init: Init Dehacked support.\n");
    DEH_Init();
#endif

	// Check for -file in shareware
	if (modifiedgame)
	{
		// These are the lumps that will be checked in IWAD,
		// if any one is not present, execution will be aborted.
		char name[23][8]=
		{
			"e2m1","e2m2","e2m3","e2m4","e2m5","e2m6","e2m7","e2m8","e2m9",
			"e3m1","e3m3","e3m3","e3m4","e3m5","e3m6","e3m7","e3m8","e3m9",
			"dphoof","bfgga0","heada1","cybra1","spida1d1"
		};
		int i;
	
		if ( gamemode == shareware)
			I_Error("\nYou cannot -file with the shareware version. Register!");

		// Check for fake IWAD with right name,
		// but w/o all the lumps of the registered version. 
		if (gamemode == registered)
			for (i = 0;i < 23; i++)
				if (W_CheckNumForName(name[i])<0)
					I_Error("\nThis is not the registered version.");
	}


	// If additonal PWAD files are used, print modified banner
	if (modifiedgame)
	{
		printf (
			"===========================================================================\n"
			"ATTENTION:  This version of DOOM has been modified.  If you would like to\n"
			"get a copy of the original game, call 1-800-IDGAMES or see the readme file.\n"
			"        You will not receive technical support for modified games.\n"
#ifdef PAUSEMODIFIED
			"                      press enter to continue\n"
#endif
			"===========================================================================\n"
		);
#ifdef PAUSEMODIFIED
		getchar ();
#endif
	}

//
// check which version
//
	if (gamemode == registered || gamemode == retail) // FS
	{
		DEH_printf ("	registered version.\n");
		DEH_snprintf(bannerbuffer, 285,
			"===========================================================================\n"
			"             This version is NOT SHAREWARE, do not distribute!\n"
			"         Please report software piracy to the SPA: 1-800-388-PIR8\n"
			"===========================================================================\n"
		);
		DEH_printf(bannerbuffer);
	}
	
	if (gamemode == shareware)
		DEH_printf ("	shareware version.\n");

	if (gamemode == commercial)
	{
		DEH_printf ("	commercial version.\n");
		DEH_snprintf(bannerbuffer, 269,
			"===========================================================================\n"
			"                            Do not distribute!\n"
			"         Please report software piracy to the SPA: 1-800-388-PIR8\n"
			"===========================================================================\n"
		);
		DEH_printf(bannerbuffer);
	}

	DEH_printf ("M_Init: Init miscellaneous info.\n");
	M_Init ();
	CheckAbortStartup(); // FS: Check if ESC key is held during startup

	DEH_printf ("R_Init: Init DOOM refresh daemon - ");
	R_Init ();
	CheckAbortStartup(); // FS: Check if ESC key is held during startup

	DEH_printf ("\nP_Init: Init Playloop state.\n");
	P_Init ();
	CheckAbortStartup(); // FS: Check if ESC key is held during startup

	DEH_printf ("I_Init: Setting up machine state.\n");
	I_Init ();
	CheckAbortStartup(); // FS: Check if ESC key is held during startup

	DEH_printf ("D_CheckNetGame: Checking network game status.\n");
	D_CheckNetGame ();
	CheckAbortStartup(); // FS: Check if ESC key is held during startup

	DEH_printf ("S_Init: Setting up sound.\n"); // FS: This is fake, it's just to make the startup look the same as vanilla

	mprintf ("HU_Init: Setting up heads up display.\n");
	HU_Init ();
	CheckAbortStartup(); // FS: Check if ESC key is held during startup

	mprintf ("ST_Init: Init status bar.\n");
	ST_Init ();
	CheckAbortStartup(); // FS: Check if ESC key is held during startup

	// check for a driver that wants intermission stats
	p = M_CheckParm ("-statcopy");
	if (p && p<myargc-1)
	{
		// for statistics driver
		extern  void*	statcopy;							

		statcopy = (void*)atoi(myargv[p+1]);
		DEH_printf ("External statistics registered.\n");
	}
	
	// start the apropriate game based on parms
	p = M_CheckParm ("-record");

	if (p && p < myargc-1)
	{
		G_RecordDemo (myargv[p+1]);
		autostart = true;
	}
	
	p = M_CheckParm ("-playdemo");
	if (p && p < myargc-1)
	{
		singledemo = true;			  // quit after one demo
		G_DeferedPlayDemo (myargv[p+1]);
		D_DoomLoop ();  // never returns
	}
	
	p = M_CheckParm ("-timedemo");
	if (p && p < myargc-1)
	{
		G_TimeDemo (myargv[p+1]);
		D_DoomLoop ();  // never returns
	}
	
	p = M_CheckParm ("-loadgame");
	if (p && p < myargc-1)
	{
		if (M_CheckParm("-cdrom"))
			sprintf(file, "c:\\doomdata\\"SAVEGAMENAME"%c.dsg",myargv[p+1][0]);
		else
			sprintf(file, SAVEGAMENAME"%c.dsg",myargv[p+1][0]);
		LoadDef.lastOn = SaveDef.lastOn = atoi(myargv[p+1]); // FS: set the slot
		G_LoadGame (file);
	}

	p = M_CheckParm ("-convertsave");
	if (p && p < myargc-1)
	{
		convertsave = true;
		if (M_CheckParm("-cdrom"))
			sprintf(file, "c:\\doomdata\\"SAVEGAMENAME"%c.dsg",myargv[p+1][0]);
		else
			sprintf(file, SAVEGAMENAME"%c.dsg",myargv[p+1][0]);
			saveconvertslot = atoi(myargv[p+1]); // FS: For save convert slot
		LoadDef.lastOn = SaveDef.lastOn = atoi(myargv[p+1]); // FS: set the slot
		G_LoadGame (file);
	}


	if (M_CheckParm("-noheadbob")) // FS
	{
			headBob = 0;
	}

	if (M_CheckParm("-nopalflash")) // FS
	{
			usePalFlash = 0;
	}
	
	if ( gameaction != ga_loadgame )
	{
	  	if (autostart || netgame)
			G_InitNew (startskill, startepisode, startmap);
		else
			D_StartTitle ();				// start up intro loop
	}


	D_DoomLoop ();  // never returns
}
