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

#ifdef HAVE_CONFIG_H
	#include <config.h>
#endif

#include <stdarg.h>
#include <string.h>

#include <glib/gi18n-lib.h>

#include <libxml/parser.h>
#include <libxml/xpath.h>

#include <libzakutils/libzakutils.h>

#include "queryeditormarshal.h"
#include "queryeditor.h"
#include "queryeditorentry.h"
#include "queryeditorentrydate.h"

#define GROUP "{--group--}"

typedef struct
	{
		gchar *name;
		gchar *name_visible;
		gboolean visible;

		GHashTable *fields;	/* GdaExQueryEditorField */
	} GdaExQueryEditorTable;

typedef struct
	{
		GdaExQueryEditorTable *table1;
		GdaExQueryEditorTable *table2;

		GdaExQueryEditorJoinType join_type;

		GSList *fields1;
		GSList *fields2;
	} GdaExQueryEditorRelation;

typedef enum
	{
		GDAEX_QE_LINK_TYPE_AND = 1,
		GDAEX_QE_LINK_TYPE_OR
	} GdaExQueryEditorLinkType;

enum
	{
		GDAEX_QE_PAGE_SHOW,
		GDAEX_QE_PAGE_WHERE,
		GDAEX_QE_PAGE_ORDER
	};

static void gdaex_query_editor_class_init (GdaExQueryEditorClass *class);
static void gdaex_query_editor_init (GdaExQueryEditor *gdaex_query_editor);

static void gdaex_query_editor_dispose (GObject *object);
static void gdaex_query_editor_finalize (GObject *object);

static void gdaex_query_editor_set_property (GObject *object,
                               guint property_id,
                               const GValue *value,
                               GParamSpec *pspec);
static void gdaex_query_editor_get_property (GObject *object,
                               guint property_id,
                               GValue *value,
                               GParamSpec *pspec);

static GdaExQueryEditorIWidget *gdaex_query_editor_iwidget_construct (GdaExQueryEditor *qe,
																	  const gchar *type,
																	  xmlNode *xnode);

static void gdaex_query_editor_clean (GdaExQueryEditor *gdaex_query_editor);

static gboolean _gdaex_query_editor_add_table (GdaExQueryEditor *qe,
                              const gchar *table_name,
                              const gchar *table_name_visibile,
                              gboolean is_visible);

static void gdaex_query_editor_refresh_gui (GdaExQueryEditor *qe);
static void gdaex_query_editor_refresh_gui_add_fields (GdaExQueryEditor *qe,
                                                       GdaExQueryEditorTable *table,
                                                       GtkTreeIter *iter_parent);

static void gdaex_query_editor_refill_always_show (GdaExQueryEditor *qe);

static void gdaex_query_editor_store_remove_iter (GdaExQueryEditor *qe,
                                       GtkTreeSelection *sel,
                                       GObject *store);
static void gdaex_query_editor_store_move_iter_up_down (GdaExQueryEditor *qe,
                                      GtkTreeSelection *sel,
                                      GObject *store,
                                      gboolean up);

static gboolean gdaex_query_editor_model_has_value (GtkTreeModel *model,
                                                    guint column,
                                                    GValue *value);

static void gdaex_query_editor_remove_child_from_vbx_values (GdaExQueryEditor *qe);

static gchar *gdaex_query_editor_get_where_type_str_from_type (guint where_type);
static gchar *gdaex_query_editor_get_link_type_str_from_type (guint link_type);

static void gdaex_query_editor_on_cb_where_type_changed (GtkComboBox *widget,
                                                         gpointer user_data);
static void gdaex_query_editor_on_btn_cancel_clicked (GtkButton *button,
                                    gpointer user_data);
static void gdaex_query_editor_on_btn_save_clicked (GtkButton *button,
                                    gpointer user_data);

static void gdaex_query_editor_on_sel_fields_changed (GtkTreeSelection *treeselection,
                                                    gpointer user_data);
static void gdaex_query_editor_on_trv_fields_row_activated (GtkTreeView *tree_view,
                                                            GtkTreePath *path,
                                                            GtkTreeViewColumn *column,
                                                            gpointer user_data);

static void gdaex_query_editor_show_add_iter (GdaExQueryEditor *qe, GtkTreeIter *iter);

static void gdaex_query_editor_on_btn_show_add_clicked (GtkButton *button,
                                    gpointer user_data);
static void gdaex_query_editor_on_btn_show_remove_clicked (GtkButton *button,
                                    gpointer user_data);
static void gdaex_query_editor_on_btn_show_up_clicked (GtkButton *button,
                                    gpointer user_data);
static void gdaex_query_editor_on_btn_show_down_clicked (GtkButton *button,
                                    gpointer user_data);
static void gdaex_query_editor_on_btn_show_clean_clicked (GtkButton *button,
                                    gpointer user_data);
static void gdaex_query_editor_on_sel_show_changed (GtkTreeSelection *treeselection,
                                                    gpointer user_data);

static void gdaex_query_editor_on_btn_where_add_clicked (GtkButton *button,
                                    gpointer user_data);
static void gdaex_query_editor_on_btn_where_add_group_clicked (GtkButton *button,
                                    gpointer user_data);
static void gdaex_query_editor_on_btn_where_remove_clicked (GtkButton *button,
                                    gpointer user_data);
static void gdaex_query_editor_on_btn_where_up_clicked (GtkButton *button,
                                    gpointer user_data);
static void gdaex_query_editor_on_btn_where_down_clicked (GtkButton *button,
                                    gpointer user_data);
static void gdaex_query_editor_on_btn_where_clean_clicked (GtkButton *button,
                                    gpointer user_data);
static void gdaex_query_editor_on_sel_where_changed (GtkTreeSelection *treeselection,
                                                    gpointer user_data);

static void gdaex_query_editor_on_btn_order_add_clicked (GtkButton *button,
                                    gpointer user_data);
static void gdaex_query_editor_on_btn_order_remove_clicked (GtkButton *button,
                                    gpointer user_data);
static void gdaex_query_editor_on_btn_order_up_clicked (GtkButton *button,
                                    gpointer user_data);
static void gdaex_query_editor_on_btn_order_down_clicked (GtkButton *button,
                                    gpointer user_data);
static void gdaex_query_editor_on_btn_order_clean_clicked (GtkButton *button,
                                    gpointer user_data);
static void gdaex_query_editor_on_sel_order_changed (GtkTreeSelection *treeselection,
                                                    gpointer user_data);

static void gdaex_query_editor_on_txt1_btn_browse_clicked (gpointer instance, gpointer user_data);
static void gdaex_query_editor_on_txt2_btn_browse_clicked (gpointer instance, gpointer user_data);


#define GDAEX_QUERY_EDITOR_GET_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE ((obj), TYPE_GDAEX_QUERY_EDITOR, GdaExQueryEditorPrivate))

typedef struct _GdaExQueryEditorPrivate GdaExQueryEditorPrivate;
struct _GdaExQueryEditorPrivate
	{
		GdaEx *gdaex;

		GPtrArray *ar_modules;

		GtkBuilder *gtkbuilder;

		GtkWidget *notebook;

		GtkWidget *hpaned_main;

		GtkWidget *vbx_values_container;
		GtkWidget *vbx_values;

		GtkWidget *trv_where;

		GtkTreeStore *tstore_fields;
		GtkListStore *lstore_show;
		GtkTreeStore *tstore_where;
		GtkListStore *lstore_order;

		GtkTreeSelection *sel_fields;
		GtkTreeSelection *sel_show;
		GtkTreeSelection *sel_where;
		GtkTreeSelection *sel_order;

		GHashTable *tables;	/* GdaExQueryEditorTable */

		GSList *relations;	/* GdaExQueryEditorRelation */

		/* for value choosing */
		guint editor_type;

		GtkWidget *hbox_show;
		GtkWidget *hbox_where;
		GtkWidget *hbox_order;

		GtkWidget *frame_show;
		GtkWidget *frame_where;
		GtkWidget *frame_order;

		GtkWidget *tbl;
		GtkWidget *lbl_link_type;
		GtkWidget *cb_link_type;
		GtkWidget *lbl_not;
		GtkWidget *chk_not;
		GtkWidget *lbl_where_type;
		GtkWidget *cb_where_type;
		GtkWidget *lbl_txt_alias;
		GtkWidget *txt_alias;
		GtkWidget *lbl_from;
		GtkWidget *txt_from;
		GtkWidget *lbl_to;
		GtkWidget *txt_to;
		GtkWidget *opt_asc;
		GtkWidget *opt_desc;

		GtkListStore *lstore_link_type;
		GtkListStore *lstore_where_type;
	};

G_DEFINE_TYPE (GdaExQueryEditor, gdaex_query_editor, G_TYPE_OBJECT)

enum
	{
		COL_FIELDS_TABLE_NAME,
		COL_FIELDS_NAME,
		COL_FIELDS_VISIBLE_NAME,
		COL_FIELDS_DESCRIPTION
	};

enum
	{
		COL_SHOW_TABLE_NAME,
		COL_SHOW_NAME,
		COL_SHOW_VISIBLE_NAME,
		COL_SHOW_ALIAS
	};

enum
	{
		COL_WHERE_LINK_TYPE,
		COL_WHERE_LINK_TYPE_VISIBLE,
		COL_WHERE_TABLE_NAME,
		COL_WHERE_NAME,
		COL_WHERE_VISIBLE_NAME,
		COL_WHERE_CONDITION_NOT,
		COL_WHERE_CONDITION_TYPE,
		COL_WHERE_CONDITION_TYPE_VISIBLE,
		COL_WHERE_CONDITION_FROM,
		COL_WHERE_CONDITION_FROM_VISIBLE,
		COL_WHERE_CONDITION_FROM_SQL,
		COL_WHERE_CONDITION_TO,
		COL_WHERE_CONDITION_TO_VISIBLE,
		COL_WHERE_CONDITION_TO_SQL
	};

enum
	{
		COL_ORDER_TABLE_NAME,
		COL_ORDER_NAME,
		COL_ORDER_VISIBLE_NAME,
		COL_ORDER_ORDER
	};

static void
gdaex_query_editor_class_init (GdaExQueryEditorClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);

	object_class->set_property = gdaex_query_editor_set_property;
	object_class->get_property = gdaex_query_editor_get_property;

	object_class->dispose = gdaex_query_editor_dispose;
	object_class->finalize = gdaex_query_editor_finalize;

	g_type_class_add_private (object_class, sizeof (GdaExQueryEditorPrivate));

	/**
	 * GdaExQueryEditor::iwidget-init:
	 *
	 */
	klass->iwidget_init_signal_id = g_signal_new ("iwidget-init",
												  G_TYPE_FROM_CLASS (object_class),
												  G_SIGNAL_RUN_LAST,
												  0,
												  NULL,
												  NULL,
												  _gdaex_query_editor_marshal_VOID__OBJECT_STRING_STRING_BOOLEAN,
												  G_TYPE_NONE,
												  4,
												  G_TYPE_OBJECT,
												  G_TYPE_STRING,
												  G_TYPE_STRING,
												  G_TYPE_BOOLEAN);
}

static void
gdaex_query_editor_init (GdaExQueryEditor *gdaex_query_editor)
{
	GdaExQueryEditorPrivate *priv = GDAEX_QUERY_EDITOR_GET_PRIVATE (gdaex_query_editor);

}

static void
gdaex_query_editor_load_modules (GdaExQueryEditor *gdaex_query_editor)
{
	GdaExQueryEditorPrivate *priv;

	gchar *modulesdir;
	GDir *dir;
	GError *error;

	GModule *module;
	const gchar *filename;

	if (g_module_supported ())
		{
			priv = GDAEX_QUERY_EDITOR_GET_PRIVATE (gdaex_query_editor);

			modulesdir = (gchar *)g_getenv ("LIBGDAEX_MODULESDIR");
			if (modulesdir == NULL)
				{
#ifdef G_OS_WIN32

					gchar *moddir;
					gchar *p;

					moddir = g_win32_get_package_installation_directory_of_module (NULL);

					p = g_strrstr (moddir, g_strdup_printf ("%c", G_DIR_SEPARATOR));
					if (p != NULL
						&& (g_ascii_strcasecmp (p + 1, "src") == 0
							|| g_ascii_strcasecmp (p + 1, ".libs") == 0))
						{
							modulesdir = g_strdup (MODULESDIR);
						}
					else
						{
							modulesdir = g_build_filename (moddir, "lib", PACKAGE, "modules", NULL);
						}

#else

					modulesdir = g_strdup (MODULESDIR);

#endif
				}

			/* load myself as module (for filters and validators) */
			module = g_module_open (NULL, G_MODULE_BIND_LAZY);
			if (module == NULL)
				{
					g_warning (_("Unable to load module of myself"));
				}
			else
				{
					if (priv->ar_modules == NULL)
						{
							priv->ar_modules = g_ptr_array_new ();
						}
					g_ptr_array_add (priv->ar_modules, (gpointer)module);
				}

			/* for each file in MODULESDIR */
			error = NULL;
			dir = g_dir_open (modulesdir, 0, &error);
			if (dir != NULL && error == NULL)
				{
					while ((filename = g_dir_read_name (dir)) != NULL)
						{
							/* trying to open the module */
							module = g_module_open (filename, G_MODULE_BIND_LAZY);
							if (module != NULL)
								{
									if (priv->ar_modules == NULL)
										{
											priv->ar_modules = g_ptr_array_new ();
										}
								    g_ptr_array_add (priv->ar_modules, (gpointer)module);
								}
							else
								{
									g_warning (_("Unable to load %s: %s."), filename, g_module_error ());
								}
						}

					g_dir_close (dir);
				}
			else
				{
					g_warning (_("Unable to open modules dir: %s."),
							   error != NULL && error->message != NULL ? error->message : _("no details"));
				}
		}
	else
		{
			g_warning (_("Modules not supported by this operating system."));
		}
}

/**
 * gdaex_query_editor_new:
 * @gdaex:
 *
 * Returns: the newly created #GdaExQueryEditor object.
 */
GdaExQueryEditor
*gdaex_query_editor_new (GdaEx *gdaex)
{
	GError *error;

	GdaExQueryEditor *gdaex_query_editor;
	GdaExQueryEditorPrivate *priv;

	g_return_val_if_fail (IS_GDAEX (gdaex), NULL);

	gdaex_query_editor = GDAEX_QUERY_EDITOR (g_object_new (gdaex_query_editor_get_type (), NULL));

	priv = GDAEX_QUERY_EDITOR_GET_PRIVATE (gdaex_query_editor);

	gdaex_query_editor_clean (gdaex_query_editor);

	priv->lstore_link_type = gtk_list_store_new (2,
	                                             G_TYPE_UINT,
	                                             G_TYPE_STRING);
	priv->lstore_where_type = gtk_list_store_new (2,
	                                              G_TYPE_UINT,
	                                              G_TYPE_STRING);

	priv->gdaex = gdaex;

	gdaex_query_editor_load_modules (gdaex_query_editor);

	priv->gtkbuilder = gdaex_get_gtkbuilder (priv->gdaex);

	error = NULL;
	gtk_builder_add_objects_from_file (priv->gtkbuilder,
	                                   gdaex_get_guifile (priv->gdaex),
	                                   g_strsplit ("tstore_fields"
	                                               "|lstore_show"
	                                               "|tstore_where"
	                                               "|lstore_order"
	                                               "|hpaned_query_editor",
	                                               "|", -1),
	                                   &error);
	if (error != NULL)
		{
			g_warning (_("Error on gui initialization: %s."),
			           error->message != NULL ? error->message : _("no details"));
			return NULL;
		}

	priv->notebook = GTK_WIDGET (gtk_builder_get_object (priv->gtkbuilder, "notebook1"));

	priv->trv_where = GTK_WIDGET (gtk_builder_get_object (priv->gtkbuilder, "treeview3"));

	priv->tstore_fields = GTK_TREE_STORE (gtk_builder_get_object (priv->gtkbuilder, "tstore_fields"));
	priv->lstore_show = GTK_LIST_STORE (gtk_builder_get_object (priv->gtkbuilder, "lstore_show"));
	priv->tstore_where = GTK_TREE_STORE (gtk_builder_get_object (priv->gtkbuilder, "tstore_where"));
	priv->lstore_order = GTK_LIST_STORE (gtk_builder_get_object (priv->gtkbuilder, "lstore_order"));

	priv->sel_fields = gtk_tree_view_get_selection (GTK_TREE_VIEW (gtk_builder_get_object (priv->gtkbuilder, "treeview1")));
	priv->sel_show = gtk_tree_view_get_selection (GTK_TREE_VIEW (gtk_builder_get_object (priv->gtkbuilder, "treeview2")));
	priv->sel_where = gtk_tree_view_get_selection (GTK_TREE_VIEW (priv->trv_where));
	priv->sel_order = gtk_tree_view_get_selection (GTK_TREE_VIEW (gtk_builder_get_object (priv->gtkbuilder, "treeview4")));

	priv->vbx_values_container = GTK_WIDGET (gtk_builder_get_object (priv->gtkbuilder, "vbox3"));
	priv->vbx_values = GTK_WIDGET (gtk_builder_get_object (priv->gtkbuilder, "vbox4"));

	g_signal_connect (gtk_builder_get_object (priv->gtkbuilder, "button16"), "clicked",
	                  G_CALLBACK (gdaex_query_editor_on_btn_cancel_clicked), (gpointer)gdaex_query_editor);
	g_signal_connect (gtk_builder_get_object (priv->gtkbuilder, "button15"), "clicked",
	                  G_CALLBACK (gdaex_query_editor_on_btn_save_clicked), (gpointer)gdaex_query_editor);

	g_signal_connect (priv->sel_fields, "changed",
	                  G_CALLBACK (gdaex_query_editor_on_sel_fields_changed), (gpointer)gdaex_query_editor);
	g_signal_connect (gtk_builder_get_object (priv->gtkbuilder, "treeview1"), "row-activated",
	                  G_CALLBACK (gdaex_query_editor_on_trv_fields_row_activated), (gpointer)gdaex_query_editor);

	g_signal_connect (gtk_builder_get_object (priv->gtkbuilder, "button3"), "clicked",
	                  G_CALLBACK (gdaex_query_editor_on_btn_show_add_clicked), (gpointer)gdaex_query_editor);
	g_signal_connect (gtk_builder_get_object (priv->gtkbuilder, "button4"), "clicked",
	                  G_CALLBACK (gdaex_query_editor_on_btn_show_remove_clicked), (gpointer)gdaex_query_editor);
	g_signal_connect (gtk_builder_get_object (priv->gtkbuilder, "button5"), "clicked",
	                  G_CALLBACK (gdaex_query_editor_on_btn_show_up_clicked), (gpointer)gdaex_query_editor);
	g_signal_connect (gtk_builder_get_object (priv->gtkbuilder, "button6"), "clicked",
	                  G_CALLBACK (gdaex_query_editor_on_btn_show_down_clicked), (gpointer)gdaex_query_editor);
	g_signal_connect (gtk_builder_get_object (priv->gtkbuilder, "button1"), "clicked",
	                  G_CALLBACK (gdaex_query_editor_on_btn_show_clean_clicked), (gpointer)gdaex_query_editor);
	g_signal_connect (priv->sel_show, "changed",
	                  G_CALLBACK (gdaex_query_editor_on_sel_show_changed), (gpointer)gdaex_query_editor);

	g_signal_connect (gtk_builder_get_object (priv->gtkbuilder, "button7"), "clicked",
	                  G_CALLBACK (gdaex_query_editor_on_btn_where_add_clicked), (gpointer)gdaex_query_editor);
	g_signal_connect (gtk_builder_get_object (priv->gtkbuilder, "button18"), "clicked",
	                  G_CALLBACK (gdaex_query_editor_on_btn_where_add_group_clicked), (gpointer)gdaex_query_editor);
	g_signal_connect (gtk_builder_get_object (priv->gtkbuilder, "button8"), "clicked",
	                  G_CALLBACK (gdaex_query_editor_on_btn_where_remove_clicked), (gpointer)gdaex_query_editor);
	g_signal_connect (gtk_builder_get_object (priv->gtkbuilder, "button9"), "clicked",
	                  G_CALLBACK (gdaex_query_editor_on_btn_where_up_clicked), (gpointer)gdaex_query_editor);
	g_signal_connect (gtk_builder_get_object (priv->gtkbuilder, "button10"), "clicked",
	                  G_CALLBACK (gdaex_query_editor_on_btn_where_down_clicked), (gpointer)gdaex_query_editor);
	g_signal_connect (gtk_builder_get_object (priv->gtkbuilder, "button2"), "clicked",
	                  G_CALLBACK (gdaex_query_editor_on_btn_where_clean_clicked), (gpointer)gdaex_query_editor);
	g_signal_connect (priv->sel_where, "changed",
	                  G_CALLBACK (gdaex_query_editor_on_sel_where_changed), (gpointer)gdaex_query_editor);

	g_signal_connect (gtk_builder_get_object (priv->gtkbuilder, "button11"), "clicked",
	                  G_CALLBACK (gdaex_query_editor_on_btn_order_add_clicked), (gpointer)gdaex_query_editor);
	g_signal_connect (gtk_builder_get_object (priv->gtkbuilder, "button12"), "clicked",
	                  G_CALLBACK (gdaex_query_editor_on_btn_order_remove_clicked), (gpointer)gdaex_query_editor);
	g_signal_connect (gtk_builder_get_object (priv->gtkbuilder, "button13"), "clicked",
	                  G_CALLBACK (gdaex_query_editor_on_btn_order_up_clicked), (gpointer)gdaex_query_editor);
	g_signal_connect (gtk_builder_get_object (priv->gtkbuilder, "button14"), "clicked",
	                  G_CALLBACK (gdaex_query_editor_on_btn_order_down_clicked), (gpointer)gdaex_query_editor);
	g_signal_connect (gtk_builder_get_object (priv->gtkbuilder, "button17"), "clicked",
	                  G_CALLBACK (gdaex_query_editor_on_btn_order_clean_clicked), (gpointer)gdaex_query_editor);
	g_signal_connect (priv->sel_order, "changed",
	                  G_CALLBACK (gdaex_query_editor_on_sel_order_changed), (gpointer)gdaex_query_editor);

	gtk_widget_set_sensitive (GTK_WIDGET (gtk_builder_get_object (priv->gtkbuilder, "button3")), FALSE);
	gtk_widget_set_sensitive (GTK_WIDGET (gtk_builder_get_object (priv->gtkbuilder, "button7")), FALSE);
	gtk_widget_set_sensitive (GTK_WIDGET (gtk_builder_get_object (priv->gtkbuilder, "button11")), FALSE);

	return gdaex_query_editor;
}

