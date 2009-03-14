/*
===========================================================================
Copyright (C) 1999-2005 Id Software, Inc.
Copyright (C) 2000-2003 Iron Claw Interactive
Copyright (C) 2005-2009 Smokin' Guns

This file is part of Smokin' Guns.

Smokin' Guns is free software; you can redistribute it
and/or modify it under the terms of the GNU General Public License as
published by the Free Software Foundation; either version 2 of the License,
or (at your option) any later version.

Smokin' Guns is distributed in the hope that it will be
useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Smokin' Guns; if not, write to the Free Software
Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
===========================================================================
*/
// cg_info.c -- display information while data is being loading

#include "cg_local.h"

#define MAX_LOADING_PLAYER_ICONS	16
#define MAX_LOADING_ITEM_ICONS		26

static int			loadingPlayerIconCount;
static int			loadingItemIconCount;
static qhandle_t	loadingPlayerIcons[MAX_LOADING_PLAYER_ICONS];
static qhandle_t	loadingItemIcons[MAX_LOADING_ITEM_ICONS];

#define LOADING_ALL 50

//Spoon new loading-draw
static int			loadingstage;
//there is no division thru zero :)
static float			loading_all=LOADING_ALL;

/*
===================
CG_LoadingStage
by Spoon
===================
*/
void CG_LoadingStage( int amount){
	if(amount == 0){
		int i;
		char items[MAX_ITEMS +1];

		loadingstage = 0;

		loading_all = LOADING_ALL;
		// set loading_all
		// +items
		strcpy( items, CG_ConfigString( CS_ITEMS) );
		for ( i = 1 ; i < bg_numItems ; i++ ) {
			if ( items[ i ] == '1' || cg_buildScript.integer )
				loading_all += 1.5;
		}

		// +clients
		for (i=0 ; i<MAX_CLIENTS ; i++) {
			const char		*clientInfo;

			if (cg.clientNum == i) {
				continue;
			}

			clientInfo = CG_ConfigString( CS_PLAYERS+i );
			if ( !clientInfo[0]) {
				continue;
			}
			loading_all += 1.5;
		}
	}else
		loadingstage += amount;

	trap_UpdateScreen();
}
/*
===================
CG_DrawLoadingStage
by Spoon
my Artwork :)
===================
*/
#define LOADING_HEIGHT	10
#define	LINE			3
#define	DISTANCE		10
void CG_DrawLoadingStage(void){
	float	loaded;
	vec4_t	color = {0.6f, 0.4f, 0.0f, 0.6f};//{0.67f, 0.06f, 0.06f, 0.6f};
	float	progress;

	loaded = loadingstage + loadingPlayerIconCount*1.5 + loadingItemIconCount*1.5;

	progress = ((640-2*DISTANCE-4*LINE)/loading_all)*loaded;

	if(progress > (640-2*DISTANCE-4*LINE))
		progress = 640-2*DISTANCE-4*LINE;

	CG_FillRect(DISTANCE+2*LINE, 480-DISTANCE-2*LINE-LOADING_HEIGHT, progress , LOADING_HEIGHT ,color);

	//lines

	//bottom
	CG_FillRect(DISTANCE, 480-DISTANCE-LINE, 640-2*DISTANCE , LINE ,color);
	//top
	CG_FillRect(DISTANCE, 480-DISTANCE-4*LINE-LOADING_HEIGHT, 640-2*DISTANCE , LINE ,color);
	//left
	CG_FillRect(DISTANCE, 480-DISTANCE-3*LINE-LOADING_HEIGHT, LINE , LOADING_HEIGHT+2*LINE ,color);
	//right
	CG_FillRect(640-DISTANCE-LINE, 480-DISTANCE-3*LINE-LOADING_HEIGHT, LINE , LOADING_HEIGHT+2*LINE ,color);
}
/*
===================
CG_DrawLoadingIcons
===================
*/
static void CG_DrawLoadingIcons( void ) {
	int		n;
	int		x, y;

	for( n = 0; n < loadingPlayerIconCount; n++ ) {
		x = 16 + n * 78;
		y = 324-40;
		CG_DrawPic( x, y, 64, 64, loadingPlayerIcons[n] );
	}

	for( n = 0; n < loadingItemIconCount; n++ ) {
		y = 400-40;
		if( n >= 13 ) {
			y += 40;
		}
		x = 16 + n % 13 * 48;
		CG_DrawPic( x, y, 32, 32, loadingItemIcons[n] );
	}
}


