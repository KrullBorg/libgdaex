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

#include <glib/gi18n-lib.h>

#include <libzakutils/libzakutils.h>

#include "libgdaex.h"

#include "queryeditorcheck.h"

static void gdaex_query_editor_check_class_init (GdaExQueryEditorCheckClass *klass);
static void gdaex_query_editor_check_init (GdaExQueryEditorCheck *masked_check);

static void gdaex_query_editor_check_gdaex_query_editor_iwidget_interface_init (GdaExQueryEditorIWidgetIface *iface);

static void gdaex_query_editor_check_set_property (GObject *object,
                                           guint property_id,
                                           const GValue *value,
                                           GParamSpec *pspec);
static void gdaex_query_editor_check_get_property (GObject *object,
                                           guint property_id,
                                           GValue *value,
                                           GParamSpec *pspec);

static const gchar *gdaex_query_editor_check_get_value (GdaExQueryEditorIWidget *iwidget);
static const gchar *gdaex_query_editor_check_get_value_visible (GdaExQueryEditorIWidget *iwidget);
static const gchar *gdaex_query_editor_check_get_value_sql (GdaExQueryEditorIWidget *iwidget);
static void gdaex_query_editor_check_set_value (GdaExQueryEditorIWidget *iwidget, const gchar *value);

#define GDAEX_QUERY_EDITOR_CHECK_GET_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE ((obj), GTK_TYPE_CHECK_BUTTON, GdaExQueryEditorCheckPrivate))

typedef struct _GdaExQueryEditorCheckPrivate GdaExQueryEditorCheckPrivate;
struct _GdaExQueryEditorCheckPrivate
	{
		gpointer foo;
	};

G_DEFINE_TYPE_WITH_CODE (GdaExQueryEditorCheck, gdaex_query_editor_check, GTK_TYPE_CHECK_BUTTON,
                         G_IMPLEMENT_INTERFACE (GDAEX_QUERY_EDITOR_TYPE_IWIDGET,
                                                gdaex_query_editor_check_gdaex_query_editor_iwidget_interface_init));

static void
gdaex_query_editor_check_class_init (GdaExQueryEditorCheckClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);

	g_type_class_add_private (object_class, sizeof (GdaExQueryEditorCheckPrivate));

	object_class->set_property = gdaex_query_editor_check_set_property;
	object_class->get_property = gdaex_query_editor_check_get_property;
}

static void
gdaex_query_editor_check_init (GdaExQueryEditorCheck *check)
{
	GdaExQueryEditorCheckPrivate *priv = GDAEX_QUERY_EDITOR_CHECK_GET_PRIVATE (check);
}

static void
gdaex_query_editor_check_gdaex_query_editor_iwidget_interface_init (GdaExQueryEditorIWidgetIface *iface)
{
	iface->get_value = gdaex_query_editor_check_get_value;
	iface->get_value_visible = gdaex_query_editor_check_get_value_visible;
	iface->get_value_sql = gdaex_query_editor_check_get_value_sql;
	iface->set_value = gdaex_query_editor_check_set_value;
}

/**
 * gdaex_query_editor_check_new:
 *
 * Creates a new #GdaExQueryEditorCheck widget.
 *
 * Returns: the newly created #GdaExQueryEditorCheck widget.
 */
GtkWidget*
gdaex_query_editor_check_new ()
{
	return GTK_WIDGET (g_object_new (gdaex_query_editor_check_get_type (), NULL));
}

static void
gdaex_query_editor_check_set_property (GObject *object, guint property_id, const GValue *value, GParamSpec *pspec)
{
	GdaExQueryEditorCheck *check = GDAEX_QUERY_EDITOR_CHECK (object);
	GdaExQueryEditorCheckPrivate *priv = GDAEX_QUERY_EDITOR_CHECK_GET_PRIVATE (check);

	switch (property_id)
		{
			default:
				G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
				break;
		}
}

static void
gdaex_query_editor_check_get_property (GObject *object, guint property_id, GValue *value, GParamSpec *pspec)
{
	GdaExQueryEditorCheck *check = GDAEX_QUERY_EDITOR_CHECK (object);
	GdaExQueryEditorCheckPrivate *priv = GDAEX_QUERY_EDITOR_CHECK_GET_PRIVATE (check);

	switch (property_id)
		{
			default:
				G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
				break;
		}
}

static const gchar
*gdaex_query_editor_check_get_value (GdaExQueryEditorIWidget *iwidget)
{
	return g_strdup (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (iwidget)) ? "1" : "0");
}

static const gchar
*gdaex_query_editor_check_get_value_visible (GdaExQueryEditorIWidget *iwidget)
{
	return g_strdup (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (iwidget)) ? _("True") : _("False"));
}

static const gchar
*gdaex_query_editor_check_get_value_sql (GdaExQueryEditorIWidget *iwidget)
{
	return g_strdup (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (iwidget)) ? "TRUE" : "FALSE");
}

static void
gdaex_query_editor_check_set_value (GdaExQueryEditorIWidget *iwidget,
                            const gchar *value)
{
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (iwidget), zak_utils_string_to_boolean (value));
}
