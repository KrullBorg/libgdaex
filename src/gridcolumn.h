/*
 *  gridcolumn.h
 *
 *  Copyright (C) 2010 Andrea Zagli <azagli@libero.it>
 *
 *  This file is part of libgdaex.
 *
 *  libgdaex is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  libgdaex is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with libgdaex; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *  
 */

#ifndef __GDAEX_GRID_COLUMN_H__
#define __GDAEX_GRID_COLUMN_H__

#include <glib.h>
#include <glib-object.h>

G_BEGIN_DECLS


#define GDAEX_TYPE_GRID_COLUMN                 (gdaex_get_type ())
#define GDAEX_GRID_COLUMN(obj)                 (G_TYPE_CHECK_INSTANCE_CAST ((obj), GDAEX_TYPE_GRID_COLUMN, GdaExGridColumn))
#define GDAEX_GRID_COLUMN_CLASS(klass)         (G_TYPE_CHECK_CLASS_CAST ((klass), GDAEX_TYPE_GRID_COLUMN, GdaExGridColumnClass))
#define GDAEX_IS_GRID_COLUMN(obj)              (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GDAEX_TYPE_GRID_COLUMN))
#define GDAEX_IS_GRID_COLUMN_CLASS(klass)      (G_TYPE_CHECK_CLASS_TYPE ((klass), GDAEX_TYPE_GRID_COLUMN))
#define GDAEX_GRID_COLUMN_GET_CLASS(obj)       (G_TYPE_INSTANCE_GET_CLASS ((obj), GDAEX_TYPE_GRID_COLUMN, GdaExGridColumnClass))


typedef struct _GdaExGridColumn GdaExGridColumn;
typedef struct _GdaExGridColumnClass GdaExGridColumnClass;

struct _GdaExGridColumn
	{
		GObject parent;
	};

struct _GdaExGridColumnClass
	{
		GObjectClass parent_class;
	};

GType gdaex_grid_column_get_type (void) G_GNUC_CONST;


GdaExGridColumn *gdaex_grid_column_new (const gchar *title,
                                        const gchar *field_name,
                                        gboolean visible,
                                        gboolean resizable,
                                        gboolean sortable,
                                        gboolean reorderable);

void gdaex_grid_column_set_title (GdaExGridColumn *column, const gchar *title);
const gchar *gdaex_grid_column_get_title (GdaExGridColumn *column);

void gdaex_grid_column_set_field_name (GdaExGridColumn *column, const gchar *field_name);
const gchar *gdaex_grid_column_get_field_name (GdaExGridColumn *column);

void gdaex_grid_column_set_visible (GdaExGridColumn *column, gboolean visible);
gboolean gdaex_grid_column_get_visible (GdaExGridColumn *column);

void gdaex_grid_column_set_resizable (GdaExGridColumn *column, gboolean resizable);
gboolean gdaex_grid_column_get_resizable (GdaExGridColumn *column);

void gdaex_grid_column_set_sortable (GdaExGridColumn *column, gboolean sortable);
gboolean gdaex_grid_column_get_sortable (GdaExGridColumn *column);

void gdaex_grid_column_set_reorderable (GdaExGridColumn *column, gboolean reorderable);
gboolean gdaex_grid_column_get_reorderable (GdaExGridColumn *column);


G_END_DECLS

#endif /* __GDAEX_GRID_COLUMN_H__ */
