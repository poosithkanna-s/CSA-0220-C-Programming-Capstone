#include <gtk/gtk.h>
#include <ctype.h>
#include <string.h>
#include <stdlib.h>
 
/* ---------- Diagnosis data ---------- */
#define NDIS 3
#define NFEAT 11
#define NLAB 4
 
static const char *DIS[NDIS] = {
    "Rheumatoid arthritis",
    "Systemic lupus erythematosus",
    "Sjogren's syndrome"
};
 
typedef struct { const char *label; int w[NDIS]; } Feature;
static const Feature FEATS[NFEAT] = {
    {"Joint pain / swelling",        {2, 2, 1}},
    {"Morning stiffness > 1 hour",   {3, 0, 0}},
    {"Butterfly facial rash",        {0, 3, 0}},
    {"Photosensitivity",             {0, 2, 1}},
    {"Fatigue",                      {1, 1, 1}},
    {"Recurrent fever",              {0, 1, 0}},
    {"Mouth ulcers",                 {0, 2, 0}},
    {"Dry eyes / dry mouth",         {0, 0, 3}},
    {"ANA positive",                 {0, 3, 2}},
    {"Anti-dsDNA positive",          {0, 4, 0}},
    {"Anti-SSA/Ro positive",         {0, 1, 4}}
};
 
typedef struct { const char *label; double cutoff; int w[NDIS]; } Lab;
static const Lab LABS[NLAB] = {
    {"Rheumatoid factor (IU/mL, normal <= 20)", 20.0, {3, 0, 1}},
    {"Anti-CCP (U/mL, normal <= 20)",           20.0, {4, 0, 0}},
    {"CRP (mg/L, normal <= 5)",                  5.0, {1, 1, 0}},
    {"ESR (mm/hr, normal <= 20)",               20.0, {1, 1, 1}}
};
 
/* ---------- Antigen settings ---------- */
#define WINDOW 6
#define VALID "ACDEFGHIKLMNPQRSTVWY"
#define HYDROPHOBIC "AVILMFWC"
#define HYDROPHILIC "DEKRNQSTH"
 
typedef struct {
    GtkWidget *checks[NFEAT];
    GtkWidget *entries[NLAB];
    GtkLabel *diag_out;
    GtkTextBuffer *seq_in;
    GtkLabel *seq_out;
} App;
 
static App app;
 
/* ---------- Diagnosis tab ---------- */
static void on_diagnose(GtkButton *btn, gpointer data) {
    App *a = data;
    int got[NDIS] = {0}, mx[NDIS] = {0};
 
    for (int f = 0; f < NFEAT; f++) {
        gboolean on = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(a->checks[f]));
        for (int d = 0; d < NDIS; d++) {
            mx[d] += FEATS[f].w[d];
            if (on) got[d] += FEATS[f].w[d];
        }
    }
 
    for (int l = 0; l < NLAB; l++) {
        const char *t = gtk_entry_get_text(GTK_ENTRY(a->entries[l]));
        while (isspace((unsigned char)*t)) t++;
        if (*t == '\0') continue; /* not entered: ignored */
 
        char *end;
        double v = g_ascii_strtod(t, &end);
        while (*end && isspace((unsigned char)*end)) end++;
        if (end == t || *end != '\0' || v < 0) {
            gchar *msg = g_strdup_printf("Error: enter a non-negative number for\n%s", LABS[l].label);
            gtk_label_set_text(a->diag_out, msg);
            g_free(msg);
            return;
        }
        for (int d = 0; d < NDIS; d++) {
            mx[d] += LABS[l].w[d];
            if (v > LABS[l].cutoff) got[d] += LABS[l].w[d];
        }
    }
 
    GString *out = g_string_new("Match scores (educational rule-based):\n\n");
    int best = 0;
    double bestpct = -1.0;
 
    for (int d = 0; d < NDIS; d++) {
        double pct = mx[d] > 0 ? 100.0 * got[d] / mx[d] : 0.0;
        if (pct > bestpct) { bestpct = pct; best = d; }
        int n = (int)(pct / 10.0 + 0.5);
        char bar[11];
        for (int i = 0; i < 10; i++) bar[i] = i < n ? '#' : '-';
        bar[10] = '\0';
        const char *level = pct < 30 ? "Low" : (pct < 60 ? "Moderate" : "High");
        g_string_append_printf(out, "%s\n  %5.1f%%  [%s]  %s\n\n", DIS[d], pct, bar, level);
    }
 
    if (bestpct <= 0.0)
        g_string_append(out, "No matching features selected.\n");
    else
        g_string_append_printf(out, "Highest match: %s (%.1f%%)\n", DIS[best], bestpct);
 
    g_string_append(out, "\nThis is a teaching tool, not a diagnosis.\n"
                         "Real diagnosis needs a clinician and full criteria.");
 
    gtk_label_set_text(a->diag_out, out->str);
    g_string_free(out, TRUE);
}
 
