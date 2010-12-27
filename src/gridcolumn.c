/*
 *  gridcolumn.c
 *
 *  Copyright (C) 2005-2010 Andrea Zagli <azagli@libero.it>
 *
 *  This file is part of libgdaex_grid_column.
 *  
 *  libgdaex_grid_column is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *  
 *  libgdaex_grid_column is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *  
 *  You should have received a copy of the GNU General Public License
 *  along with libgdaex; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 */

#ifdef HAVE_CONFIG_H
	#include <config.h>
#endif

#include "gridcolumn.h"

static void gdaex_grid_column_class_init (GdaExGridColumnClass *klass);
static void gdaex_grid_column_init (GdaExGridColumn *gdaex_grid_column);

static void gdaex_grid_column_set_property (GObject *object,
                               guint property_id,
                               const GValue *value,
                               GParamSpec *pspec);
static void gdaex_grid_column_get_property (GObject *object,
                               guint property_id,
                               GValue *value,
                               GParamSpec *pspec);

#define GDAEX_GRID_COLUMN_GET_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE ((obj), TYPE_GDAEX_GRID_COLUMN, GdaExGridColumnPrivate))

typedef struct _GdaExGridColumnPrivate GdaExGridColumnPrivate;
struct _GdaExGridColumnPrivate
	{
		gchar *title;
		gchar *field_name;
		gboolean visible;
		gboolean resizable;
		gboolean sortable;
		gboolean reorderable;
	};

G_DEFINE_TYPE (GdaExGridColumn, gdaex_grid_column, G_TYPE_OBJECT)

static void
gdaex_grid_column_class_init (GdaExGridColumnClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);

	g_type_class_add_private (object_class, sizeof (GdaExGridColumnPrivate));

	object_class->set_property = gdaex_grid_column_set_property;
	object_class->get_property = gdaex_grid_column_get_property;
}

static void
gdaex_grid_column_init (GdaExGridColumn *gdaex_grid_column)
{
	GdaExGridColumnPrivate *priv = GDAEX_GRID_COLUMN_GET_PRIVATE (gdaex_grid_column);
}

GdaExGridColumn
*gdaex_grid_column_new (const gchar *title,
                        const gchar *field_name,
                        gboolean visible,
                        gboolean resizable,
                        gboolean sortable,
                        gboolean reorderable)
{
	GdaExGridColumn *gdaex_grid_column = GDAEX_GRID_COLUMN (g_object_new (gdaex_grid_column_get_type (), NULL));

	GdaExGridColumnPrivate *priv = GDAEX_GRID_COLUMN_GET_PRIVATE (gdaex_grid_column);

	gdaex_grid_column_set_title (gdaex_grid_column, title);
	gdaex_grid_column_set_field_name (gdaex_grid_column, field_name);
	gdaex_grid_column_set_visible (gdaex_grid_column, visible);
	gdaex_grid_column_set_resizable (gdaex_grid_column, resizable);
	gdaex_grid_column_set_sortable (gdaex_grid_column, sortable);
	gdaex_grid_column_set_reorderable (gdaex_grid_column, reorderable);

	return gdaex_grid_column;
}

void
gdaex_grid_column_set_title (GdaExGridColumn *column, const gchar *title)
{

}

const gchar
*gdaex_grid_column_get_title (GdaExGridColumn *column)
{

}

void
gdaex_grid_column_set_field_name (GdaExGridColumn *column, const gchar *field_name)
{

}

const gchar
*gdaex_grid_column_get_field_name (GdaExGridColumn *column)
{

}

void
gdaex_grid_column_set_visible (GdaExGridColumn *column, gboolean visible)
{

}

gboolean
gdaex_grid_column_get_visible (GdaExGridColumn *column)
{

}

void
gdaex_grid_column_set_resizable (GdaExGridColumn *column, gboolean resizable)
{

}

gboolean
gdaex_grid_column_get_resizable (GdaExGridColumn *column)
{

}

void
gdaex_grid_column_set_sortable (GdaExGridColumn *column, gboolean sortable)
{

}

gboolean
gdaex_grid_column_get_sortable (GdaExGridColumn *column)
{

}

void
gdaex_grid_column_set_reorderable (GdaExGridColumn *column, gboolean reorderable)
{

}

gboolean
gdaex_grid_column_get_reorderable (GdaExGridColumn *column)
{

}

/* PRIVATE */
static void
gdaex_grid_column_set_property (GObject *object, guint property_id, const GValue *value, GParamSpec *pspec)
{
	GdaExGridColumn *gdaex_grid_column = GDAEX_GRID_COLUMN (object);

	GdaExGridColumnPrivate *priv = GDAEX_GRID_COLUMN_GET_PRIVATE (gdaex_grid_column);

	switch (property_id)
		{
			default:
				G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
				break;
		}
}

static void
gdaex_grid_column_get_property (GObject *object, guint property_id, GValue *value, GParamSpec *pspec)
{
	GdaExGridColumn *gdaex_grid_column = GDAEX_GRID_COLUMN (object);

	GdaExGridColumnPrivate *priv = GDAEX_GRID_COLUMN_GET_PRIVATE (gdaex_grid_column);

	switch (property_id)
		{
			default:
				G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
				break;
		}
}
