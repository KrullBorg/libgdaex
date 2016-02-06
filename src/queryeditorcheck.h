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

#ifndef __GDAEX_QUERY_EDITOR_CHECK_H__
#define __GDAEX_QUERY_EDITOR_CHECK_H__

#include <gtk/gtk.h>


G_BEGIN_DECLS


#define GDAEX_QUERY_EDITOR_TYPE_CHECK                 (gdaex_query_editor_check_get_type ())
#define GDAEX_QUERY_EDITOR_CHECK(obj)                 (G_TYPE_CHECK_INSTANCE_CAST ((obj), GDAEX_QUERY_EDITOR_TYPE_CHECK, GdaExQueryEditorCheck))
#define GDAEX_QUERY_EDITOR_CHECK_CLASS(klass)         (G_TYPE_CHECK_CLASS_CAST ((klass), GDAEX_QUERY_EDITOR_TYPE_CHECK, GdaExQueryEditorCheckClass))
#define GDAEX_QUERY_EDITOR_IS_CHECK(obj)              (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GDAEX_QUERY_EDITOR_TYPE_CHECK))
#define GDAEX_QUERY_EDITOR_IS_CHECK_CLASS(klass)      (G_TYPE_CHECK_CLASS_TYPE ((klass), GDAEX_QUERY_EDITOR_TYPE_CHECK))
#define GDAEX_QUERY_EDITOR_CHECK_GET_CLASS(obj)       (G_TYPE_INSTANCE_GET_CLASS ((obj), GDAEX_QUERY_EDITOR_TYPE_CHECK, GdaExQueryEditorCheckClass))


typedef struct _GdaExQueryEditorCheck        GdaExQueryEditorCheck;
typedef struct _GdaExQueryEditorCheckClass   GdaExQueryEditorCheckClass;


struct _GdaExQueryEditorCheck
{
	GtkCheckButton check;
};

struct _GdaExQueryEditorCheckClass
{
	GtkCheckButtonClass parent_class;
};


GType gdaex_query_editor_check_get_type (void) G_GNUC_CONST;

GtkWidget *gdaex_query_editor_check_new (void);


G_END_DECLS


#endif /* __GDAEX_QUERY_EDITOR_CHECK_H__ */