static GtkWidget *build_diagnosis_page(void) {
    GtkWidget *box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    gtk_container_set_border_width(GTK_CONTAINER(box), 12);
 
    GtkWidget *note = gtk_label_new("Educational scoring tool. Not medical advice or a diagnosis.");
    gtk_label_set_line_wrap(GTK_LABEL(note), TRUE);
    gtk_box_pack_start(GTK_BOX(box), note, FALSE, FALSE, 0);
 
    /* Symptoms and markers */
    GtkWidget *f1 = gtk_frame_new("Symptoms and antibody markers");
    GtkWidget *g1 = gtk_grid_new();
    gtk_grid_set_column_spacing(GTK_GRID(g1), 18);
    gtk_grid_set_row_spacing(GTK_GRID(g1), 6);
    gtk_container_set_border_width(GTK_CONTAINER(g1), 10);
    for (int f = 0; f < NFEAT; f++) {
        app.checks[f] = gtk_check_button_new_with_label(FEATS[f].label);
        gtk_grid_attach(GTK_GRID(g1), app.checks[f], f % 2, f / 2, 1, 1);
    }
    gtk_container_add(GTK_CONTAINER(f1), g1);
    gtk_box_pack_start(GTK_BOX(box), f1, FALSE, FALSE, 0);
 
    /* Lab values */
    GtkWidget *f2 = gtk_frame_new("Lab values (leave blank if not available)");
    GtkWidget *g2 = gtk_grid_new();
    gtk_grid_set_column_spacing(GTK_GRID(g2), 12);
    gtk_grid_set_row_spacing(GTK_GRID(g2), 6);
    gtk_container_set_border_width(GTK_CONTAINER(g2), 10);
    for (int l = 0; l < NLAB; l++) {
        GtkWidget *lab = gtk_label_new(LABS[l].label);
        gtk_label_set_xalign(GTK_LABEL(lab), 0.0);
        app.entries[l] = gtk_entry_new();
        gtk_entry_set_width_chars(GTK_ENTRY(app.entries[l]), 10);
        gtk_grid_attach(GTK_GRID(g2), lab, 0, l, 1, 1);
        gtk_grid_attach(GTK_GRID(g2), app.entries[l], 1, l, 1, 1);
    }
    gtk_container_add(GTK_CONTAINER(f2), g2);
    gtk_box_pack_start(GTK_BOX(box), f2, FALSE, FALSE, 0);
 
    GtkWidget *btn = gtk_button_new_with_label("Analyze diagnosis");
    g_signal_connect(btn, "clicked", G_CALLBACK(on_diagnose), &app);
    gtk_box_pack_start(GTK_BOX(box), btn, FALSE, FALSE, 0);
 
    GtkWidget *res = gtk_label_new("");
    gtk_label_set_xalign(GTK_LABEL(res), 0.0);
    gtk_label_set_yalign(GTK_LABEL(res), 0.0);
    gtk_label_set_selectable(GTK_LABEL(res), TRUE);
    app.diag_out = GTK_LABEL(res);
    gtk_box_pack_start(GTK_BOX(box), res, FALSE, FALSE, 0);
 
    GtkWidget *scroll = gtk_scrolled_window_new(NULL, NULL);
    gtk_container_add(GTK_CONTAINER(scroll), box);
    return scroll;
}
 
/* ---------- Antigen tab ---------- */
static size_t count_in(const char *seq, size_t n, const char *set) {
    size_t c = 0;
    for (size_t i = 0; i < n; i++)
        if (strchr(set, seq[i])) c++;
    return c;
}
 
