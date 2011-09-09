#include <stdlib.h>
#include <string.h>
#include <thunarx/thunarx.h>
#include "thunar-plugger.h"

static void
thunar_plugger_file_dispose (GObject *object)
{

}

static void
thunar_plugger_file_finalize (GObject *object)
{
  ThunarPluggerFile *file = THUNAR_PLUGGER_FILE (object);
  g_object_unref (file->gfile);
}

static void
thunar_plugger_file_class_init (ThunarPluggerFileClass *klass)
{
  GObjectClass *gobject_class;

  gobject_class = G_OBJECT_CLASS (klass);
  gobject_class->dispose = thunar_plugger_file_dispose;
  gobject_class->finalize = thunar_plugger_file_finalize;
}

static void
thunar_plugger_file_init (ThunarPluggerFile *file)
{

}

static gchar *
thunar_file_info_get_name (ThunarxFileInfo *file_info)
{
  return g_file_get_path(THUNAR_PLUGGER_FILE (file_info)->gfile);
}

static gchar*
thunar_file_info_get_uri (ThunarxFileInfo *file_info)
{
  return g_file_get_uri (THUNAR_PLUGGER_FILE (file_info)->gfile);
}

static gchar*
thunar_file_info_get_parent_uri (ThunarxFileInfo *file_info)
{
  GFile *parent;
  gchar *uri = NULL;

  parent = g_file_get_parent (THUNAR_PLUGGER_FILE (file_info)->gfile);
  if (G_LIKELY (parent != NULL))
    {
      uri = g_file_get_uri (parent);
      g_object_unref (parent);
    }

  return uri;
}

static gchar*
thunar_file_info_get_uri_scheme (ThunarxFileInfo *file_info)
{
  return g_file_get_uri_scheme (THUNAR_PLUGGER_FILE (file_info)->gfile);
}

static gchar*
thunar_file_info_get_mime_type (ThunarxFileInfo *file_info)
{
  return NULL;
}

static gboolean
thunar_file_info_has_mime_type (ThunarxFileInfo *file_info,
                                const gchar     *mime_type)
{
  return FALSE;
}

static gboolean
thunar_file_info_is_directory (ThunarxFileInfo *file_info)
{
  GFileType ftype;

  ftype = g_file_query_file_type(THUNAR_PLUGGER_FILE (file_info)->gfile,
				 G_FILE_QUERY_INFO_NONE,
				 NULL);
  if (G_FILE_TYPE_DIRECTORY == ftype) {
    return TRUE;
  } else {
    return FALSE;
  }
}

static GFileInfo *
thunar_file_info_get_file_info (ThunarxFileInfo *file_info)
{
  GError *error;
  return g_file_query_info(THUNAR_PLUGGER_FILE(file_info)->gfile,
			   "*",
			   G_FILE_QUERY_INFO_NONE,
			   NULL,
			   &error);
}

static GFileInfo *
thunar_file_info_get_filesystem_info (ThunarxFileInfo *file_info)
{

  return g_file_query_filesystem_info (THUNAR_PLUGGER_FILE (file_info)->gfile, 
                                       THUNARX_FILESYSTEM_INFO_NAMESPACE,
                                       NULL, NULL);
}

static GFile *
thunar_file_info_get_location (ThunarxFileInfo *file_info)
{
  return g_object_ref (THUNAR_PLUGGER_FILE (file_info)->gfile);
}

static void
thunar_file_info_changed (ThunarxFileInfo *file_info)
{

}

static void
thunar_file_info_init (ThunarxFileInfoIface *iface)
{
  iface->get_name = thunar_file_info_get_name;
  iface->get_uri = thunar_file_info_get_uri;
  iface->get_parent_uri = thunar_file_info_get_parent_uri;
  iface->get_uri_scheme = thunar_file_info_get_uri_scheme;
  iface->get_mime_type = thunar_file_info_get_mime_type;
  iface->has_mime_type = thunar_file_info_has_mime_type;
  iface->is_directory = thunar_file_info_is_directory;
  iface->get_file_info = thunar_file_info_get_file_info;
  iface->get_filesystem_info = thunar_file_info_get_filesystem_info;
  iface->get_location = thunar_file_info_get_location;
  iface->changed = thunar_file_info_changed;
}

void
thunar_plugger_file_destroy (ThunarPluggerFile *file)
{
  g_object_run_dispose (G_OBJECT (file));
}

G_DEFINE_TYPE_WITH_CODE (ThunarPluggerFile, thunar_plugger_file, G_TYPE_OBJECT,
    G_IMPLEMENT_INTERFACE (THUNARX_TYPE_FILE_INFO, thunar_file_info_init))

