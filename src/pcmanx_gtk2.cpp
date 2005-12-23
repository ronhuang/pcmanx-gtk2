/**
 * Copyright (c) 2005 PCMan <hzysoft@sina.com.tw>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software Foundation,
 * Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#if defined(HAVE_GETTEXT)
#include <libintl.h>
#define _(T) gettext(T)
#else
#define _(T) (T)
#endif
#if defined(HAVE_LC_MESSAGES)
#include <locale.h>
#endif

#include <gtk/gtk.h>
#include <gdk/gdk.h>
#include <string.h>

#include "mainframe.h"
#include "appconfig.h"
#include "telnetcon.h"

#include "debug.h"

#ifdef USE_DOCKLET
#include "docklet/api.h"
#endif

#ifdef USE_NOTIFIER
#include "notifier/api.h"
#endif

#ifdef USE_SCRIPT
#include "script/api.h"
#endif

int main(int argc, char *argv[])
{
	bindtextdomain (GETTEXT_PACKAGE, LOCALEDIR);
	bind_textdomain_codeset(GETTEXT_PACKAGE, "UTF-8");
	textdomain(GETTEXT_PACKAGE);

	if (!g_thread_supported ())
		g_thread_init (NULL);

	int fake_argc = 1;
	char **fake_argv;

	/* GTK requires a program's argc and argv variables, and 
	 * requires that they be valid. Set it up. */
	char *_fake_argv[] = {"", NULL};
	fake_argv = _fake_argv;
	
	gtk_init (&fake_argc, &fake_argv);

#ifdef USE_DOCKLET
	/* if we are already running, silently exit */
	if (! detect_get_clipboard())
	{
#ifndef USE_DEBUG
		return 1;
#endif
	}
#endif

	AppConfig.SetToDefault();
	AppConfig.Load();
	AppConfig.LoadFavorites();
	if(AppConfig.RowsPerPage < 24)
		AppConfig.RowsPerPage=24;
	if(AppConfig.ColsPerPage<80)
		AppConfig.ColsPerPage=80;

	CTelnetCon::Init();
	CTelnetCon::SetSocketTimeout( AppConfig.SocketTimeout );

	CMainFrame* main_frm = new CMainFrame;
	gtk_window_move(GTK_WINDOW(main_frm->m_Widget), AppConfig.MainWndX, AppConfig.MainWndY);
	gtk_window_resize(GTK_WINDOW(main_frm->m_Widget), AppConfig.MainWndW, AppConfig.MainWndH);
	main_frm->Show();
#ifdef USE_DOCKLET
	if( AppConfig.ShowTrayIcon )
		main_frm->ShowTrayIcon();
#endif

#ifdef USE_NOTIFIER
	popup_notifier_init(main_frm->GetMainIcon());
	popup_notifier_set_timeout( AppConfig.PopupTimeout );
#endif

#ifdef USE_SCRIPT
	InitScriptInterface(".");
#endif

	gtk_main ();
	
	CTelnetCon::Cleanup();

	AppConfig.SaveFavorites();
	AppConfig.Save();

	return 0;
}