GtkWidget
*gdaex_query_editor_get_widget (GdaExQueryEditor *gdaex_query_editor)
{
	GdaExQueryEditorPrivate *priv;

	g_return_val_if_fail (GDAEX_IS_QUERY_EDITOR (gdaex_query_editor), NULL);

	priv = GDAEX_QUERY_EDITOR_GET_PRIVATE (gdaex_query_editor);

	if (priv->hpaned_main == NULL)
		{
			priv->hpaned_main = GTK_WIDGET (gtk_builder_get_object (priv->gtkbuilder, "hpaned_query_editor"));
		}

	gdaex_query_editor_refresh_gui (gdaex_query_editor);

	return priv->hpaned_main;
}

void
gdaex_query_editor_set_show_visible (GdaExQueryEditor *qe, gboolean visibile)
{
	GdaExQueryEditorPrivate *priv;

	GtkWidget *wpage;

	g_return_if_fail (GDAEX_IS_QUERY_EDITOR (qe));

	priv = GDAEX_QUERY_EDITOR_GET_PRIVATE (qe);

	wpage = gtk_notebook_get_nth_page (GTK_NOTEBOOK (priv->notebook), GDAEX_QE_PAGE_SHOW);
	if (visibile)
		{
			gtk_widget_show_all (wpage);
		}
	else
		{
			gtk_widget_hide (wpage);
		}
}

void
gdaex_query_editor_set_where_visible (GdaExQueryEditor *qe, gboolean visibile)
{
	GdaExQueryEditorPrivate *priv;

	GtkWidget *wpage;

	g_return_if_fail (GDAEX_IS_QUERY_EDITOR (qe));

	priv = GDAEX_QUERY_EDITOR_GET_PRIVATE (qe);

	wpage = gtk_notebook_get_nth_page (GTK_NOTEBOOK (priv->notebook), GDAEX_QE_PAGE_WHERE);
	if (visibile)
		{
			gtk_widget_show_all (wpage);
		}
	else
		{
			gtk_widget_hide (wpage);
		}
}

void
gdaex_query_editor_set_order_visible (GdaExQueryEditor *qe, gboolean visibile)
{
	GdaExQueryEditorPrivate *priv;

	GtkWidget *wpage;

	g_return_if_fail (GDAEX_IS_QUERY_EDITOR (qe));

	priv = GDAEX_QUERY_EDITOR_GET_PRIVATE (qe);

	wpage = gtk_notebook_get_nth_page (GTK_NOTEBOOK (priv->notebook), GDAEX_QE_PAGE_ORDER);
	if (visibile)
		{
			gtk_widget_show_all (wpage);
		}
	else
		{
			gtk_widget_hide (wpage);
		}
}

gboolean
gdaex_query_editor_get_show_visible (GdaExQueryEditor *qe)
{
	GdaExQueryEditorPrivate *priv;

	GtkWidget *wpage;

	g_return_val_if_fail (GDAEX_IS_QUERY_EDITOR (qe), FALSE);

	priv = GDAEX_QUERY_EDITOR_GET_PRIVATE (qe);

	wpage = gtk_notebook_get_nth_page (GTK_NOTEBOOK (priv->notebook), GDAEX_QE_PAGE_SHOW);
	return gtk_widget_get_visible (wpage);
}

gboolean
gdaex_query_editor_get_where_visible (GdaExQueryEditor *qe)
{
	GdaExQueryEditorPrivate *priv;

	GtkWidget *wpage;

	g_return_val_if_fail (GDAEX_IS_QUERY_EDITOR (qe), FALSE);

	priv = GDAEX_QUERY_EDITOR_GET_PRIVATE (qe);

	wpage = gtk_notebook_get_nth_page (GTK_NOTEBOOK (priv->notebook), GDAEX_QE_PAGE_WHERE);
	return gtk_widget_get_visible (wpage);
}

gboolean
gdaex_query_editor_get_order_visible (GdaExQueryEditor *qe)
{
	GdaExQueryEditorPrivate *priv;

	GtkWidget *wpage;

	g_return_val_if_fail (GDAEX_IS_QUERY_EDITOR (qe), FALSE);

	priv = GDAEX_QUERY_EDITOR_GET_PRIVATE (qe);

	wpage = gtk_notebook_get_nth_page (GTK_NOTEBOOK (priv->notebook), GDAEX_QE_PAGE_ORDER);
	return gtk_widget_get_visible (wpage);
}

gboolean
gdaex_query_editor_add_table (GdaExQueryEditor *qe,
                              const gchar *table_name,
                              const gchar *table_name_visibile)
{
	return _gdaex_query_editor_add_table (qe, table_name, table_name_visibile, TRUE);
}

gboolean
gdaex_query_editor_table_add_field (GdaExQueryEditor *qe,
                                    const gchar *table_name,
                                    GdaExQueryEditorField field)
{
	gboolean ret;

	GdaExQueryEditorPrivate *priv;
	GdaExQueryEditorTable *table;
	GdaExQueryEditorField *_field;

	g_return_val_if_fail (GDAEX_IS_QUERY_EDITOR (qe), FALSE);
	g_return_val_if_fail (table_name != NULL, FALSE);

	priv = GDAEX_QUERY_EDITOR_GET_PRIVATE (qe);

	table = g_hash_table_lookup (priv->tables, table_name);
	if (table == NULL)
		{
			g_warning (_("Table «%s» doesn't exists."), table_name);
			return FALSE;
		}

	_field = g_memdup (&field, sizeof (GdaExQueryEditorField));
	_field->table_name = g_strdup (table_name);

	if (_field->name == NULL)
		{
			g_warning (_("No field added: the field must have a name."));
			g_free (_field);
			return FALSE;
		}
	else
		{
			if (g_strcmp0 (g_strstrip (_field->name), "") == 0)
				{
					g_warning (_("No field added: the field must have a name."));
					g_free (_field);
					return FALSE;
				}
		}

	if (_field->name_visible == NULL)
		{
			_field->name_visible = g_strdup (_field->name);
		}
	else
		{
			if (g_strcmp0 (g_strstrip (_field->name_visible), "") == 0)
				{
					_field->name_visible = g_strdup (_field->name);
				}
		}

	if (!GDAEX_QUERY_EDITOR_IS_IWIDGET (_field->iwidget_from))
		{
			if (_field->type == GDAEX_QE_FIELD_TYPE_DATE
				|| _field->type == GDAEX_QE_FIELD_TYPE_DATETIME
				|| _field->type == GDAEX_QE_FIELD_TYPE_TIME)
				{
					_field->iwidget_from = GDAEX_QUERY_EDITOR_IWIDGET (gdaex_query_editor_entry_date_new ());
					/* TODO
					 * read format from locale */
					if (_field->type == GDAEX_QE_FIELD_TYPE_DATE)
						{
							gdaex_query_editor_entry_date_set_format (GDAEX_QUERY_EDITOR_ENTRY_DATE (_field->iwidget_from), "%d/%m/%Y");
							gtk_entry_set_max_length (GTK_ENTRY (_field->iwidget_from), 10);
						}
					else if (_field->type == GDAEX_QE_FIELD_TYPE_DATETIME)
						{
							gdaex_query_editor_entry_date_set_format (GDAEX_QUERY_EDITOR_ENTRY_DATE (_field->iwidget_from), "%d/%m/%Y %H:%M:%S");
							gtk_entry_set_max_length (GTK_ENTRY (_field->iwidget_from), 19);
						}
					else if (_field->type == GDAEX_QE_FIELD_TYPE_TIME)
						{
							gdaex_query_editor_entry_date_set_format (GDAEX_QUERY_EDITOR_ENTRY_DATE (_field->iwidget_from), "%H:%M:%S");
							gtk_entry_set_max_length (GTK_ENTRY (_field->iwidget_from), 8);
						}
				}
			else
				{
					_field->iwidget_from = GDAEX_QUERY_EDITOR_IWIDGET (gdaex_query_editor_entry_new ());
				}
		}
	if (!GDAEX_QUERY_EDITOR_IS_IWIDGET (_field->iwidget_to))
		{
			if (_field->type == GDAEX_QE_FIELD_TYPE_DATE
				|| _field->type == GDAEX_QE_FIELD_TYPE_DATETIME
				|| _field->type == GDAEX_QE_FIELD_TYPE_TIME)
				{
					_field->iwidget_to = GDAEX_QUERY_EDITOR_IWIDGET (gdaex_query_editor_entry_date_new ());
					/* TODO
					 * read format from locale */
					if (_field->type == GDAEX_QE_FIELD_TYPE_DATE)
						{
							gdaex_query_editor_entry_date_set_format (GDAEX_QUERY_EDITOR_ENTRY_DATE (_field->iwidget_to), "%d/%m/%Y");
							gtk_entry_set_max_length (GTK_ENTRY (_field->iwidget_to), 10);
						}
					else if (_field->type == GDAEX_QE_FIELD_TYPE_DATETIME)
						{
							gdaex_query_editor_entry_date_set_format (GDAEX_QUERY_EDITOR_ENTRY_DATE (_field->iwidget_to), "%d/%m/%Y %H:%M:%S");
							gtk_entry_set_max_length (GTK_ENTRY (_field->iwidget_to), 19);
						}
					else if (_field->type == GDAEX_QE_FIELD_TYPE_TIME)
						{
							gdaex_query_editor_entry_date_set_format (GDAEX_QUERY_EDITOR_ENTRY_DATE (_field->iwidget_to), "%H:%M:%S");
							gtk_entry_set_max_length (GTK_ENTRY (_field->iwidget_to), 8);
						}
				}
			else
				{
					_field->iwidget_to = GDAEX_QUERY_EDITOR_IWIDGET (gdaex_query_editor_entry_new ());
				}
		}

	g_hash_table_insert (table->fields, _field->name, _field);

	ret = TRUE;

	return ret;
}

/**
 * gdaex_query_editor_add_relation:
 * @table1: relation's left part.
 * @table2: relation's right part.
 * @fields_joined: couples of fields name, one from @table1 and one from @table2.
 *
 */
gboolean
gdaex_query_editor_add_relation_slist (GdaExQueryEditor *qe,
                                       const gchar *table1,
                                       const gchar *table2,
                                       GdaExQueryEditorJoinType join_type,
                                       GSList *fields_joined)
{
	GdaExQueryEditorPrivate *priv;

	GdaExQueryEditorTable *table;
	GdaExQueryEditorField *field1;
	GdaExQueryEditorField *field2;

	GdaExQueryEditorRelation *relation;

	gboolean create_relation;

	g_return_val_if_fail (GDAEX_IS_QUERY_EDITOR (qe), FALSE);
	g_return_val_if_fail (fields_joined != NULL, FALSE);

	priv = GDAEX_QUERY_EDITOR_GET_PRIVATE (qe);

	create_relation = FALSE;

	relation = g_new0 (GdaExQueryEditorRelation, 1);

	/* check if table exists */
	table = g_hash_table_lookup (priv->tables, table1);
	if (table == NULL)
		{
			g_warning (_("Table «%s» doesn't exists."), table1);
			return FALSE;
		}
	relation->table1 = table;

	table = g_hash_table_lookup (priv->tables, table2);
	if (table == NULL)
		{
			g_warning (_("Table «%s» doesn't exists."), table2);
			return FALSE;
		}
	relation->table2 = table;

	while (fields_joined != NULL)
		{
			field1 = g_hash_table_lookup (relation->table1->fields, (gchar *)fields_joined->data);
			if (field1 == NULL)
				{
					continue;
				}

			fields_joined = g_slist_next (fields_joined);
			if (fields_joined != NULL)
				{
					field2 = g_hash_table_lookup (relation->table2->fields, (gchar *)fields_joined->data);
					if (field2 == NULL)
						{
							continue;
						}
				}
			else
				{
					continue;
				}

			relation->fields1 = g_slist_append (relation->fields1, field1);
			relation->fields2 = g_slist_append (relation->fields2, field2);
			create_relation = TRUE;

			fields_joined = g_slist_next (fields_joined);
		}

	if (create_relation)
		{
			relation->join_type = join_type;
			priv->relations = g_slist_append (priv->relations, relation);
		}
	else
		{
			g_warning (_("Relation not created: no field added to the relation."));
			g_free (relation);
			return FALSE;
		}

	return TRUE;
}

/**
 * gdaex_query_editor_add_relation:
 * @table1: relation's left part.
 * @table2: relation's right part.
 * @...: couples of fields name, one from @table1 and one from @table2;
 *       must be terminated with a #NULL value.
 *
 */
gboolean
gdaex_query_editor_add_relation (GdaExQueryEditor *qe,
                                 const gchar *table1,
                                 const gchar *table2,
                                 GdaExQueryEditorJoinType join_type,
                                 ...)
{
	GdaExQueryEditorPrivate *priv;

	va_list fields;
	gchar *field_name1;
	gchar *field_name2;

	GSList *fields_joined;

	g_return_val_if_fail (GDAEX_IS_QUERY_EDITOR (qe), FALSE);

	priv = GDAEX_QUERY_EDITOR_GET_PRIVATE (qe);

	fields_joined = NULL;
	va_start (fields, join_type);

	while (TRUE)
		{
			field_name1 = va_arg (fields, gchar *);
			if (field_name1 == NULL)
				{
					break;
				}
			field_name2 = va_arg (fields, gchar *);
			if (field_name2 == NULL)
				{
					break;
				}

			fields_joined = g_slist_append (fields_joined, field_name1);
			fields_joined = g_slist_append (fields_joined, field_name2);
		}

	va_end (fields);

	return gdaex_query_editor_add_relation_slist (qe, table1, table2, join_type, fields_joined);
}

static GdaExQueryEditorFieldType
gdaex_query_editor_str_to_field_type (gchar *str)
{
	GdaExQueryEditorFieldType ret;

	g_return_val_if_fail (str != NULL, 0);

	ret = 0;

	if (g_strcmp0 (str, "text") == 0)
		{
			ret = GDAEX_QE_FIELD_TYPE_TEXT;
		}
	else if (g_strcmp0 (str, "integer") == 0)
		{
			ret = GDAEX_QE_FIELD_TYPE_INTEGER;
		}
	else if (g_strcmp0 (str, "double") == 0)
		{
			ret = GDAEX_QE_FIELD_TYPE_DOUBLE;
		}
	else if (g_strcmp0 (str, "date") == 0)
		{
			ret = GDAEX_QE_FIELD_TYPE_DATE;
		}
	else if (g_strcmp0 (str, "datetime") == 0)
		{
			ret = GDAEX_QE_FIELD_TYPE_DATETIME;
		}
	else if (g_strcmp0 (str, "time") == 0)
		{
			ret = GDAEX_QE_FIELD_TYPE_TIME;
		}

	return ret;
}

static guint
gdaex_query_editor_str_to_where_type (gchar *str)
{
	guint ret;

	gchar **types;

	guint i;
	guint l;

	g_return_val_if_fail (str != NULL, 0);

	ret = 0;

	types = g_strsplit (str, "|", 0);

	l = g_strv_length (types);
	for (i = 0; i < l; i++)
		{
			if (g_strcmp0 (types[i], "equal") == 0)
				{
					ret |= GDAEX_QE_WHERE_TYPE_EQUAL;
				}
			else if (g_strcmp0 (types[i], "starts") == 0)
				{
					ret |= GDAEX_QE_WHERE_TYPE_STARTS;
				}
			else if (g_strcmp0 (types[i], "contains") == 0)
				{
					ret |= GDAEX_QE_WHERE_TYPE_CONTAINS;
				}
			else if (g_strcmp0 (types[i], "ends") == 0)
				{
					ret |= GDAEX_QE_WHERE_TYPE_ENDS;
				}
			else if (g_strcmp0 (types[i], "istarts") == 0)
				{
					ret |= GDAEX_QE_WHERE_TYPE_ISTARTS;
				}
			else if (g_strcmp0 (types[i], "icontains") == 0)
				{
					ret |= GDAEX_QE_WHERE_TYPE_ICONTAINS;
				}
			else if (g_strcmp0 (types[i], "iends") == 0)
				{
					ret |= GDAEX_QE_WHERE_TYPE_IENDS;
				}
			else if (g_strcmp0 (types[i], "great") == 0)
				{
					ret |= GDAEX_QE_WHERE_TYPE_GREAT;
				}
			else if (g_strcmp0 (types[i], "great_equal") == 0)
				{
					ret |= GDAEX_QE_WHERE_TYPE_GREAT_EQUAL;
				}
			else if (g_strcmp0 (types[i], "less") == 0)
				{
					ret |= GDAEX_QE_WHERE_TYPE_LESS;
				}
			else if (g_strcmp0 (types[i], "less_equal") == 0)
				{
					ret |= GDAEX_QE_WHERE_TYPE_LESS_EQUAL;
				}
			else if (g_strcmp0 (types[i], "between") == 0)
				{
					ret |= GDAEX_QE_WHERE_TYPE_BETWEEN;
				}
			else if (g_strcmp0 (types[i], "is_null") == 0)
				{
					ret |= GDAEX_QE_WHERE_TYPE_IS_NULL;
				}
			else if (g_strcmp0 (types[i], "string") == 0)
				{
					ret |= GDAEX_QE_WHERE_TYPE_STRING;
				}
			else if (g_strcmp0 (types[i], "number") == 0)
				{
					ret |= GDAEX_QE_WHERE_TYPE_NUMBER;
				}
			else if (g_strcmp0 (types[i], "datetime") == 0)
				{
					ret |= GDAEX_QE_WHERE_TYPE_DATETIME;
				}
		}

	g_strfreev (types);

	return ret;
}

static GdaExQueryEditorJoinType
gdaex_query_editor_str_to_join_type (gchar *str)
{
	GdaExQueryEditorJoinType ret;

	g_return_val_if_fail (str != NULL, 0);

	ret = 0;

	if (g_strcmp0 (str, "inner") == 0)
		{
			ret = GDAEX_QE_JOIN_TYPE_INNER;
		}
	else if (g_strcmp0 (str, "left") == 0)
		{
			ret = GDAEX_QE_JOIN_TYPE_LEFT;
		}

	return ret;
}

typedef GdaExQueryEditorIWidget *(* IWidgetConstructorFunc) (void);
typedef gboolean (* IWidgetXmlParsingFunc) (GdaExQueryEditorIWidget *, xmlNodePtr);

static GdaExQueryEditorIWidget
*gdaex_query_editor_iwidget_construct (GdaExQueryEditor *qe,
									   const gchar *type,
									   xmlNode *xnode)
{
	GdaExQueryEditorPrivate *priv;

	guint i;

	IWidgetConstructorFunc iwidget_constructor;
	IWidgetXmlParsingFunc iwidget_xml_parsing;

	GdaExQueryEditorIWidget *iwidget;

	priv = GDAEX_QUERY_EDITOR_GET_PRIVATE (qe);

	iwidget = NULL;
	for (i = 0; i < priv->ar_modules->len; i++)
		{
			if (g_module_symbol ((GModule *)g_ptr_array_index (priv->ar_modules, i),
								 g_strconcat (type, "_new", NULL),
								 (gpointer *)&iwidget_constructor))
				{
					if (iwidget_constructor != NULL)
						{
							iwidget = iwidget_constructor ();
							if (iwidget != NULL)
								{
									if (g_module_symbol ((GModule *)g_ptr_array_index (priv->ar_modules, i),
														 g_strconcat (type, "_xml_parsing", NULL),
														 (gpointer *)&iwidget_xml_parsing))
										{
											if (iwidget_xml_parsing != NULL)
												{
													iwidget_xml_parsing (iwidget, xnode);
												}
										}
								}
							break;
						}
				}
		}

	return iwidget;
}