ThunarPluggerFile*
thunar_plugger_file_get (const char *path)
{
  GFile *gfile;
  ThunarPluggerFile *file;

  file = g_object_new (THUNAR_PLUGGER_TYPE_FILE, NULL);
  gfile = g_file_new_for_commandline_arg(path);
  file->gfile = g_object_ref(gfile);
  g_object_unref (gfile);

  return file;
}

int
get_property_page (ThunarxProviderFactory* f,
		   GList *flist,
		   const char *class_name,
		   GtkWidget **pp,
		   char *title,
		   int tlen)
{
  GList *ps, *lp;

  ps = thunarx_provider_factory_list_providers(f, THUNARX_TYPE_PROPERTY_PAGE_PROVIDER);

  *pp = NULL;
  for (lp = ps; lp != NULL; lp = lp->next) {
    GList *pgs, *lpg;

    pgs = thunarx_property_page_provider_get_pages(lp->data, flist);
    for (lpg = pgs; lpg != NULL && *pp == NULL; lpg = lpg->next) {
      if (strncmp(class_name, G_OBJECT_TYPE_NAME(lpg->data), 256) == 0) {
	*pp = GTK_WIDGET(g_object_ref(lpg->data));
      }
    }

    g_list_foreach (pgs, (GFunc) g_object_ref_sink, NULL);
    g_list_foreach (pgs, (GFunc) g_object_unref, NULL);
    g_list_free (pgs);
  }

  g_list_foreach (ps, (GFunc) g_object_unref, NULL);
  g_list_free (ps);

  if (*pp != NULL)
    {
      if (g_list_length(flist) == 1)
	{
	  snprintf(title,
		   tlen,
		   "%s: %s",
		   gtk_label_get_text(GTK_LABEL(thunarx_property_page_get_label_widget(THUNARX_PROPERTY_PAGE(*pp)))),
		   thunarx_file_info_get_name(THUNARX_FILE_INFO(flist->data)));
      }
    else
      {
	snprintf(title,
		 tlen,
		 "%s",
		 gtk_label_get_text(GTK_LABEL(thunarx_property_page_get_label_widget(THUNARX_PROPERTY_PAGE(*pp)))));
      }
    }

  return *pp != NULL;
}

GList*
g_list_merge (GList *dst, GList *src)
{
  for (src; src != NULL; src = src->next)
    {
      if (g_list_find(dst, src->data) == NULL)
	{
	  dst = g_list_append(dst, g_object_ref(src->data));
	}
    }

  return dst;
}

int
get_actions_page (ThunarxProviderFactory* f,
		  GList *flist,
		  gboolean dirs_as_files,
		  GtkWindow *win,
		  const char *class_name,
		  GtkWidget **page,
		  char *title,
		  int tlen)
{
  GList *ps, *lp, *files, *dirs, *fas, *das, *as;
  GtkWidget *vbox;

  files = NULL;
  dirs = NULL;

  if (! dirs_as_files)
    {
      for (flist; flist != NULL; flist = flist->next)
	{
	  if (thunarx_file_info_is_directory(THUNARX_FILE_INFO(flist->data)))
	    {
	      dirs = g_list_append(dirs, flist->data);
	    }
	  else
	    {
	      files = g_list_append(files, flist->data);
	    }
	}
    }
  else
    {
      files = flist;
    }

  ps = thunarx_provider_factory_list_providers(f, THUNARX_TYPE_MENU_PROVIDER);

  snprintf(title, tlen, "Choose actions for %i selected objects\n",
	   g_list_length(files) + g_list_length(dirs));
  
  *page = gtk_frame_new (title);
  gtk_container_set_border_width (GTK_CONTAINER(*page), 10);
  vbox = gtk_vbox_new(TRUE, 10);
  gtk_container_set_border_width (GTK_CONTAINER(vbox), 10);
  gtk_container_add(GTK_CONTAINER(*page), GTK_WIDGET(vbox));

  fas = NULL;
  das = NULL;
  for (lp = ps; lp != NULL; lp = lp->next)
    {
      GList *dp, *as;
      if (strncmp(class_name, G_OBJECT_TYPE_NAME(lp->data), 256) == 0) {
	as = thunarx_menu_provider_get_file_actions(lp->data,
						    GTK_WIDGET(win),
						    files);
	fas = g_list_merge(fas, as);
	for (dp = dirs; dp != NULL; dp = dp->next)
	  {
	    GList *as;
	    as = thunarx_menu_provider_get_folder_actions(lp->data,
							  GTK_WIDGET(win),
							  dp->data);
	    das = g_list_merge(das, as);
	    g_list_foreach (as, (GFunc) g_object_unref, NULL);
	    g_list_free (as);
	  }
	g_list_foreach (as, (GFunc) g_object_unref, NULL);
	g_list_free (as);
      }
    }

  g_list_foreach (ps, (GFunc) g_object_unref, NULL);
  g_list_free (ps);

  fas = g_list_merge(fas, das);
  g_list_foreach (das, (GFunc) g_object_unref, NULL);
  g_list_free (das);
  
  if (g_list_length(fas) > 0)
    {
      GList *ap;

      for (ap = fas; ap != NULL; ap = ap->next)
	{
	  GtkWidget *b;

	  b = gtk_button_new ();
	  gtk_action_connect_proxy (GTK_ACTION(ap->data), b);
	  gtk_container_add(GTK_CONTAINER(vbox), GTK_WIDGET(b));
	  g_signal_connect_object (G_OBJECT(b),
				   "clicked",
				   G_CALLBACK(gtk_widget_destroy),
				   win,
				   G_CONNECT_AFTER | G_CONNECT_SWAPPED);
	}
      return 1;
    }
  else
    {
      gtk_container_add(GTK_CONTAINER(vbox), gtk_label_new ("No actions available"));
      return 1;
    }
}