static void on_analyze(GtkButton *btn, gpointer data) {
    App *a = data;
    GtkTextIter s, e;
    gtk_text_buffer_get_bounds(a->seq_in, &s, &e);
    gchar *raw = gtk_text_buffer_get_text(a->seq_in, &s, &e, FALSE);
 
    char *seq = g_malloc0(strlen(raw) + 1);
    size_t n = 0;
    gboolean in_header = FALSE;
 
    for (const char *p = raw; *p; p++) {
        if (*p == '>') { in_header = TRUE; continue; }
        if (*p == '\n') { in_header = FALSE; continue; }
        if (in_header || isspace((unsigned char)*p)) continue;
        char c = (char)toupper((unsigned char)*p);
        if (!strchr(VALID, c)) {
            gchar *msg = g_strdup_printf("Error: invalid amino-acid letter '%c'.", *p);
            gtk_label_set_text(a->seq_out, msg);
            g_free(msg); g_free(raw); g_free(seq);
            return;
        }
        seq[n++] = c;
    }
 
    if (n == 0) {
        gtk_label_set_text(a->seq_out, "Error: please enter a sequence.");
        g_free(raw); g_free(seq);
        return;
    }
 
    double hydrophobic = 100.0 * count_in(seq, n, HYDROPHOBIC) / n;
    double hydrophilic = 100.0 * count_in(seq, n, HYDROPHILIC) / n;
 
    char *flag = g_malloc0(n + 1);
    for (size_t i = 0; i + WINDOW <= n; i++) {
        int c = 0;
        for (int j = 0; j < WINDOW; j++)
            if (strchr(HYDROPHILIC, seq[i + j])) c++;
        if (c >= WINDOW - 1) memset(flag + i, 1, WINDOW);
    }
 
    GString *out = g_string_new(NULL);
    g_string_append_printf(out, "Residues: %zu\nHydrophobicity: %.1f%%\n"
                                "Hydrophilic content: %.1f%%\n\n"
                                "Hydrophilic candidate regions (1-based):\n",
                           n, hydrophobic, hydrophilic);
 
    gboolean found = FALSE;
    size_t i = 0;
    while (i < n) {
        if (!flag[i]) { i++; continue; }
        size_t st = i;
        while (i < n && flag[i]) i++;
        g_string_append_printf(out, "%zu-%zu  %.*s\n", st + 1, i, (int)(i - st), seq + st);
        found = TRUE;
    }
    if (!found) g_string_append(out, "None detected by this simple heuristic.\n");
 
    gtk_label_set_text(a->seq_out, out->str);
    g_string_free(out, TRUE);
    g_free(flag); g_free(raw); g_free(seq);
}
 
static GtkWidget *build_antigen_page(void) {
    GtkWidget *box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 8);
    gtk_container_set_border_width(GTK_CONTAINER(box), 12);
 
    GtkWidget *note = gtk_label_new("Sequence-based educational heuristic; not a confirmed epitope prediction.");
    gtk_label_set_line_wrap(GTK_LABEL(note), TRUE);
    gtk_box_pack_start(GTK_BOX(box), note, FALSE, FALSE, 0);
 
    GtkWidget *text = gtk_text_view_new();
    gtk_text_view_set_wrap_mode(GTK_TEXT_VIEW(text), GTK_WRAP_CHAR);
    GtkWidget *sc_in = gtk_scrolled_window_new(NULL, NULL);
    gtk_widget_set_size_request(sc_in, -1, 150);
    gtk_container_add(GTK_CONTAINER(sc_in), text);
    gtk_box_pack_start(GTK_BOX(box), sc_in, FALSE, TRUE, 0);
    app.seq_in = gtk_text_view_get_buffer(GTK_TEXT_VIEW(text));
 
    GtkWidget *btn = gtk_button_new_with_label("Analyze sequence");
    g_signal_connect(btn, "clicked", G_CALLBACK(on_analyze), &app);
    gtk_box_pack_start(GTK_BOX(box), btn, FALSE, FALSE, 0);
 
    GtkWidget *res = gtk_label_new("");
    gtk_label_set_xalign(GTK_LABEL(res), 0.0);
    gtk_label_set_yalign(GTK_LABEL(res), 0.0);
    gtk_label_set_selectable(GTK_LABEL(res), TRUE);
    app.seq_out = GTK_LABEL(res);
    GtkWidget *sc_out = gtk_scrolled_window_new(NULL, NULL);
    gtk_container_add(GTK_CONTAINER(sc_out), res);
    gtk_box_pack_start(GTK_BOX(box), sc_out, TRUE, TRUE, 0);
 
    return box;
}
 
/* ---------- main ---------- */
int main(int argc, char *argv[]) {
    gtk_init(&argc, &argv);
 
    GtkWidget *win = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(win), "Immunology System (educational)");
    gtk_window_set_default_size(GTK_WINDOW(win), 680, 720);
    g_signal_connect(win, "destroy", G_CALLBACK(gtk_main_quit), NULL);
 
    GtkWidget *nb = gtk_notebook_new();
    gtk_notebook_append_page(GTK_NOTEBOOK(nb), build_diagnosis_page(),
                             gtk_label_new("Disease Diagnosis"));
    gtk_notebook_append_page(GTK_NOTEBOOK(nb), build_antigen_page(),
                             gtk_label_new("Antigen Analysis"));
    gtk_container_add(GTK_CONTAINER(win), nb);
 
    gtk_widget_show_all(win);
    gtk_main();
    return 0;
}
 