void
gdaex_query_editor_load_tables_from_xml (GdaExQueryEditor *qe,
                                         xmlNode *root,
                                         gboolean clean)
{
	GdaExQueryEditorPrivate *priv;
	GdaExQueryEditorClass *klass;

	xmlDoc *xdoc;
	xmlXPathContextPtr xpcontext;
	xmlXPathObjectPtr xpresult;
	xmlNodeSetPtr xnodeset;

	xmlNode *xnode;
	xmlNode *xtable;
	xmlNode *xfield;
	xmlNode *xrelation;
	xmlNode *xdecode;
	xmlNode *xfields_joined;
	xmlNode *cur;

	guint i;

	gchar *table_name;
	gchar *name;
	gchar *name_visible;

	GdaExQueryEditorField *field;

	gchar *table_left;
	gchar *table_right;
	GdaExQueryEditorJoinType join_type;
	GSList *fields_joined;

	g_return_if_fail (GDAEX_IS_QUERY_EDITOR (qe));
	g_return_if_fail (root != NULL);
	g_return_if_fail (xmlStrcmp (root->name, "gdaex_query_editor") == 0);

	klass = GDAEX_QUERY_EDITOR_GET_CLASS (qe);

	priv = GDAEX_QUERY_EDITOR_GET_PRIVATE (qe);

	if (clean)
		{
			gdaex_query_editor_clean (qe);
			gdaex_query_editor_clean_choices (qe);
		}

	table_name = NULL;
	name = NULL;
	name_visible = NULL;
	join_type = 0;
	fields_joined = NULL;

	xdoc = xmlNewDoc ("");
	xmlDocSetRootElement (xdoc, root);

	/* search tables node */
	xpcontext = xmlXPathNewContext (xdoc);
	xpcontext->node = xmlDocGetRootElement (xdoc);
	xpresult = xmlXPathEvalExpression ((const xmlChar *)"child::tables", xpcontext);
	if (xpresult != NULL && !xmlXPathNodeSetIsEmpty (xpresult->nodesetval))
		{
			xnodeset = xpresult->nodesetval;
			for (i = 0; i < xnodeset->nodeNr; i++)
				{
					xtable = xnodeset->nodeTab[i]->children;
					while (xtable != NULL)
						{
							if (xmlStrcmp (xtable->name, "table") == 0)
								{
									if (table_name != NULL)
										{
											g_free (table_name);
											table_name = NULL;
										}
									if (name_visible != NULL)
										{
											g_free (name_visible);
											name_visible = NULL;
										}

									cur = xtable->children;
									while (cur != NULL)
										{
											if (xmlStrcmp (cur->name, "name") == 0)
												{
													table_name = xmlNodeGetContent (cur);
													if (table_name != NULL)
														{
															table_name = g_strstrip (g_strdup (table_name));
														}
												}
											else if (xmlStrcmp (cur->name, "name_visible") == 0)
												{
													name_visible = xmlNodeGetContent (cur);
													if (name_visible != NULL)
														{
															name_visible = g_strstrip (g_strdup (name_visible));
														}
												}

											if (table_name != NULL)
												{
													gdaex_query_editor_add_table (qe, table_name, name_visible);
												}

											cur = cur->next;
										}
								}

							xtable = xtable->next;
						}
				}
		}
	else
		{
			g_warning (_("No table's definitions on xml file."));
		}

	/* search fields node */
	if (table_name != NULL)
		{
			g_free (table_name);
			table_name = NULL;
		}

	xpcontext = xmlXPathNewContext (xdoc);
	xpcontext->node = xmlDocGetRootElement (xdoc);
	xpresult = xmlXPathEvalExpression ((const xmlChar *)"child::fields", xpcontext);
	if (xpresult != NULL && !xmlXPathNodeSetIsEmpty (xpresult->nodesetval))
		{
			xnodeset = xpresult->nodesetval;
			for (i = 0; i < xnodeset->nodeNr; i++)
				{
					table_name = xmlGetProp (xnodeset->nodeTab[i], "table");

					xfield = xnodeset->nodeTab[i]->children;
					while (xfield != NULL)
						{
							if (xmlStrcmp (xfield->name, "field") == 0)
								{
									field = g_new0 (GdaExQueryEditorField, 1);
									field->for_show = TRUE;
									field->for_where = TRUE;
									field->for_order = TRUE;

									cur = xfield->children;
									while (cur != NULL)
										{
											if (xmlStrcmp (cur->name, "name") == 0)
												{
													field->name = xmlNodeGetContent (cur);
													if (field->name != NULL)
														{
															field->name = g_strstrip (g_strdup (field->name));
														}
												}
											else if (xmlStrcmp (cur->name, "name_visible") == 0)
												{
													field->name_visible = xmlNodeGetContent (cur);
													if (field->name_visible != NULL)
														{
															field->name_visible = g_strstrip (g_strdup (field->name_visible));
														}
												}
											else if (xmlStrcmp (cur->name, "description") == 0)
												{
													field->description = xmlNodeGetContent (cur);
												}
											else if (xmlStrcmp (cur->name, "alias") == 0)
												{
													field->alias = xmlNodeGetContent (cur);
												}
											else if (xmlStrcmp (cur->name, "type") == 0)
												{
													field->type = gdaex_query_editor_str_to_field_type (xmlNodeGetContent (cur));
												}
											else if (xmlStrcmp (cur->name, "for_show") == 0)
												{
													field->for_show = zak_utils_string_to_boolean (xmlNodeGetContent (cur));
												}
											else if (xmlStrcmp (cur->name, "always_showed") == 0)
												{
													field->always_showed = zak_utils_string_to_boolean (xmlNodeGetContent (cur));
												}
											else if (xmlStrcmp (cur->name, "for_where") == 0)
												{
													field->for_where = zak_utils_string_to_boolean (xmlNodeGetContent (cur));
												}
											else if (xmlStrcmp (cur->name, "available_where_type") == 0)
												{
													field->available_where_type = gdaex_query_editor_str_to_where_type (xmlNodeGetContent (cur));
												}
											else if (xmlStrcmp (cur->name, "for_order") == 0)
												{
													field->for_order = zak_utils_string_to_boolean (xmlNodeGetContent (cur));
												}
											else if (xmlStrcmp (cur->name, "decode") == 0)
												{
													xdecode = cur->children;
													while (xdecode != NULL)
														{
															if (xmlStrcmp (xdecode->name, "table_name") == 0)
																{
																	field->decode_table2 = xmlNodeGetContent (xdecode);
																}
															else if (xmlStrcmp (xdecode->name, "join_type") == 0)
																{
																	field->decode_join_type = gdaex_query_editor_str_to_join_type (xmlNodeGetContent (xdecode));
																}
															else if (xmlStrcmp (xdecode->name, "field_name_to_join") == 0)
																{
																	field->decode_field2 = xmlNodeGetContent (xdecode);
																}
															else if (xmlStrcmp (xdecode->name, "field_name_to_show") == 0)
																{
																	field->decode_field_to_show = xmlNodeGetContent (xdecode);
																}
															else if (xmlStrcmp (xdecode->name, "alias") == 0)
																{
																	field->decode_field_alias = xmlNodeGetContent (xdecode);
																}

															xdecode = xdecode->next;
														}
												}
											else if (xmlStrcmp (cur->name, "widget") == 0
													 || xmlStrcmp (cur->name, "widget_from") == 0
													 || xmlStrcmp (cur->name, "widget_to") == 0)
												{
													gchar *type;

													type = xmlGetProp (cur, (const xmlChar *)"type");

													if (xmlStrcmp (cur->name, "widget") == 0
														|| xmlStrcmp (cur->name, "widget_from") == 0)
														{
															field->iwidget_from = gdaex_query_editor_iwidget_construct (qe,
																														type,
																														cur);
															if (field->iwidget_from == NULL)
																{
																	g_warning (_("Unknown iwidget_from type «%s»."), type);
																}
															else
																{
																	g_signal_emit (qe, klass->iwidget_init_signal_id,
																				   0,
																				   field->iwidget_from,
																				   table_name,
																				   field->name,
																				   TRUE);
																}
														}
													if (xmlStrcmp (cur->name, "widget") == 0
														|| xmlStrcmp (cur->name, "widget_to") == 0)
														{
															field->iwidget_to = gdaex_query_editor_iwidget_construct (qe,
																													  type,
																													  cur);
															if (field->iwidget_to == NULL)
																{
																	g_warning (_("Unknown iwidget_to type «%s»."), type);
																}
															else
																{
																	g_signal_emit (qe, klass->iwidget_init_signal_id,
																				   0,
																				   field->iwidget_from,
																				   table_name,
																				   field->name,
																				   FALSE);
																}
														}
												}

											cur = cur->next;
										}

									gdaex_query_editor_table_add_field (qe, table_name, *field);
									g_free (field);
								}

							xfield = xfield->next;
						}

					if (table_name != NULL)
						{
							g_free (table_name);
							table_name = NULL;
						}
				}
		}
	else
		{
			g_warning (_("No field's definitions on xml file."));
		}

	/* search relations node */
	xpcontext = xmlXPathNewContext (xdoc);
	xpcontext->node = xmlDocGetRootElement (xdoc);
	xpresult = xmlXPathEvalExpression ((const xmlChar *)"child::relations", xpcontext);
	if (xpresult != NULL && !xmlXPathNodeSetIsEmpty (xpresult->nodesetval))
		{
			xnodeset = xpresult->nodesetval;
			for (i = 0; i < xnodeset->nodeNr; i++)
				{
					xrelation = xnodeset->nodeTab[i]->children;
					while (xrelation != NULL)
						{
							if (xmlStrcmp (xrelation->name, "relation") == 0)
								{
									cur = xrelation->children;
									while (cur != NULL)
										{
											if (xmlStrcmp (cur->name, "table_left") == 0)
												{
													table_left = xmlNodeGetContent (cur);
													if (table_left != NULL)
														{
															table_left = g_strstrip (g_strdup (table_left));
														}
												}
											else if (xmlStrcmp (cur->name, "table_right") == 0)
												{
													table_right = xmlNodeGetContent (cur);
													if (table_right != NULL)
														{
															table_right = g_strstrip (g_strdup (table_right));
														}
												}
											else if (xmlStrcmp (cur->name, "join_type") == 0)
												{
													join_type = gdaex_query_editor_str_to_join_type (xmlNodeGetContent (cur));
												}
											else if (xmlStrcmp (cur->name, "fields_joined") == 0)
												{
													xfields_joined = cur->children;
													while (xfields_joined != NULL)
														{
															if (xmlStrcmp (xfields_joined->name, "field_left") == 0)
																{
																	name = xmlNodeGetContent (xfields_joined);
																	if (name != NULL)
																		{
																			name = g_strstrip (g_strdup (name));
																			fields_joined = g_slist_append (fields_joined, name);
																		}
																}
															else if (xmlStrcmp (xfields_joined->name, "field_right") == 0)
																{
																	name = xmlNodeGetContent (xfields_joined);
																	if (name != NULL)
																		{
																			name = g_strstrip (g_strdup (name));
																			fields_joined = g_slist_append (fields_joined, name);
																		}
																}

															xfields_joined = xfields_joined->next;
														}
												}

											cur = cur->next;
										}

									if (table_left != NULL
									    && table_right != NULL
									    && join_type > 0
									    && fields_joined != NULL)
										{
											gdaex_query_editor_add_relation_slist (qe, table_left, table_right, join_type, fields_joined);
										}
									if (table_left != NULL)
										{
											g_free (table_left);
											table_left = NULL;
										}
									if (table_right != NULL)
										{
											g_free (table_right);
											table_right = NULL;
										}
									if (fields_joined != NULL)
										{
											g_slist_free (fields_joined);
											fields_joined = NULL;
										}
									join_type = 0;
								}

							xrelation = xrelation->next;
						}
				}
		}
}

void
gdaex_query_editor_load_tables_from_file (GdaExQueryEditor *qe,
                                          const gchar *filename,
                                          gboolean clean)
{
	xmlDoc *xdoc;
	xmlNode *xroot;

	g_return_if_fail (filename != NULL);

	xdoc = xmlParseFile (filename);
	if (xdoc != NULL)
		{
			xroot = xmlDocGetRootElement (xdoc);
			if (xroot != NULL)
				{
					gdaex_query_editor_load_tables_from_xml (qe, xroot, clean);
				}
		}
}

void
gdaex_query_editor_clean_choices (GdaExQueryEditor *qe)
{
	GdaExQueryEditorPrivate *priv;

	g_return_if_fail (GDAEX_IS_QUERY_EDITOR (qe));

	priv = GDAEX_QUERY_EDITOR_GET_PRIVATE (qe);

	gtk_list_store_clear (priv->lstore_show);
	gtk_tree_store_clear (priv->tstore_where);
	gtk_list_store_clear (priv->lstore_order);

	gdaex_query_editor_refill_always_show (qe);

	gtk_button_clicked (GTK_BUTTON (gtk_builder_get_object (priv->gtkbuilder, "button16")));
}

static GDate
*gdaex_query_editor_get_gdate_from_sql (const gchar *sql)
{
	GDate *ret;

	gint year;
	gint month;
	gint day;

	ret = NULL;

	year = 0;
	month = 0;
	day = 0;

	if (strlen (sql + 1) >= 10)
		{
			year = strtol (g_strndup (sql + 1, 4), NULL, 10);
			month = strtol (g_strndup (sql + 6, 2), NULL, 10);
			day = strtol (g_strndup (sql + 9, 2), NULL, 10);

			ret = g_date_new_dmy (day,
			                      month,
			                      year);
		}

	return ret;
}

static GdaTimestamp
*gdaex_query_editor_get_gdatimestamp_from_sql (const gchar *sql)
{
	GdaTimestamp *ret;

	ret = NULL;

	if (strlen (sql + 1) >= 10)
		{
			ret = g_new0 (GdaTimestamp, 1);

			ret->year = strtol (g_strndup (sql + 1, 4), NULL, 10);
			ret->month = strtol (g_strndup (sql + 6, 2), NULL, 10);
			ret->day = strtol (g_strndup (sql + 9, 2), NULL, 10);

			if (strlen (sql + 1) >= 12)
				{
					ret->hour = strtol (g_strndup (sql + 12, 2), NULL, 10);
					ret->minute = strtol (g_strndup (sql + 15, 2), NULL, 10);
					if (strlen (sql + 1) >= 16)
						{
							ret->second = strtol (g_strndup (sql + 18, 2), NULL, 10);
						}
				}
		}

	return ret;
}

GdaExQueryEditorRelation
*gdaex_query_editor_get_table_relation (GdaExQueryEditor *qe,
                                        const gchar *table_name)
{
	GdaExQueryEditorRelation *ret;

	GdaExQueryEditorPrivate *priv;

	GSList *relations;

	priv = GDAEX_QUERY_EDITOR_GET_PRIVATE (qe);

	if (priv->relations == NULL)
		{
			return NULL;
		}

	ret = NULL;

	relations = priv->relations;
	while (relations != NULL)
		{
			if (g_strcmp0 (((GdaExQueryEditorRelation *)relations->data)->table2->name, table_name) == 0)
				{
					ret = (GdaExQueryEditorRelation *)relations->data;
					break;
				}

			relations = g_slist_next (relations);
		}

	return ret;
}

static void
gdaex_query_editor_add_table_relation_to_gdasqlbuilder (GdaExQueryEditor *qe,
                                                        GdaSqlBuilder *sqlbuilder,
                                                        const gchar *table_name)
{
	GdaExQueryEditorRelation *relation;

	GSList *fields1;
	GSList *fields2;

	guint id_target1;
	guint id_target2;
	guint id_join1;
	guint id_join2;
	guint join_cond;

	GdaExQueryEditorPrivate *priv;

	priv = GDAEX_QUERY_EDITOR_GET_PRIVATE (qe);

	/* check if table is part of a relation */
	relation = gdaex_query_editor_get_table_relation (qe, table_name);
	if (relation != NULL)
		{
			id_target1 = gda_sql_builder_select_add_target_id (sqlbuilder,
			                                                   gda_sql_builder_add_id (sqlbuilder, relation->table1->name),
			                                                   NULL);
			id_target2 = gda_sql_builder_select_add_target_id (sqlbuilder,
			                                                   gda_sql_builder_add_id (sqlbuilder, relation->table2->name),
			                                                   NULL);

			/* for each fields couple */
			fields1 = relation->fields1;
			fields2 = relation->fields2;
			while (fields1 != NULL)
				{
					id_join1 = gda_sql_builder_add_id (sqlbuilder, g_strconcat (relation->table1->name, ".", ((GdaExQueryEditorField *)fields1->data)->name, NULL));
					id_join2 = gda_sql_builder_add_id (sqlbuilder, g_strconcat (relation->table2->name, ".", ((GdaExQueryEditorField *)fields2->data)->name, NULL));

					join_cond = gda_sql_builder_add_cond (sqlbuilder, GDA_SQL_OPERATOR_TYPE_EQ,
					                                      id_join1, id_join2, 0);

					gda_sql_builder_select_join_targets (sqlbuilder, id_target1, id_target2,
					                                     relation->join_type == GDAEX_QE_JOIN_TYPE_LEFT ? GDA_SQL_SELECT_JOIN_LEFT : GDA_SQL_SELECT_JOIN_INNER,
					                                     join_cond);

					fields1 = g_slist_next (fields1);
					fields2 = g_slist_next (fields2);
				}
		}
}

