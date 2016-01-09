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

#ifndef __GDAEX_QUERY_EDITOR_ENTRY_DATE_H__
#define __GDAEX_QUERY_EDITOR_ENTRY_DATE_H__

#include <gtk/gtk.h>


G_BEGIN_DECLS


#define GDAEX_QUERY_EDITOR_TYPE_ENTRY_DATE                 (gdaex_query_editor_entry_date_get_type ())
#define GDAEX_QUERY_EDITOR_ENTRY_DATE(obj)                 (G_TYPE_CHECK_INSTANCE_CAST ((obj), GDAEX_QUERY_EDITOR_TYPE_ENTRY_DATE, GdaExQueryEditorEntryDate))
#define GDAEX_QUERY_EDITOR_ENTRY_DATE_CLASS(klass)         (G_TYPE_CHECK_CLASS_CAST ((klass), GDAEX_QUERY_EDITOR_TYPE_ENTRY_DATE, GdaExQueryEditorEntryDateClass))
#define GDAEX_QUERY_EDITOR_IS_ENTRY_DATE(obj)              (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GDAEX_QUERY_EDITOR_TYPE_ENTRY_DATE))
#define GDAEX_QUERY_EDITOR_IS_ENTRY_DATE_CLASS(klass)      (G_TYPE_CHECK_CLASS_TYPE ((klass), GDAEX_QUERY_EDITOR_TYPE__ENTRY_DATE))
#define GDAEX_QUERY_EDITOR_ENTRY_DATE_GET_CLASS(obj)       (G_TYPE_INSTANCE_GET_CLASS ((obj), GDAEX_QUERY_EDITOR_TYPE_ENTRY_DATE, GdaExQueryEditorEntryDateClass))


typedef struct _GdaExQueryEditorEntryDate        GdaExQueryEditorEntryDate;
typedef struct _GdaExQueryEditorEntryDateClass   GdaExQueryEditorEntryDateClass;


struct _GdaExQueryEditorEntryDate
{
	GtkEntry entry;
};

struct _GdaExQueryEditorEntryDateClass
{
	GtkEntryClass parent_class;
};


GType gdaex_query_editor_entry_date_get_type (void) G_GNUC_CONST;

GtkWidget *gdaex_query_editor_entry_date_new (void);

void gdaex_query_editor_entry_date_set_format (GdaExQueryEditorEntryDate *entry, const gchar *format);
gchar *gdaex_query_editor_entry_date_get_format (GdaExQueryEditorEntryDate* entry);


G_END_DECLS


#endif /* __GDAEX_QUERY_EDITOR_ENTRY_DATE_H__ */