/* Command-line options */

const char *class_name = NULL;
gboolean page_mode = FALSE;
gboolean menu_mode = FALSE;
gboolean dirs_as_files = FALSE;

static GOptionEntry opts[] =
{
  { "class-name",
    'c',
    G_OPTION_FLAG_IN_MAIN,
    G_OPTION_ARG_STRING,
    &class_name,
    "Name of the plugin object class",
    "CLASS" },
  { "page",
    'p',
    G_OPTION_FLAG_IN_MAIN,
    G_OPTION_ARG_NONE,
    &page_mode,
    "Search for a property page",
    NULL },
  { "menu",
    'm',
    G_OPTION_FLAG_IN_MAIN,
    G_OPTION_ARG_NONE,
    &menu_mode,
    "Search for a menu",
    NULL },
  { "dirs-as-files",
    'F',
    G_OPTION_FLAG_IN_MAIN,
    G_OPTION_ARG_NONE,
    &dirs_as_files,
    "Treat directories as files",
    NULL },
  { NULL }
};

void main(int argc, char **argv)
{
  ThunarPluggerFile *file;
  GList *flist;
  GtkWidget *win;
  char title[1024] = "";
  ThunarxProviderFactory* f;
  GtkWidget *page;
  int ret = 0;
  GOptionContext *octx;
  GError *error = NULL;
  int i;

  octx = g_option_context_new ("PATH [PATH ...] - display a Thunar plugin dialog for path(s)");
  g_option_context_add_main_entries (octx, opts, NULL);
  g_option_context_add_group (octx, gtk_get_option_group (TRUE));
  if (!g_option_context_parse (octx, &argc, &argv, &error))
    {
      fprintf (stderr, "option parsing failed: %s\n", error->message);
      exit (1);
    }

  if (class_name == NULL || strlen(class_name) == 0)
    {
      fprintf (stderr, "Specify the plugin page class name, please.\n");
      exit (1);
    } 
  
  if (argc < 2)
    {
      fprintf (stderr, "Please, specify one path at least.\n");
      exit (1);
    }

  gtk_init(&argc, &argv);

  flist = NULL;
  for (i = 1; i < argc; i++)
    {
      file = thunar_plugger_file_get(argv[i]);
      flist = g_list_append(flist, file);
    }

  win = gtk_window_new(GTK_WINDOW_TOPLEVEL);
  gtk_window_set_position (GTK_WINDOW(win), GTK_WIN_POS_CENTER_ALWAYS);
  g_signal_connect_swapped(G_OBJECT(win), "destroy",
			   G_CALLBACK(gtk_main_quit), NULL);

  f = thunarx_provider_factory_get_default();

  page = NULL;
  if (!page_mode && !menu_mode)
    {
      page_mode = TRUE;
    }
  if (page_mode)
    {
      ret = ! get_property_page(f, flist, class_name, &page, title, sizeof(title));
    }
  else if (menu_mode)
    {
      ret = ! get_actions_page (f, flist, dirs_as_files, GTK_WINDOW(win), class_name, &page, title, sizeof(title));
    }

  g_object_unref(f);

  if (page == NULL || ret) {
    GtkWidget *label;
    label = gtk_label_new ("Unable to initialize the plugin\n");
    gtk_container_add(GTK_CONTAINER(win), label);
  } else {
    gtk_container_add(GTK_CONTAINER(win), GTK_WIDGET(page));
    if (strlen(title) > 0)
      {
	gtk_window_set_title(GTK_WINDOW(win), title);
      }
  }

  gtk_widget_show_all(win);
  gtk_main();

  g_list_foreach (flist, (GFunc) g_object_unref, NULL);
  g_list_free (flist);

  exit(ret);
}

