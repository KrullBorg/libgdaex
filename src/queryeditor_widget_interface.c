/*
 * Copyright (C) 2011 Andrea Zagli <azagli@libero.it>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */

#ifdef HAVE_CONFIG_H
	#include <config.h>
#endif

#include "queryeditor_widget_interface.h"

typedef GdaExQueryEditorIWidgetIface GdaExQueryEditorIWidgetInterface;

G_DEFINE_INTERFACE (GdaExQueryEditorIWidget, gdaex_query_editor_iwidget, G_TYPE_OBJECT)

static void
gdaex_query_editor_iwidget_default_init (GdaExQueryEditorIWidgetInterface *iface)
{
}

const gchar
*gdaex_query_editor_iwidget_get_value (GdaExQueryEditorIWidget *iwidget)
{
	GdaExQueryEditorIWidgetIface *iface;

	gchar *ret;

	ret = NULL;

	g_return_val_if_fail (GDAEX_QUERY_EDITOR_IS_IWIDGET (iwidget), ret);

	iface = GDAEX_QUERY_EDITOR_IWIDGET_GET_IFACE (iwidget);

	if (iface->get_value)
		{
			ret = (gchar *)(* iface->get_value) (iwidget);
		}

	return ret;
}

const gchar
*gdaex_query_editor_iwidget_get_value_sql (GdaExQueryEditorIWidget *iwidget)
{
	GdaExQueryEditorIWidgetIface *iface;

	gchar *ret;

	ret = NULL;

	g_return_val_if_fail (GDAEX_QUERY_EDITOR_IS_IWIDGET (iwidget), ret);

	iface = GDAEX_QUERY_EDITOR_IWIDGET_GET_IFACE (iwidget);

	if (iface->get_value_sql)
		{
			ret = (gchar *)(* iface->get_value_sql) (iwidget);
		}

	return ret;
}

void
gdaex_query_editor_iwidget_set_value (GdaExQueryEditorIWidget *iwidget,
                                      const gchar *value)
{
	GdaExQueryEditorIWidgetIface *iface;

	g_return_if_fail (GDAEX_QUERY_EDITOR_IS_IWIDGET (iwidget));

	iface = GDAEX_QUERY_EDITOR_IWIDGET_GET_IFACE (iwidget);

	if (iface->set_value)
		{
			(* iface->set_value) (iwidget, value);
		}
}