/*
======================
CG_LoadingString

======================
*/
void CG_LoadingString( const char *s ) {
	Q_strncpyz( cg.infoScreenText, s, sizeof( cg.infoScreenText ) );

	trap_UpdateScreen();
}

/*
===================
CG_LoadingItem
===================
*/
void CG_LoadingItem( int itemNum ) {
	gitem_t		*item;

	item = &bg_itemlist[itemNum];

	if ( item->icon && loadingItemIconCount < MAX_LOADING_ITEM_ICONS ) {
		loadingItemIcons[loadingItemIconCount++] = trap_R_RegisterShaderNoMip( item->icon );
	}

	CG_LoadingString( item->pickup_name );
}

/*
===================
CG_LoadingClient
===================
*/
void CG_LoadingClient( int clientNum ) {
	const char		*info;
	char			*skin;
	char			personality[MAX_QPATH];
	char			model[MAX_QPATH];
	char			iconName[MAX_QPATH];

	info = CG_ConfigString( CS_PLAYERS + clientNum );

	if ( loadingPlayerIconCount < MAX_LOADING_PLAYER_ICONS ) {
		Q_strncpyz( model, Info_ValueForKey( info, "model" ), sizeof( model ) );

		skin = Q_strrchr( model, '/' );
		if ( skin ) {
			*skin++ = '\0';
		} else {
			skin = "red";
		}

		Com_sprintf( iconName, MAX_QPATH, "models/wq3_players/%s/icon_%s.tga", model, skin );

		loadingPlayerIcons[loadingPlayerIconCount] = trap_R_RegisterShaderNoMip( iconName );
		if ( !loadingPlayerIcons[loadingPlayerIconCount] ) {
			Com_sprintf( iconName, MAX_QPATH, "models/wq3_players/%s/icon_%s.tga", DEFAULT_MODEL, "red" );
			loadingPlayerIcons[loadingPlayerIconCount] = trap_R_RegisterShaderNoMip( iconName );
		}
		if ( loadingPlayerIcons[loadingPlayerIconCount] ) {
			loadingPlayerIconCount++;
		}
	}

	Q_strncpyz( personality, Info_ValueForKey( info, "n" ), sizeof(personality) );
	Q_CleanStr( personality );

	if( cgs.gametype == GT_SINGLE_PLAYER ) {
		trap_S_RegisterSound( va( "sound/player/announce/%s.wav", personality ), qtrue );
	}

	CG_LoadingString( personality );
}


