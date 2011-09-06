#include <stdlib.h>
#include <thunarx/thunarx.h>
#include "shareman.h"

static void
shareman_file_dispose (GObject *object)
{

}

static void
shareman_file_finalize (GObject *object)
{
  SharemanFile *file = SHAREMAN_FILE (object);
  g_object_unref (file->gfile);
}

static void
shareman_file_class_init (SharemanFileClass *klass)
{
  GObjectClass *gobject_class;

  gobject_class = G_OBJECT_CLASS (klass);
  gobject_class->dispose = shareman_file_dispose;
  gobject_class->finalize = shareman_file_finalize;
}

static void
shareman_file_init (SharemanFile *file)
{

}

static gchar *
thunar_file_info_get_name (ThunarxFileInfo *file_info)
{
  return g_file_get_path(SHAREMAN_FILE (file_info)->gfile);
}

static gchar*
thunar_file_info_get_uri (ThunarxFileInfo *file_info)
{
  return g_file_get_uri (SHAREMAN_FILE (file_info)->gfile);
}

static gchar*
thunar_file_info_get_parent_uri (ThunarxFileInfo *file_info)
{
  GFile *parent;
  gchar *uri = NULL;

  parent = g_file_get_parent (SHAREMAN_FILE (file_info)->gfile);
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
  return g_file_get_uri_scheme (SHAREMAN_FILE (file_info)->gfile);
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

  ftype = g_file_query_file_type(SHAREMAN_FILE (file_info)->gfile,
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
  return g_file_query_info(SHAREMAN_FILE(file_info)->gfile,
			   "*",
			   G_FILE_QUERY_INFO_NONE,
			   NULL,
			   &error);
}

static GFileInfo *
thunar_file_info_get_filesystem_info (ThunarxFileInfo *file_info)
{

  return g_file_query_filesystem_info (SHAREMAN_FILE (file_info)->gfile, 
                                       THUNARX_FILESYSTEM_INFO_NAMESPACE,
                                       NULL, NULL);
}

static GFile *
thunar_file_info_get_location (ThunarxFileInfo *file_info)
{
  return g_object_ref (SHAREMAN_FILE (file_info)->gfile);
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
shareman_file_destroy (SharemanFile *file)
{
  g_object_run_dispose (G_OBJECT (file));
}

G_DEFINE_TYPE_WITH_CODE (SharemanFile, shareman_file, G_TYPE_OBJECT,
    G_IMPLEMENT_INTERFACE (THUNARX_TYPE_FILE_INFO, thunar_file_info_init))

SharemanFile*
shareman_file_get (const char *path)
{
  GFile *gfile;
  SharemanFile *file;

  file = g_object_new (SHAREMAN_TYPE_FILE, NULL);
  gfile = g_file_new_for_commandline_arg(path);
  file->gfile = g_object_ref(gfile);
  g_object_unref (gfile);

  return file;
}


void main(int argc, char **argv)
{
  SharemanFile *file;
  GList *flist;
  GtkWidget *win;
  char title[1024] = "";
  ThunarxProviderFactory* f;
  ThunarxPropertyPage *tsp;
  GList *ps, *lp;
  int ret = 0;
  
  gtk_init(&argc, &argv);

  file = shareman_file_get(argv[1]);
  flist = g_list_append(NULL, file);

  win = gtk_window_new(GTK_WINDOW_TOPLEVEL);
  g_signal_connect_swapped(G_OBJECT(win), "destroy",
			   G_CALLBACK(gtk_main_quit), NULL);
  gtk_widget_show(win);

  f = thunarx_provider_factory_get_default();
  ps = thunarx_provider_factory_list_providers(f, THUNARX_TYPE_PROPERTY_PAGE_PROVIDER);

  tsp = NULL;
  for (lp = ps; lp != NULL && tsp == NULL; lp = lp->next) {
    GList *pgs, *lpg;

    pgs = thunarx_property_page_provider_get_pages(lp->data, flist);
    for (lpg = pgs; lpg != NULL && tsp == NULL; lpg = lpg->next) {
      if (strncmp("TspPage", G_OBJECT_TYPE_NAME(lpg->data), 7) == 0) {
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
    fprintf(stderr, "Unable to found the Thunar Shares Plugin\n");
    fprintf(stderr, "Please, check that the thunar-shares-plugin packages is installed on your system.\n");
    ret = 1;
  } else {
    ret = 0;
    gtk_container_add(GTK_CONTAINER(win), GTK_WIDGET(tsp));
    snprintf(title,
	     sizeof(title),
	     "%s: %s",
	     gtk_label_get_text(GTK_LABEL(thunarx_property_page_get_label_widget(tsp))),
	     thunarx_file_info_get_name(THUNARX_FILE_INFO(file)));
    gtk_window_set_title(GTK_WINDOW(win), title);
    gtk_widget_show_all(win);
    gtk_main();
  }

  g_list_foreach (flist, (GFunc) g_object_unref, NULL);
  g_list_free (flist);

  exit(ret);
}