static guint
gdaex_query_editor_sql_where (GdaExQueryEditor *qe,
                              GdaSqlBuilder *sqlbuilder,
                              GtkTreeIter *iter,
                              GtkTreeIter *iter_parent,
                              guint id_cond_parent)
{
	GdaExQueryEditorPrivate *priv;

	guint id_ret;

	gchar *table_name;
	gchar *field_name;
	gchar *field_name_with_table;

	GdaExQueryEditorTable *table;
	GdaExQueryEditorField *field;

	guint link_type;
	GdaSqlOperatorType link_op;
	gboolean not;
	guint where_type;
	GdaSqlOperatorType where_op;
	gchar *from_str;
	gchar *to_str;
	GDate *from_date;
	GDateTime *from_datetime;

	guint id_field;
	guint id_value1;
	guint id_value2;
	guint id_cond_iter;

	gboolean case_insensitive;

	gboolean is_group;

	GtkTreeIter children;

	priv = GDAEX_QUERY_EDITOR_GET_PRIVATE (qe);

	id_ret = 0;

	do
		{
			id_value2 = 0;

			gtk_tree_model_get (GTK_TREE_MODEL (priv->tstore_where), iter,
			                    COL_WHERE_LINK_TYPE, &link_type,
			                    COL_WHERE_TABLE_NAME, &table_name,
			                    COL_WHERE_NAME, &field_name,
			                    COL_WHERE_CONDITION_NOT, &not,
			                    COL_WHERE_CONDITION_TYPE, &where_type,
			                    COL_WHERE_CONDITION_FROM_SQL, &from_str,
			                    COL_WHERE_CONDITION_TO_SQL, &to_str,
			                    -1);

			is_group = (g_strcmp0 (table_name, GROUP) == 0);

			switch (link_type)
				{
					case GDAEX_QE_LINK_TYPE_AND:
						link_op = GDA_SQL_OPERATOR_TYPE_AND;
						break;

					case GDAEX_QE_LINK_TYPE_OR:
						link_op = GDA_SQL_OPERATOR_TYPE_OR;
						break;
				}

			if (!is_group)
				{
					case_insensitive = (where_type == GDAEX_QE_WHERE_TYPE_ISTARTS
					                    || where_type == GDAEX_QE_WHERE_TYPE_ICONTAINS
					                    || where_type == GDAEX_QE_WHERE_TYPE_IENDS);

					if (from_str == NULL)
						{
							from_str = g_strdup ("");
						}
					else
						{
							if (case_insensitive)
								{
									from_str = g_utf8_strdown (from_str, -1);
								}
						}
					if (to_str != NULL && g_strcmp0 (g_strstrip (to_str), "") != 0)
						{
							if (case_insensitive)
								{
									to_str = g_utf8_strdown (to_str, -1);
								}
						}
					else
						{
							to_str = NULL;
						}

					switch (where_type)
						{
							case GDAEX_QE_WHERE_TYPE_STARTS:
							case GDAEX_QE_WHERE_TYPE_ISTARTS:
								from_str = g_strconcat (from_str, "%", NULL);
								if (to_str != NULL)
									{
										to_str = g_strconcat (to_str, "%", NULL);
									}
								break;

							case GDAEX_QE_WHERE_TYPE_CONTAINS:
							case GDAEX_QE_WHERE_TYPE_ICONTAINS:
								from_str = g_strconcat ("%", from_str, "%", NULL);
								if (to_str != NULL)
									{
										to_str = g_strconcat ("%", to_str, "%", NULL);
									}
								break;

							case GDAEX_QE_WHERE_TYPE_ENDS:
							case GDAEX_QE_WHERE_TYPE_IENDS:
								from_str = g_strconcat ("%", from_str, NULL);
								if (to_str != NULL)
									{
										to_str = g_strconcat ("%", to_str, NULL);
									}
								break;
						}

					table = g_hash_table_lookup (priv->tables, table_name);
					field = g_hash_table_lookup (table->fields, field_name);

					gdaex_query_editor_add_table_relation_to_gdasqlbuilder (qe, sqlbuilder, table_name);

					field_name_with_table = g_strconcat (case_insensitive ? "LOWER(" : "",
					                                     table->name, ".", field->name,
					                                     case_insensitive ? ")" : "",
					                                     NULL);
					id_field = gda_sql_builder_add_id (sqlbuilder, field_name_with_table);
					g_free (field_name_with_table);

					if (where_type == GDAEX_QE_WHERE_TYPE_IS_NULL)
						{
							id_value1 = 0;
						}
					else
						{
							switch (field->type)
								{
									case GDAEX_QE_FIELD_TYPE_TEXT:
										id_value1 = gda_sql_builder_add_expr (sqlbuilder, NULL, G_TYPE_STRING, from_str);
										if (to_str != NULL)
											{
												id_value2 = gda_sql_builder_add_expr (sqlbuilder, NULL, G_TYPE_STRING, to_str);
											}
										break;

									case GDAEX_QE_FIELD_TYPE_INTEGER:
										id_value1 = gda_sql_builder_add_expr (sqlbuilder, NULL, G_TYPE_INT, strtol (from_str, NULL, 10));
										if (to_str != NULL)
											{
												id_value2 = gda_sql_builder_add_expr (sqlbuilder, NULL, G_TYPE_INT, strtol (to_str, NULL, 10));
											}
										break;

									case GDAEX_QE_FIELD_TYPE_DOUBLE:
										id_value1 = gda_sql_builder_add_expr (sqlbuilder, NULL, G_TYPE_DOUBLE, g_strtod (from_str, NULL));
										if (to_str != NULL)
											{
												id_value2 = gda_sql_builder_add_expr (sqlbuilder, NULL, G_TYPE_DOUBLE, g_strtod (to_str, NULL));
											}
										break;

									case GDAEX_QE_FIELD_TYPE_DATE:
										id_value1 = gda_sql_builder_add_expr (sqlbuilder, NULL, G_TYPE_DATE, gdaex_query_editor_get_gdate_from_sql (from_str));
										if (to_str != NULL)
											{
												id_value2 = gda_sql_builder_add_expr (sqlbuilder, NULL, G_TYPE_DATE, gdaex_query_editor_get_gdate_from_sql (to_str));
											}
										break;

									case GDAEX_QE_FIELD_TYPE_DATETIME:
										id_value1 = gda_sql_builder_add_expr (sqlbuilder, NULL, GDA_TYPE_TIMESTAMP, gdaex_query_editor_get_gdatimestamp_from_sql (from_str));
										if (to_str != NULL)
											{
												id_value2 = gda_sql_builder_add_expr (sqlbuilder, NULL, GDA_TYPE_TIMESTAMP, gdaex_query_editor_get_gdatimestamp_from_sql (to_str));
											}
										break;

									case GDAEX_QE_FIELD_TYPE_TIME:
										/* TODO
										type = G_TYPE_DATE_TIME; */
										break;
								};
						}

					switch (where_type)
						{
							case GDAEX_QE_WHERE_TYPE_EQUAL:
								where_op = GDA_SQL_OPERATOR_TYPE_EQ;
								break;

							case GDAEX_QE_WHERE_TYPE_STARTS:
							case GDAEX_QE_WHERE_TYPE_CONTAINS:
							case GDAEX_QE_WHERE_TYPE_ENDS:
							case GDAEX_QE_WHERE_TYPE_ISTARTS:
							case GDAEX_QE_WHERE_TYPE_ICONTAINS:
							case GDAEX_QE_WHERE_TYPE_IENDS:
								where_op = GDA_SQL_OPERATOR_TYPE_LIKE;
								break;

							case GDAEX_QE_WHERE_TYPE_GREAT:
								where_op = GDA_SQL_OPERATOR_TYPE_GT;
								break;

							case GDAEX_QE_WHERE_TYPE_GREAT_EQUAL:
								where_op = GDA_SQL_OPERATOR_TYPE_GEQ;
								break;

							case GDAEX_QE_WHERE_TYPE_LESS:
								where_op = GDA_SQL_OPERATOR_TYPE_LT;
								break;

							case GDAEX_QE_WHERE_TYPE_LESS_EQUAL:
								where_op = GDA_SQL_OPERATOR_TYPE_LEQ;
								break;

							case GDAEX_QE_WHERE_TYPE_BETWEEN:
								where_op = GDA_SQL_OPERATOR_TYPE_BETWEEN;
								break;

							case GDAEX_QE_WHERE_TYPE_IS_NULL:
								if (not)
									{
										where_op = GDA_SQL_OPERATOR_TYPE_ISNOTNULL;
									}
								else
									{
										where_op = GDA_SQL_OPERATOR_TYPE_ISNULL;
									}
								break;

							default:
								g_warning (_("Where type «%d» not valid."), where_type);
								continue;
						}

					id_cond_iter = gda_sql_builder_add_cond (sqlbuilder, where_op, id_field, id_value1, id_value2);
					if (id_cond_iter == 0)
						{
							g_warning (_("Unable to create GdaSqlBuilder condition."));
							continue;
						}
					else
						{
							if (not && where_type != GDAEX_QE_WHERE_TYPE_IS_NULL)
								{
									id_cond_iter = gda_sql_builder_add_cond (sqlbuilder, GDA_SQL_OPERATOR_TYPE_NOT, id_cond_iter, 0, 0);
								}
							if (id_ret == 0)
								{
									id_ret = id_cond_iter;
								}
							else
								{
									id_ret = gda_sql_builder_add_cond (sqlbuilder, link_op, id_ret, id_cond_iter, 0);
								}
						}
				}
			else
				{
					if (gtk_tree_model_iter_children (GTK_TREE_MODEL (priv->tstore_where), &children, iter))
						{
							id_cond_iter = gdaex_query_editor_sql_where (qe, sqlbuilder, &children, iter, 0);
							if (id_cond_iter != 0)
								{
									if (not)
										{
											id_cond_iter = gda_sql_builder_add_cond (sqlbuilder, GDA_SQL_OPERATOR_TYPE_NOT, id_cond_iter, 0, 0);
										}
									if (id_ret == 0)
										{
											id_ret = id_cond_iter;
										}
									else
										{
											id_ret = gda_sql_builder_add_cond (sqlbuilder, link_op, id_ret, id_cond_iter, 0);
										}
								}
						}
				}
		} while (gtk_tree_model_iter_next (GTK_TREE_MODEL (priv->tstore_where), iter));

	return id_ret;
}

GdaSqlBuilder
*gdaex_query_editor_get_sql_as_gdasqlbuilder (GdaExQueryEditor *qe)
{
	GdaExQueryEditorPrivate *priv;

	GdaSqlBuilder *sqlbuilder;

	GtkTreeIter iter;

	gchar *table_name;
	gchar *field_name;
	gchar *alias;
	gchar *asc_desc;

	GdaExQueryEditorTable *table;
	GdaExQueryEditorField *field;

	g_return_val_if_fail (GDAEX_IS_QUERY_EDITOR (qe), NULL);

	priv = GDAEX_QUERY_EDITOR_GET_PRIVATE (qe);

	sqlbuilder = gda_sql_builder_new (GDA_SQL_STATEMENT_SELECT);

	/* SHOW */
	if (gtk_tree_model_get_iter_first (GTK_TREE_MODEL (priv->lstore_show), &iter))
		{
			guint id_target1;
			guint id_target2;
			guint id_join1;
			guint id_join2;
			guint join_cond;

			do
				{
					gtk_tree_model_get (GTK_TREE_MODEL (priv->lstore_show), &iter,
					                    COL_SHOW_TABLE_NAME, &table_name,
					                    COL_SHOW_NAME, &field_name,
					                    COL_SHOW_ALIAS, &alias,
					                    -1);

					table = g_hash_table_lookup (priv->tables, table_name);
					field = g_hash_table_lookup (table->fields, field_name);

					gdaex_query_editor_add_table_relation_to_gdasqlbuilder (qe, sqlbuilder, table_name);

					if (field->decode_table2 != NULL)
						{
							id_target1 = gda_sql_builder_select_add_target_id (sqlbuilder,
							                                                   gda_sql_builder_add_id (sqlbuilder, table->name),
							                                                   NULL);
							id_target2 = gda_sql_builder_select_add_target_id (sqlbuilder,
							                                                   gda_sql_builder_add_id (sqlbuilder, field->decode_table2),
							                                                   NULL);

							id_join1 = gda_sql_builder_add_id (sqlbuilder, g_strconcat (field->table_name, ".", field->name, NULL));
							id_join2 = gda_sql_builder_add_id (sqlbuilder, g_strconcat (field->decode_table2, ".", field->decode_field2, NULL));

							join_cond = gda_sql_builder_add_cond (sqlbuilder, GDA_SQL_OPERATOR_TYPE_EQ,
							                                      id_join1, id_join2, 0);

							gda_sql_builder_select_join_targets (sqlbuilder, id_target1, id_target2,
							                                     field->decode_join_type == GDAEX_QE_JOIN_TYPE_LEFT ? GDA_SQL_SELECT_JOIN_LEFT : GDA_SQL_SELECT_JOIN_INNER,
							                                     join_cond);
							gda_sql_builder_select_add_field (sqlbuilder, field->decode_field_to_show,
							                                  field->decode_table2,
							                                  alias != NULL && g_strcmp0 (alias, "") != 0 ? alias : field->decode_field_alias);
						}
					else
						{
							gda_sql_builder_select_add_field (sqlbuilder, field->name, table->name,
							                                  alias != NULL && g_strcmp0 (alias, "") != 0 ? alias : field->alias);
							gda_sql_builder_select_add_target_id (sqlbuilder,
							                                      gda_sql_builder_add_id (sqlbuilder, table->name),
							                                      NULL);
						}
				} while (gtk_tree_model_iter_next (GTK_TREE_MODEL (priv->lstore_show), &iter));
		}

	/* WHERE */
	if (gtk_tree_model_get_iter_first (GTK_TREE_MODEL (priv->tstore_where), &iter))
		{
			guint id_cond;

			id_cond = gdaex_query_editor_sql_where (qe, sqlbuilder, &iter, NULL, 0);

			if (id_cond != 0)
				{
					gda_sql_builder_set_where (sqlbuilder, id_cond);
				}
		}

	/* ORDER */
	if (gtk_tree_model_get_iter_first (GTK_TREE_MODEL (priv->lstore_order), &iter))
		{
			do
				{
					gtk_tree_model_get (GTK_TREE_MODEL (priv->lstore_order), &iter,
					                    COL_ORDER_TABLE_NAME, &table_name,
					                    COL_ORDER_NAME, &field_name,
					                    COL_ORDER_ORDER, &asc_desc,
					                    -1);

					table = g_hash_table_lookup (priv->tables, table_name);
					field = g_hash_table_lookup (table->fields, field_name);

					gdaex_query_editor_add_table_relation_to_gdasqlbuilder (qe, sqlbuilder, table_name);

					gda_sql_builder_select_order_by (sqlbuilder,
					                                 gda_sql_builder_add_id (sqlbuilder, field->name),
					                                 (g_strcmp0 (asc_desc, "ASC") == 0),
					                                 NULL);
				} while (gtk_tree_model_iter_next (GTK_TREE_MODEL (priv->lstore_order), &iter));
		}

	return sqlbuilder;
}

const gchar
*gdaex_query_editor_get_sql (GdaExQueryEditor *qe)
{
	const gchar *ret;

	GdaExQueryEditorPrivate *priv;

	GdaSqlBuilder *sqlbuilder;
	GdaStatement *stmt;
	GError *error;

	ret = NULL;

	sqlbuilder = gdaex_query_editor_get_sql_as_gdasqlbuilder (qe);
	if (sqlbuilder == NULL)
		{
			return ret;
		}

	priv = GDAEX_QUERY_EDITOR_GET_PRIVATE (qe);

	error = NULL;
	stmt = gda_sql_builder_get_statement (sqlbuilder, &error);
	if (stmt == NULL || error != NULL)
		{
			g_object_unref (sqlbuilder);
			g_warning (_("Unable to create GdaStatement: %s."),
			           error != NULL && error->message != NULL ? error->message : _("no details"));
			return NULL;
		}

	error = NULL;
	ret = gda_statement_to_sql_extended (stmt,
	                                     (GdaConnection *)gdaex_get_gdaconnection (priv->gdaex),
	                                     NULL, 0, NULL, &error);
	if (error != NULL)
		{
			ret = NULL;
			g_warning (_("Unable to create sql: %s."),
			           error->message != NULL ? error->message : _("no details"));
		}

	g_object_unref (sqlbuilder);

	return ret;
}

const gchar
*gdaex_query_editor_get_sql_select (GdaExQueryEditor *qe)
{
	gchar *ret;
	const gchar *sql;

	gchar *start;
	gchar *end;

	ret = NULL;

	sql = gdaex_query_editor_get_sql (qe);
	if (sql == NULL)
		{
			return ret;
		}

	start = g_strstr_len (sql, -1, "SELECT");
	if (start == NULL)
		{
			return ret;
		}

	end = g_strstr_len (sql, -1, "FROM");
	if (end == NULL)
		{
			return ret;
		}

	ret = g_strndup (start + 7, strlen (sql) - 8 - strlen (end));

	return ret;
}

const gchar
*gdaex_query_editor_get_sql_from (GdaExQueryEditor *qe)
{
	gchar *ret;
	const gchar *sql;

	gchar *start;
	gchar *end;

	ret = NULL;

	sql = gdaex_query_editor_get_sql (qe);
	if (sql == NULL)
		{
			return ret;
		}

	start = g_strstr_len (sql, -1, "FROM");
	if (start == NULL)
		{
			return ret;
		}

	end = g_strstr_len (sql, -1, "WHERE");
	if (end == NULL)
		{
			end = g_strstr_len (sql, -1, "ORDER BY");
		}

	ret = g_strndup (start + 5, strlen (start) - (end != NULL ? 6 : 5) - (end != NULL ? strlen (end) : 0));

	return ret;
}

const gchar
*gdaex_query_editor_get_sql_where (GdaExQueryEditor *qe)
{
	gchar *ret;
	const gchar *sql;

	gchar *start;
	gchar *end;

	ret = NULL;

	sql = gdaex_query_editor_get_sql (qe);
	if (sql == NULL)
		{
			return ret;
		}

	start = g_strstr_len (sql, -1, "WHERE");
	if (start == NULL)
		{
			return ret;
		}

	end = g_strstr_len (sql, -1, "ORDER BY");

	ret = g_strndup (start + 6, strlen (start) - (end != NULL ? 7 : 6) - (end != NULL ? strlen (end) : 0));

	return ret;
}

const gchar
*gdaex_query_editor_get_sql_order (GdaExQueryEditor *qe)
{
	gchar *ret;
	const gchar *sql;

	gchar *start;

	ret = NULL;

	sql = gdaex_query_editor_get_sql (qe);
	if (sql == NULL)
		{
			return ret;
		}

	start = g_strstr_len (sql, -1, "ORDER BY");
	if (start == NULL)
		{
			return ret;
		}

	ret = g_strndup (start + 9, strlen (start) - 9);

	return ret;
}

static void
gdaex_query_editor_xml_where (GdaExQueryEditor *qe,
                              GtkTreeIter *iter,
                              GtkTreeIter *iter_parent,
                              xmlNode *xnode_parent)
{
	GdaExQueryEditorPrivate *priv;

	gchar *table_name;
	gchar *field_name;

	GdaExQueryEditorTable *table;
	GdaExQueryEditorField *field;

	GtkTreePath *path;
	gint *indices;

	gboolean is_group;

	gboolean not;
	guint link_type;
	guint where_type;
	gchar *from_str;
	gchar *to_str;
	gchar *str_link;
	gchar *str_op;

	xmlNode *xnode;
	GtkTreeIter children;

	priv = GDAEX_QUERY_EDITOR_GET_PRIVATE (qe);

	do
		{
			gtk_tree_model_get (GTK_TREE_MODEL (priv->tstore_where), iter,
			                    COL_WHERE_LINK_TYPE, &link_type,
			                    COL_WHERE_TABLE_NAME, &table_name,
			                    COL_WHERE_NAME, &field_name,
			                    COL_WHERE_CONDITION_NOT, &not,
			                    COL_WHERE_CONDITION_TYPE, &where_type,
			                    COL_WHERE_CONDITION_FROM_SQL, &from_str,
			                    COL_WHERE_CONDITION_TO_SQL, &to_str,
			                    -1);

			is_group = (g_strcmp0 (table_name, GROUP) == 0);

			switch (link_type)
				{
					case GDAEX_QE_LINK_TYPE_AND:
						str_link = g_strdup ("AND");
						break;

					case GDAEX_QE_LINK_TYPE_OR:
						str_link = g_strdup ("OR");
						break;

					default:
						path = gtk_tree_model_get_path (GTK_TREE_MODEL (priv->tstore_where), iter);
						indices = gtk_tree_path_get_indices (path);
						if (indices[gtk_tree_path_get_depth (path) - 1] != 0)
							{
								g_warning (_("Link type «%d» not valid."), link_type);
								continue;
							}
						else
							{
								str_link = g_strdup ("");
								break;
							}
				}

			if (!is_group)
				{
					switch (where_type)
						{
							case GDAEX_QE_WHERE_TYPE_EQUAL:
								str_op = g_strdup ("EQUAL");
								break;

							case GDAEX_QE_WHERE_TYPE_STARTS:
								str_op = g_strdup ("STARTS");
								break;

							case GDAEX_QE_WHERE_TYPE_CONTAINS:
								str_op = g_strdup ("CONTAINS");
								break;

							case GDAEX_QE_WHERE_TYPE_ENDS:
								str_op = g_strdup ("ENDS");
								break;

							case GDAEX_QE_WHERE_TYPE_ISTARTS:
								str_op = g_strdup ("ISTARTS");
								break;

							case GDAEX_QE_WHERE_TYPE_ICONTAINS:
								str_op = g_strdup ("ICONTAINS");
								break;

							case GDAEX_QE_WHERE_TYPE_IENDS:
								str_op = g_strdup ("IENDS");
								break;

							case GDAEX_QE_WHERE_TYPE_GREAT:
								str_op = g_strdup ("GREAT");
								break;

							case GDAEX_QE_WHERE_TYPE_GREAT_EQUAL:
								str_op = g_strdup ("GREAT_EQUAL");
								break;

							case GDAEX_QE_WHERE_TYPE_LESS:
								str_op = g_strdup ("LESS");
								break;

							case GDAEX_QE_WHERE_TYPE_LESS_EQUAL:
								str_op = g_strdup ("LESS_EQUAL");
								break;

							case GDAEX_QE_WHERE_TYPE_BETWEEN:
								str_op = g_strdup ("BETWEEN");
								break;

							case GDAEX_QE_WHERE_TYPE_IS_NULL:
								str_op = g_strdup ("IS_NULL");
								break;

							default:
								g_warning (_("Where type «%d» not valid."), where_type);
								continue;
						}

					xnode = xmlNewNode (NULL, "field");

					xmlAddChild (xnode_parent, xnode);

					xmlNewProp (xnode, "table", table_name);
					xmlNewProp (xnode, "field", field_name);
					xmlNewProp (xnode, "link_type", str_link);
					xmlNewProp (xnode, "not", (not ? "y" : "n"));
					xmlNewProp (xnode, "where_type", str_op);
					xmlNewProp (xnode, "from", from_str);
					xmlNewProp (xnode, "to", to_str);

					g_free (str_op);
				}
			else
				{
					if (gtk_tree_model_iter_children (GTK_TREE_MODEL (priv->tstore_where), &children, iter))
						{
							xnode = xmlNewNode (NULL, "group");

							xmlAddChild (xnode_parent, xnode);

							xmlNewProp (xnode, "not", (not ? "y" : "n"));

							gdaex_query_editor_xml_where (qe, &children, iter, xnode);
						}
				}

			g_free (str_link);
		} while (gtk_tree_model_iter_next (GTK_TREE_MODEL (priv->tstore_where), iter));
}

xmlNode
*gdaex_query_editor_get_sql_as_xml (GdaExQueryEditor *qe)
{
	GdaExQueryEditorPrivate *priv;

	xmlNode *ret;

	GtkTreeIter iter;

	gchar *table_name;
	gchar *field_name;

	GdaExQueryEditorTable *table;
	GdaExQueryEditorField *field;

	xmlNode *node;

	ret = NULL;

	g_return_val_if_fail (GDAEX_IS_QUERY_EDITOR (qe), NULL);

	priv = GDAEX_QUERY_EDITOR_GET_PRIVATE (qe);

	ret = xmlNewNode (NULL, "gdaex_query_editor_choices");

	/* SHOW */
	if (gtk_tree_model_get_iter_first (GTK_TREE_MODEL (priv->lstore_show), &iter))
		{
			gchar *alias;
			xmlNode *node_show;

			node_show = xmlNewNode (NULL, "show");
			xmlAddChild (ret, node_show);

			do
				{
					gtk_tree_model_get (GTK_TREE_MODEL (priv->lstore_show), &iter,
					                    COL_SHOW_TABLE_NAME, &table_name,
					                    COL_SHOW_NAME, &field_name,
					                    COL_SHOW_ALIAS, &alias,
					                    -1);

					table = g_hash_table_lookup (priv->tables, table_name);
					field = g_hash_table_lookup (table->fields, field_name);

					node = xmlNewNode (NULL, "field");
					xmlAddChild (node_show, node);
					xmlNewProp (node, "table", table_name);
					xmlNewProp (node, "field", field_name);
					if (alias != NULL && g_strcmp0 (alias, "") != 0)
						{
							xmlNewProp (node, "alias", alias);
						}
					else
						{
							xmlNewProp (node, "alias", field->alias);
						}
				} while (gtk_tree_model_iter_next (GTK_TREE_MODEL (priv->lstore_show), &iter));
		}

	/* WHERE */
	if (gtk_tree_model_get_iter_first (GTK_TREE_MODEL (priv->tstore_where), &iter))
		{
			xmlNode *node_where;

			node_where = xmlNewNode (NULL, "where");
			xmlAddChild (ret, node_where);

			gdaex_query_editor_xml_where (qe, &iter, NULL, node_where);
		}

	/* ORDER */
	if (gtk_tree_model_get_iter_first (GTK_TREE_MODEL (priv->lstore_order), &iter))
		{
			gchar *asc_desc;
			xmlNode *node_order;

			node_order = xmlNewNode (NULL, "order");
			xmlAddChild (ret, node_order);

			do
				{
					gtk_tree_model_get (GTK_TREE_MODEL (priv->lstore_order), &iter,
					                    COL_ORDER_TABLE_NAME, &table_name,
					                    COL_ORDER_NAME, &field_name,
					                    COL_ORDER_ORDER, &asc_desc,
					                    -1);

					node = xmlNewNode (NULL, "field");
					xmlAddChild (node_order, node);
					xmlNewProp (node, "table", table_name);
					xmlNewProp (node, "field", field_name);
					xmlNewProp (node, "asc_desc", asc_desc);
				} while (gtk_tree_model_iter_next (GTK_TREE_MODEL (priv->lstore_order), &iter));
		}

	return ret;
}