/*
====================
CG_DrawInformation

Draw all the status / pacifier stuff during level loading
====================
*/
void CG_DrawInformation( void ) {
	const char	*s;
	const char	*info;
	const char	*sysInfo;
	int			y;
	int			value;
	qhandle_t	levelshot;
	qhandle_t	background, photo;
	//qhandle_t	detail;
	char		buf[1024];
	float		*color;

	info = CG_ConfigString( CS_SERVERINFO );
	sysInfo = CG_ConfigString( CS_SYSTEMINFO );

	s = Info_ValueForKey( info, "mapname" );

	// draw the background
	background = trap_R_RegisterShaderNoMip( "ui/bg/main.tga");

	trap_R_SetColor( NULL );
	CG_DrawPic( 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, background );

	levelshot = trap_R_RegisterShaderNoMip( va( "levelshots/%s.tga", s ) );
	if ( levelshot ) {
		//levelshot = trap_R_RegisterShaderNoMip( "ui/bg/main.tga" );
		photo = trap_R_RegisterShaderNoMip( "ui/bg/photo.tga");

		CG_DrawPic( 80, 75, 480, 360, photo );
		CG_DrawPic( 100, 90, 440, 330, levelshot );

		color = colorBlack;
	} else
		color = colorWhite;

	// blend a detail texture over it
	//detail = trap_R_RegisterShader( "levelShotDetail" );
	//trap_R_DrawStretchPic( 0, 0, cgs.glconfig.vidWidth, cgs.glconfig.vidHeight, 0, 0, 2.5, 2, detail );

	// draw the icons of things as they are loaded
	//CG_DrawLoadingIcons();
	CG_DrawLoadingStage();

	// the first 150 rows are reserved for the client connection
	// screen to write into
	if ( cg.infoScreenText[0] ) {
		CG_Text_PaintCenter( 320, 464, 0.25f, colorWhite,
			va("Loading... %s", cg.infoScreenText), 0, 0, 0);
	} else {
		CG_Text_PaintCenter( 320, 464, 0.25f, colorWhite,
			"Awaiting snapshot...", 0, 0, 0);
	}

	// draw info string information

	y = 30;

	// don't print server lines if playing a local game
	trap_Cvar_VariableStringBuffer( "sv_running", buf, sizeof( buf ) );
	if ( !atoi( buf ) ) {
		// server hostname
		Q_strncpyz(buf, Info_ValueForKey( info, "sv_hostname" ), 1024);
		Q_CleanStr(buf);
		CG_Text_PaintCenter(320, y, 0.3f, colorWhite,
			buf, 0, 0, 3);
		y += 15;

		// server-specific message of the day
		s = CG_ConfigString( CS_MOTD );
		if ( s[0] ) {
			CG_Text_PaintCenter(320, y, 0.25f, colorWhite,
				s, 0, 0, 3);
			y += 12;
		}

		// pure server
		s = Info_ValueForKey( sysInfo, "sv_pure" );
		if ( s[0] == '1' ) {
			CG_Text_PaintCenter(320, y, 0.25f, colorWhite,
				"Pure Server", 0, 0, 3);
			y += 12;
		}

		// some extra space after hostname and motd
		y += 10;
	}

	if(levelshot)
		y = 120;
	else
		y = 200;

	// map-specific message (long map name)
	s = CG_ConfigString( CS_MESSAGE );
	if ( s[0] ) {
		CG_Text_PaintCenter(320, y, 0.3f, color,
				s, 0, 0, 3);
		y += 15;
	}

	// cheats warning
	s = Info_ValueForKey( sysInfo, "sv_cheats" );
	if ( s[0] == '1' ) {
		CG_Text_PaintCenter(320, y, 0.3f, color,
				"Cheats are Enabled", 0, 0, 3);
		y += 15;
		cg_cheats = qtrue;
	} else
		cg_cheats = qfalse;

	// game type
	switch ( cgs.gametype ) {
	case GT_FFA:
		s = "Deathmatch";
		break;
	case GT_SINGLE_PLAYER:
		s = "Single Player";
		break;
	case GT_DUEL:
		s = "Duel";
		break;
	case GT_TEAM:
		s = "Team Deathmatch";
		break;
	case GT_RTP:
		s = "Round Teamplay";
		break;
	case GT_BR:
		s = "Bank Robbery";
		break;
	case GT_CTF:
		s = "Capture The Flag";
		break;
#ifdef MISSIONPACK
	case GT_1FCTF:
		s = "One Flag CTF";
		break;
	case GT_OBELISK:
		s = "Overload";
		break;
	case GT_HARVESTER:
		s = "Harvester";
		break;
#endif
	default:
		s = "Unknown Gametype";
		break;
	}
	CG_Text_PaintCenter(320, y, 0.3f, color,
		s, 0, 0, 3);
	y += 15;

	value = atoi( Info_ValueForKey( info, "timelimit" ) );
	if ( value ) {
		CG_Text_PaintCenter(320, y, 0.3f, color,
				va( "Timelimit %i", value ), 0, 0, 3);
		y += 15;
	}

	if (cgs.gametype < GT_RTP && cgs.gametype != GT_DUEL) {
		value = atoi( Info_ValueForKey( info, "fraglimit" ) );
		if ( value ) {
			CG_Text_PaintCenter(320, y, 0.3f, color,
				va( "Fraglimit %i", value ), 0, 0, 3);
			y += 15;
		}
	}

	if (cgs.gametype == GT_DUEL) {
		value = atoi( Info_ValueForKey( info, "duellimit" ) );
		if ( value ) {
			CG_Text_PaintCenter(320, y, 0.3f, color,
				va( "Duellimit %i", value ), 0, 0, 3);
			y += 15;
		}
	}

	if (cgs.gametype >= GT_RTP) {
		value = atoi( Info_ValueForKey( info, "scorelimit" ) );
		if ( value ) {
			CG_Text_PaintCenter(320, y, 0.3f, color,
				va( "Scorelimit %i", value ), 0, 0, 3);
			y += 15;
		}
	}
}

