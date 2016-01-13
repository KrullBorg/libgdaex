/*
 * Copyright (C) 2016 Andrea Zagli <azagli@libero.it>
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

#include <libzakutils/libzakutils.h>

#include "libgdaex.h"

#include "queryeditorentrydate.h"

static void gdaex_query_editor_entry_date_class_init (GdaExQueryEditorEntryDateClass *klass);
static void gdaex_query_editor_entry_date_init (GdaExQueryEditorEntryDate *masked_entry);

static void gdaex_query_editor_entry_date_gdaex_query_editor_iwidget_interface_init (GdaExQueryEditorIWidgetIface *iface);

static void gdaex_query_editor_entry_date_set_property (GObject *object,
                                           guint property_id,
                                           const GValue *value,
                                           GParamSpec *pspec);
static void gdaex_query_editor_entry_date_get_property (GObject *object,
                                           guint property_id,
                                           GValue *value,
                                           GParamSpec *pspec);

static const gchar *gdaex_query_editor_entry_date_get_value (GdaExQueryEditorIWidget *iwidget);
static const gchar *gdaex_query_editor_entry_date_get_value_visible (GdaExQueryEditorIWidget *iwidget);
static const gchar *gdaex_query_editor_entry_date_get_value_sql (GdaExQueryEditorIWidget *iwidget);
static void gdaex_query_editor_entry_date_set_value (GdaExQueryEditorIWidget *iwidget, const gchar *value);

#define GDAEX_QUERY_EDITOR_ENTRY_DATE_GET_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE ((obj), GDAEX_QUERY_EDITOR_TYPE_ENTRY_DATE, GdaExQueryEditorEntryDatePrivate))

typedef struct _GdaExQueryEditorEntryDatePrivate GdaExQueryEditorEntryDatePrivate;
struct _GdaExQueryEditorEntryDatePrivate
	{
		gchar *format;
	};

G_DEFINE_TYPE_WITH_CODE (GdaExQueryEditorEntryDate, gdaex_query_editor_entry_date, GTK_TYPE_ENTRY,
                         G_IMPLEMENT_INTERFACE (GDAEX_QUERY_EDITOR_TYPE_IWIDGET,
                                                gdaex_query_editor_entry_date_gdaex_query_editor_iwidget_interface_init));

static void
gdaex_query_editor_entry_date_class_init (GdaExQueryEditorEntryDateClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);

	g_type_class_add_private (object_class, sizeof (GdaExQueryEditorEntryDatePrivate));

	object_class->set_property = gdaex_query_editor_entry_date_set_property;
	object_class->get_property = gdaex_query_editor_entry_date_get_property;
}

static void
gdaex_query_editor_entry_date_init (GdaExQueryEditorEntryDate *entry)
{
	GdaExQueryEditorEntryDatePrivate *priv = GDAEX_QUERY_EDITOR_ENTRY_DATE_GET_PRIVATE (entry);

	priv->format = NULL;
}

static void
gdaex_query_editor_entry_date_gdaex_query_editor_iwidget_interface_init (GdaExQueryEditorIWidgetIface *iface)
{
	iface->get_value = gdaex_query_editor_entry_date_get_value;
	iface->get_value_visible = gdaex_query_editor_entry_date_get_value_visible;
	iface->get_value_sql = gdaex_query_editor_entry_date_get_value_sql;
	iface->set_value = gdaex_query_editor_entry_date_set_value;
}

/**
 * gdaex_query_editor_entry_date_new:
 *
 * Creates a new #GdaExQueryEditorEntryDate widget.
 *
 * Returns: the newly created #GdaExQueryEditorEntryDate widget.
 */
GtkWidget*
gdaex_query_editor_entry_date_new ()
{
	return GTK_WIDGET (g_object_new (gdaex_query_editor_entry_date_get_type (), NULL));
}

void
gdaex_query_editor_entry_date_set_format (GdaExQueryEditorEntryDate *entry, const gchar *format)
{
	GdaExQueryEditorEntryDatePrivate *priv = GDAEX_QUERY_EDITOR_ENTRY_DATE_GET_PRIVATE (entry);

	if (priv->format != NULL)
		{
			g_free (priv->format);
		}
	priv->format = g_strdup (format);
}
gchar
*gdaex_query_editor_entry_date_get_format (GdaExQueryEditorEntryDate* entry)
{
	GdaExQueryEditorEntryDatePrivate *priv = GDAEX_QUERY_EDITOR_ENTRY_DATE_GET_PRIVATE (entry);

	return g_strdup (priv->format);
}

static void
gdaex_query_editor_entry_date_set_property (GObject *object, guint property_id, const GValue *value, GParamSpec *pspec)
{
	GdaExQueryEditorEntryDate *entry = GDAEX_QUERY_EDITOR_ENTRY_DATE (object);
	GdaExQueryEditorEntryDatePrivate *priv = GDAEX_QUERY_EDITOR_ENTRY_DATE_GET_PRIVATE (entry);

	switch (property_id)
		{
			default:
				G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
				break;
		}
}

static void
gdaex_query_editor_entry_date_get_property (GObject *object, guint property_id, GValue *value, GParamSpec *pspec)
{
	GdaExQueryEditorEntryDate *entry = GDAEX_QUERY_EDITOR_ENTRY_DATE (object);
	GdaExQueryEditorEntryDatePrivate *priv = GDAEX_QUERY_EDITOR_ENTRY_DATE_GET_PRIVATE (entry);

	switch (property_id)
		{
			default:
				G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
				break;
		}
}

static const gchar
*gdaex_query_editor_entry_date_get_value (GdaExQueryEditorIWidget *iwidget)
{
	return gtk_entry_get_text (GTK_ENTRY (iwidget));
}

static const gchar
*gdaex_query_editor_entry_date_get_value_visible (GdaExQueryEditorIWidget *iwidget)
{
	return gtk_entry_get_text (GTK_ENTRY (iwidget));
}

static const gchar
*gdaex_query_editor_entry_date_get_value_sql (GdaExQueryEditorIWidget *iwidget)
{
	gchar *ret;
	GDateTime *gdt;

	GdaExQueryEditorEntryDatePrivate *priv = GDAEX_QUERY_EDITOR_ENTRY_DATE_GET_PRIVATE (GDAEX_QUERY_EDITOR_ENTRY_DATE (iwidget));

	gdt = zak_utils_get_gdatetime_from_string (gtk_entry_get_text (GTK_ENTRY (iwidget)), priv->format);
	ret = g_strdup (zak_utils_gdatetime_to_sql (gdt, NULL));

	return ret;
}

static void
gdaex_query_editor_entry_date_set_value (GdaExQueryEditorIWidget *iwidget,
                            const gchar *value)
{
	gtk_entry_set_text (GTK_ENTRY (iwidget), value);
}