void
gdaex_query_editor_load_choices_from_xml (GdaExQueryEditor *qe, xmlNode *root,
                                          gboolean clean)
{
	GdaExQueryEditorPrivate *priv;

	GtkTreeIter iter;

	gchar *table_name;
	gchar *field_name;
	gchar *name_visible;
	gchar *alias;
	GValue *v_table_field;

	GdaExQueryEditorTable *table;
	GdaExQueryEditorField *field;

	xmlNode *node;
	xmlNode *node_field;

	g_return_if_fail (GDAEX_IS_QUERY_EDITOR (qe));

	g_return_if_fail (xmlStrEqual (root->name, "gdaex_query_editor_choices"));

	priv = GDAEX_QUERY_EDITOR_GET_PRIVATE (qe);

	if (clean)
		{
			gdaex_query_editor_clean_choices (qe);
		}

	node = root->children;
	while (node != NULL)
	{
		if (xmlStrEqual (node->name, "show")
		    || xmlStrEqual (node->name, "where")
		    || xmlStrEqual (node->name, "order"))
			{
				node_field = node->children;
				while (node_field != NULL)
					{
						if (xmlStrEqual (node_field->name, "field"))
							{
								table_name = xmlGetProp (node_field, "table");
								table = g_hash_table_lookup (priv->tables, table_name);
								if (table == NULL)
									{
										g_warning (_("Table «%s» not found."), table_name);
										continue;
									}

								field_name = xmlGetProp (node_field, "field");
								field = g_hash_table_lookup (table->fields, field_name);
								if (field == NULL)
									{
										g_warning (_("Field «%s» not found in table «%s»."), field_name, table_name);
										continue;
									}

								name_visible = g_strconcat (table->name_visible, " - ", field->name_visible, NULL);
								v_table_field = gda_value_new (G_TYPE_STRING);
								g_value_set_string (v_table_field, name_visible);

								if (xmlStrEqual (node->name, "show"))
									{
										if (!gdaex_query_editor_model_has_value (GTK_TREE_MODEL (priv->lstore_show), COL_SHOW_VISIBLE_NAME, v_table_field))
											{
												alias = xmlGetProp (node_field, "alias");
												if (alias == NULL)
													{
														alias = g_strdup ("");
													}

												gtk_list_store_append (priv->lstore_show, &iter);
												gtk_list_store_set (priv->lstore_show, &iter,
												                    COL_SHOW_TABLE_NAME, field->table_name,
												                    COL_SHOW_NAME, field_name,
												                    COL_SHOW_VISIBLE_NAME, name_visible,
												                    COL_SHOW_ALIAS, alias,
												                    -1);
											}
									}
								else if (xmlStrEqual (node->name, "where"))
									{
										gchar *link;
										gchar *not;
										gchar *condition;
										gchar *from;
										gchar *to;
										gchar *from_sql;
										gchar *to_sql;
										gchar *from_visible;
										gchar *to_visible;

										GdaExQueryEditorLinkType link_type;
										GdaExQueryEditorWhereType where_type;

										link = xmlGetProp (node_field, "link_type");
										not = xmlGetProp (node_field, "not");
										condition = xmlGetProp (node_field, "where_type");

										from_sql = xmlGetProp (node_field, "from");
										to_sql = xmlGetProp (node_field, "to");
										from_visible = xmlGetProp (node_field, "from_visible");
										to_visible = xmlGetProp (node_field, "to_visible");


										if (field->type == GDAEX_QE_FIELD_TYPE_DATE)
											{
												GDate *gdate = gdaex_query_editor_get_gdate_from_sql (from_sql);
												from = g_strdup_printf ("%02d/%02d/%04d",
												                        g_date_get_day (gdate),
												                        g_date_get_month (gdate),
												                        g_date_get_year (gdate));

												gdate = gdaex_query_editor_get_gdate_from_sql (to_sql);
												to = g_strdup_printf ("%02d/%02d/%04d",
												                      g_date_get_day (gdate),
												                      g_date_get_month (gdate),
												                      g_date_get_year (gdate));
											}
										else if (field->type = GDAEX_QE_FIELD_TYPE_DATETIME)
											{
												GdaTimestamp *gdate = gdaex_query_editor_get_gdatimestamp_from_sql (from_sql);
												from = g_strdup_printf ("%02d/%02d/%04d %02d.%02d.%02d",
												                        gdate->day,
												                        gdate->month,
												                        gdate->year,
												                        gdate->hour,
												                        gdate->minute,
												                        gdate->second);

												gdate = gdaex_query_editor_get_gdatimestamp_from_sql (to_sql);
												to = g_strdup_printf ("%02d/%02d/%04d %02d.%02d.%02d",
												                      gdate->day,
												                      gdate->month,
												                      gdate->year,
												                      gdate->hour,
												                      gdate->minute,
												                      gdate->second);
											}
										else if (field->type = GDAEX_QE_FIELD_TYPE_TIME)
											{
												/* TODO */
											}
										else
											{
												from = xmlGetProp (node_field, "from");
												to = xmlGetProp (node_field, "to");
											}

										link_type = 0;
										if (g_strcmp0 (link, "AND") == 0)
											{
												link_type = GDAEX_QE_LINK_TYPE_AND;
											}
										else if (g_strcmp0 (link, "OR") == 0)
											{
												link_type = GDAEX_QE_LINK_TYPE_OR;
											}

										if (g_strcmp0 (condition, "EQUAL") == 0)
											{
												where_type = GDAEX_QE_WHERE_TYPE_EQUAL;
											}
										else if (g_strcmp0 (condition, "STARTS") == 0)
											{
												where_type = GDAEX_QE_WHERE_TYPE_STARTS;
											}
										else if (g_strcmp0 (condition, "CONTAINS") == 0)
											{
												where_type = GDAEX_QE_WHERE_TYPE_CONTAINS;
											}
										else if (g_strcmp0 (condition, "ENDS") == 0)
											{
												where_type = GDAEX_QE_WHERE_TYPE_ENDS;
											}
										else if (g_strcmp0 (condition, "ISTARTS") == 0)
											{
												where_type = GDAEX_QE_WHERE_TYPE_ISTARTS;
											}
										else if (g_strcmp0 (condition, "ICONTAINS") == 0)
											{
												where_type = GDAEX_QE_WHERE_TYPE_ICONTAINS;
											}
										else if (g_strcmp0 (condition, "IENDS") == 0)
											{
												where_type = GDAEX_QE_WHERE_TYPE_IENDS;
											}
										else if (g_strcmp0 (condition, "GREAT") == 0)
											{
												where_type = GDAEX_QE_WHERE_TYPE_GREAT;
											}
										else if (g_strcmp0 (condition, "GREAT_EQUAL") == 0)
											{
												where_type = GDAEX_QE_WHERE_TYPE_GREAT_EQUAL;
											}
										else if (g_strcmp0 (condition, "LESS") == 0)
											{
												where_type = GDAEX_QE_WHERE_TYPE_LESS;
											}
										else if (g_strcmp0 (condition, "LESS_EQUAL") == 0)
											{
												where_type = GDAEX_QE_WHERE_TYPE_LESS_EQUAL;
											}
										else if (g_strcmp0 (condition, "BETWEEN") == 0)
											{
												where_type = GDAEX_QE_WHERE_TYPE_BETWEEN;
											}
										else if (g_strcmp0 (condition, "IS_NULL") == 0)
											{
												where_type = GDAEX_QE_WHERE_TYPE_IS_NULL;
											}

										if (where_type != 0)
											{
												gtk_tree_store_append (priv->tstore_where, &iter, NULL);
												gtk_tree_store_set (priv->tstore_where, &iter,
												                    COL_WHERE_LINK_TYPE, link_type,
												                    COL_WHERE_LINK_TYPE_VISIBLE, gdaex_query_editor_get_link_type_str_from_type (link_type),
												                    COL_WHERE_TABLE_NAME, table_name,
												                    COL_WHERE_NAME, field_name,
												                    COL_WHERE_VISIBLE_NAME, name_visible,
												                    COL_WHERE_CONDITION_NOT, (g_strcmp0 (not, "n") != 0),
												                    COL_WHERE_CONDITION_TYPE, where_type,
												                    COL_WHERE_CONDITION_TYPE_VISIBLE, gdaex_query_editor_get_where_type_str_from_type (where_type),
												                    COL_WHERE_CONDITION_FROM_VISIBLE, from_visible,
												                    COL_WHERE_CONDITION_FROM, from,
												                    COL_WHERE_CONDITION_FROM_SQL, from_sql,
												                    COL_WHERE_CONDITION_TO_VISIBLE, (where_type == GDAEX_QE_WHERE_TYPE_BETWEEN ? to_visible : ""),
												                    COL_WHERE_CONDITION_TO, (where_type == GDAEX_QE_WHERE_TYPE_BETWEEN ? to : ""),
												                    COL_WHERE_CONDITION_TO_SQL, (where_type == GDAEX_QE_WHERE_TYPE_BETWEEN ? to_sql : ""),
												                    -1);
											}

										g_free (from);
										g_free (to);
										g_free (from_sql);
										g_free (to_sql);
										g_free (from_visible);
										g_free (to_visible);
										g_free (link);
										g_free (not);
										g_free (condition);
									}
								else if (xmlStrEqual (node->name, "order"))
									{
										if (!gdaex_query_editor_model_has_value (GTK_TREE_MODEL (priv->lstore_show), COL_SHOW_VISIBLE_NAME, v_table_field))
											{
												gchar *asc_desc;
												asc_desc = xmlGetProp (node_field, "asc_desc");

												if (g_strcmp0 (asc_desc, _("ASC")) == 0
												    || g_strcmp0 (asc_desc, _("DESC")) == 0)
													{
														gtk_list_store_append (priv->lstore_order, &iter);
														gtk_list_store_set (priv->lstore_order, &iter,
														                    COL_ORDER_TABLE_NAME, field->table_name,
														                    COL_ORDER_NAME, field_name,
														                    COL_ORDER_VISIBLE_NAME, name_visible,
														                    COL_ORDER_ORDER, asc_desc,
														                    -1);
													}
											}
									}

								g_free (field_name);
								g_free (table_name);
								g_free (name_visible);
							}

						node_field = node_field->next;
					}
			}
		node = node->next;
	}
}

/* PRIVATE */
static void
gdaex_query_editor_set_property (GObject *object,
                   guint property_id,
                   const GValue *value,
                   GParamSpec *pspec)
{
	GdaExQueryEditor *gdaex_query_editor = (GdaExQueryEditor *)object;
	GdaExQueryEditorPrivate *priv = GDAEX_QUERY_EDITOR_GET_PRIVATE (gdaex_query_editor);

	switch (property_id)
		{
			default:
				G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
				break;
		}
}

static void
gdaex_query_editor_get_property (GObject *object,
                   guint property_id,
                   GValue *value,
                   GParamSpec *pspec)
{
	GdaExQueryEditor *gdaex_query_editor = (GdaExQueryEditor *)object;
	GdaExQueryEditorPrivate *priv = GDAEX_QUERY_EDITOR_GET_PRIVATE (gdaex_query_editor);

	switch (property_id)
		{
			default:
				G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
				break;
		}
}

static void
gdaex_query_editor_dispose (GObject *object)
{
	GdaExQueryEditor *gdaex_query_editor = (GdaExQueryEditor *)object;
	GdaExQueryEditorPrivate *priv = GDAEX_QUERY_EDITOR_GET_PRIVATE (gdaex_query_editor);

	gtk_tree_store_clear (priv->tstore_fields);
	gtk_list_store_clear (priv->lstore_show);
	gtk_tree_store_clear (priv->tstore_where);
	gtk_list_store_clear (priv->lstore_order);

	if (priv->tables != NULL)
		{
			g_hash_table_destroy (priv->tables);
			priv->tables = NULL;
		}

	if (priv->relations != NULL)
		{
			g_slist_free (priv->relations);
			priv->relations = NULL;
		}

	G_OBJECT_CLASS (gdaex_query_editor_parent_class)->dispose (object);
}

static void
gdaex_query_editor_finalize (GObject *object)
{
	GdaExQueryEditor *gdaex_query_editor = (GdaExQueryEditor *)object;
	GdaExQueryEditorPrivate *priv = GDAEX_QUERY_EDITOR_GET_PRIVATE (gdaex_query_editor);

	gtk_widget_destroy (priv->hpaned_main);

	G_OBJECT_CLASS (gdaex_query_editor_parent_class)->finalize (object);
}

static void
gdaex_query_editor_clean (GdaExQueryEditor *gdaex_query_editor)
{
	GdaExQueryEditorPrivate *priv = GDAEX_QUERY_EDITOR_GET_PRIVATE (gdaex_query_editor);

	if (priv->tables != NULL)
		{
			g_hash_table_destroy (priv->tables);
		}
	priv->tables = g_hash_table_new (g_str_hash, g_str_equal);

	if (priv->relations != NULL)
		{
			g_slist_free (priv->relations);
		}
	priv->relations = NULL;
}

static gboolean
_gdaex_query_editor_add_table (GdaExQueryEditor *qe,
                              const gchar *table_name,
                              const gchar *table_name_visible,
                              gboolean is_visible)
{
	GdaExQueryEditorPrivate *priv;
	GdaExQueryEditorTable *table;

	gchar *_table_name;

	g_return_val_if_fail (GDAEX_IS_QUERY_EDITOR (qe), FALSE);
	g_return_val_if_fail (table_name != NULL, FALSE);

	priv = GDAEX_QUERY_EDITOR_GET_PRIVATE (qe);

	table = g_new0 (GdaExQueryEditorTable, 1);

	_table_name = g_strdup (table_name);

	g_return_val_if_fail (g_strcmp0 (g_strstrip (_table_name), "") != 0, FALSE);

	table->name = g_strdup (_table_name);
	g_free (_table_name);

	if (table_name_visible == NULL)
		{
			table->name_visible = g_strdup (table->name);
		}
	else
		{
			_table_name = g_strdup (table_name_visible);
			if (g_strcmp0 (g_strstrip (_table_name), "") == 0)
				{
					table->name_visible = g_strdup (table->name);
					g_free (_table_name);
				}
			else
				{
					table->name_visible = g_strdup (_table_name);
					g_free (_table_name);
				}
		}

	table->fields = g_hash_table_new (g_str_hash, g_str_equal);
	table->visible = is_visible;

	g_hash_table_insert (priv->tables, table->name, table);

	return TRUE;
}

static void
gdaex_query_editor_refresh_gui (GdaExQueryEditor *qe)
{
	GdaExQueryEditorPrivate *priv;

	GtkTreeIter iter;
	GHashTableIter hiter;
	gpointer key, value;
	GdaExQueryEditorTable *table;

	g_return_if_fail (GDAEX_IS_QUERY_EDITOR (qe));

	priv = GDAEX_QUERY_EDITOR_GET_PRIVATE (qe);

	gtk_tree_store_clear (priv->tstore_fields);

	g_hash_table_iter_init (&hiter, priv->tables);
	while (g_hash_table_iter_next (&hiter, &key, &value))
		{
			table = (GdaExQueryEditorTable *)value;

			if (table->visible)
				{
					gtk_tree_store_append (priv->tstore_fields, &iter, NULL);
					gtk_tree_store_set (priv->tstore_fields, &iter,
					                    COL_FIELDS_NAME, table->name,
					                    COL_FIELDS_VISIBLE_NAME, table->name_visible,
					                    -1);

					gdaex_query_editor_refresh_gui_add_fields (qe, table, &iter);

					/* if no fields, remove table */
					if (!gtk_tree_model_iter_has_child (GTK_TREE_MODEL (priv->tstore_fields), &iter))
						{
							gtk_tree_store_remove (priv->tstore_fields, &iter);
						}
				}
		}
}

static void
gdaex_query_editor_refresh_gui_add_fields (GdaExQueryEditor *qe,
                                           GdaExQueryEditorTable *table,
                                           GtkTreeIter *iter_parent)
{
	GdaExQueryEditorPrivate *priv;

	GtkTreeIter iter;
	GHashTableIter hiter;
	gpointer key, value;
	GdaExQueryEditorField *field;

	priv = GDAEX_QUERY_EDITOR_GET_PRIVATE (qe);

	g_hash_table_iter_init (&hiter, table->fields);
	while (g_hash_table_iter_next (&hiter, &key, &value))
		{
			field = (GdaExQueryEditorField *)value;

			if (field->for_show || field->for_where || field->for_order)
				{
					gtk_tree_store_append (priv->tstore_fields, &iter, iter_parent);
					gtk_tree_store_set (priv->tstore_fields, &iter,
					                    COL_FIELDS_TABLE_NAME, table->name,
					                    COL_FIELDS_NAME, field->name,
					                    COL_FIELDS_VISIBLE_NAME, field->name_visible,
					                    COL_FIELDS_DESCRIPTION, field->description,
					                    -1);
				}

			if (field->for_show && field->always_showed)
				{
					gdaex_query_editor_show_add_iter (qe, &iter);
				}
		}
}

static void
gdaex_query_editor_refill_always_show (GdaExQueryEditor *qe)
{
	GdaExQueryEditorPrivate *priv;

	GtkTreeIter iter_table;
	GtkTreeIter iter;

	gchar *table_name;
	gchar *field_name;

	GdaExQueryEditorTable *table;
	GdaExQueryEditorField *field;

	priv = GDAEX_QUERY_EDITOR_GET_PRIVATE (qe);

	if (gtk_tree_model_get_iter_first (GTK_TREE_MODEL (priv->tstore_fields), &iter_table))
		{
			do
				{
					if (gtk_tree_model_iter_children (GTK_TREE_MODEL (priv->tstore_fields), &iter, &iter_table))
						{
							do
								{
									gtk_tree_model_get (GTK_TREE_MODEL (priv->tstore_fields), &iter,
									                    COL_FIELDS_TABLE_NAME, &table_name,
									                    COL_FIELDS_NAME, &field_name,
									                    -1);
									table = g_hash_table_lookup (priv->tables, table_name);
									field = g_hash_table_lookup (table->fields, field_name);
									if (field->always_showed)
										{
											gdaex_query_editor_show_add_iter (qe, &iter);
										}
								} while (gtk_tree_model_iter_next (GTK_TREE_MODEL (priv->tstore_fields), &iter));
						}
				} while (gtk_tree_model_iter_next (GTK_TREE_MODEL (priv->tstore_fields), &iter_table));
		}
}

static void
gdaex_query_editor_store_remove_iter (GdaExQueryEditor *qe,
                                      GtkTreeSelection *sel,
                                      GObject *store)
{
	GdaExQueryEditorPrivate *priv;

	GtkTreeIter iter;
	GtkWidget *dialog;

	guint risp;

	g_return_if_fail (GTK_IS_LIST_STORE (store) || GTK_IS_TREE_STORE (store));

	priv = GDAEX_QUERY_EDITOR_GET_PRIVATE (qe);

	if (gtk_tree_selection_get_selected (sel, NULL, &iter))
		{
			dialog = gtk_message_dialog_new (GTK_WINDOW (gtk_widget_get_toplevel (priv->hpaned_main)),
			                                 GTK_DIALOG_DESTROY_WITH_PARENT,
			                                 GTK_MESSAGE_QUESTION,
			                                 GTK_BUTTONS_YES_NO,
			                                 _("Are you sure you want to remove the selected field?"));
			risp = gtk_dialog_run (GTK_DIALOG (dialog));
			if (risp == GTK_RESPONSE_YES)
				{
					if (GTK_IS_LIST_STORE (store))
						{
							gtk_list_store_remove (GTK_LIST_STORE (store), &iter);
						}
					else
						{
							gtk_tree_store_remove (GTK_TREE_STORE (store), &iter);
						}
				}
			gtk_widget_destroy (dialog);
		}
	else
		{
			dialog = gtk_message_dialog_new (GTK_WINDOW (gtk_widget_get_toplevel (priv->hpaned_main)),
			                                 GTK_DIALOG_DESTROY_WITH_PARENT,
			                                 GTK_MESSAGE_WARNING,
			                                 GTK_BUTTONS_OK,
			                                 _("You must select a field before."));
			gtk_dialog_run (GTK_DIALOG (dialog));
			gtk_widget_destroy (dialog);
		}
}

