/*
 * mpdcontroller.cpp
 * Copyright (C) 2008 Grigory Holomiev
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this library; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */

#include "libmpdclient.h"

#include "mpdcontroller.h"


/**
 * \class mpdController
 * \brief A controller for mpd.
 */


/**
 * \brief Constructs the controller.
 */
MPDController::MPDController() : PollingTuneController()
{
setInterval(1000);
}


Tune MPDController::currentTune()
{
	Tune tune;
	mpd_Connection * conn;
	char *hostname = getenv("MPD_HOST");
	char *port = getenv("MPD_PORT");
	if(hostname == NULL)
		hostname = "127.0.0.1";
	if(port == NULL)
		port = "6600";

	conn = mpd_newConnection(hostname,atoi(port),10);

	if(conn->error) {
		mpd_closeConnection(conn);
		return tune;
	}

	mpd_Status * status;
	mpd_InfoEntity * entity;

	mpd_sendCommandListOkBegin(conn);
	mpd_sendStatusCommand(conn);
	mpd_sendCurrentSongCommand(conn);
	mpd_sendCommandListEnd(conn);

	if((status = mpd_getStatus(conn))==NULL) {
		mpd_closeConnection(conn);
		return tune;
	}

	if(status->state == MPD_STATUS_STATE_STOP || status->state == MPD_STATUS_STATE_UNKNOWN) {
    		mpd_freeStatus(status);
    		mpd_closeConnection(conn);
		return tune;
	}

	if(status->state == MPD_STATUS_STATE_PLAY || status->state == MPD_STATUS_STATE_PAUSE) {
	    tune.setTime(status->elapsedTime / 1000);
	}

	mpd_nextListOkCommand(conn);

	while((entity = mpd_getNextInfoEntity(conn))) {
		mpd_Song * song = entity->info.song;
		if(entity->type!=MPD_INFO_ENTITY_TYPE_SONG) {
			mpd_freeInfoEntity(entity);
			continue;
		}

		QString info;
		if(song->artist) {
		    info = QString::fromUtf8(song->artist) + " - ";
		}
		if(song->title) {
		    info += QString::fromUtf8(song->title);
		}
		if(!info.length()) {
		    info = QString::fromUtf8(song->file);
		}
		tune.setName(info);
		mpd_freeInfoEntity(entity);
	}
	if(conn->error) {
		mpd_closeConnection(conn);
		return tune;
	}

	mpd_finishCommand(conn);
	if(conn->error) {
		mpd_closeConnection(conn);
		return tune;
	}
	
	mpd_freeStatus(status);
	mpd_closeConnection(conn);

	return tune;
}
