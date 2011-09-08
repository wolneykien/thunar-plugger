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

/* Command-line options */

const char *class_name = NULL;

static GOptionEntry opts[] =
{
  { "class-name",
    'c',
    G_OPTION_FLAG_IN_MAIN,
    G_OPTION_ARG_STRING,
    &class_name,
    "Name of the plugin page class",
    "CLASS" },
  { NULL }
};

void main(int argc, char **argv)
{
  ThunarPluggerFile *file;
  GList *flist;
  GtkWidget *win;
  char title[1024] = "";
  ThunarxProviderFactory* f;
  ThunarxPropertyPage *tsp;
  GList *ps, *lp;
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
  ps = thunarx_provider_factory_list_providers(f, THUNARX_TYPE_PROPERTY_PAGE_PROVIDER);

  tsp = NULL;
  for (lp = ps; lp != NULL && tsp == NULL; lp = lp->next) {
    GList *pgs, *lpg;

    pgs = thunarx_property_page_provider_get_pages(lp->data, flist);
    for (lpg = pgs; lpg != NULL && tsp == NULL; lpg = lpg->next) {
      if (strncmp(class_name, G_OBJECT_TYPE_NAME(lpg->data), 256) == 0) {
	tsp = THUNARX_PROPERTY_PAGE(g_object_ref(lpg->data));
      }
    }

    g_list_foreach (pgs, (GFunc) g_object_ref_sink, NULL);
    g_list_foreach (pgs, (GFunc) g_object_unref, NULL);
    g_list_free (pgs);
  }

  g_list_foreach (ps, (GFunc) g_object_unref, NULL);
  g_list_free (ps);

  g_object_unref(f);

  if (tsp == NULL) {
    GtkWidget *label;
    label = gtk_label_new ("Unable to initialize the plugin\n");
    gtk_container_add(GTK_CONTAINER(win), label);
    ret = 1;
  } else {
    ret = 0;
    gtk_container_add(GTK_CONTAINER(win), GTK_WIDGET(tsp));
    if (g_list_length(flist) == 1)
      {
	snprintf(title,
		 sizeof(title),
		 "%s: %s",
		 gtk_label_get_text(GTK_LABEL(thunarx_property_page_get_label_widget(tsp))),
		 thunarx_file_info_get_name(THUNARX_FILE_INFO(file)));
	gtk_window_set_title(GTK_WINDOW(win), title);
      }
    else
      {
	gtk_window_set_title(GTK_WINDOW(win),
			     gtk_label_get_text(GTK_LABEL(thunarx_property_page_get_label_widget(tsp))));
      }
  }

  gtk_widget_show_all(win);
  gtk_main();

  g_list_foreach (flist, (GFunc) g_object_unref, NULL);
  g_list_free (flist);

  exit(ret);
}

