// ============================================================================
// Name        : EditorCajasV0.cpp
// Author      : DjSteker
// Version     : 3.0
// Description : Punto de entrada del editor. Inicializa GTK y la interfaz.
// Compilar    : g++ main.cpp editor_interfaz.cpp procesador_celdas.cpp dialogo_archivo.cpp -o EditorCajasV3 `pkg-config --cflags --libs gtk4` -std=c++17
// $(shell pkg-config --cflags gtk4)  $(shell pkg-config --libs gtk4)
// ============================================================================

#include <gtk/gtk.h>
#include "editor_interfaz.hpp"
#include <iostream>
using namespace std;

// Función estática para el callback de activación
static void on_app_activate(GApplication *app, gpointer user_data) {
	EditorInterfaz *editor = static_cast<EditorInterfaz*>(user_data);
	editor->inicializar(GTK_APPLICATION(app));
}

int main(int argc, char **argv) {
	int status = 0;
	try {
		GtkApplication *app = gtk_application_new("com.example.boxeditor", G_APPLICATION_DEFAULT_FLAGS);
		EditorInterfaz editor;

		g_signal_connect(app, "activate", G_CALLBACK(on_app_activate), &editor);

		status = g_application_run(G_APPLICATION(app), argc, argv);
		g_object_unref(app);
	} catch (const std::exception &e) {
		std::cerr << "[Fatal Exception] " << e.what() << "\n";
		status = EXIT_FAILURE;
	} catch (...) {
		std::cerr << "[Fatal Exception] Error indeterminado.\n";
		status = EXIT_FAILURE;
	}

	return status;
}

// ============================================================================
// Name        : EditorCajasV2_GTK4_ModoInsertReplace_Fixed.cpp
// Author      : Iván Mar Villa
// Version     : 2.9
// Description : Editor de esquemas con selección múltiple, relleno,
//               edición de filas/columnas, detección automática de
//               uniones entre líneas y modo Insertar/Reemplazar.
//               v2.9: (1) Deteección de cruces mejorada: ahora detecta
//               correctamente cuando una línea gruesa/doble se cruza con
//               otra línea perpendicular de distinto grosor (usando ╋, ╬, ╪, ╫).
//               (2) Arreglado bug que alteraba celdas sueltas o selecciones
//               previas al inicio del trazo en modo dibujo.
//               (3) Las celdas aisladas (sin vecinos) ya no se reorientan
//               mágicamente al mover el cursor por el lienzo.
// Compilar    : g++ EditorCajasV2_GTK4_ModoInsertReplace_Fixed.cpp -o EditorCajasV2_GTK4_ModoInsertReplace_Fixed `pkg-config --cflags --libs gtk4` -std=c++17
// ============================================================================

