/*
 * Copyright (C) 2011-2016 Andrea Zagli <azagli@libero.it>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

#ifdef HAVE_CONFIG_H
	#include "config.h"
#endif

#include "libgdaex.h"

#include "queryeditorentry.h"

static void gdaex_query_editor_entry_class_init (GdaExQueryEditorEntryClass *klass);
static void gdaex_query_editor_entry_init (GdaExQueryEditorEntry *masked_entry);

static void gdaex_query_editor_entry_gdaex_query_editor_iwidget_interface_init (GdaExQueryEditorIWidgetIface *iface);

static void gdaex_query_editor_entry_set_property (GObject *object,
                                           guint property_id,
                                           const GValue *value,
                                           GParamSpec *pspec);
static void gdaex_query_editor_entry_get_property (GObject *object,
                                           guint property_id,
                                           GValue *value,
                                           GParamSpec *pspec);

static const gchar *gdaex_query_editor_entry_get_value (GdaExQueryEditorIWidget *iwidget);
static const gchar *gdaex_query_editor_entry_get_value_sql (GdaExQueryEditorIWidget *iwidget);
static void gdaex_query_editor_entry_set_value (GdaExQueryEditorIWidget *iwidget, const gchar *value);

#define GDAEX_QUERY_EDITOR_ENTRY_GET_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE ((obj), GTK_TYPE_ENTRY, GdaExQueryEditorEntryPrivate))

typedef struct _GdaExQueryEditorEntryPrivate GdaExQueryEditorEntryPrivate;
struct _GdaExQueryEditorEntryPrivate
	{
		gpointer foo;
	};

G_DEFINE_TYPE_WITH_CODE (GdaExQueryEditorEntry, gdaex_query_editor_entry, GTK_TYPE_ENTRY,
                         G_IMPLEMENT_INTERFACE (GDAEX_QUERY_EDITOR_TYPE_IWIDGET,
                                                gdaex_query_editor_entry_gdaex_query_editor_iwidget_interface_init));

static void
gdaex_query_editor_entry_class_init (GdaExQueryEditorEntryClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);

	g_type_class_add_private (object_class, sizeof (GdaExQueryEditorEntryPrivate));

	object_class->set_property = gdaex_query_editor_entry_set_property;
	object_class->get_property = gdaex_query_editor_entry_get_property;
}

static void
gdaex_query_editor_entry_init (GdaExQueryEditorEntry *entry)
{
	GdaExQueryEditorEntryPrivate *priv = GDAEX_QUERY_EDITOR_ENTRY_GET_PRIVATE (entry);
}

static void
gdaex_query_editor_entry_gdaex_query_editor_iwidget_interface_init (GdaExQueryEditorIWidgetIface *iface)
{
	iface->get_value = gdaex_query_editor_entry_get_value;
	iface->get_value_sql = gdaex_query_editor_entry_get_value_sql;
	iface->set_value = gdaex_query_editor_entry_set_value;
}

/**
 * gdaex_query_editor_entry_new:
 *
 * Creates a new #GdaExQueryEditorEntry widget.
 *
 * Returns: the newly created #GdaExQueryEditorEntry widget.
 */
GtkWidget*
gdaex_query_editor_entry_new ()
{
	return GTK_WIDGET (g_object_new (gdaex_query_editor_entry_get_type (), NULL));
}

static void
gdaex_query_editor_entry_set_property (GObject *object, guint property_id, const GValue *value, GParamSpec *pspec)
{
	GdaExQueryEditorEntry *entry = GDAEX_QUERY_EDITOR_ENTRY (object);
	GdaExQueryEditorEntryPrivate *priv = GDAEX_QUERY_EDITOR_ENTRY_GET_PRIVATE (entry);

	switch (property_id)
		{
			default:
				G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
				break;
		}
}

static void
gdaex_query_editor_entry_get_property (GObject *object, guint property_id, GValue *value, GParamSpec *pspec)
{
	GdaExQueryEditorEntry *entry = GDAEX_QUERY_EDITOR_ENTRY (object);
	GdaExQueryEditorEntryPrivate *priv = GDAEX_QUERY_EDITOR_ENTRY_GET_PRIVATE (entry);

	switch (property_id)
		{
			default:
				G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
				break;
		}
}

static const gchar
*gdaex_query_editor_entry_get_value (GdaExQueryEditorIWidget *iwidget)
{
	return gtk_entry_get_text (GTK_ENTRY (iwidget));
}

static const gchar
*gdaex_query_editor_entry_get_value_sql (GdaExQueryEditorIWidget *iwidget)
{
	return gtk_entry_get_text (GTK_ENTRY (iwidget));
}

static void
gdaex_query_editor_entry_set_value (GdaExQueryEditorIWidget *iwidget,
                            const gchar *value)
{
	gtk_entry_set_text (GTK_ENTRY (iwidget), value);
}