static void
gdaex_query_editor_store_move_iter_up_down (GdaExQueryEditor *qe,
                                      GtkTreeSelection *sel,
                                      GObject *store,
                                      gboolean up)
{
	GdaExQueryEditorPrivate *priv;

	GtkTreePath *path;
	GtkTreeIter iter;
	GtkTreeIter iter_prev;

	gboolean swap_link;
	guint link_type;

	GtkWidget *dialog;

	g_return_if_fail (GTK_IS_LIST_STORE (store) || GTK_IS_TREE_STORE (store));

	priv = GDAEX_QUERY_EDITOR_GET_PRIVATE (qe);

	if (gtk_tree_selection_get_selected (sel, NULL, &iter))
		{
			path = gtk_tree_model_get_path (GTK_TREE_MODEL (store), &iter);
			if (path != NULL)
				{
					/* if the first item of where is changed,
					   swap link type */
					swap_link = FALSE;
					if (sel == priv->sel_where)
						{
							gint *indices = gtk_tree_path_get_indices (path);
							if (indices[0] == (up ? 1 : 0))
								{
									swap_link = TRUE;
								}
						}

					up ? gtk_tree_path_prev (path) : gtk_tree_path_next (path);
					if (gtk_tree_model_get_iter (GTK_TREE_MODEL (store), &iter_prev, path))
						{
							if (up)
								{
									if (GTK_IS_LIST_STORE (store))
										{
											gtk_list_store_move_before (GTK_LIST_STORE (store), &iter, &iter_prev);
										}
									else
										{
											gtk_tree_store_move_before (GTK_TREE_STORE (store), &iter, &iter_prev);
										}
								}
							else
								{
									if (GTK_IS_LIST_STORE (store))
										{
											gtk_list_store_move_after (GTK_LIST_STORE (store), &iter, &iter_prev);
										}
									else
										{
											gtk_tree_store_move_after (GTK_TREE_STORE (store), &iter, &iter_prev);
										}
								}

							if (swap_link)
								{
									gtk_tree_model_get_iter_first (GTK_TREE_MODEL (store), &iter_prev);
									gtk_tree_model_get (GTK_TREE_MODEL (store), &iter_prev,
									                    COL_WHERE_LINK_TYPE, &link_type,
									                    -1);
									gtk_tree_store_set (GTK_TREE_STORE (store), &iter_prev,
									                    COL_WHERE_LINK_TYPE, 0,
									                    COL_WHERE_LINK_TYPE_VISIBLE, "",
									                    -1);

									gtk_tree_model_iter_next (GTK_TREE_MODEL (store), &iter_prev);
									gtk_tree_store_set (GTK_TREE_STORE (store), &iter_prev,
									                    COL_WHERE_LINK_TYPE, link_type,
									                    COL_WHERE_LINK_TYPE_VISIBLE, gdaex_query_editor_get_link_type_str_from_type (link_type),
									                    -1);
								}
						}
				}
		}
	else
		{
			dialog = gtk_message_dialog_new (GTK_WINDOW (gtk_widget_get_toplevel (priv->hpaned_main)),
			                                 GTK_DIALOG_DESTROY_WITH_PARENT,
			                                 GTK_MESSAGE_WARNING,
			                                 GTK_BUTTONS_OK,
			                                 _("You must select a field before."));
			gtk_dialog_run (GTK_DIALOG (dialog));
			gtk_widget_destroy (dialog);
		}
}

static gboolean
gdaex_query_editor_model_has_value (GtkTreeModel *model,
                                    guint column,
                                    GValue *value)
{
	gboolean ret;

	GtkTreeIter iter;
	GValue iter_value = { 0 };

	g_return_val_if_fail (gtk_tree_model_get_column_type (model, column) == G_VALUE_TYPE (value), FALSE);

	ret = FALSE;

	if (gtk_tree_model_get_iter_first (model, &iter))
		{
			do
				{
					gtk_tree_model_get_value (model, &iter, column, &iter_value);
					if (!gda_value_differ (value, &iter_value))
						{
							g_value_unset (&iter_value);
							return TRUE;
						}
					g_value_unset (&iter_value);
				} while (gtk_tree_model_iter_next (model, &iter));
		}

	return ret;
}

static void
gdaex_query_editor_remove_child_from_vbx_values (GdaExQueryEditor *qe)
{
	GdaExQueryEditorPrivate *priv = GDAEX_QUERY_EDITOR_GET_PRIVATE (qe);

	GList *children;

	children = gtk_container_get_children (GTK_CONTAINER (priv->vbx_values));
	if (children != NULL && children->data != NULL)
		{
			g_object_ref ((GObject *)children->data);
			gtk_container_remove (GTK_CONTAINER (priv->vbx_values), (GtkWidget *)children->data);
		}
}

static gchar *
gdaex_query_editor_get_where_type_str_from_type (guint where_type)
{
	gchar *ret;

	switch (where_type)
		{
			case GDAEX_QE_WHERE_TYPE_EQUAL:
				ret = g_strdup (_("Equal"));
				break;

			case GDAEX_QE_WHERE_TYPE_STARTS:
				ret = g_strdup (_("Starts with"));
				break;

			case GDAEX_QE_WHERE_TYPE_CONTAINS:
				ret = g_strdup (_("Contains"));
				break;

			case GDAEX_QE_WHERE_TYPE_ENDS:
				ret = g_strdup (_("Ends with"));
				break;

			case GDAEX_QE_WHERE_TYPE_ISTARTS:
				ret = g_strdup (_("Case-insensitive starts with"));
				break;

			case GDAEX_QE_WHERE_TYPE_ICONTAINS:
				ret = g_strdup (_("Case-insensitive contains"));
				break;

			case GDAEX_QE_WHERE_TYPE_IENDS:
				ret = g_strdup (_("Case-insensitive ends with"));
				break;

			case GDAEX_QE_WHERE_TYPE_GREAT:
				ret = g_strdup (_("Greater"));
				break;

			case GDAEX_QE_WHERE_TYPE_GREAT_EQUAL:
				ret = g_strdup (_("Greater or equal"));
				break;

			case GDAEX_QE_WHERE_TYPE_LESS:
				ret = g_strdup (_("Lesser"));
				break;

			case GDAEX_QE_WHERE_TYPE_LESS_EQUAL:
				ret = g_strdup (_("Lesser or equal"));
				break;

			case GDAEX_QE_WHERE_TYPE_BETWEEN:
				ret = g_strdup (_("Between"));
				break;

			case GDAEX_QE_WHERE_TYPE_IS_NULL:
				ret = g_strdup (_("Is NULL"));
				break;

			default:
				ret = g_strdup ("");
				break;
		};

	return ret;
}

static gchar *
gdaex_query_editor_get_link_type_str_from_type (guint link_type)
{
	gchar *ret;

	ret = NULL;

	switch (link_type)
		{
			case GDAEX_QE_LINK_TYPE_AND:
				ret = g_strdup (_("And"));
				break;

			case GDAEX_QE_LINK_TYPE_OR:
				ret = g_strdup (_("Or"));
				break;
		};

	return ret;
}

static void
gdaex_query_editor_on_sel_fields_changed (GtkTreeSelection *treeselection,
                                          gpointer user_data)
{
	GtkTreeIter iter;

	gchar *table_name;
	gchar *field_name;
	GdaExQueryEditorTable *table;
	GdaExQueryEditorField *field;

	gchar *table_field;
	GValue *v_table_field;

	GdaExQueryEditor *qe = (GdaExQueryEditor *)user_data;
	GdaExQueryEditorPrivate *priv = GDAEX_QUERY_EDITOR_GET_PRIVATE (qe);

	gtk_widget_set_sensitive (GTK_WIDGET (gtk_builder_get_object (priv->gtkbuilder, "button3")), FALSE);
	gtk_widget_set_sensitive (GTK_WIDGET (gtk_builder_get_object (priv->gtkbuilder, "button7")), FALSE);
	gtk_widget_set_sensitive (GTK_WIDGET (gtk_builder_get_object (priv->gtkbuilder, "button11")), FALSE);

	if (gtk_tree_selection_get_selected (priv->sel_fields, NULL, &iter))
		{
			gtk_tree_model_get (GTK_TREE_MODEL (priv->tstore_fields), &iter,
			                    COL_FIELDS_TABLE_NAME, &table_name,
			                    COL_FIELDS_NAME, &field_name,
			                    -1);

			if (table_name == NULL || g_strcmp0 (table_name, "") == 0)
				{
					/* cannot add a table */
					return;
				}

			table = g_hash_table_lookup (priv->tables, table_name);
			field = g_hash_table_lookup (table->fields, field_name);

			if (field->for_where)
				{
					gtk_widget_set_sensitive (GTK_WIDGET (gtk_builder_get_object (priv->gtkbuilder, "button7")), TRUE);
				}

			table_field = g_strconcat (table->name_visible, " - ", field->name_visible, NULL);
			v_table_field = gda_value_new (G_TYPE_STRING);
			g_value_set_string (v_table_field, table_field);
			if (!gdaex_query_editor_model_has_value (GTK_TREE_MODEL (priv->lstore_show),
			                                         COL_ORDER_VISIBLE_NAME,
			                                         v_table_field))
				{
					if (!field->always_showed && field->for_show)
						{
							gtk_widget_set_sensitive (GTK_WIDGET (gtk_builder_get_object (priv->gtkbuilder, "button3")), TRUE);
						}
				}
			if (!gdaex_query_editor_model_has_value (GTK_TREE_MODEL (priv->lstore_order),
			                                         COL_ORDER_VISIBLE_NAME,
			                                         v_table_field))
				{
				if (field->for_order)
					{
						gtk_widget_set_sensitive (GTK_WIDGET (gtk_builder_get_object (priv->gtkbuilder, "button11")), TRUE);
					}
				}

			g_free (table_field);
			g_value_unset (v_table_field);
		}
}

static void
gdaex_query_editor_on_trv_fields_row_activated (GtkTreeView *tree_view,
                                                GtkTreePath *path,
                                                GtkTreeViewColumn *column,
                                                gpointer user_data)
{
	GdaExQueryEditor *qe;
	GdaExQueryEditorPrivate *priv;

	GtkTreeIter iter;
	guint page;

	qe = (GdaExQueryEditor *)user_data;
	priv = GDAEX_QUERY_EDITOR_GET_PRIVATE (qe);

	if (!gtk_tree_selection_get_selected (priv->sel_fields, NULL, &iter))
		{
			return;
		}

	page = gtk_notebook_get_current_page (GTK_NOTEBOOK (priv->notebook));
	switch (page)
		{
			case GDAEX_QE_PAGE_SHOW:
				if (gtk_widget_is_sensitive (GTK_WIDGET (gtk_builder_get_object (priv->gtkbuilder, "button3"))))
					{
						gtk_button_clicked (GTK_BUTTON (gtk_builder_get_object (priv->gtkbuilder, "button3")));
					}
				break;

			case GDAEX_QE_PAGE_WHERE:
				if (gtk_widget_is_sensitive (GTK_WIDGET (gtk_builder_get_object (priv->gtkbuilder, "button7"))))
					{
						gtk_button_clicked (GTK_BUTTON (gtk_builder_get_object (priv->gtkbuilder, "button7")));
					}
				break;

			case GDAEX_QE_PAGE_ORDER:
				if (gtk_widget_is_sensitive (GTK_WIDGET (gtk_builder_get_object (priv->gtkbuilder, "button11"))))
					{
						gtk_button_clicked (GTK_BUTTON (gtk_builder_get_object (priv->gtkbuilder, "button11")));
					}
				break;
		}
}

static void
gdaex_query_editor_on_cb_where_type_changed (GtkComboBox *widget,
                                             gpointer user_data)
{
	GdaExQueryEditor *qe;
	GdaExQueryEditorPrivate *priv;

	GtkTreeIter iter;
	guint where_type;

	qe = (GdaExQueryEditor *)user_data;
	priv = GDAEX_QUERY_EDITOR_GET_PRIVATE (qe);

	if (gtk_combo_box_get_active_iter (GTK_COMBO_BOX (priv->cb_where_type), &iter))
		{
			gtk_tree_model_get (GTK_TREE_MODEL (priv->lstore_where_type), &iter,
			                    0, &where_type,
			                    -1);

			gtk_widget_set_visible (priv->lbl_from, where_type != GDAEX_QE_WHERE_TYPE_IS_NULL);
			gtk_widget_set_visible (priv->txt_from, where_type != GDAEX_QE_WHERE_TYPE_IS_NULL);

			if (where_type == GDAEX_QE_WHERE_TYPE_IS_NULL)
				{
					gdaex_query_editor_iwidget_set_value (GDAEX_QUERY_EDITOR_IWIDGET (priv->txt_from), "");
				}

			gtk_widget_set_visible (priv->lbl_to, where_type == GDAEX_QE_WHERE_TYPE_BETWEEN);
			gtk_widget_set_visible (priv->txt_to, where_type == GDAEX_QE_WHERE_TYPE_BETWEEN);

			if (where_type != GDAEX_QE_WHERE_TYPE_BETWEEN)
				{
					gdaex_query_editor_iwidget_set_value (GDAEX_QUERY_EDITOR_IWIDGET (priv->txt_to), "");
					gtk_grid_set_row_spacing (GTK_GRID (priv->tbl), 0);
				}
			else
				{
					gdaex_query_editor_iwidget_set_value (GDAEX_QUERY_EDITOR_IWIDGET (priv->txt_from), "");
					gdaex_query_editor_iwidget_set_value (GDAEX_QUERY_EDITOR_IWIDGET (priv->txt_to), "");
					gtk_grid_set_row_spacing (GTK_GRID (priv->tbl), 5);
				}
		}
}

static void
gdaex_query_editor_on_btn_cancel_clicked (GtkButton *button,
                                    gpointer user_data)
{
	GdaExQueryEditor *qe;
	GdaExQueryEditorPrivate *priv;

	qe = (GdaExQueryEditor *)user_data;
	priv = GDAEX_QUERY_EDITOR_GET_PRIVATE (qe);

	gdaex_query_editor_remove_child_from_vbx_values (qe);
	gtk_widget_hide (priv->vbx_values_container);

	switch (priv->editor_type)
		{
			case GDAEX_QE_PAGE_SHOW:
				gtk_tree_selection_unselect_all (priv->sel_show);
				break;

			case GDAEX_QE_PAGE_WHERE:
				gtk_tree_selection_unselect_all (priv->sel_where);
				break;

			case GDAEX_QE_PAGE_ORDER:
				gtk_tree_selection_unselect_all (priv->sel_order);
				break;
		}
}

static void
gdaex_query_editor_on_btn_save_clicked (GtkButton *button,
                                    gpointer user_data)
{
	GdaExQueryEditor *qe;
	GdaExQueryEditorPrivate *priv;

	GtkWidget *dialog;

	GtkTreeIter iter;

	gchar *val1;
	gchar *val2;
	gchar *asc_desc;

	qe = (GdaExQueryEditor *)user_data;
	priv = GDAEX_QUERY_EDITOR_GET_PRIVATE (qe);

	val1 = NULL;
	val2 = NULL;
	asc_desc = NULL;

	switch (priv->editor_type)
		{
			case GDAEX_QE_PAGE_SHOW:
				if (gtk_tree_selection_get_selected (priv->sel_show, NULL, &iter))
					{
						val1 = (gchar *)gtk_entry_get_text (GTK_ENTRY (priv->txt_alias));
						if (val1 == NULL)
							{
								val1 = g_strdup ("");
							}
						else
							{
								val1 = g_strstrip (g_strdup (val1));
							}
						gtk_list_store_set (priv->lstore_show, &iter,
						                    COL_SHOW_ALIAS, val1,
						                    -1);

						gtk_tree_selection_unselect_all (priv->sel_show);
					}
				break;

			case GDAEX_QE_PAGE_WHERE:
				if (gtk_tree_selection_get_selected (priv->sel_where, NULL, &iter))
					{
						GtkTreeModel *model;
						GtkTreeIter iter_val;

						guint link_type;
						guint where_type;

						gchar *val1_visible;
						gchar *val2_visible;
						gchar *val1_sql;
						gchar *val2_sql;

						val1_visible = NULL;
						val2_visible = NULL;
						val1_sql = NULL;
						val2_sql = NULL;

						/* if it is the first condition, "link" isn't visibile */
						GtkTreePath *path = gtk_tree_model_get_path (GTK_TREE_MODEL (priv->tstore_where), &iter);
						gint *indices = gtk_tree_path_get_indices (path);
						if (indices[gtk_tree_path_get_depth (path) - 1] != 0)
							{
								model = gtk_combo_box_get_model (GTK_COMBO_BOX (priv->cb_link_type));
								if (!gtk_combo_box_get_active_iter (GTK_COMBO_BOX (priv->cb_link_type), &iter_val))
									{
										dialog = gtk_message_dialog_new (GTK_WINDOW (gtk_widget_get_toplevel (priv->hpaned_main)),
										                                 GTK_DIALOG_DESTROY_WITH_PARENT,
										                                 GTK_MESSAGE_WARNING,
										                                 GTK_BUTTONS_OK,
										                                 _("You must select a link's type before."));
										gtk_dialog_run (GTK_DIALOG (dialog));
										gtk_widget_destroy (dialog);
										return;
									}

								gtk_tree_model_get (model, &iter_val,
								                    0, &link_type,
								                    -1);
							}
						else
							{
								link_type = 0;
							}

						if (gtk_widget_get_visible (priv->cb_where_type))
							{
								model = gtk_combo_box_get_model (GTK_COMBO_BOX (priv->cb_where_type));
								if (!gtk_combo_box_get_active_iter (GTK_COMBO_BOX (priv->cb_where_type), &iter_val))
									{
										dialog = gtk_message_dialog_new (GTK_WINDOW (gtk_widget_get_toplevel (priv->hpaned_main)),
								                                 GTK_DIALOG_DESTROY_WITH_PARENT,
								                                 GTK_MESSAGE_WARNING,
								                                 GTK_BUTTONS_OK,
								                                 _("You must select a condition's type before."));
										gtk_dialog_run (GTK_DIALOG (dialog));
										gtk_widget_destroy (dialog);
										return;
									}

								gtk_tree_model_get (model, &iter_val,
								                    0, &where_type,
								                    -1);

								val1 = (gchar *)gdaex_query_editor_iwidget_get_value (GDAEX_QUERY_EDITOR_IWIDGET (priv->txt_from));
								val1_visible = (gchar *)gdaex_query_editor_iwidget_get_value_visible (GDAEX_QUERY_EDITOR_IWIDGET (priv->txt_from));
								val1_sql = (gchar *)gdaex_query_editor_iwidget_get_value_sql (GDAEX_QUERY_EDITOR_IWIDGET (priv->txt_from));
								if (val1 == NULL)
									{
										val1 = g_strdup ("");
									}
								else
									{
										val1 = g_strdup (val1);
									}
								if (val1_sql == NULL
									|| g_strcmp0 (val1_sql, "NULL") == 0)
									{
										val1_sql = g_strdup ("");
									}
								else
									{
										val1_sql = g_strdup (val1_sql);
									}

								val2 = (gchar *)gdaex_query_editor_iwidget_get_value (GDAEX_QUERY_EDITOR_IWIDGET (priv->txt_to));
								val2_visible = (gchar *)gdaex_query_editor_iwidget_get_value_visible (GDAEX_QUERY_EDITOR_IWIDGET (priv->txt_to));
								val2_sql = (gchar *)gdaex_query_editor_iwidget_get_value_sql (GDAEX_QUERY_EDITOR_IWIDGET (priv->txt_to));
								if (val2 == NULL)
									{
										val2 = g_strdup ("");
									}
								else
									{
										val2 = g_strdup (val2);
									}
								if (val2_sql == NULL
									|| g_strcmp0 (val2_sql, "NULL") == 0)
									{
										val2_sql = g_strdup ("");
									}
								else
									{
										val2_sql = g_strdup (val2_sql);
									}
							}
						else
							{
								where_type = 0;
								val1 = g_strdup ("");
								val1_sql = g_strdup ("");
								val2 = g_strdup ("");
								val2_sql = g_strdup ("");
							}

						gtk_tree_store_set (priv->tstore_where, &iter,
						                    COL_WHERE_LINK_TYPE, link_type,
						                    COL_WHERE_LINK_TYPE_VISIBLE, gdaex_query_editor_get_link_type_str_from_type (link_type),
						                    COL_WHERE_CONDITION_NOT, gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (priv->chk_not)),
						                    COL_WHERE_CONDITION_TYPE, where_type,
						                    COL_WHERE_CONDITION_TYPE_VISIBLE, gdaex_query_editor_get_where_type_str_from_type (where_type),
						                    COL_WHERE_CONDITION_FROM_VISIBLE, val1_visible,
						                    COL_WHERE_CONDITION_FROM, val1,
						                    COL_WHERE_CONDITION_FROM_SQL, val1_sql,
						                    COL_WHERE_CONDITION_TO_VISIBLE, val2_visible,
						                    COL_WHERE_CONDITION_TO, val2,
						                    COL_WHERE_CONDITION_TO_SQL, val2_sql,
						                    -1);

						gtk_tree_selection_unselect_all (priv->sel_where);

						g_free (val1_sql);
						g_free (val2_sql);
					}
				break;

			case GDAEX_QE_PAGE_ORDER:
				if (gtk_tree_selection_get_selected (priv->sel_order, NULL, &iter))
					{
						if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (priv->opt_asc)))
							{
								asc_desc = g_strdup (_("ASC"));
							}
						else
							{
								asc_desc = g_strdup (_("DESC"));
							}

						gtk_list_store_set (priv->lstore_order, &iter,
						                    COL_ORDER_ORDER, asc_desc,
						                    -1);

						gtk_tree_selection_unselect_all (priv->sel_order);
					}
				break;
		}

	g_free (asc_desc);
	g_free (val1);
	g_free (val2);

	gtk_widget_hide (priv->vbx_values_container);
}

