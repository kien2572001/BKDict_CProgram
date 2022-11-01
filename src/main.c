#include <gtk/gtk.h>
#include <string.h>
#include "inc/btree.h"
#include "soundex.h"

//Quy dinh trang thai cua phan mem: 0 la tim kiem 1 la them tu 2 la sua nghia cua tu 3 la xoa tu
static int status=0;

//Tao giao dien
GtkBuilder      *builder;
GtkWidget       *window;

//Xu li Btree
BTA *btfile;
void docFile();

GtkWidget *g_word_label;
GtkWidget *g_meaning_textview;
GtkTextBuffer *g_text_buffer;
GtkWidget *g_search_entry;
GtkWidget *g_add_button;
GtkWidget *g_delete_button;
GtkWidget *g_modify_button;
GtkWidget *g_yes_button;
GtkWidget *g_no_button;
GtkWidget *g_notice_label;
GtkWidget *g_about_button;

GObject *completion, *list_store;

void load_css(void){
    GtkCssProvider *provider;
     GdkDisplay *display;
    GdkScreen *screen;

    const gchar *css_style_file = "style.css";
    GFile *css_fp = g_file_new_for_path(css_style_file);
    GError *error = 0;

    provider = gtk_css_provider_new();
    display = gdk_display_get_default();
   screen = gdk_display_get_default_screen(display);

    gtk_style_context_add_provider_for_screen(screen,
        GTK_STYLE_PROVIDER(provider),GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
    gtk_css_provider_load_from_file(provider,css_fp,&error);
    g_object_unref(provider);
}


//Cac ham giao dien
void yes_button_clicked();
void no_button_clicked();
void add_button_clicked();
void modify_button_clicked();
void delete_button_clicked();
void search_entry_activate();
void about_button_clicked(){
    GError* err = NULL;
    GdkPixbuf *pixbuf = gdk_pixbuf_new_from_file("res/logo.png",&err);
    GtkWidget *dialog = gtk_about_dialog_new();
    gtk_about_dialog_set_logo(dialog,pixbuf);
    gtk_about_dialog_set_program_name(GTK_ABOUT_DIALOG(dialog), "EV DICTIONARY");
    gtk_about_dialog_set_version(GTK_ABOUT_DIALOG(dialog), "v1.0");
    gtk_about_dialog_set_comments(GTK_ABOUT_DIALOG(dialog), "Đây là sản phẩm của nhóm 4 thành viên:\nNguyễn Trung Kiên\nLê Anh Dũng\nNguyễn Tiến Việt\nNguyễn Hoài Nam");
    gtk_window_set_transient_for(dialog, window);
    gtk_dialog_run(GTK_DIALOG(dialog));
    gtk_widget_destroy(dialog);
}
int isBlank(char *text);

//Cac ham xu li auto completion
int prefix(const char *string1, const char *string2);
void show_completion(char word[]);
void find_next_word(char word[]);
void show_matching_soundex_word(char word[]);
gboolean search_entry_key_press( GtkEntry *g_search_entry, GdkEvent *event, gpointer none);

int main(int argc, char *argv[])
{
    //Khoi tao BTree
    //btinit();
    btfile = btopn("Data", 0, 0);
    if (btfile==NULL) {
        docFile();
    }

    //Khoi tao
    gtk_init(&argc, &argv);

    //Lay giao dien
    builder = gtk_builder_new_from_file("glade/window_main.glade");


    //Ket noi ui vs backend
    g_meaning_textview=GTK_WIDGET(gtk_builder_get_object(builder,"meaning_textview"));
    g_word_label=GTK_WIDGET(gtk_builder_get_object(builder,"word_label"));
    g_text_buffer=GTK_TEXT_BUFFER(gtk_builder_get_object(builder,"textbuffer"));
    g_search_entry=GTK_WIDGET(gtk_builder_get_object(builder,"search_entry"));
    g_add_button=GTK_WIDGET(gtk_builder_get_object(builder,"add_button"));
    g_modify_button=GTK_WIDGET(gtk_builder_get_object(builder,"modify_button"));
    g_delete_button=GTK_WIDGET(gtk_builder_get_object(builder,"delete_button"));
    g_yes_button=GTK_WIDGET(gtk_builder_get_object(builder,"yes_button"));
    g_no_button=GTK_WIDGET(gtk_builder_get_object(builder,"no_button"));
    g_notice_label=GTK_WIDGET(gtk_builder_get_object(builder,"notice_label"));
    g_about_button=GTK_WIDGET(gtk_builder_get_object(builder,"about_button"));

    //Apply css
    gtk_widget_set_name(g_add_button,"b1");

    window = GTK_WIDGET(gtk_builder_get_object(builder, "window_main"));
    gtk_window_set_title(window,"Từ điển Anh-Việt");
    gtk_builder_connect_signals(builder, NULL);

    //Lay file css
    //gtk_widget_set_name(g_add_button,"button");
    load_css();

    //Tao ra cai goi y va hoan thien tu tren thanh tim kiem
    completion = gtk_entry_completion_new();
    gtk_entry_completion_set_text_column(completion, 0);
    list_store = gtk_list_store_new(10, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING);
    gtk_entry_completion_set_model(completion, GTK_TREE_MODEL(list_store));
    gtk_entry_set_completion(GTK_ENTRY(g_search_entry), completion);


    //Ket noi widget vs function
    g_signal_connect(g_search_entry, "key-press-event", G_CALLBACK(search_entry_key_press), NULL);
    g_signal_connect(g_search_entry, "activate", G_CALLBACK(search_entry_activate), NULL);
    g_signal_connect(g_add_button,"clicked",G_CALLBACK(add_button_clicked),NULL);
    g_signal_connect(g_yes_button,"clicked",G_CALLBACK(yes_button_clicked),NULL);
    g_signal_connect(g_no_button,"clicked",G_CALLBACK(no_button_clicked),NULL);
    g_signal_connect(g_modify_button,"clicked",G_CALLBACK(modify_button_clicked),NULL);
    g_signal_connect(g_delete_button,"clicked",G_CALLBACK(delete_button_clicked),NULL);
    g_signal_connect(g_about_button,"clicked",G_CALLBACK(about_button_clicked),NULL);

    g_object_unref(builder);

    gtk_widget_show(window);
    gtk_main();

    //Dong Btree
    btcls(btfile);

    return 0;
}

void docFile(){
    char tu[1000]= "";
    char *nghia = (char *)malloc(100000);
    strcpy(nghia, "");
    char tmp[10000];
    int a;
    btfile = btcrt("Data",0, 0);
    FILE *fp = fopen("anhviet.txt", "r");
    if(fp == NULL)
    {
        printf("Co loi khi mo file!");
        exit(0);
    }
    else
    {
        while(fgets(tmp, 10000, fp) != NULL)
        {
            if(tmp[0] == '@')
            {
                btins(btfile, tu, nghia, strlen(nghia) + 1);
                //printf("**%s\n%s", tu, nghia);
                strcpy(nghia, "");
                strcpy(tu, tmp + 1);
                int check = 0;
                for(int i = 0; i < strlen(tu); i++)
                {
                    if(tu[i] == '/')
                    {
                        check = 1;
                        strcpy(nghia, tu + i);
                        tu[i - 1] = '\0';
                        break;
                    }
                }
                if(check == 0)
                {
                    tu[strlen(tu) - 1] = '\0';
                }
            }
            else
            {
                strcat(nghia, tmp);
            }
        }
        btins(btfile, tu, nghia, strlen(nghia) + 1);
    }
    fclose(fp);
}


// called when window is closed
void on_window_main_destroy()
{
    btcls(btfile);
    gtk_main_quit();
}

void yes_button_clicked(){
    int n;
    if (status==1){
        //THem tu
        char *word = gtk_label_get_text(GTK_LABEL(g_word_label));
        GtkTextIter startIter;
        GtkTextIter endIter;

        gtk_text_buffer_get_start_iter(g_text_buffer, &startIter);
        gtk_text_buffer_get_end_iter(g_text_buffer, &endIter);

        char *meaning = gtk_text_buffer_get_text(GTK_TEXT_BUFFER(g_text_buffer), &startIter, &endIter, FALSE);
        if (isBlank(meaning)) {
            gtk_label_set_label(GTK_LABEL(g_notice_label), "Không được để trống");
            gtk_widget_set_visible(GTK_LABEL(g_notice_label), TRUE);
            return;
        }
        n=btins(btfile,strdup(word),strdup(meaning),strlen(meaning)+1);
        if (n==0){
            gtk_label_set_label(GTK_LABEL(g_notice_label), "Đã thêm  ");
            gtk_widget_set_visible(GTK_LABEL(g_notice_label), TRUE);
            gtk_widget_set_visible(GTK_BUTTON(g_yes_button), FALSE);
            gtk_widget_set_visible(GTK_BUTTON(g_no_button), FALSE);
        }
        else {
            gtk_label_set_label(GTK_LABEL(g_notice_label), "Thêm không thành công  ");
            gtk_widget_set_visible(GTK_LABEL(g_notice_label), TRUE);
            gtk_widget_set_visible(GTK_BUTTON(g_yes_button), FALSE);
            gtk_widget_set_visible(GTK_BUTTON(g_no_button), FALSE);
        }
        return ;
    }
    else if (status==2){
        //Sua nghia cua tu
        char *word = gtk_label_get_text(GTK_LABEL(g_word_label));
        GtkTextIter startIter;
        GtkTextIter endIter;

        gtk_text_buffer_get_start_iter(g_text_buffer, &startIter);
        gtk_text_buffer_get_end_iter(g_text_buffer, &endIter);

        char *meaning = gtk_text_buffer_get_text(GTK_TEXT_BUFFER(g_text_buffer), &startIter, &endIter, FALSE);
        if (isBlank(meaning)) {
            gtk_label_set_label(GTK_LABEL(g_notice_label), "Không được để trống");
            gtk_widget_set_visible(GTK_LABEL(g_notice_label), TRUE);
            return;
        }
        n=btupd(btfile, strdup(word), strdup(meaning), strlen(meaning) + 1);
        if (n==0){
            gtk_label_set_label(GTK_LABEL(g_notice_label), "Đã sửa nghĩa của từ ");
            gtk_widget_set_visible(GTK_LABEL(g_notice_label), TRUE);
            gtk_widget_set_visible(GTK_BUTTON(g_yes_button), FALSE);
            gtk_widget_set_visible(GTK_BUTTON(g_no_button), FALSE);
        }
        else {
            gtk_label_set_label(GTK_LABEL(g_notice_label), "Sửa không thành công ");
            gtk_widget_set_visible(GTK_LABEL(g_notice_label), TRUE);
            gtk_widget_set_visible(GTK_BUTTON(g_yes_button), FALSE);
            gtk_widget_set_visible(GTK_BUTTON(g_no_button), FALSE);
        }
        return ;
    }
    else if (status == 3){
        char *word = gtk_label_get_text(GTK_LABEL(g_word_label));
        n=btdel(btfile,word);
        if (n==0){
            gtk_label_set_label(GTK_LABEL(g_notice_label), "Xóa thành công ");
            gtk_widget_set_visible(GTK_LABEL(g_notice_label), TRUE);
            gtk_widget_set_visible(GTK_BUTTON(g_yes_button), FALSE);
            gtk_widget_set_visible(GTK_BUTTON(g_no_button), FALSE);
        }
        else{
            gtk_label_set_label(GTK_LABEL(g_notice_label), "Xóa không thành công ");
            gtk_widget_set_visible(GTK_LABEL(g_notice_label), TRUE);
            gtk_widget_set_visible(GTK_BUTTON(g_yes_button), FALSE);
            gtk_widget_set_visible(GTK_BUTTON(g_no_button), FALSE);
        }
    }
}

void no_button_clicked(){
    gtk_label_set_text(GTK_LABEL(g_word_label),"");
    gtk_text_buffer_set_text(GTK_TEXT_BUFFER(g_text_buffer), "", -1);
    gtk_text_view_set_editable(GTK_TEXT_VIEW(g_meaning_textview), FALSE);
    gtk_text_view_set_cursor_visible(GTK_TEXT_VIEW(g_meaning_textview), FALSE);
    gtk_widget_set_visible(GTK_LABEL(g_notice_label),FALSE);
    gtk_widget_set_visible(GTK_BUTTON(g_yes_button),FALSE);
    gtk_widget_set_visible(GTK_BUTTON(g_no_button),FALSE);

}

void delete_button_clicked(){
    gtk_label_set_text(GTK_LABEL(g_notice_label),"Bạn có muốn xóa từ này ?");
    gtk_widget_set_visible(GTK_LABEL(g_notice_label),TRUE);
    gtk_widget_set_visible(GTK_BUTTON(g_yes_button),TRUE);
    gtk_widget_set_visible(GTK_BUTTON(g_no_button),TRUE);
    status=3;
}

void add_button_clicked(){
    char  word[20];
    strcpy(word,gtk_entry_get_text(GTK_ENTRY(g_search_entry)));
    gtk_label_set_text(GTK_LABEL(g_word_label),word);
    gtk_text_buffer_set_text(GTK_TEXT_BUFFER(g_text_buffer), "", -1);
    gtk_text_view_set_buffer(GTK_TEXT_VIEW(g_meaning_textview), GTK_TEXT_BUFFER(g_text_buffer));
    gtk_text_view_set_editable(GTK_TEXT_VIEW(g_meaning_textview), TRUE);
    gtk_text_view_set_cursor_visible(GTK_TEXT_VIEW(g_meaning_textview), TRUE);

    gtk_label_set_text(GTK_LABEL(g_notice_label),"Bạn có muốn thêm từ này ? ");
    gtk_widget_set_visible(GTK_LABEL(g_notice_label),TRUE);
    gtk_widget_set_visible(GTK_BUTTON(g_yes_button),TRUE);
    gtk_widget_set_visible(GTK_BUTTON(g_no_button),TRUE);
    status=1;
}

void modify_button_clicked(){
    char  word[20];
    strcpy(word,gtk_entry_get_text(GTK_ENTRY(g_search_entry)));
    gtk_label_set_text(GTK_LABEL(g_word_label),word);
    gtk_text_view_set_editable(GTK_TEXT_VIEW(g_meaning_textview), TRUE);
    gtk_text_view_set_cursor_visible(GTK_TEXT_VIEW(g_meaning_textview), TRUE);
    gtk_label_set_text(GTK_LABEL(g_notice_label),"Bạn có muốn sửa nghĩa từ này ? ");
    gtk_widget_set_visible(GTK_LABEL(g_notice_label),TRUE);
    gtk_widget_set_visible(GTK_BUTTON(g_yes_button),TRUE);
    gtk_widget_set_visible(GTK_BUTTON(g_no_button),TRUE);
    status=2;
}

void search_entry_activate(){
    char  word[20];
    char mean[10000]="";
    strcpy(word,gtk_entry_get_text(GTK_ENTRY(g_search_entry)));
    printf("Tu can tim: %s\n",word);
    int size=0,n=0;
    n=btsel(btfile,word,mean,10000,&size);
    printf("n: %d\nsize: %d\n",n,size);
    printf("Nghia: %s\n",mean);
    //Xoa cac nut yes no label di thoi, ko lien quan
    gtk_widget_set_visible(GTK_LABEL(g_notice_label),FALSE);
    gtk_widget_set_visible(GTK_BUTTON(g_yes_button),FALSE);
    gtk_widget_set_visible(GTK_BUTTON(g_no_button),FALSE);
    if (n==0){
        //Neu tim thay tu
        gtk_label_set_text(GTK_LABEL(g_word_label),word);
        gtk_text_buffer_set_text(g_text_buffer,mean,-1);
        gtk_text_view_set_buffer(GTK_TEXT_VIEW(g_meaning_textview), GTK_TEXT_BUFFER(g_text_buffer));
        gtk_text_view_set_editable(GTK_TEXT_VIEW(g_meaning_textview), FALSE);
        gtk_text_view_set_cursor_visible(GTK_TEXT_VIEW(g_meaning_textview), FALSE);
        gtk_widget_set_sensitive(GTK_BUTTON(g_add_button),FALSE);
        gtk_widget_set_sensitive(GTK_WIDGET(g_modify_button), TRUE);
        gtk_widget_set_sensitive(GTK_WIDGET(g_delete_button), TRUE);
        //Hien thi cac tu gan giong
    }
    else {
        //Neu ko tim thay
        gtk_label_set_text(GTK_LABEL(g_word_label),"Không tìm thấy ");
        //gtk_text_buffer_set_text(g_text_buffer,"Cac tu gan giong: ",-1);
        show_matching_soundex_word(word);
        gtk_widget_set_sensitive(GTK_WIDGET(g_add_button), TRUE);
        gtk_widget_set_sensitive(GTK_WIDGET(g_modify_button), FALSE);
        gtk_widget_set_sensitive(GTK_WIDGET(g_delete_button), FALSE);
    }
}

int isBlank(char *text){
    int k = 0;  //?
    int d = strlen(text);   //?
    while (k < d)
    {
        if (!((text[k] == ' ') || (text[k] == '\n')))
        {
            return 0;
        }
        else
            k++;
    }
    return 1;
}

void find_next_word(char word[]){
    int n,size,val;
    char word_next[100];
    n=bfndky(btfile,word,&size);
    if (n==0){
        bnxtky(btfile,word_next,&size);
        gtk_entry_set_text(g_search_entry,word_next);
        search_entry_activate();
    }
    else{
        btins(btfile,strdup(word),"",1);
        bnxtky(btfile,word_next,&size);
        gtk_entry_set_text(g_search_entry,word_next);
        search_entry_activate();
    }
}

void show_matching_soundex_word(char word[]){
    char temp_word[100];
    int n=10,i,rsize;
    char list_suggest_word[1000]="";
    char *soundex_code=soundex(word);
    btins(btfile,word,"",1);
    bfndky(btfile,word,&rsize);
    int count=0;
    char temp_str[10][100];
    for (i=0;i<n;i++){
        if (bprvky(btfile,temp_word,&rsize)==0){
            if (strcmp(soundex(temp_word),soundex_code)==0){
                strcpy(temp_str[count++],temp_word);
            }
        }
    }
    for (i=count-1;i>=0;i--){
        strcat(list_suggest_word,temp_str[i]);
        strcat(list_suggest_word,"\n");
    }
    bfndky(btfile,word,&rsize);
    for (i=0;i<n;i++){
        if (bnxtky(btfile,temp_word,&rsize)==0){
            if (strcmp(soundex(temp_word),soundex_code)==0){
                strcat(list_suggest_word,temp_word);
                strcat(list_suggest_word,"\n");
            }
        }
    }
    btdel(btfile,word);

    //In ra man hinh
    char output[1000]="Có phải bạn đang tìm : \n";
    strcat(output,list_suggest_word);
    gtk_text_buffer_set_text(g_text_buffer,output,-1);
}

int prefix(const char *string1, const char *string2){
	int length1 = strlen(string1);
	int length2 = strlen(string2);
	int index;

	if (length1 > length2)
		return 0;

	for (index = 0; index < length1; index++)
	{
		if (string1[index] != string2[index])
			return 0;
	}

	return 1;
}

void show_completion(char word[]){
    GtkTreeIter Iter;
    gtk_list_store_clear(list_store);
    char tu_goi_y[30];
    int rsize;

    if (!bfndky(btfile,word,&rsize)){
        gtk_list_store_append(list_store,&Iter);
        gtk_list_store_set(list_store,&Iter,0,word,-1);
    }

    int i,number=10,value;

    strcpy(tu_goi_y,word);
    for (i=0;i<number&&number<100;i++){
        if (!bnxtky(btfile,tu_goi_y,&value)){
            if (prefix(word,tu_goi_y)){
                gtk_list_store_append(list_store,&Iter);
                gtk_list_store_set(list_store,&Iter,0,tu_goi_y,-1);
            }
            else number++;
        }
    }
}

gboolean search_entry_key_press( GtkEntry *g_search_entry, GdkEvent *event, gpointer none){
    gtk_widget_set_sensitive(g_add_button,FALSE);
    gtk_widget_set_sensitive(g_modify_button,FALSE);
    gtk_widget_set_sensitive(g_delete_button,FALSE);

    char word[30];
    GdkEventKey *key=(GdkEventKey *)event;
    int l;
    strcpy(word,gtk_entry_get_text(GTK_ENTRY(g_search_entry)));
    l=strlen(word);
    if(key->keyval == GDK_KEY_Tab)
	{
		gtk_widget_grab_focus(g_modify_button);
        find_next_word(word);
	}
	else{
        if(key->keyval != GDK_KEY_BackSpace)
		{
			if((key->keyval != 65364)&&(key->keyval != 65362))
				word[l]=key->keyval;
			word[l+1]='\0';
		}
		else word[l-1]='\0';
		show_completion(word);
	}
	return FALSE;
}
