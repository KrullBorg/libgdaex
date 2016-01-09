/*
 * Copyright (C) 2011-2016 Andrea Zagli <azagli@libero.it>
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

#ifndef __GDAEX_QUERY_EDITOR_IWIDGET_H__
#define __GDAEX_QUERY_EDITOR_IWIDGET_H__

#include <glib-object.h>

#include <libxml/tree.h>


G_BEGIN_DECLS


#define GDAEX_QUERY_EDITOR_TYPE_IWIDGET             (gdaex_query_editor_iwidget_get_type ())
#define GDAEX_QUERY_EDITOR_IWIDGET(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), GDAEX_QUERY_EDITOR_TYPE_IWIDGET, GdaExQueryEditorIWidget))
#define GDAEX_QUERY_EDITOR_IS_IWIDGET(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GDAEX_QUERY_EDITOR_TYPE_IWIDGET))
#define GDAEX_QUERY_EDITOR_IWIDGET_GET_IFACE(obj)   (G_TYPE_INSTANCE_GET_INTERFACE ((obj), GDAEX_QUERY_EDITOR_TYPE_IWIDGET, GdaExQueryEditorIWidgetIface))


typedef struct _GdaExQueryEditorIWidget GdaExQueryEditorIWidget;
typedef struct _GdaExQueryEditorIWidgetIface GdaExQueryEditorIWidgetIface;

struct _GdaExQueryEditorIWidgetIface
	{
		GTypeInterface g_iface;

		const gchar *(*get_value) (GdaExQueryEditorIWidget *iwidget);
		const gchar *(*get_value_sql) (GdaExQueryEditorIWidget *iwidget);
		void (*set_value) (GdaExQueryEditorIWidget *iwidget, const gchar *value);

		void (*xml_parsing) (GdaExQueryEditorIWidget *iwidget, xmlNode *xnode);
	};

GType gdaex_query_editor_iwidget_get_type (void) G_GNUC_CONST;

const gchar *gdaex_query_editor_iwidget_get_value (GdaExQueryEditorIWidget *iwidget);
const gchar *gdaex_query_editor_iwidget_get_value_sql (GdaExQueryEditorIWidget *iwidget);
void gdaex_query_editor_iwidget_set_value (GdaExQueryEditorIWidget *iwidget, const gchar *value);

void gdaex_query_editor_xml_parsing (GdaExQueryEditorIWidget *iwidget, xmlNode *xnode);


G_END_DECLS


#endif /* __GDAEX_QUERY_EDITOR_IWIDGET_H__ */