static void
gdaex_query_editor_show_add_iter (GdaExQueryEditor *qe, GtkTreeIter *iter)
{
	GdaExQueryEditorPrivate *priv;

	gchar *table_name;
	gchar *field_name;
	GdaExQueryEditorTable *table;
	GdaExQueryEditorField *field;

	priv = GDAEX_QUERY_EDITOR_GET_PRIVATE (qe);

	gtk_tree_model_get (GTK_TREE_MODEL (priv->tstore_fields), iter,
	                    COL_FIELDS_TABLE_NAME, &table_name,
	                    COL_FIELDS_NAME, &field_name,
	                    -1);

	table = g_hash_table_lookup (priv->tables, table_name);
	field = g_hash_table_lookup (table->fields, field_name);

	gtk_list_store_append (priv->lstore_show, iter);
	gtk_list_store_set (priv->lstore_show, iter,
	                    COL_SHOW_TABLE_NAME, field->table_name,
	                    COL_SHOW_NAME, field_name,
	                    COL_SHOW_VISIBLE_NAME, g_strconcat (table->name_visible, " - ", field->name_visible, NULL),
	                    -1);

	gtk_tree_selection_select_iter (priv->sel_show, iter);

	g_free (table_name);
	g_free (field_name);
}

static void
gdaex_query_editor_on_btn_show_add_clicked (GtkButton *button,
                                    gpointer user_data)
{
	GdaExQueryEditor *qe;
	GdaExQueryEditorPrivate *priv;

	GtkTreeIter iter;
	GtkWidget *dialog;

	qe = (GdaExQueryEditor *)user_data;
	priv = GDAEX_QUERY_EDITOR_GET_PRIVATE (qe);

	if (gtk_tree_selection_get_selected (priv->sel_fields, NULL, &iter))
		{
			gdaex_query_editor_show_add_iter (qe, &iter);
			gdaex_query_editor_on_sel_fields_changed (NULL, user_data);
		}
	else
		{
			dialog = gtk_message_dialog_new (GTK_WINDOW (gtk_widget_get_toplevel (priv->hpaned_main)),
			                                 GTK_DIALOG_DESTROY_WITH_PARENT,
			                                 GTK_MESSAGE_WARNING,
			                                 GTK_BUTTONS_OK,
			                                 _("You must select a field before."));
			gtk_dialog_run (GTK_DIALOG (dialog));
			gtk_widget_destroy (dialog);
		}
}

static void
gdaex_query_editor_on_btn_show_remove_clicked (GtkButton *button,
                                    gpointer user_data)
{
	GdaExQueryEditor *qe = (GdaExQueryEditor *)user_data;
	GdaExQueryEditorPrivate *priv = GDAEX_QUERY_EDITOR_GET_PRIVATE (qe);

	gdaex_query_editor_store_remove_iter (qe,
	                                      priv->sel_show,
	                                      G_OBJECT (priv->lstore_show));
}

static void
gdaex_query_editor_on_btn_show_up_clicked (GtkButton *button,
                                    gpointer user_data)
{
	GdaExQueryEditor *qe = (GdaExQueryEditor *)user_data;
	GdaExQueryEditorPrivate *priv = GDAEX_QUERY_EDITOR_GET_PRIVATE (qe);

	gdaex_query_editor_store_move_iter_up_down (qe,
	                                      priv->sel_show,
	                                      G_OBJECT (priv->lstore_show),
	                                      TRUE);
}

static void
gdaex_query_editor_on_btn_show_down_clicked (GtkButton *button,
                                    gpointer user_data)
{
	GdaExQueryEditor *qe = (GdaExQueryEditor *)user_data;
	GdaExQueryEditorPrivate *priv = GDAEX_QUERY_EDITOR_GET_PRIVATE (qe);

	gdaex_query_editor_store_move_iter_up_down (qe,
	                                      priv->sel_show,
	                                      G_OBJECT (priv->lstore_show),
	                                      FALSE);
}

static void
gdaex_query_editor_on_btn_show_clean_clicked (GtkButton *button,
                                    gpointer user_data)
{
	GdaExQueryEditor *qe = (GdaExQueryEditor *)user_data;
	GdaExQueryEditorPrivate *priv = GDAEX_QUERY_EDITOR_GET_PRIVATE (qe);

	gtk_list_store_clear (priv->lstore_show);
	gdaex_query_editor_refill_always_show (qe);
}

static void
gdaex_query_editor_on_sel_show_changed (GtkTreeSelection *treeselection,
                                          gpointer user_data)
{
	GtkTreeIter iter;

	gchar *table_name;
	gchar *field_name;
	gchar *alias;

	GdaExQueryEditorTable *table;
	GdaExQueryEditorField *field;

	GtkWidget *lbl;

	GdaExQueryEditor *qe = (GdaExQueryEditor *)user_data;
	GdaExQueryEditorPrivate *priv = GDAEX_QUERY_EDITOR_GET_PRIVATE (qe);

	gtk_widget_hide (priv->vbx_values_container);
	gdaex_query_editor_remove_child_from_vbx_values (qe);

	if (gtk_tree_selection_get_selected (priv->sel_show, NULL, &iter))
		{
			priv->editor_type = GDAEX_QE_PAGE_SHOW;

			gtk_tree_selection_unselect_all (priv->sel_where);
			gtk_tree_selection_unselect_all (priv->sel_order);

			gtk_tree_model_get (GTK_TREE_MODEL (priv->lstore_show), &iter,
			                    COL_SHOW_TABLE_NAME, &table_name,
			                    COL_SHOW_NAME, &field_name,
			                    COL_SHOW_ALIAS, &alias,
			                    -1);

			table = g_hash_table_lookup (priv->tables, table_name);
			field = g_hash_table_lookup (table->fields, field_name);

			gtk_widget_set_sensitive (GTK_WIDGET (gtk_builder_get_object (priv->gtkbuilder, "button4")), !field->always_showed);

			if (!GTK_IS_BOX (priv->hbox_show))
				{
					GtkWidget *tbl;

					priv->hbox_show = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0);

					priv->frame_show = gtk_frame_new ("");
					g_object_set (G_OBJECT (priv->frame_show),
								  "margin", 5,
								  NULL);
					gtk_frame_set_shadow_type (GTK_FRAME (priv->frame_show), GTK_SHADOW_ETCHED_IN);
					gtk_box_pack_start (GTK_BOX (priv->hbox_show), priv->frame_show, TRUE, TRUE, 0);

					tbl = gtk_grid_new ();
					g_object_set (G_OBJECT (tbl),
								  "margin", 5,
								  NULL);
					gtk_grid_set_row_spacing (GTK_GRID (tbl), 5);
					gtk_grid_set_column_spacing (GTK_GRID (tbl), 5);
					gtk_container_add (GTK_CONTAINER (priv->frame_show), tbl);

					lbl = gtk_label_new ("Alias");
					gtk_grid_attach (GTK_GRID (tbl), lbl, 0, 0, 1, 1);

					priv->txt_alias = gtk_entry_new ();
					gtk_grid_attach (GTK_GRID (tbl), priv->txt_alias, 0, 1, 1, 1);
				}

			gtk_label_set_markup (GTK_LABEL (gtk_frame_get_label_widget (GTK_FRAME (priv->frame_show))),
								  g_strconcat ("<b>", table->name_visible, " - ", field->name_visible, "</b>", NULL));
			gtk_entry_set_text (GTK_ENTRY (priv->txt_alias), alias == NULL ? "" : alias);

			gtk_box_pack_start (GTK_BOX (priv->vbx_values), priv->hbox_show, FALSE, FALSE, 0);

			gtk_widget_show_all (priv->vbx_values);
			gtk_widget_show (priv->vbx_values_container);

			g_free (table_name);
			g_free (field_name);
			g_free (alias);
		}
}

static void
gdaex_query_editor_on_btn_where_add_clicked (GtkButton *button,
                                    gpointer user_data)
{
	GdaExQueryEditor *qe;
	GdaExQueryEditorPrivate *priv;

	GtkTreeIter iter;
	GtkTreeIter iter_parent;
	GtkWidget *dialog;

	gchar *group;
	gboolean with_parent;

	gchar *table_name;
	gchar *field_name;
	GdaExQueryEditorTable *table;
	GdaExQueryEditorField *field;

	qe = (GdaExQueryEditor *)user_data;
	priv = GDAEX_QUERY_EDITOR_GET_PRIVATE (qe);

	if (gtk_tree_selection_get_selected (priv->sel_fields, NULL, &iter))
		{
			with_parent = FALSE;

			gtk_tree_model_get (GTK_TREE_MODEL (priv->tstore_fields), &iter,
			                    COL_FIELDS_TABLE_NAME, &table_name,
			                    COL_FIELDS_NAME, &field_name,
			                    -1);

			table = g_hash_table_lookup (priv->tables, table_name);
			field = g_hash_table_lookup (table->fields, field_name);

			if (gtk_tree_selection_get_selected (priv->sel_where, NULL, &iter_parent))
				{
					gtk_tree_model_get (GTK_TREE_MODEL (priv->tstore_where), &iter_parent,
					                    COL_WHERE_TABLE_NAME, &group,
					                    -1);
					if (g_strcmp0 (group, GROUP) != 0)
						{
							if (gtk_tree_model_iter_parent (GTK_TREE_MODEL (priv->tstore_where), &iter, &iter_parent))
								{
									with_parent = TRUE;
									iter_parent = iter;
								}
						}
					else
						{
							with_parent = TRUE;
						}
				}

			gtk_tree_store_append (priv->tstore_where, &iter, with_parent ? &iter_parent : NULL);
			gtk_tree_store_set (priv->tstore_where, &iter,
			                    COL_WHERE_TABLE_NAME, field->table_name,
			                    COL_WHERE_NAME, field_name,
			                    COL_WHERE_VISIBLE_NAME, g_strconcat (table->name_visible, " - ", field->name_visible, NULL),
			                    -1);

			if (with_parent)
				{
					gtk_tree_view_expand_to_path (GTK_TREE_VIEW (priv->trv_where),
					                              gtk_tree_model_get_path (GTK_TREE_MODEL (priv->tstore_where), &iter_parent));
				}

			gtk_tree_selection_select_iter (priv->sel_where, &iter);

			g_free (table_name);
			g_free (field_name);
		}
	else
		{
			dialog = gtk_message_dialog_new (GTK_WINDOW (gtk_widget_get_toplevel (priv->hpaned_main)),
			                                 GTK_DIALOG_DESTROY_WITH_PARENT,
			                                 GTK_MESSAGE_WARNING,
			                                 GTK_BUTTONS_OK,
			                                 _("You must select a field before."));
			gtk_dialog_run (GTK_DIALOG (dialog));
			gtk_widget_destroy (dialog);
		}
}

static void
gdaex_query_editor_on_btn_where_add_group_clicked (GtkButton *button,
                                    gpointer user_data)
{
	GdaExQueryEditor *qe;
	GdaExQueryEditorPrivate *priv;

	GtkTreeIter iter_parent;
	GtkTreeIter iter;

	gchar *group;

	qe = (GdaExQueryEditor *)user_data;
	priv = GDAEX_QUERY_EDITOR_GET_PRIVATE (qe);

	if (gtk_tree_selection_get_selected (priv->sel_where, NULL, &iter_parent))
		{
			gtk_tree_model_get (GTK_TREE_MODEL (priv->tstore_where), &iter_parent,
			                    COL_WHERE_TABLE_NAME, &group,
			                    -1);
			if (g_strcmp0 (group, GROUP) != 0)
				{
					if (gtk_tree_model_iter_parent (GTK_TREE_MODEL (priv->tstore_where), &iter, &iter_parent))
						{
							gtk_tree_store_append (priv->tstore_where, &iter, &iter);
						}
					else
						{
							gtk_tree_store_append (priv->tstore_where, &iter, NULL);
						}
				}
			else
				{
					gtk_tree_store_append (priv->tstore_where, &iter, &iter_parent);
				}
		}
	else
		{
			gtk_tree_store_append (priv->tstore_where, &iter, NULL);
		}
	gtk_tree_store_set (priv->tstore_where, &iter,
	                    COL_WHERE_TABLE_NAME, GROUP,
	                    COL_WHERE_NAME, GROUP,
	                    COL_WHERE_VISIBLE_NAME, "(...)",
	                    -1);
	gtk_tree_view_expand_to_path (GTK_TREE_VIEW (priv->trv_where),
	                              gtk_tree_model_get_path (GTK_TREE_MODEL (priv->tstore_where), &iter));
}

static void
gdaex_query_editor_on_btn_where_remove_clicked (GtkButton *button,
                                    gpointer user_data)
{
	GdaExQueryEditor *qe;
	GdaExQueryEditorPrivate *priv;

	qe = (GdaExQueryEditor *)user_data;
	priv = GDAEX_QUERY_EDITOR_GET_PRIVATE (qe);

	gdaex_query_editor_store_remove_iter (qe,
	                                      priv->sel_where,
	                                      G_OBJECT (priv->tstore_where));
}

static void
gdaex_query_editor_on_btn_where_up_clicked (GtkButton *button,
                                    gpointer user_data)
{
	GdaExQueryEditor *qe;
	GdaExQueryEditorPrivate *priv;

	qe = (GdaExQueryEditor *)user_data;
	priv = GDAEX_QUERY_EDITOR_GET_PRIVATE (qe);

	gdaex_query_editor_store_move_iter_up_down (qe,
	                                      priv->sel_where,
	                                      G_OBJECT (priv->tstore_where),
	                                      TRUE);
}

static void
gdaex_query_editor_on_btn_where_down_clicked (GtkButton *button,
                                    gpointer user_data)
{
	GdaExQueryEditor *qe;
	GdaExQueryEditorPrivate *priv;

	qe = (GdaExQueryEditor *)user_data;
	priv = GDAEX_QUERY_EDITOR_GET_PRIVATE (qe);

	gdaex_query_editor_store_move_iter_up_down (qe,
	                                      priv->sel_where,
	                                      G_OBJECT (priv->tstore_where),
	                                      FALSE);
}

static void
gdaex_query_editor_on_btn_where_clean_clicked (GtkButton *button,
                                    gpointer user_data)
{
	GdaExQueryEditor *qe = (GdaExQueryEditor *)user_data;
	GdaExQueryEditorPrivate *priv = GDAEX_QUERY_EDITOR_GET_PRIVATE (qe);

	gtk_tree_store_clear (priv->tstore_where);

	gtk_button_clicked (GTK_BUTTON (gtk_builder_get_object (priv->gtkbuilder, "button16")));
}

