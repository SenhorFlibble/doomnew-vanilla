<h3>DoomNew Vanilla</h3>

This is a vanillified fork of <a href="https://doomwiki.org/wiki/DoomNew">DoomNew</a>, a DOS <i>Doom</i> source port with built-in DeHackEd support and other extra features.

What's changed in this version:
<ul><li>restored original <i>Doom</i> planar mode (imported from <a href="https://github.com/AXDOOMER/doom-vanille">Doom Vanille</a> and <a href="https://doomwiki.org/wiki/Gamesrc-ver-recreation">gamesrc-ver-recreation</a>)</li>
<li>restored low detail mode (F5)</li>
<li>reset static vanilla limits</li>
<li>restored vanilla player movement speed</li>
<li>restored vanilla mouse sensitivity scale</li></ul>

This version is compiled with the APODMX wrapper and the Apogee Sound System instead of the proprietary DMX sound library, making it fully GPL-compliant.

Note that even though this version can still run <a href="http://drnostromo.com/hacx/">HacX 1.2</a>, the maps may have glitches because they're not all vanilla-compatible.

<h3>Compiling</h3>

I compiled this using the freeware Watcom C 11.0c available <a href="https://www.openwatcom.org/ftp/archive/11.0c/">here</a>. (Full disclosure: I first installed Open Watcom C 1.9 in DOSBox, then installed Watcom C 11.0c elsewhere and copied those files over the Open Watcom ones, overwriting them.) Compiling with Open Watcom C might work, but will likely throw up a bunch of warnings. You will also need Borland Turbo Assembler 3.1 to compile the Assembler parts of the code.

<h3>Original project description</h3>

DoomNew is a enhanced DOS source port created by Frank Sapone, otherwise known as Maraakate. It is derived from Linux Doom. It aims to enhance the stock executable with system drivers and bug fixes derived from Heretic and new features, including the use of the DMX sound engine, internal and external DeHackEd support and the raising of static limits, such as visplanes.

<h3>History</h3>

Frank Sapone was previously active in the Daikatana community and impressed John Romero with the community patch for it that improved the stability of the game. Wanting to make an improvement to Doom, he acquired the DMX sound engine from Romero himself. The first activity was on December 19, 2012, and DMX was linked on June 1, 2013. The port is known as Doom 1.9a. Sapone kept a detailed record of his progress in a file called CHANGES.TXT.

September 2013 was easily the most productive month for the port. On September 9, music was added in, with graphics and sound following on the 10th. From the 12th to the 20th, the port was brought up to speed, gaining support for all versions of Doom, along with TNT: Evilution, The Plutonia Experiment, Perdition's Gate, Hell to Pay, Chex Quest and HacX. A lot of effort was spent taking code from Sapone's other port: an enhanced version of Heretic. On September 24, preliminary DeHackEd support, adapted from Chocolate Doom, was added in. On October 10, experimental support for internal DeHackEd patches was added.

2013 and 2014 were thus spent on fixing bugs, partially using code from his enhanced Heretic fork. After November 11, 2014, development stopped until August 31, 2015 when Sapone released a massive code clean up. Experimental support for the SimulEyes Virtual reality glasses was provided utilizing a secondary executable called DOOMVR.EXE. The last update and release was on October 21, 2015.

<h3>Features</h3>
<ul><li>Utilizes the DMX sound engine</li>
<li>Support for Doom, The Ultimate Doom, Doom 2, TNT: Evilution, The Plutonia Experiment, Perdition's Gate, Hell to Pay, Chex Quest and HacX through their own parameters
<br>-doom, -doomu, -doom2, -tnt, -plutonia, -perdgate, -hell2pay/-helltopay, -hacx</li>
<li>A additional configuration file, called EXTEND.CFG. With this you can set parameters:
<br>-novert to disable vertical movement
<br>-noprecache to disable graphics precaching
<br>-nowipe to disable the screen melt effect</li>
<li>Doom.wad will search for Episode 4 and auto-load The Ultimate Doom. This can be overridden using the -doomu parameter</li>
<li>Specify the main WAD file utilizing the -mainwad parameter</li>
<li>Ability to load older save files using the -oldsave parameter</li>
<li>Ability to convert older save files utilizing the -convertsave <slot #> parameter</li>
<li>Various new cheats from Heretic 1.3a:
<br>MASSACRE
<br>HHSCOTT: This instantly finishes the level</li>
<li>Speed enhancements from Heretic, such as linear.asm</li>
<li>Raised limits:
<br>SAVEGAMESIZE to 1024 KB
<br>SAVESTRINGSIZE from 24 to 256
<br>Visplane limits are removed, utilizing code from wHeretic</li>
<li>Support for internal and external DeHackEd patches</li>
<li>Ability to toggel loading internal DeHackEd files through the -deh_internal parameter</li>
<li>Virtual reality support through DOOMVR.EXE</li></ul>
