#ifndef __CFG_EXT_H
#define __CFG_EXT_H

/* FS: Extended.cfg stuff */
extern boolean headBob;
extern int	drawTime; /* FS: Draw Time on Automap */
extern boolean	useIntGus; /* FS: Use internal GUS*.WADs */
extern boolean novert; /* FS: No vertical mouse movement */
extern boolean noprecache;
extern boolean noquitsound;
extern boolean nowipe;
extern boolean usePalFlash;
extern boolean alwaysRun; // 2024/11/10 make always run optional
extern boolean secretCount; // 2024/11/10 make secret counter optional
extern boolean bugFix; // 2024/11/10 vanilla bugfixes now optional
extern boolean newCheats; // 2024/11/10 new cheats now optional
extern boolean varPitch; // 2024/11/12 variable sound pitch

/* FS: Custom weapon keys */
extern boolean	use_wpnbinds;
extern int	wpn_shotgun;
extern int	wpn_chaingun;
extern int	wpn_rocket;
extern int	wpn_plasma;

#endif