static void
gdaex_query_editor_on_sel_where_changed (GtkTreeSelection *treeselection,
                                         gpointer user_data)
{
	GtkTreeIter iter;
	GtkTreePath *path;
	gint *indices;

	gboolean is_group;

	guint link_type;
	gchar *table_name;
	gchar *field_name;
	gboolean not;
	guint where_type;
	gchar *from;
	gchar *to;
	gchar *from_sql;
	gchar *to_sql;

	GdaExQueryEditorTable *table;
	GdaExQueryEditorField *field;

	GtkWidget *lbl;

	GtkTreeIter iter_cb;
	guint link_type_cb;
	guint where_type_cb;

	GtkCellRenderer *renderer;

	GdaExQueryEditor *qe = (GdaExQueryEditor *)user_data;
	GdaExQueryEditorPrivate *priv = GDAEX_QUERY_EDITOR_GET_PRIVATE (qe);

	gtk_widget_hide (priv->vbx_values_container);
	gdaex_query_editor_remove_child_from_vbx_values (qe);

	if (gtk_tree_selection_get_selected (priv->sel_where, NULL, &iter))
		{
			priv->editor_type = GDAEX_QE_PAGE_WHERE;

			gtk_tree_selection_unselect_all (priv->sel_show);
			gtk_tree_selection_unselect_all (priv->sel_order);

			gtk_tree_model_get (GTK_TREE_MODEL (priv->tstore_where), &iter,
			                    COL_WHERE_LINK_TYPE, &link_type,
			                    COL_WHERE_TABLE_NAME, &table_name,
			                    COL_WHERE_NAME, &field_name,
			                    COL_WHERE_CONDITION_NOT, &not,
			                    COL_WHERE_CONDITION_TYPE, &where_type,
			                    COL_WHERE_CONDITION_FROM, &from,
			                    COL_WHERE_CONDITION_FROM_SQL, &from_sql,
			                    COL_WHERE_CONDITION_TO, &to,
			                    COL_WHERE_CONDITION_TO_SQL, &to_sql,
			                    -1);

			is_group = (g_strcmp0 (table_name, GROUP) == 0);
			if (!is_group)
				{
					table = g_hash_table_lookup (priv->tables, table_name);
					field = g_hash_table_lookup (table->fields, field_name);
				}

			path = gtk_tree_model_get_path (GTK_TREE_MODEL (priv->tstore_where), &iter);
			indices = gtk_tree_path_get_indices (path);

			if (GTK_IS_BOX (priv->hbox_where))
				{
					gtk_combo_box_set_active_iter (GTK_COMBO_BOX (priv->cb_link_type), NULL);
					gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (priv->chk_not), not);
					gtk_container_remove (GTK_CONTAINER (priv->tbl), GTK_WIDGET (priv->txt_from));
					gtk_container_remove (GTK_CONTAINER (priv->tbl), GTK_WIDGET (priv->txt_to));
				}
			else
				{
					priv->hbox_where = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0);

					priv->frame_where = gtk_frame_new ("");
					g_object_set (G_OBJECT (priv->frame_where),
								  "margin", 5,
								  NULL);
					gtk_frame_set_shadow_type (GTK_FRAME (priv->frame_where), GTK_SHADOW_ETCHED_IN);
					gtk_box_pack_start (GTK_BOX (priv->hbox_where), priv->frame_where, TRUE, TRUE, 0);

					priv->tbl = gtk_grid_new ();
					g_object_set (G_OBJECT (priv->tbl),
								  "margin", 5,
								  NULL);
					gtk_grid_set_row_spacing (GTK_GRID (priv->tbl), 5);
					gtk_grid_set_column_spacing (GTK_GRID (priv->tbl), 5);
					gtk_container_add (GTK_CONTAINER (priv->frame_where), priv->tbl);

					/* if it is the first condition, "link" isn't visibile */
					priv->lbl_link_type = gtk_label_new (_("Link"));
					gtk_widget_set_no_show_all (priv->lbl_link_type, TRUE);
					gtk_grid_attach (GTK_GRID (priv->tbl), priv->lbl_link_type, 0, 0, 1, 1);

					priv->lbl_not = gtk_label_new (_("Not"));
					gtk_grid_attach (GTK_GRID (priv->tbl), priv->lbl_not, 2, 0, 1, 1);

					priv->lbl_where_type = gtk_label_new (_("Condition"));
					gtk_grid_attach (GTK_GRID (priv->tbl), priv->lbl_where_type, 3, 0, 1, 1);

					priv->lbl_from = gtk_label_new (_("Value"));
					gtk_grid_attach (GTK_GRID (priv->tbl), priv->lbl_from, 4, 0, 1, 1);

					/* link */
					priv->cb_link_type = gtk_combo_box_new_with_model (GTK_TREE_MODEL (priv->lstore_link_type));
					gtk_widget_set_no_show_all (priv->cb_link_type, TRUE);

					renderer = gtk_cell_renderer_text_new ();
					gtk_cell_layout_pack_start (GTK_CELL_LAYOUT (priv->cb_link_type), renderer, TRUE);
					gtk_cell_layout_add_attribute (GTK_CELL_LAYOUT (priv->cb_link_type), renderer, "text", 1);

					gtk_list_store_clear (priv->lstore_link_type);

					gtk_list_store_append (priv->lstore_link_type, &iter_cb);
					gtk_list_store_set (priv->lstore_link_type, &iter_cb,
					                    0, GDAEX_QE_LINK_TYPE_AND,
					                    1, gdaex_query_editor_get_link_type_str_from_type (GDAEX_QE_LINK_TYPE_AND),
					                    -1);
					gtk_list_store_append (priv->lstore_link_type, &iter_cb);
					gtk_list_store_set (priv->lstore_link_type, &iter_cb,
					                    0, GDAEX_QE_LINK_TYPE_OR,
					                    1, gdaex_query_editor_get_link_type_str_from_type (GDAEX_QE_LINK_TYPE_OR),
					                    -1);
					gtk_grid_attach (GTK_GRID (priv->tbl), priv->cb_link_type, 0, 1, 1, 1);

					/* not */
					priv->chk_not = gtk_check_button_new ();
					gtk_grid_attach (GTK_GRID (priv->tbl), priv->chk_not, 2, 1, 1, 1);

					/* where */
					priv->cb_where_type = gtk_combo_box_new_with_model (GTK_TREE_MODEL (priv->lstore_where_type));

					renderer = gtk_cell_renderer_text_new ();
					gtk_cell_layout_pack_start (GTK_CELL_LAYOUT (priv->cb_where_type), renderer, TRUE);
					gtk_cell_layout_add_attribute (GTK_CELL_LAYOUT (priv->cb_where_type), renderer, "text", 1);

					gtk_grid_attach (GTK_GRID (priv->tbl), priv->cb_where_type, 3, 1, 1, 1);

					g_signal_connect (G_OBJECT (priv->cb_where_type), "changed",
					                  G_CALLBACK (gdaex_query_editor_on_cb_where_type_changed), user_data);

					priv->lbl_to = gtk_label_new (_("and"));
					gtk_label_set_xalign (GTK_LABEL (priv->lbl_to), 1.0);
					gtk_label_set_yalign (GTK_LABEL (priv->lbl_to), 0.5);
					gtk_grid_attach (GTK_GRID (priv->tbl), priv->lbl_to, 5, 1, 1, 1);
				}

			if (is_group)
				{
					gtk_label_set_label (GTK_LABEL (gtk_frame_get_label_widget (GTK_FRAME (priv->frame_where))), "");
				}
			else
				{
					gtk_label_set_markup (GTK_LABEL (gtk_frame_get_label_widget (GTK_FRAME (priv->frame_where))),
										  g_strconcat ("<b>", table->name_visible, " - ", field->name_visible, "</b>", NULL));
				}

			/* if it is the first condition, "link" isn't visibile */
			if (indices[gtk_tree_path_get_depth (path) - 1] != 0)
				{
					if (link_type != 0
					    && gtk_tree_model_get_iter_first (GTK_TREE_MODEL (priv->lstore_link_type), &iter_cb))
						{
							do
								{
									gtk_tree_model_get (GTK_TREE_MODEL (priv->lstore_link_type), &iter_cb,
									                    0, &link_type_cb,
									                    -1);
									if (link_type == link_type_cb)
										{
											gtk_combo_box_set_active_iter (GTK_COMBO_BOX (priv->cb_link_type), &iter_cb);
											break;
										}
								} while (gtk_tree_model_iter_next (GTK_TREE_MODEL (priv->lstore_link_type), &iter_cb));
						}
					else
						{
							gtk_combo_box_set_active_iter (GTK_COMBO_BOX (priv->cb_link_type), NULL);
						}

					gtk_widget_show (priv->lbl_link_type);
					gtk_widget_show (priv->cb_link_type);
				}
			else
				{
					gtk_widget_hide (priv->lbl_link_type);
					gtk_widget_hide (priv->cb_link_type);
				}

			gtk_list_store_clear (priv->lstore_where_type);
			if (!is_group)
				{
					if (field->available_where_type & GDAEX_QE_WHERE_TYPE_EQUAL)
						{
							gtk_list_store_append (priv->lstore_where_type, &iter_cb);
							gtk_list_store_set (priv->lstore_where_type, &iter_cb,
							                    0, GDAEX_QE_WHERE_TYPE_EQUAL,
							                    1, gdaex_query_editor_get_where_type_str_from_type (GDAEX_QE_WHERE_TYPE_EQUAL),
							                    -1);
						}
					if (field->available_where_type & GDAEX_QE_WHERE_TYPE_STARTS)
						{
							gtk_list_store_append (priv->lstore_where_type, &iter_cb);
							gtk_list_store_set (priv->lstore_where_type, &iter_cb,
							                    0, GDAEX_QE_WHERE_TYPE_STARTS,
							                    1, gdaex_query_editor_get_where_type_str_from_type (GDAEX_QE_WHERE_TYPE_STARTS),
							                    -1);
						}
					if (field->available_where_type & GDAEX_QE_WHERE_TYPE_CONTAINS)
						{
							gtk_list_store_append (priv->lstore_where_type, &iter_cb);
							gtk_list_store_set (priv->lstore_where_type, &iter_cb,
							                    0, GDAEX_QE_WHERE_TYPE_CONTAINS,
							                    1, gdaex_query_editor_get_where_type_str_from_type (GDAEX_QE_WHERE_TYPE_CONTAINS),
							                    -1);
						}
					if (field->available_where_type & GDAEX_QE_WHERE_TYPE_ENDS)
						{
							gtk_list_store_append (priv->lstore_where_type, &iter_cb);
							gtk_list_store_set (priv->lstore_where_type, &iter_cb,
							                    0, GDAEX_QE_WHERE_TYPE_ENDS,
							                    1, gdaex_query_editor_get_where_type_str_from_type (GDAEX_QE_WHERE_TYPE_ENDS),
							                    -1);
						}
					if (field->available_where_type & GDAEX_QE_WHERE_TYPE_ISTARTS)
						{
							gtk_list_store_append (priv->lstore_where_type, &iter_cb);
							gtk_list_store_set (priv->lstore_where_type, &iter_cb,
							                    0, GDAEX_QE_WHERE_TYPE_ISTARTS,
							                    1, gdaex_query_editor_get_where_type_str_from_type (GDAEX_QE_WHERE_TYPE_ISTARTS),
							                    -1);
						}
					if (field->available_where_type & GDAEX_QE_WHERE_TYPE_ICONTAINS)
						{
							gtk_list_store_append (priv->lstore_where_type, &iter_cb);
							gtk_list_store_set (priv->lstore_where_type, &iter_cb,
					                    0, GDAEX_QE_WHERE_TYPE_ICONTAINS,
					                    1, gdaex_query_editor_get_where_type_str_from_type (GDAEX_QE_WHERE_TYPE_ICONTAINS),
					                    -1);
						}
					if (field->available_where_type & GDAEX_QE_WHERE_TYPE_IENDS)
						{
							gtk_list_store_append (priv->lstore_where_type, &iter_cb);
							gtk_list_store_set (priv->lstore_where_type, &iter_cb,
							                    0, GDAEX_QE_WHERE_TYPE_IENDS,
							                    1, gdaex_query_editor_get_where_type_str_from_type (GDAEX_QE_WHERE_TYPE_IENDS),
							                    -1);
						}
					if (field->available_where_type & GDAEX_QE_WHERE_TYPE_GREAT)
						{
							gtk_list_store_append (priv->lstore_where_type, &iter_cb);
							gtk_list_store_set (priv->lstore_where_type, &iter_cb,
							                    0, GDAEX_QE_WHERE_TYPE_GREAT,
							                    1, gdaex_query_editor_get_where_type_str_from_type (GDAEX_QE_WHERE_TYPE_GREAT),
							                    -1);
						}
					if (field->available_where_type & GDAEX_QE_WHERE_TYPE_GREAT_EQUAL)
						{
							gtk_list_store_append (priv->lstore_where_type, &iter_cb);
							gtk_list_store_set (priv->lstore_where_type, &iter_cb,
							                    0, GDAEX_QE_WHERE_TYPE_GREAT_EQUAL,
							                    1, gdaex_query_editor_get_where_type_str_from_type (GDAEX_QE_WHERE_TYPE_GREAT_EQUAL),
							                    -1);
						}
					if (field->available_where_type & GDAEX_QE_WHERE_TYPE_LESS)
						{
							gtk_list_store_append (priv->lstore_where_type, &iter_cb);
							gtk_list_store_set (priv->lstore_where_type, &iter_cb,
							                    0, GDAEX_QE_WHERE_TYPE_LESS,
							                    1, gdaex_query_editor_get_where_type_str_from_type (GDAEX_QE_WHERE_TYPE_LESS),
							                    -1);
						}
					if (field->available_where_type & GDAEX_QE_WHERE_TYPE_LESS_EQUAL)
						{
							gtk_list_store_append (priv->lstore_where_type, &iter_cb);
							gtk_list_store_set (priv->lstore_where_type, &iter_cb,
							                    0, GDAEX_QE_WHERE_TYPE_LESS_EQUAL,
							                    1, gdaex_query_editor_get_where_type_str_from_type (GDAEX_QE_WHERE_TYPE_LESS_EQUAL),
							                    -1);
						}
					if (field->available_where_type & GDAEX_QE_WHERE_TYPE_BETWEEN)
						{
							gtk_list_store_append (priv->lstore_where_type, &iter_cb);
							gtk_list_store_set (priv->lstore_where_type, &iter_cb,
							                    0, GDAEX_QE_WHERE_TYPE_BETWEEN,
							                    1, gdaex_query_editor_get_where_type_str_from_type (GDAEX_QE_WHERE_TYPE_BETWEEN),
							                    -1);
						}
					if (field->available_where_type & GDAEX_QE_WHERE_TYPE_IS_NULL)
						{
							gtk_list_store_append (priv->lstore_where_type, &iter_cb);
							gtk_list_store_set (priv->lstore_where_type, &iter_cb,
							                    0, GDAEX_QE_WHERE_TYPE_IS_NULL,
							                    1, gdaex_query_editor_get_where_type_str_from_type (GDAEX_QE_WHERE_TYPE_IS_NULL),
							                    -1);
						}
				}

			if (!is_group)
				{
					switch (field->type)
						{
							case GDAEX_QE_FIELD_TYPE_TEXT:
							case GDAEX_QE_FIELD_TYPE_INTEGER:
							case GDAEX_QE_FIELD_TYPE_DOUBLE:
							case GDAEX_QE_FIELD_TYPE_DATE:
							case GDAEX_QE_FIELD_TYPE_DATETIME:
							case GDAEX_QE_FIELD_TYPE_TIME:
								priv->txt_from = GTK_WIDGET (field->iwidget_from);
								g_object_ref (G_OBJECT (field->iwidget_from));
								priv->txt_to = GTK_WIDGET (field->iwidget_to);
								g_object_ref (G_OBJECT (field->iwidget_to));
								break;

							default:
								priv->txt_from = NULL;
								priv->txt_to = NULL;
								g_warning (_("Field's type «%d» not valid."), field->type);
								break;
						};

					gtk_grid_attach (GTK_GRID (priv->tbl), priv->txt_from, 4, 1, 1, 1);
					gtk_grid_attach (GTK_GRID (priv->tbl), priv->txt_to, 6, 1, 1, 1);

					if (gtk_tree_model_get_iter_first (GTK_TREE_MODEL (priv->lstore_where_type), &iter_cb))
						{
							if (where_type == 0)
								{
									/* if no where type selected and if only one where type, it is immediately selected */
									if (gtk_tree_model_iter_n_children (GTK_TREE_MODEL (priv->lstore_where_type), NULL) == 1)
										{
											gtk_combo_box_set_active_iter (GTK_COMBO_BOX (priv->cb_where_type), &iter_cb);
										}
								}
							else
								{
									do
										{
											gtk_tree_model_get (GTK_TREE_MODEL (priv->lstore_where_type), &iter_cb,
											                    0, &where_type_cb,
											                    -1);

											if (where_type == where_type_cb)
												{
													gtk_combo_box_set_active_iter (GTK_COMBO_BOX (priv->cb_where_type), &iter_cb);
													break;
												}
										} while (gtk_tree_model_iter_next (GTK_TREE_MODEL (priv->lstore_where_type), &iter_cb));
								}
						}

					switch (field->type)
						{
							case GDAEX_QE_FIELD_TYPE_TEXT:
							case GDAEX_QE_FIELD_TYPE_INTEGER:
							case GDAEX_QE_FIELD_TYPE_DOUBLE:
							case GDAEX_QE_FIELD_TYPE_DATE:
							case GDAEX_QE_FIELD_TYPE_DATETIME:
							case GDAEX_QE_FIELD_TYPE_TIME:
								gdaex_query_editor_iwidget_set_value (field->iwidget_from, from_sql == NULL ? "" : from_sql);
								gdaex_query_editor_iwidget_set_value (field->iwidget_to, to_sql == NULL ? "" : to_sql);
								break;

							default:
								break;
						};

					gtk_widget_set_visible (priv->lbl_from, where_type != GDAEX_QE_WHERE_TYPE_IS_NULL);
					gtk_widget_set_visible (priv->txt_from, where_type != GDAEX_QE_WHERE_TYPE_IS_NULL);

					gtk_widget_set_visible (priv->lbl_to, where_type == GDAEX_QE_WHERE_TYPE_BETWEEN);
					gtk_widget_set_visible (priv->txt_to, where_type == GDAEX_QE_WHERE_TYPE_BETWEEN);

					gtk_widget_set_visible (gtk_frame_get_label_widget (GTK_FRAME (priv->frame_where)), TRUE);
					gtk_widget_set_visible (priv->lbl_where_type, TRUE);
					gtk_widget_set_visible (priv->cb_where_type, TRUE);
					gtk_widget_set_visible (priv->lbl_from, TRUE);
					gtk_widget_set_visible (priv->txt_from, TRUE);
				}
			else
				{
					priv->txt_from = NULL;
					priv->txt_to = NULL;

					gtk_widget_set_visible (gtk_frame_get_label_widget (GTK_FRAME (priv->frame_where)), FALSE);
					gtk_widget_set_visible (priv->lbl_where_type, FALSE);
					gtk_widget_set_visible (priv->cb_where_type, FALSE);
					gtk_widget_set_visible (priv->lbl_from, FALSE);
					gtk_widget_set_visible (priv->txt_from, FALSE);
					gtk_widget_set_visible (priv->lbl_to, FALSE);
					gtk_widget_set_visible (priv->txt_to, FALSE);
				}

			gtk_box_pack_start (GTK_BOX (priv->vbx_values), priv->hbox_where, FALSE, FALSE, 0);

			gtk_widget_show (priv->lbl_not);
			gtk_widget_show (priv->chk_not);
			gtk_widget_show (priv->tbl);
			gtk_widget_show (priv->frame_where);
			gtk_widget_show (priv->hbox_where);
			gtk_widget_show (priv->vbx_values);
			gtk_widget_show (priv->vbx_values_container);
		}
}

static void
gdaex_query_editor_on_btn_order_add_clicked (GtkButton *button,
                                    gpointer user_data)
{
	GdaExQueryEditor *qe;
	GdaExQueryEditorPrivate *priv;

	GtkTreeIter iter;
	GtkWidget *dialog;

	gchar *table_name;
	gchar *field_name;

	GdaExQueryEditorTable *table;
	GdaExQueryEditorField *field;

	qe = (GdaExQueryEditor *)user_data;
	priv = GDAEX_QUERY_EDITOR_GET_PRIVATE (qe);

	if (gtk_tree_selection_get_selected (priv->sel_fields, NULL, &iter))
		{
			gtk_tree_model_get (GTK_TREE_MODEL (priv->tstore_fields), &iter,
			                    COL_FIELDS_TABLE_NAME, &table_name,
			                    COL_FIELDS_NAME, &field_name,
			                    -1);

			table = g_hash_table_lookup (priv->tables, table_name);
			field = g_hash_table_lookup (table->fields, field_name);

			gtk_list_store_append (priv->lstore_order, &iter);
			gtk_list_store_set (priv->lstore_order, &iter,
			                    COL_ORDER_TABLE_NAME, field->table_name,
			                    COL_ORDER_NAME, field_name,
			                    COL_ORDER_VISIBLE_NAME, g_strconcat (table->name_visible, " - ", field->name_visible, NULL),
			                    COL_ORDER_ORDER, "ASC",
			                    -1);

			gtk_tree_selection_select_iter (priv->sel_order, &iter);

			gdaex_query_editor_on_sel_fields_changed (NULL, user_data);

			g_free (table_name);
			g_free (field_name);
		}
	else
		{
			dialog = gtk_message_dialog_new (GTK_WINDOW (gtk_widget_get_toplevel (priv->hpaned_main)),
			                                 GTK_DIALOG_DESTROY_WITH_PARENT,
			                                 GTK_MESSAGE_WARNING,
			                                 GTK_BUTTONS_OK,
			                                 _("You must select a field before."));
			gtk_dialog_run (GTK_DIALOG (dialog));
			gtk_widget_destroy (dialog);
		}
}

static void
gdaex_query_editor_on_btn_order_remove_clicked (GtkButton *button,
                                    gpointer user_data)
{
	GdaExQueryEditor *qe = (GdaExQueryEditor *)user_data;
	GdaExQueryEditorPrivate *priv = GDAEX_QUERY_EDITOR_GET_PRIVATE (qe);

	gdaex_query_editor_store_remove_iter (qe,
	                                      priv->sel_order,
	                                      G_OBJECT (priv->lstore_order));
}

static void
gdaex_query_editor_on_btn_order_up_clicked (GtkButton *button,
                                    gpointer user_data)
{
	GdaExQueryEditor *qe = (GdaExQueryEditor *)user_data;
	GdaExQueryEditorPrivate *priv = GDAEX_QUERY_EDITOR_GET_PRIVATE (qe);

	gdaex_query_editor_store_move_iter_up_down (qe,
	                                      priv->sel_order,
	                                      G_OBJECT (priv->lstore_order),
	                                      TRUE);
}

static void
gdaex_query_editor_on_btn_order_down_clicked (GtkButton *button,
                                    gpointer user_data)
{
	GdaExQueryEditor *qe = (GdaExQueryEditor *)user_data;
	GdaExQueryEditorPrivate *priv = GDAEX_QUERY_EDITOR_GET_PRIVATE (qe);

	gdaex_query_editor_store_move_iter_up_down (qe,
	                                      priv->sel_order,
	                                      G_OBJECT (priv->lstore_order),
	                                      FALSE);
}

static void
gdaex_query_editor_on_btn_order_clean_clicked (GtkButton *button,
                                    gpointer user_data)
{
	GdaExQueryEditor *qe = (GdaExQueryEditor *)user_data;
	GdaExQueryEditorPrivate *priv = GDAEX_QUERY_EDITOR_GET_PRIVATE (qe);

	gtk_list_store_clear (priv->lstore_order);

	gtk_button_clicked (GTK_BUTTON (gtk_builder_get_object (priv->gtkbuilder, "button16")));
}

static void
gdaex_query_editor_on_sel_order_changed (GtkTreeSelection *treeselection,
                                         gpointer user_data)
{
	GtkTreeIter iter;

	gchar *table_name;
	gchar *field_name;
	gchar *order;

	GtkWidget *tbl;
	GtkWidget *lbl;

	GdaExQueryEditorTable *table;
	GdaExQueryEditorField *field;

	GdaExQueryEditor *qe = (GdaExQueryEditor *)user_data;
	GdaExQueryEditorPrivate *priv = GDAEX_QUERY_EDITOR_GET_PRIVATE (qe);

	gtk_widget_hide (priv->vbx_values_container);
	gdaex_query_editor_remove_child_from_vbx_values (qe);

	if (gtk_tree_selection_get_selected (priv->sel_order, NULL, &iter))
		{
			priv->editor_type = GDAEX_QE_PAGE_ORDER;

			gtk_tree_selection_unselect_all (priv->sel_show);
			gtk_tree_selection_unselect_all (priv->sel_where);

			gtk_tree_model_get (GTK_TREE_MODEL (priv->lstore_order), &iter,
			                    COL_ORDER_TABLE_NAME, &table_name,
			                    COL_ORDER_NAME, &field_name,
			                    COL_ORDER_ORDER, &order,
			                    -1);

			table = g_hash_table_lookup (priv->tables, table_name);
			field = g_hash_table_lookup (table->fields, field_name);

			if (!GTK_IS_BOX (priv->hbox_order))
				{
					priv->hbox_order = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 5);

					priv->frame_order = gtk_frame_new ("");
					g_object_set (G_OBJECT (priv->frame_order),
								  "margin", 5,
								  NULL);
					gtk_frame_set_shadow_type (GTK_FRAME (priv->frame_order), GTK_SHADOW_ETCHED_IN);
					gtk_box_pack_start (GTK_BOX (priv->hbox_order), priv->frame_order, TRUE, TRUE, 0);

					tbl = gtk_grid_new ();
					g_object_set (G_OBJECT (tbl),
								  "margin", 5,
								  NULL);
					gtk_grid_set_row_spacing (GTK_GRID (tbl), 5);
					gtk_grid_set_column_spacing (GTK_GRID (tbl), 5);
					gtk_container_add (GTK_CONTAINER (priv->frame_order), tbl);

					priv->opt_asc = gtk_radio_button_new_with_label (NULL, _("Ascending"));
					gtk_grid_attach (GTK_GRID (tbl), priv->opt_asc, 0, 0, 1, 1);

					priv->opt_desc = gtk_radio_button_new_with_label_from_widget (GTK_RADIO_BUTTON (priv->opt_asc), _("Descending"));
					gtk_grid_attach (GTK_GRID (tbl), priv->opt_desc, 1, 0, 1, 1);
				}

			gtk_label_set_markup (GTK_LABEL (gtk_frame_get_label_widget (GTK_FRAME (priv->frame_order))),
								  g_strconcat ("<b>", table->name_visible, " - ", field->name_visible, "</b>", NULL));
			if (g_strcmp0 (order, "ASC") == 0)
				{
					gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (priv->opt_asc), TRUE);
				}
			else
				{
					gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (priv->opt_desc), TRUE);
				}

			gtk_box_pack_start (GTK_BOX (priv->vbx_values), priv->hbox_order, FALSE, FALSE, 0);

			gtk_widget_show_all (priv->vbx_values);
			gtk_widget_show (priv->vbx_values_container);

			g_free (order);
		}
}

static void
gdaex_query_editor_on_txt1_btn_browse_clicked (gpointer instance, gpointer user_data)
{
	g_debug ("Txt1 Open clicked.");
}

static void
gdaex_query_editor_on_txt2_btn_browse_clicked (gpointer instance, gpointer user_data)
{
	g_debug ("Txt2 Open clicked.");
}