//#include <iostream>
//#include <vector>
//#include <cstdint>
//#include <cstdlib>
//#include <cstring>
//#include <algorithm>
//using namespace std;
//#include <gtk/gtk.h>
//#include "dialogo_archivo.hpp"
//
///* --- Configuración inicial --- */
//static const int INIT_COLS = 80;
//static const int INIT_ROWS = 40;
//static const int CELL_W = 16;
//static const int CELL_H = 24;
//
///* --- Familia de línea y estilo de celda --- */
//enum class LineFamily : uint8_t {
//	NONE = 0, LIGHT, DOUBLE, DASHED, THICK
//};
//
//enum class Tool {
//	SELECT, LINE
//};
//
//typedef struct {
//	GtkWidget *drawing_area;
//	GtkWidget *status;
//	GtkWidget *btn_insert;
//	GtkWidget *btn_tool_line;
//	GtkDropDown *style_dropdown;
//
//	int cols;
//	int rows;
//	std::vector<std::vector<gunichar>> grid;
//	std::vector<std::vector<LineFamily>> family;
//
//	int cx;
//	int cy;
//	gboolean insert_mode;
//	gboolean is_selecting;
//	int sel_x1;
//	int sel_y1;
//	int sel_x2;
//	int sel_y2;
//
//	Tool tool;
//	LineFamily line_style;
//	gboolean line_last_horizontal;
//	int line_last_x;
//	int line_last_y;
//
//	gint64 last_mouse_time;
//
//} EditorState;
//
//static EditorState state;
//
///* --- Cache de descripción de fuente Pango --- */
//static PangoFontDescription *cached_font_desc = nullptr;
//
///* --- Tablas de unión (bitmask N=8 E=4 S=2 W=1) --- */
//static const gunichar LIGHT_TABLE[16] = {
///*0*/0, /*1 W*/U'╴', /*2 S*/U'╷', /*3 SW*/U'┐',
///*4 E*/U'╶', /*5 EW*/U'─', /*6 ES*/U'┌', /*7 ESW*/U'┬',
///*8 N*/U'╵', /*9 NW*/U'┘', /*10 NS*/U'│', /*11 NSW*/U'┤',
///*12 NE*/U'└', /*13 NEW*/U'┴', /*14 NES*/U'├', /*15 NESW*/U'┼' };
//
//static const gunichar DOUBLE_TABLE[16] = {
///*0*/0, /*1 W*/U'═', /*2 S*/U'║', /*3 SW*/U'╗',
///*4 E*/U'═', /*5 EW*/U'═', /*6 ES*/U'╔', /*7 ESW*/U'╦',
///*8 N*/U'║', /*9 NW*/U'╝', /*10 NS*/U'║', /*11 NSW*/U'╣',
///*12 NE*/U'╚', /*13 NEW*/U'╩', /*14 NES*/U'╠', /*15 NESW*/U'╬' };
//
//static const gunichar THICK_TABLE[16] = {
///*0*/0, /*1 W*/U'╸', /*2 S*/U'╻', /*3 SW*/U'┓',
///*4 E*/U'╺', /*5 EW*/U'━', /*6 ES*/U'┏', /*7 ESW*/U'┳',
///*8 N*/U'╹', /*9 NW*/U'┛', /*10 NS*/U'┃', /*11 NSW*/U'┫',
///*12 NE*/U'┗', /*13 NEW*/U'┻', /*14 NES*/U'┣', /*15 NESW*/U'╋' };
//
///* --- Inicialización --- */
//static void resize_canvas(void) {
//	gtk_drawing_area_set_content_width(GTK_DRAWING_AREA(state.drawing_area), state.cols * CELL_W);
//	gtk_drawing_area_set_content_height(GTK_DRAWING_AREA(state.drawing_area), state.rows * CELL_H);
//	gtk_widget_queue_draw(state.drawing_area);
//}
//
//static void grid_init(void) {
//	state.cols = INIT_COLS;
//	state.rows = INIT_ROWS;
//	state.grid.assign(state.rows, std::vector<gunichar>(state.cols, ' '));
//	state.family.assign(state.rows, std::vector<LineFamily>(state.cols, LineFamily::NONE));
//	state.cx = 0;
//	state.cy = 0;
//	state.insert_mode = FALSE;
//	state.is_selecting = FALSE;
//	state.sel_x1 = state.sel_y1 = state.sel_x2 = state.sel_y2 = 0;
//	state.tool = Tool::SELECT;
//	state.line_style = LineFamily::LIGHT;
//	state.line_last_horizontal = TRUE;
//	state.line_last_x = -1;
//	state.line_last_y = -1;
//
//	state.last_mouse_time = 0;
//
//	if (!cached_font_desc) {
//		cached_font_desc = pango_font_description_from_string("Monospace 16");
//	}
//}
//
///* --- Helpers --- */
//static const char* style_name(LineFamily f) {
//	switch (f) {
//	case LineFamily::LIGHT:
//		return "Sencilla";
//	case LineFamily::DOUBLE:
//		return "Doble";
//	case LineFamily::DASHED:
//		return "Punteada";
//	case LineFamily::THICK:
//		return "Gruesa";
//	default:
//		return "-";
//	}
//}
//
//static void update_status(void) {
//	gchar *buf = g_strdup_printf("Modo: %s | Herramienta: %s (%s) | Cursor: (%d,%d) | Tamaño: %dx%d | "
//			"Insert=toggle | %s | Click+Drag=%s", state.insert_mode ? "INSERT" : "REPLACE", state.tool == Tool::LINE ? "Línea" : "Selección", style_name(state.line_style), state.cx, state.cy, state.cols,
//			state.rows, state.tool == Tool::LINE ? "Flechas=dibujar línea" : "Flechas=mover", state.tool == Tool::LINE ? "trazar línea" : "seleccionar");
//	gtk_label_set_text(GTK_LABEL(state.status), buf);
//	g_free(buf);
//}
//
///* --- Cálculo del carácter de unión para una celda --- */
//static gunichar table_lookup(LineFamily fam, int mask, bool horizontal_hint) {
//	if (fam == LineFamily::LIGHT) {
//		if (mask == 0)
//			return horizontal_hint ? U'─' : U'│';
//		return LIGHT_TABLE[mask];
//	} else if (fam == LineFamily::DOUBLE) {
//		if (mask == 0)
//			return horizontal_hint ? U'═' : U'║';
//		return DOUBLE_TABLE[mask];
//	} else if (fam == LineFamily::DASHED) {
//		if (mask == 5)
//			return U'┄';
//		if (mask == 10)
//			return U'┆';
//		if (mask == 0)
//			return horizontal_hint ? U'┄' : U'┆';
//		return LIGHT_TABLE[mask];
//	} else if (fam == LineFamily::THICK) {
//		if (mask == 0)
//			return horizontal_hint ? U'━' : U'┃';
//		return THICK_TABLE[mask];
//	} else {
//		return ' ';
//	}
//}
//
//static gunichar cross_char(LineFamily vertical, LineFamily horizontal) {
//	auto norm = [](LineFamily f) {
//		return (f == LineFamily::DASHED || f == LineFamily::NONE) ? LineFamily::LIGHT : f;
//	};
//	LineFamily v = norm(vertical);
//	LineFamily h = norm(horizontal);
//
//	if (v == LineFamily::THICK || h == LineFamily::THICK)
//		return U'╋';
//	if (v == LineFamily::DOUBLE && h == LineFamily::DOUBLE)
//		return U'╬';
//	if (v == LineFamily::LIGHT && h == LineFamily::DOUBLE)
//		return U'╪';
//	if (v == LineFamily::DOUBLE && h == LineFamily::LIGHT)
//		return U'╫';
//	return U'┼';
//}
//
//static LineFamily family_at(int x, int y) {
//	if (x < 0 || x >= state.cols || y < 0 || y >= state.rows)
//		return LineFamily::NONE;
//	return state.family[y][x];
//}
//
///* --- Recalcular el carácter de una celda según sus vecinos --- */
///* force_global_hint: se usa al pintar para forzar la orientación de celdas aisladas */
//static void recompute_cell(int x, int y, bool force_global_hint = false) {
//	if (x < 0 || x >= state.cols || y < 0 || y >= state.rows)
//		return;
//
//	LineFamily fam = state.family[y][x];
//	if (fam == LineFamily::NONE)
//		return;
//
//	bool N = family_at(x, y - 1) == fam;
//	bool E = family_at(x + 1, y) == fam;
//	bool S = family_at(x, y + 1) == fam;
//	bool W = family_at(x - 1, y) == fam;
//	int mask = (N ? 8 : 0) | (E ? 4 : 0) | (S ? 2 : 0) | (W ? 1 : 0);
//
//	if (mask == 0) {
//		// Si es celda aislada y no se está forzando su orientación (no es la actual), no la modifiquemos
//		if (!force_global_hint && state.grid[y][x] != ' ')
//			return;
//		state.grid[y][x] = table_lookup(fam, mask, state.line_last_horizontal);
//		return;
//	}
//
//	/* --- NUEVA LÓGICA DE CRUCES MEJORADA --- */
//	LineFamily fam_v = (N || S) ? fam : LineFamily::NONE;
//	LineFamily fam_h = (E || W) ? fam : LineFamily::NONE;
//
//	LineFamily ofam_v = LineFamily::NONE;
//	LineFamily ofam_h = LineFamily::NONE;
//
//	// Si en el eje vertical no hay continuación de la familia actual, miramos si hay otra familia cruzando
//	if (fam_v == LineFamily::NONE) {
//		LineFamily fn = family_at(x, y - 1);
//		LineFamily fs = family_at(x, y + 1);
//		if (fn != LineFamily::NONE && fn == fs && fn != fam) {
//			ofam_v = fn;
//		}
//	}
//	// Igual para el eje horizontal
//	if (fam_h == LineFamily::NONE) {
//		LineFamily fe = family_at(x + 1, y);
//		LineFamily fw = family_at(x - 1, y);
//		if (fe != LineFamily::NONE && fe == fw && fe != fam) {
//			ofam_h = fe;
//		}
//	}
//
//	// Si hemos encontrado otra familia cruzando en perpendicular, generamos el cruce
//	if (ofam_v != LineFamily::NONE || ofam_h != LineFamily::NONE) {
//		LineFamily v = (fam_v != LineFamily::NONE) ? fam_v : ofam_v;
//		LineFamily h = (fam_h != LineFamily::NONE) ? fam_h : ofam_h;
//		state.grid[y][x] = cross_char(v, h);
//		return;
//	}
//
//	state.grid[y][x] = table_lookup(fam, mask, state.line_last_horizontal);
//}
//
//static void recompute_row_range(int y0, int y1) {
//	int start_y = MAX(0, y0);
//	int end_y = MIN(state.rows - 1, y1);
//	for (int y = start_y; y <= end_y; y++) {
//		for (int x = 0; x < state.cols; x++) {
//			if (state.family[y][x] != LineFamily::NONE)
//				recompute_cell(x, y);
//		}
//	}
//}
//
//static void recompute_col_range(int x0, int x1) {
//	int start_x = MAX(0, x0);
//	int end_x = MIN(state.cols - 1, x1);
//	for (int y = 0; y < state.rows; y++) {
//		for (int x = start_x; x <= end_x; x++) {
//			if (state.family[y][x] != LineFamily::NONE)
//				recompute_cell(x, y);
//		}
//	}
//}
//
//static void recompute_all(void) {
//	for (int y = 0; y < state.rows; y++) {
//		for (int x = 0; x < state.cols; x++) {
//			if (state.family[y][x] != LineFamily::NONE)
//				recompute_cell(x, y);
//		}
//	}
//}
//
//static void recompute_neighbors(int x, int y) {
//	recompute_cell(x - 1, y);
//	recompute_cell(x + 1, y);
//	recompute_cell(x, y - 1);
//	recompute_cell(x, y + 1);
//}
//
//static void recompute_border(int min_x, int max_x, int min_y, int max_y) {
//	for (int x = min_x - 1; x <= max_x + 1; x++) {
//		recompute_cell(x, min_y - 1);
//		recompute_cell(x, max_y + 1);
//	}
//	for (int y = min_y - 1; y <= max_y + 1; y++) {
//		recompute_cell(min_x - 1, y);
//		recompute_cell(max_x + 1, y);
//	}
//}
//
///* --- Colocar / borrar una celda de línea --- */
//static void place_line_at(int x, int y) {
//	if (x < 0 || x >= state.cols || y < 0 || y >= state.rows)
//		return;
//	state.family[y][x] = state.line_style;
//	recompute_cell(x, y, true); // Forzamos orientación en la celda que se está pintando ahora
//	recompute_neighbors(x, y);
//}
//
//static void erase_line_at(int x, int y) {
//	if (x < 0 || x >= state.cols || y < 0 || y >= state.rows)
//		return;
//	state.family[y][x] = LineFamily::NONE;
//	state.grid[y][x] = ' ';
//	recompute_neighbors(x, y);
//}
//
//static void place_char(gunichar c, LineFamily fam = LineFamily::NONE) {
//	if (state.cy < 0 || state.cy >= state.rows || state.cx < 0 || state.cx >= state.cols)
//		return;
//	int x = state.cx;
//	int y = state.cy;
//
//	if (state.insert_mode) {
//		for (int i = state.cols - 1; i > x; i--) {
//			state.grid[y][i] = state.grid[y][i - 1];
//			state.family[y][i] = state.family[y][i - 1];
//		}
//	}
//
//	state.grid[y][x] = c;
//	state.family[y][x] = fam;
//
//	if (fam != LineFamily::NONE) {
//		recompute_cell(x - 1, y);
//		recompute_cell(x + 1, y);
//		recompute_cell(x, y - 1);
//		recompute_cell(x, y + 1);
//	}
//
//	if (state.cx < state.cols - 1)
//		state.cx++;
//}
//
//static void delete_at_cursor(void) {
//	if (state.cy < 0 || state.cy >= state.rows || state.cx < 0 || state.cx >= state.cols)
//		return;
//	int x = state.cx;
//	int y = state.cy;
//
//	if (state.insert_mode) {
//		for (int i = x; i < state.cols - 1; i++) {
//			state.grid[y][i] = state.grid[y][i + 1];
//			state.family[y][i] = state.family[y][i + 1];
//		}
//		state.grid[y][state.cols - 1] = ' ';
//		state.family[y][state.cols - 1] = LineFamily::NONE;
//		recompute_all();
//	} else {
//		state.grid[y][x] = ' ';
//		state.family[y][x] = LineFamily::NONE;
//		recompute_neighbors(x, y);
//	}
//}
//
//static void fill_selection(gunichar c, LineFamily fam) {
//	int min_x = CLAMP(MIN(state.sel_x1, state.sel_x2), 0, state.cols - 1);
//	int max_x = CLAMP(MAX(state.sel_x1, state.sel_x2), 0, state.cols - 1);
//	int min_y = CLAMP(MIN(state.sel_y1, state.sel_y2), 0, state.rows - 1);
//	int max_y = CLAMP(MAX(state.sel_y1, state.sel_y2), 0, state.rows - 1);
//
//	for (int y = min_y; y <= max_y; y++) {
//		for (int x = min_x; x <= max_x; x++) {
//			state.grid[y][x] = c;
//			state.family[y][x] = fam;
//		}
//	}
//	if (fam != LineFamily::NONE)
//		recompute_border(min_x, max_x, min_y, max_y);
//}
//
//static void clear_selection(void) {
//	int min_x = CLAMP(MIN(state.sel_x1, state.sel_x2), 0, state.cols - 1);
//	int max_x = CLAMP(MAX(state.sel_x1, state.sel_x2), 0, state.cols - 1);
//	int min_y = CLAMP(MIN(state.sel_y1, state.sel_y2), 0, state.rows - 1);
//	int max_y = CLAMP(MAX(state.sel_y1, state.sel_y2), 0, state.rows - 1);
//
//	for (int y = min_y; y <= max_y; y++) {
//		for (int x = min_x; x <= max_x; x++) {
//			state.grid[y][x] = ' ';
//			state.family[y][x] = LineFamily::NONE;
//		}
//	}
//	recompute_border(min_x, max_x, min_y, max_y);
//}
//
///* --- Edición de filas y columnas --- */
//static void insert_row(int at, bool after) {
//	int pos = CLAMP(after ? at + 1 : at, 0, state.rows);
//	state.grid.insert(state.grid.begin() + pos, std::vector<gunichar>(state.cols, ' '));
//	state.family.insert(state.family.begin() + pos, std::vector<LineFamily>(state.cols, LineFamily::NONE));
//	state.rows++;
//	if (state.cy >= pos)
//		state.cy = MIN(state.cy + 1, state.rows - 1);
//	recompute_row_range(pos - 1, pos + 1);
//	resize_canvas();
//}
//
//static void delete_row(int at) {
//	if (state.rows <= 1)
//		return;
//	at = CLAMP(at, 0, state.rows - 1);
//	state.grid.erase(state.grid.begin() + at);
//	state.family.erase(state.family.begin() + at);
//	state.rows--;
//	state.cy = CLAMP(state.cy, 0, state.rows - 1);
//	recompute_row_range(at - 1, at + 1);
//	resize_canvas();
//}
//
//static void insert_col(int at, bool after) {
//	int pos = CLAMP(after ? at + 1 : at, 0, state.cols);
//	for (int y = 0; y < state.rows; y++) {
//		state.grid[y].insert(state.grid[y].begin() + pos, ' ');
//		state.family[y].insert(state.family[y].begin() + pos, LineFamily::NONE);
//	}
//	state.cols++;
//	if (state.cx >= pos)
//		state.cx = MIN(state.cx + 1, state.cols - 1);
//	recompute_col_range(pos - 1, pos + 1);
//	resize_canvas();
//}
//
//static void delete_col(int at) {
//	if (state.cols <= 1)
//		return;
//	at = CLAMP(at, 0, state.cols - 1);
//	for (int y = 0; y < state.rows; y++) {
//		state.grid[y].erase(state.grid[y].begin() + at);
//		state.family[y].erase(state.family[y].begin() + at);
//	}
//	state.cols--;
//	state.cx = CLAMP(state.cx, 0, state.cols - 1);
//	recompute_col_range(at - 1, at + 1);
//	resize_canvas();
//}
//
///* --- Conversión robusta de tecla a carácter --- */
//static gunichar keyval_to_char(guint keyval) {
//	if (keyval >= 0x20 && keyval <= 0x7E)
//		return (gunichar) keyval;
//	gunichar c = gdk_keyval_to_unicode(keyval);
//	if (c != 0 && !g_unichar_iscntrl(c))
//		return c;
//	return 0;
//}
//
///* --- Renderizado --- */
//static void draw_grid(cairo_t *cr, int width, int height) {
//	cairo_set_source_rgb(cr, 0.96, 0.96, 0.96);
//	cairo_paint(cr);
//
//	cairo_set_source_rgb(cr, 0.82, 0.82, 0.82);
//	cairo_set_line_width(cr, 0.5);
//
//	for (int y = 0; y <= state.rows; y++) {
//		cairo_move_to(cr, 0, y * CELL_H);
//		cairo_line_to(cr, state.cols * CELL_W, y * CELL_H);
//		cairo_stroke(cr);
//	}
//	for (int x = 0; x <= state.cols; x++) {
//		cairo_move_to(cr, x * CELL_W, 0);
//		cairo_line_to(cr, x * CELL_W, state.rows * CELL_H);
//		cairo_stroke(cr);
//	}
//
//	if (state.is_selecting) {
//		int min_x = MIN(state.sel_x1, state.sel_x2);
//		int max_x = MAX(state.sel_x1, state.sel_x2);
//		int min_y = MIN(state.sel_y1, state.sel_y2);
//		int max_y = MAX(state.sel_y1, state.sel_y2);
//
//		cairo_set_source_rgba(cr, 0.2, 0.5, 0.9, 0.15);
//		cairo_rectangle(cr, min_x * CELL_W, min_y * CELL_H, (max_x - min_x + 1) * CELL_W, (max_y - min_y + 1) * CELL_H);
//		cairo_fill(cr);
//
//		cairo_set_source_rgb(cr, 0.2, 0.5, 0.9);
//		cairo_set_line_width(cr, 1.0);
//		cairo_rectangle(cr, min_x * CELL_W, min_y * CELL_H, (max_x - min_x + 1) * CELL_W, (max_y - min_y + 1) * CELL_H);
//		cairo_stroke(cr);
//	}
//
//	cairo_set_source_rgb(cr, 0.08, 0.08, 0.08);
//	PangoLayout *layout = pango_cairo_create_layout(cr);
//	pango_layout_set_font_description(layout, cached_font_desc);
//
//	for (int y = 0; y < state.rows; y++) {
//		for (int x = 0; x < state.cols; x++) {
//			char buf[8] = { 0 };
//			g_unichar_to_utf8(state.grid[y][x], buf);
//			pango_layout_set_text(layout, buf, -1);
//			cairo_move_to(cr, x * CELL_W + 2, y * CELL_H);
//			pango_cairo_show_layout(cr, layout);
//		}
//	}
//	g_object_unref(layout);
//
//	if (state.tool == Tool::LINE) {
//		if (state.line_style == LineFamily::LIGHT)
//			cairo_set_source_rgb(cr, 0.15, 0.6, 0.25);
//		else if (state.line_style == LineFamily::DOUBLE)
//			cairo_set_source_rgb(cr, 0.7, 0.3, 0.8);
//		else if (state.line_style == LineFamily::DASHED)
//			cairo_set_source_rgb(cr, 0.85, 0.55, 0.1);
//		else if (state.line_style == LineFamily::THICK)
//			cairo_set_source_rgb(cr, 0.5, 0.1, 0.5);
//
//		cairo_set_line_width(cr, 2.0);
//		cairo_rectangle(cr, state.cx * CELL_W + 1, state.cy * CELL_H + 1, CELL_W - 2, CELL_H - 2);
//		cairo_stroke(cr);
//	} else if (state.insert_mode) {
//		cairo_set_source_rgb(cr, 0.9, 0.25, 0.25);
//		cairo_set_line_width(cr, 2.0);
//		double bar_x = state.cx * CELL_W;
//		cairo_move_to(cr, bar_x, state.cy * CELL_H + 3);
//		cairo_line_to(cr, bar_x, (state.cy + 1) * CELL_H - 3);
//		cairo_stroke(cr);
//	} else {
//		cairo_set_source_rgba(cr, 0.2, 0.5, 0.9, 0.25);
//		cairo_rectangle(cr, state.cx * CELL_W, state.cy * CELL_H, CELL_W, CELL_H);
//		cairo_fill(cr);
//		cairo_set_source_rgb(cr, 0.2, 0.5, 0.9);
//		cairo_set_line_width(cr, 1.5);
//		cairo_rectangle(cr, state.cx * CELL_W, state.cy * CELL_H, CELL_W, CELL_H);
//		cairo_stroke(cr);
//	}
//}
//
//static void on_draw(GtkDrawingArea *area, cairo_t *cr, int width, int height, gpointer ud) {
//	draw_grid(cr, width, height);
//}
//
///* --- Teclado --- */
//static gboolean on_key(GtkEventControllerKey *ctrl, guint keyval, guint keycode, GdkModifierType mod, gpointer ud) {
//	if (keyval == GDK_KEY_Insert) {
//		state.insert_mode = !state.insert_mode;
//		update_status();
//		gtk_widget_queue_draw(state.drawing_area);
//		return TRUE;
//	}
//
//	state.is_selecting = FALSE;
//
//	if (state.tool == Tool::LINE) {
//		int nx = state.cx;
//		int ny = state.cy;
//		gboolean moved = FALSE;
//		switch (keyval) {
//		case GDK_KEY_Up:
//			ny = MAX(0, state.cy - 1);
//			moved = TRUE;
//			break;
//		case GDK_KEY_Down:
//			ny = MIN(state.rows - 1, state.cy + 1);
//			moved = TRUE;
//			break;
//		case GDK_KEY_Left:
//			nx = MAX(0, state.cx - 1);
//			moved = TRUE;
//			break;
//		case GDK_KEY_Right:
//			nx = MIN(state.cols - 1, state.cx + 1);
//			moved = TRUE;
//			break;
//		default:
//			break;
//		}
//		if (moved) {
//			state.line_last_horizontal = (nx != state.cx);
//			place_line_at(state.cx, state.cy);
//			place_line_at(nx, ny);
//			state.cx = nx;
//			state.cy = ny;
//			update_status();
//			gtk_widget_queue_draw(state.drawing_area);
//			return TRUE;
//		}
//		if (keyval == GDK_KEY_space || keyval == GDK_KEY_Delete) {
//			erase_line_at(state.cx, state.cy);
//			if (keyval == GDK_KEY_space && state.cx < state.cols - 1)
//				state.cx++;
//			update_status();
//			gtk_widget_queue_draw(state.drawing_area);
//			return TRUE;
//		}
//		if (keyval == GDK_KEY_BackSpace) {
//			if (state.cx > 0)
//				state.cx--;
//			erase_line_at(state.cx, state.cy);
//			update_status();
//			gtk_widget_queue_draw(state.drawing_area);
//			return TRUE;
//		}
//	}
//
//	switch (keyval) {
//	case GDK_KEY_Up:
//		state.cy = MAX(0, state.cy - 1);
//		break;
//	case GDK_KEY_Down:
//		state.cy = MIN(state.rows - 1, state.cy + 1);
//		break;
//	case GDK_KEY_Left:
//		state.cx = MAX(0, state.cx - 1);
//		break;
//	case GDK_KEY_Right:
//		state.cx = MIN(state.cols - 1, state.cx + 1);
//		break;
//	case GDK_KEY_BackSpace:
//		if (state.cx > 0)
//			state.cx--;
//		delete_at_cursor();
//		break;
//	case GDK_KEY_Delete:
//		delete_at_cursor();
//		break;
//	case GDK_KEY_space:
//		place_char(' ');
//		break;
//	case GDK_KEY_Return:
//	case GDK_KEY_KP_Enter:
//		state.cx = 0;
//		state.cy = MIN(state.rows - 1, state.cy + 1);
//		break;
//	case GDK_KEY_Home:
//		state.cx = 0;
//		break;
//	case GDK_KEY_End:
//		state.cx = state.cols - 1;
//		break;
//	case GDK_KEY_Tab:
//		state.cx = MIN(state.cols - 1, state.cx + 1);
//		break;
//	default: {
//		gunichar c = keyval_to_char(keyval);
//		if (c != 0)
//			place_char(c);
//		else
//			return FALSE;
//		break;
//	}
//	}
//
//	update_status();
//	gtk_widget_queue_draw(state.drawing_area);
//	return TRUE;
//}
//
///* --- Herramienta Línea --- */
//static void line_tool_step(int x, int y) {
//	x = CLAMP(x, 0, state.cols - 1);
//	y = CLAMP(y, 0, state.rows - 1);
//
//	if (state.line_last_x < 0) {
//		place_line_at(x, y);
//		state.line_last_horizontal = TRUE;
//	} else {
//		int dx = x - state.line_last_x;
//		int dy = y - state.line_last_y;
//		if (dx != 0 || dy != 0) {
//			state.line_last_horizontal = std::abs(dx) >= std::abs(dy);
//		}
//		int cx0 = state.line_last_x;
//		int cy0 = state.line_last_y;
//		while (cx0 != x || cy0 != y) {
//			if (cx0 != x)
//				cx0 += (x > cx0) ? 1 : -1;
//			else if (cy0 != y)
//				cy0 += (y > cy0) ? 1 : -1;
//			place_line_at(cx0, cy0);
//		}
//	}
//	state.line_last_x = x;
//	state.line_last_y = y;
//	recompute_all();
//}
//
///* --- Ratón --- */
//static void on_drag_begin(GtkGestureDrag *gest, double start_x, double start_y, gpointer ud) {
//	gint64 now = g_get_monotonic_time();
//	if (now - state.last_mouse_time < 30000) {
//		return;
//	}
//	state.last_mouse_time = now;
//
//	gtk_widget_grab_focus(state.drawing_area);
//	int gx = CLAMP((int) (start_x / CELL_W), 0, state.cols - 1);
//	int gy = CLAMP((int) (start_y / CELL_H), 0, state.rows - 1);
//	state.cx = gx;
//	state.cy = gy;
//
//	if (state.tool == Tool::LINE) {
//		state.is_selecting = FALSE; // Limpiamos selección previa al empezar a dibujar
//		state.line_last_x = -1;
//		state.line_last_y = -1;
//		line_tool_step(gx, gy);
//	} else {
//		state.is_selecting = TRUE;
//		state.sel_x1 = state.sel_x2 = gx;
//		state.sel_y1 = state.sel_y2 = gy;
//	}
//	update_status();
//	gtk_widget_queue_draw(state.drawing_area);
//}
//
//static void on_drag_update(GtkGestureDrag *gest, double offset_x, double offset_y, gpointer ud) {
//	gint64 now = g_get_monotonic_time();
//	// Si no han pasado al menos 30 ms desde el último evento, ignoramos este
//	if (now - state.last_mouse_time < 30000) {   // 30 000 µs = 30 ms
//		return;
//	}
//	state.last_mouse_time = now;
//
//	double start_x, start_y;
//	if (!gtk_gesture_drag_get_start_point(gest, &start_x, &start_y))
//		return;
//	int gx = CLAMP((int) ((start_x + offset_x) / CELL_W), 0, state.cols - 1);
//	int gy = CLAMP((int) ((start_y + offset_y) / CELL_H), 0, state.rows - 1);
//
//	if (state.tool == Tool::LINE) {
//		line_tool_step(gx, gy);
//		state.cx = gx;
//		state.cy = gy;
//	} else {
//		state.sel_x2 = gx;
//		state.sel_y2 = gy;
//	}
//	gtk_widget_queue_draw(state.drawing_area);
//}
//
//static void on_drag_end(GtkGestureDrag *gest, double offset_x, double offset_y, gpointer ud) {
//	if (state.tool == Tool::LINE) {
//		state.line_last_x = -1;
//		state.line_last_y = -1;
//		state.is_selecting = FALSE;
//	} else if (state.sel_x1 == state.sel_x2 && state.sel_y1 == state.sel_y2) {
//		state.is_selecting = FALSE;
//	}
//	gtk_widget_queue_draw(state.drawing_area);
//}
//
///* --- Paleta --- */
//static void on_palette_click(GtkButton *b, gpointer ud) {
//	gunichar c = GPOINTER_TO_UINT(ud);
//	if (state.is_selecting) {
//		fill_selection(c, LineFamily::NONE);
//		state.is_selecting = FALSE;
//	} else {
//		place_char(c, LineFamily::NONE);
//	}
//	update_status();
//	gtk_widget_queue_draw(state.drawing_area);
//}
//
//static GtkWidget* make_btn(const char *label, gunichar ch) {
//	GtkWidget *btn = gtk_button_new_with_label(label);
//	gtk_widget_set_size_request(btn, 36, 36);
//	g_signal_connect(btn, "clicked", G_CALLBACK(on_palette_click), GUINT_TO_POINTER(ch));
//	return btn;
//}
//
//static GtkWidget* make_section(const char *title, const char *chars) {
//	GtkWidget *frame = gtk_frame_new(title);
//	gtk_widget_set_margin_top(frame, 6);
//	gtk_widget_set_margin_start(frame, 6);
//	gtk_widget_set_margin_end(frame, 6);
//
//	GtkWidget *flow = gtk_flow_box_new();
//	gtk_flow_box_set_max_children_per_line(GTK_FLOW_BOX(flow), 6);
//	gtk_flow_box_set_selection_mode(GTK_FLOW_BOX(flow), GTK_SELECTION_NONE);
//	gtk_widget_set_margin_top(flow, 6);
//	gtk_widget_set_margin_bottom(flow, 6);
//	gtk_widget_set_margin_start(flow, 6);
//	gtk_widget_set_margin_end(flow, 6);
//
//	const char *p = chars;
//	while (*p) {
//		gunichar c = g_utf8_get_char(p);
//		char buf[8] = { 0 };
//		g_unichar_to_utf8(c, buf);
//		gtk_flow_box_append(GTK_FLOW_BOX(flow), make_btn(buf, c));
//		p = g_utf8_next_char(p);
//	}
//
//	gtk_frame_set_child(GTK_FRAME(frame), flow);
//	return frame;
//}
//
//static void build_palette(GtkWidget *box) {
//	gtk_box_append(GTK_BOX(box), make_section("Bloques", "○●◇◆◎◯◻◼□■▀▄▌▐▢▣◐◓"));
//	gtk_box_append(GTK_BOX(box), make_section("Formas geométricas", "▯▰▱▲△▵▶▷▸▹►▻▼▽▾▿◀◁◂◃◄◅◆◇◈◉◊○◌◍◎●"));
//	gtk_box_append(GTK_BOX(box), make_section("Líneas", "─━│┃═║┄┅┆┇"));
//	gtk_box_append(GTK_BOX(box), make_section("Medias líneas", "╴╵╶╷╸╹╺╻"));
//	gtk_box_append(GTK_BOX(box), make_section("Esquinas simples", "┌┏┍┎┐┓┑┒└┗┕┖┘┛┙┚"));
//	gtk_box_append(GTK_BOX(box), make_section("Esquinas redondas", "╭╮╯╰"));
//	gtk_box_append(GTK_BOX(box), make_section("Esquinas dobles", "╔╓╒╗╖╕╚╙╘╝╜╛"));
//	gtk_box_append(GTK_BOX(box), make_section("Cruces simples", "├┝┞┟┠┡┢┣┤┥┦┧┨┩┪┫┬┭┮┯┰┱┲┳┴┵┶┷┸┹┺┻┼┽┾┿╀╁"));
//	gtk_box_append(GTK_BOX(box), make_section("Cruces dobles", "╠╟╞╣╢╡╦╥╤╩╨╧╬╫╪"));
//}
//
///* --- Botones toolbar --- */
//static void on_toggle_insert(GtkButton *btn, gpointer ud) {
//	state.insert_mode = !state.insert_mode;
//	gtk_button_set_label(btn, state.insert_mode ? "🔴 INSERT" : "🔵 REPLACE");
//	update_status();
//	gtk_widget_queue_draw(state.drawing_area);
//}
//
//static void on_toggle_tool(GtkToggleButton *btn, gpointer ud) {
//	gboolean active = gtk_toggle_button_get_active(btn);
//	state.tool = active ? Tool::LINE : Tool::SELECT;
//	gtk_button_set_label(GTK_BUTTON(btn), active ? "✏️ Línea (ON)" : "✏️ Línea");
//	update_status();
//	gtk_widget_queue_draw(state.drawing_area);
//}
//
//static void on_style_changed(GObject *self, GParamSpec *pspec, gpointer ud) {
//	guint idx = gtk_drop_down_get_selected(GTK_DROP_DOWN(self));
//	switch (idx) {
//	case 0:
//		state.line_style = LineFamily::LIGHT;
//		break;
//	case 1:
//		state.line_style = LineFamily::DOUBLE;
//		break;
//	case 2:
//		state.line_style = LineFamily::DASHED;
//		break;
//	case 3:
//		state.line_style = LineFamily::THICK;
//		break;
//	default:
//		break;
//	}
//	update_status();
//	gtk_widget_queue_draw(state.drawing_area);
//}
//
//static void on_clear_click(GtkButton *b, gpointer ud) {
//	if (state.is_selecting) {
//		clear_selection();
//		state.is_selecting = FALSE;
//	} else {
//		delete_at_cursor();
//	}
//	update_status();
//	gtk_widget_queue_draw(state.drawing_area);
//}
//
///* --- Filas / Columnas --- */
//static void on_insert_row_above(GtkButton *b, gpointer ud) {
//	insert_row(state.cy, false);
//	update_status();
//}
//static void on_insert_row_below(GtkButton *b, gpointer ud) {
//	insert_row(state.cy, true);
//	update_status();
//}
//static void on_delete_row(GtkButton *b, gpointer ud) {
//	delete_row(state.cy);
//	update_status();
//}
//static void on_insert_col_left(GtkButton *b, gpointer ud) {
//	insert_col(state.cx, false);
//	update_status();
//}
//static void on_insert_col_right(GtkButton *b, gpointer ud) {
//	insert_col(state.cx, true);
//	update_status();
//}
//static void on_delete_col(GtkButton *b, gpointer ud) {
//	delete_col(state.cx);
//	update_status();
//}
//
///* --- Main App --- */
//static void on_app_activate(GApplication *app, gpointer ud) {
//	GtkWidget *win = gtk_application_window_new(GTK_APPLICATION(app));
//	gtk_window_set_title(GTK_WINDOW(win), "Editor de Esquemas Unicode — GTK4");
//	gtk_window_set_default_size(GTK_WINDOW(win), 1400, 850);
//
//	GtkWidget *paned = gtk_paned_new(GTK_ORIENTATION_HORIZONTAL);
//	gtk_window_set_child(GTK_WINDOW(win), paned);
//
//	GtkWidget *scroll_pal = gtk_scrolled_window_new();
//	gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scroll_pal), GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC);
//	gtk_widget_set_size_request(scroll_pal, 300, -1);
//
//	GtkWidget *vbox_pal = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
//	build_palette(vbox_pal);
//	gtk_scrolled_window_set_child(GTK_SCROLLED_WINDOW(scroll_pal), vbox_pal);
//	gtk_paned_set_start_child(GTK_PANED(paned), scroll_pal);
//
//	GtkWidget *vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
//	gtk_paned_set_end_child(GTK_PANED(paned), vbox);
//
//	GtkWidget *header = gtk_header_bar_new();
//	gtk_window_set_titlebar(GTK_WINDOW(win), header);
//
//	state.btn_insert = gtk_button_new_with_label("🔵 REPLACE");
//	g_signal_connect(state.btn_insert, "clicked", G_CALLBACK(on_toggle_insert), NULL);
//	gtk_header_bar_pack_start(GTK_HEADER_BAR(header), state.btn_insert);
//
//	state.btn_tool_line = gtk_toggle_button_new();
//	gtk_button_set_label(GTK_BUTTON(state.btn_tool_line), "✏️ Línea");
//	g_signal_connect(state.btn_tool_line, "toggled", G_CALLBACK(on_toggle_tool), NULL);
//	gtk_header_bar_pack_start(GTK_HEADER_BAR(header), state.btn_tool_line);
//
//	const char *styles[] = { "Sencilla", "Doble", "Punteada", "Gruesa", NULL };
//	state.style_dropdown = GTK_DROP_DOWN(gtk_drop_down_new_from_strings(styles));
//	g_signal_connect(state.style_dropdown, "notify::selected", G_CALLBACK(on_style_changed), NULL);
//	gtk_header_bar_pack_start(GTK_HEADER_BAR(header), GTK_WIDGET(state.style_dropdown));
//
//	GtkWidget *btn_save = gtk_button_new_with_label("💾 Guardar");
//	g_signal_connect(btn_save, "clicked", G_CALLBACK(SistemaArchivos::guardar_archivo_dialogo), NULL);
//	gtk_header_bar_pack_start(GTK_HEADER_BAR(header), btn_save);
//
//	GtkWidget *btn_clear = gtk_button_new_with_label("🗑️ Borrar");
//	g_signal_connect(btn_clear, "clicked", G_CALLBACK(on_clear_click), NULL);
//	gtk_header_bar_pack_start(GTK_HEADER_BAR(header), btn_clear);
//
//	GtkWidget *menu_btn = gtk_menu_button_new();
//	gtk_menu_button_set_label(GTK_MENU_BUTTON(menu_btn), "📐 Filas/Col");
//	GtkWidget *popover = gtk_popover_new();
//	GtkWidget *pbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 4);
//	gtk_widget_set_margin_top(pbox, 6);
//	gtk_widget_set_margin_bottom(pbox, 6);
//	gtk_widget_set_margin_start(pbox, 6);
//	gtk_widget_set_margin_end(pbox, 6);
//
//	struct {
//		const char *label;
//		GCallback cb;
//	} row_col_actions[] = { { "⬆️ Insertar fila arriba", G_CALLBACK(on_insert_row_above) }, { "⬇️ Insertar fila abajo", G_CALLBACK(on_insert_row_below) }, { "❌ Eliminar fila actual", G_CALLBACK(
//			on_delete_row) }, { "⬅️ Insertar columna izq.", G_CALLBACK(on_insert_col_left) }, { "➡️ Insertar columna der.", G_CALLBACK(on_insert_col_right) }, { "❌ Eliminar columna actual", G_CALLBACK(
//			on_delete_col) } };
//	for (auto &act : row_col_actions) {
//		GtkWidget *btn = gtk_button_new_with_label(act.label);
//		g_signal_connect(btn, "clicked", act.cb, NULL);
//		gtk_box_append(GTK_BOX(pbox), btn);
//	}
//	gtk_popover_set_child(GTK_POPOVER(popover), pbox);
//	gtk_menu_button_set_popover(GTK_MENU_BUTTON(menu_btn), popover);
//	gtk_header_bar_pack_start(GTK_HEADER_BAR(header), menu_btn);
//
//	state.drawing_area = gtk_drawing_area_new();
//	gtk_drawing_area_set_content_width(GTK_DRAWING_AREA(state.drawing_area), state.cols * CELL_W);
//	gtk_drawing_area_set_content_height(GTK_DRAWING_AREA(state.drawing_area), state.rows * CELL_H);
//	gtk_drawing_area_set_draw_func(GTK_DRAWING_AREA(state.drawing_area), on_draw, NULL, NULL);
//	gtk_widget_set_can_focus(state.drawing_area, TRUE);
//	gtk_widget_set_focusable(state.drawing_area, TRUE);
//
//	GtkWidget *scroll = gtk_scrolled_window_new();
//	gtk_scrolled_window_set_child(GTK_SCROLLED_WINDOW(scroll), state.drawing_area);
//	gtk_widget_set_vexpand(scroll, TRUE);
//	gtk_box_append(GTK_BOX(vbox), scroll);
//
//	state.status = gtk_label_new("");
//	gtk_widget_set_margin_top(state.status, 6);
//	gtk_widget_set_margin_bottom(state.status, 6);
//	gtk_widget_set_margin_start(state.status, 10);
//	gtk_box_append(GTK_BOX(vbox), state.status);
//	update_status();
//
//	GtkEventController *keyc = gtk_event_controller_key_new();
//	g_signal_connect(keyc, "key-pressed", G_CALLBACK(on_key), NULL);
//	gtk_widget_add_controller(state.drawing_area, keyc);
//
//	GtkGesture *drag = gtk_gesture_drag_new();
//	g_signal_connect(drag, "drag-begin", G_CALLBACK(on_drag_begin), NULL);
//	g_signal_connect(drag, "drag-update", G_CALLBACK(on_drag_update), NULL);
//	g_signal_connect(drag, "drag-end", G_CALLBACK(on_drag_end), NULL);
//	gtk_widget_add_controller(state.drawing_area, GTK_EVENT_CONTROLLER(drag));
//
//	gtk_widget_grab_focus(state.drawing_area);
//	g_signal_connect_swapped(win, "map", G_CALLBACK(gtk_widget_grab_focus), state.drawing_area);
//	gtk_window_present(GTK_WINDOW(win));
//}
//
//static void cleanup_resources(void) {
//	if (cached_font_desc) {
//		pango_font_description_free(cached_font_desc);
//		cached_font_desc = nullptr;
//	}
//}
//
//int main(int argc, char **argv) {
//	int st = 1;
//	try {
//		grid_init();
//		GtkApplication *app = gtk_application_new("com.example.boxeditor", G_APPLICATION_DEFAULT_FLAGS);
//		g_signal_connect(app, "activate", G_CALLBACK(on_app_activate), NULL);
//		st = g_application_run(G_APPLICATION(app), argc, argv);
//		g_object_unref(app);
//		cleanup_resources();
//	} catch (const std::exception &e) {
//		std::cerr << "[Fatal Exception] " << e.what() << "\n";
//		st = EXIT_FAILURE;
//	} catch (...) {
//		std::cerr << "[Fatal Exception] Error indeterminado.\n";
//		st = EXIT_FAILURE;
//	}
//
//	return st;
//}
