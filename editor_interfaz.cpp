/*
 * editor_interfaz.cpp
 *
 * Created on: 4 ago 2026
 * Author: DjSteker
 */

#include "editor_interfaz.hpp"
#include <algorithm>
#include <cstdio>

/* --- Inicialización estática de la fuente Pango --- */
PangoFontDescription *EditorInterfaz::cached_font_desc = nullptr;

/* --- Constructor y Destructor --- */
EditorInterfaz::EditorInterfaz() :
		ventana(nullptr), drawing_area(nullptr), status(nullptr), btn_insert(nullptr), btn_tool_line(nullptr), style_dropdown(nullptr), cx(0), cy(0), is_selecting(false), insert_mode(false), sel_x1(0), sel_y1(
				0), sel_x2(0), sel_y2(0), tool(Tool::SELECT), line_style(LineFamily::LIGHT), line_last_x(-1), line_last_y(-1), line_last_horizontal(true), last_mouse_time(0) {
}

EditorInterfaz::~EditorInterfaz() {
	if (cached_font_desc) {
		pango_font_description_free(cached_font_desc);
		cached_font_desc = nullptr;
	}
}

/* --- Métodos públicos --- */

void EditorInterfaz::inicializar(GtkApplication *app) {
	/* Crear ventana principal */
	ventana = gtk_application_window_new(app);
	gtk_window_set_title(GTK_WINDOW(ventana), "Editor de Esquemas Unicode — GTK4");
	gtk_window_set_default_size(GTK_WINDOW(ventana), 1400, 850);

	/* Paned horizontal: paleta a la izquierda, lienzo a la derecha */
	GtkWidget *paned = gtk_paned_new(GTK_ORIENTATION_HORIZONTAL);
	gtk_window_set_child(GTK_WINDOW(ventana), paned);

	/* ---------- Panel izquierdo: paleta ---------- */
	GtkWidget *scroll_pal = gtk_scrolled_window_new();
	gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scroll_pal), GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC);
	gtk_widget_set_size_request(scroll_pal, 300, -1);
	GtkWidget *vbox_pal = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
	construir_paleta(vbox_pal);
	gtk_scrolled_window_set_child(GTK_SCROLLED_WINDOW(scroll_pal), vbox_pal);
	gtk_paned_set_start_child(GTK_PANED(paned), scroll_pal);

	/* ---------- Panel derecho: lienzo + barra de estado ---------- */
	GtkWidget *vbox_der = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
	gtk_paned_set_end_child(GTK_PANED(paned), vbox_der);

	/* Barra de herramientas (header) */
	GtkWidget *header = gtk_header_bar_new();
	gtk_window_set_titlebar(GTK_WINDOW(ventana), header);

	/* Botón INSERT / REPLACE */
	btn_insert = gtk_button_new_with_label("🔵 REPLACE");
	g_signal_connect(btn_insert, "clicked", G_CALLBACK(on_toggle_insert), this);
	gtk_header_bar_pack_start(GTK_HEADER_BAR(header), btn_insert);

	/* Botón toggle herramienta Línea */
	btn_tool_line = gtk_toggle_button_new();
	gtk_button_set_label(GTK_BUTTON(btn_tool_line), "✏️ Línea");
	g_signal_connect(btn_tool_line, "toggled", G_CALLBACK(on_toggle_tool), this);
	gtk_header_bar_pack_start(GTK_HEADER_BAR(header), btn_tool_line);

	/* Selector de estilo de línea */
	const char *styles[] = { "Sencilla", "Doble", "Punteada", "Gruesa", nullptr };
	style_dropdown = GTK_DROP_DOWN(gtk_drop_down_new_from_strings(styles));
	g_signal_connect(style_dropdown, "notify::selected", G_CALLBACK(on_style_changed), this);
	gtk_header_bar_pack_start(GTK_HEADER_BAR(header), GTK_WIDGET(style_dropdown));

	/* Botón Guardar */
	GtkWidget *btn_save = gtk_button_new_with_label("💾 Guardar");
	g_signal_connect(btn_save, "clicked", G_CALLBACK(on_btn_save_clicked), this);
	gtk_header_bar_pack_start(GTK_HEADER_BAR(header), btn_save);

	/* Botón Cargar */
	GtkWidget *btn_load = gtk_button_new_with_label("📂 Cargar");
	g_signal_connect(btn_load, "clicked", G_CALLBACK(on_btn_load_clicked), this);
	gtk_header_bar_pack_start(GTK_HEADER_BAR(header), btn_load);

	/* Botón Borrar (clear) */
	GtkWidget *btn_clear = gtk_button_new_with_label("🗑️ Borrar");
	g_signal_connect(btn_clear, "clicked", G_CALLBACK(on_btn_clear_clicked), this);
	gtk_header_bar_pack_start(GTK_HEADER_BAR(header), btn_clear);

	/* Menú de filas/columnas */
	GtkWidget *menu_btn = gtk_menu_button_new();
	gtk_menu_button_set_label(GTK_MENU_BUTTON(menu_btn), "📐 Filas/Col");
	GtkWidget *popover = gtk_popover_new();
	GtkWidget *pbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 4);
	gtk_widget_set_margin_top(pbox, 6);
	gtk_widget_set_margin_bottom(pbox, 6);
	gtk_widget_set_margin_start(pbox, 6);
	gtk_widget_set_margin_end(pbox, 6);

	struct {
		const char *label;
		const char *action;
	} row_col_actions[] = { { "⬆️ Insertar fila arriba", "insert_row_above" }, { "⬇️ Insertar fila abajo", "insert_row_below" }, { "❌ Eliminar fila actual", "delete_row" }, { "⬅️ Insertar columna izq.",
			"insert_col_left" }, { "➡️ Insertar columna der.", "insert_col_right" }, { "❌ Eliminar columna actual", "delete_col" } };
	for (auto &act : row_col_actions) {
		GtkWidget *btn = gtk_button_new_with_label(act.label);
		g_object_set_data(G_OBJECT(btn), "row-col-action", (gpointer) act.action);
		g_signal_connect(btn, "clicked", G_CALLBACK(on_row_col_action), this);
		gtk_box_append(GTK_BOX(pbox), btn);
	}
	gtk_popover_set_child(GTK_POPOVER(popover), pbox);
	gtk_menu_button_set_popover(GTK_MENU_BUTTON(menu_btn), popover);
	gtk_header_bar_pack_start(GTK_HEADER_BAR(header), menu_btn);

	/* ---------- Lienzo de dibujo ---------- */
	procesador.inicializar(INIT_ROWS, INIT_COLS);
	drawing_area = gtk_drawing_area_new();
	gtk_drawing_area_set_content_width(GTK_DRAWING_AREA(drawing_area), procesador.get_columnas() * CELL_W);
	gtk_drawing_area_set_content_height(GTK_DRAWING_AREA(drawing_area), procesador.get_filas() * CELL_H);
	gtk_drawing_area_set_draw_func(GTK_DRAWING_AREA(drawing_area), dibujar_lienzo, this, nullptr);
	gtk_widget_set_can_focus(drawing_area, TRUE);
	gtk_widget_set_focusable(drawing_area, TRUE);

	GtkWidget *scroll_lienzo = gtk_scrolled_window_new();
	gtk_scrolled_window_set_child(GTK_SCROLLED_WINDOW(scroll_lienzo), drawing_area);
	gtk_widget_set_vexpand(scroll_lienzo, TRUE);
	gtk_box_append(GTK_BOX(vbox_der), scroll_lienzo);

	/* Barra de estado */
	status = gtk_label_new("");
	gtk_widget_set_margin_top(status, 6);
	gtk_widget_set_margin_bottom(status, 6);
	gtk_widget_set_margin_start(status, 10);
	gtk_box_append(GTK_BOX(vbox_der), status);
	actualizar_estado();

	/* ---------- Controladores de eventos ---------- */
	/* Teclado */
	GtkEventController *keyc = gtk_event_controller_key_new();
	g_signal_connect(keyc, "key-pressed", G_CALLBACK(on_key_press), this);
	gtk_widget_add_controller(drawing_area, keyc);

	/* Gestos de arrastre (ratón) */
	GtkGesture *drag = gtk_gesture_drag_new();
	g_signal_connect(drag, "drag-begin", G_CALLBACK(on_drag_begin), this);
	g_signal_connect(drag, "drag-update", G_CALLBACK(on_drag_update), this);
	g_signal_connect(drag, "drag-end", G_CALLBACK(on_drag_end), this);
	gtk_widget_add_controller(drawing_area, GTK_EVENT_CONTROLLER(drag));

	/* Dar foco al lienzo para capturar teclado */
	gtk_widget_grab_focus(drawing_area);
	g_signal_connect_swapped(ventana, "map", G_CALLBACK(gtk_widget_grab_focus), drawing_area);

	gtk_window_present(GTK_WINDOW(ventana));
}

void EditorInterfaz::actualizar_estado() {
	gchar *buf = g_strdup_printf("Modo: %s | Herramienta: %s (%s) | Cursor: (%d,%d) | Tamaño: %dx%d | "
			"Insert=toggle | %s | Click+Drag=%s", insert_mode ? "INSERT" : "REPLACE", tool == Tool::LINE ? "Línea" : "Selección", ProcesadorCeldas::nombre_estilo(line_style), cx, cy,
			procesador.get_columnas(), procesador.get_filas(), tool == Tool::LINE ? "Flechas=dibujar línea" : "Flechas=mover", tool == Tool::LINE ? "trazar línea" : "seleccionar");
	gtk_label_set_text(GTK_LABEL(status), buf);
	g_free(buf);
}

void EditorInterfaz::redimensionar_lienzo() {
	gtk_drawing_area_set_content_width(GTK_DRAWING_AREA(drawing_area), procesador.get_columnas() * CELL_W);
	gtk_drawing_area_set_content_height(GTK_DRAWING_AREA(drawing_area), procesador.get_filas() * CELL_H);
	gtk_widget_queue_draw(drawing_area);
}

void EditorInterfaz::callback_cargar_archivo(const char *contenido, gsize longitud, gpointer user_data) {
	EditorInterfaz *editor = static_cast<EditorInterfaz*>(user_data);
	if (contenido && longitud > 0) {
		editor->procesador.importar_desde_string(contenido, longitud);
		editor->redimensionar_lienzo();
		editor->actualizar_estado();
	}
}

/* --- Métodos privados: construcción de la paleta --- */
GtkWidget* EditorInterfaz::crear_boton_paleta(const char *label, gunichar ch) {
	GtkWidget *btn = gtk_button_new_with_label(label);
	gtk_widget_set_size_request(btn, 36, 36);
	g_object_set_data(G_OBJECT(btn), "char-code", GUINT_TO_POINTER(ch));
	g_signal_connect(btn, "clicked", G_CALLBACK(on_palette_click), this);
	return btn;
}

GtkWidget* EditorInterfaz::crear_seccion_paleta(const char *title, const char *chars) {
	GtkWidget *frame = gtk_frame_new(title);
	gtk_widget_set_margin_top(frame, 6);
	gtk_widget_set_margin_start(frame, 6);
	gtk_widget_set_margin_end(frame, 6);

	GtkWidget *flow = gtk_flow_box_new();
	gtk_flow_box_set_max_children_per_line(GTK_FLOW_BOX(flow), 6);
	gtk_flow_box_set_selection_mode(GTK_FLOW_BOX(flow), GTK_SELECTION_NONE);
	gtk_widget_set_margin_top(flow, 6);
	gtk_widget_set_margin_bottom(flow, 6);
	gtk_widget_set_margin_start(flow, 6);
	gtk_widget_set_margin_end(flow, 6);

	const char *p = chars;
	while (*p) {
		gunichar c = g_utf8_get_char(p);
		char buf[8] = { 0 };
		g_unichar_to_utf8(c, buf);
		gtk_flow_box_append(GTK_FLOW_BOX(flow), crear_boton_paleta(buf, c));
		p = g_utf8_next_char(p);
	}

	gtk_frame_set_child(GTK_FRAME(frame), flow);
	return frame;
}

void EditorInterfaz::construir_paleta(GtkWidget *box) {
	gtk_box_append(GTK_BOX(box), crear_seccion_paleta("Bloques", "○●◇◆◎◯◻◼□■▀▄▌▐▢▣◐◓"));
	gtk_box_append(GTK_BOX(box), crear_seccion_paleta("Formas geométricas", "▯▰▱▲△▵▶▷▸▹►▻▼▽▾▿◀◁◂◃◄◅◆◇◈◉◊○◌◍◎●"));
	gtk_box_append(GTK_BOX(box), crear_seccion_paleta("Líneas", "─━│┃═║┄┅┆┇"));
	gtk_box_append(GTK_BOX(box), crear_seccion_paleta("Medias líneas", "╴╵╶╷╸╹╺╻"));
	gtk_box_append(GTK_BOX(box), crear_seccion_paleta("Esquinas simples", "┌┏┍┎┐┓┑┒└┗┕┖┘┛┙┚"));
	gtk_box_append(GTK_BOX(box), crear_seccion_paleta("Esquinas redondas", "╭╮╯╰"));
	gtk_box_append(GTK_BOX(box), crear_seccion_paleta("Esquinas dobles", "╔╓╒╗╖╕╚╙╘╝╜╛"));
	gtk_box_append(GTK_BOX(box), crear_seccion_paleta("Cruces simples", "├┝┞┟┠┡┢┣┤┥┦┧┨┩┪┫┬┭┮┯┰┱┲┳┴┵┶┷┸┹┺┻┼┽┾┿╀╁"));
	gtk_box_append(GTK_BOX(box), crear_seccion_paleta("Cruces dobles", "╠╟╞╣╢╡╦╥╤╩╨╧╬╫╪"));
}

/* --- Herramienta Línea --- */
void EditorInterfaz::line_tool_step(int gx, int gy) {
	gx = std::clamp(gx, 0, procesador.get_columnas() - 1);
	gy = std::clamp(gy, 0, procesador.get_filas() - 1);

	if (line_last_x < 0) {
		/* Primer punto del trazo: aún no hay dirección que decidir */
		if (line_last_horizontal) {
			procesador.colocar_linea_horizontal(gx, gy, line_style);
		} else {
			procesador.colocar_linea_vertical(gx, gy, line_style);
		}
		line_last_x = gx;
		line_last_y = gy;
		procesador.recalcular_todo();
		return;
	}

	int dx = gx - line_last_x;
	int dy = gy - line_last_y;
	if (dx == 0 && dy == 0) {
		return;
	}

	/* Recorrido en "L": primero el eje dominante, luego el otro, celda a
	 * celda. colocar_linea_horizontal/vertical NUNCA borran la familia
	 * contraria, así que la celda de giro acumula ambas familias y
	 * recalcular_celda puede detectar la esquina correctamente. */
	int cx = line_last_x;
	int cy = line_last_y;
	bool horiz_first = std::abs(dx) >= std::abs(dy);

	if (horiz_first) {
		int stepx = (gx > cx) ? 1 : -1;
		while (cx != gx) {
			int nx = cx + stepx;
			procesador.colocar_linea_horizontal(cx, cy, line_style);
			procesador.colocar_linea_horizontal(nx, cy, line_style);
			cx = nx;
		}
		int stepy = (gy > cy) ? 1 : -1;
		while (cy != gy) {
			int ny = cy + stepy;
			procesador.colocar_linea_vertical(cx, cy, line_style);
			procesador.colocar_linea_vertical(cx, ny, line_style);
			cy = ny;
		}
		line_last_horizontal = (gy == line_last_y);
	} else {
		int stepy = (gy > cy) ? 1 : -1;
		while (cy != gy) {
			int ny = cy + stepy;
			procesador.colocar_linea_vertical(cx, cy, line_style);
			procesador.colocar_linea_vertical(cx, ny, line_style);
			cy = ny;
		}
		int stepx = (gx > cx) ? 1 : -1;
		while (cx != gx) {
			int nx = cx + stepx;
			procesador.colocar_linea_horizontal(cx, cy, line_style);
			procesador.colocar_linea_horizontal(nx, cy, line_style);
			cx = nx;
		}
		line_last_horizontal = (gx != line_last_x);
	}

	line_last_x = gx;
	line_last_y = gy;

	procesador.recalcular_todo();
}

/* ======================================================================
 * Callbacks estáticos
 * ====================================================================== */

/* Dibujar el lienzo */
void EditorInterfaz::dibujar_lienzo(GtkDrawingArea *area, cairo_t *cr, int width, int height, gpointer user_data) {
	EditorInterfaz *editor = static_cast<EditorInterfaz*>(user_data);
	ProcesadorCeldas &proc = editor->procesador;

	/* Fondo */
	cairo_set_source_rgb(cr, 0.96, 0.96, 0.96);
	cairo_paint(cr);

	/* Rejilla gris claro */
	cairo_set_source_rgb(cr, 0.82, 0.82, 0.82);
	cairo_set_line_width(cr, 0.5);
	for (int y = 0; y <= proc.get_filas(); y++) {
		cairo_move_to(cr, 0, y * CELL_H);
		cairo_line_to(cr, proc.get_columnas() * CELL_W, y * CELL_H);
		cairo_stroke(cr);
	}
	for (int x = 0; x <= proc.get_columnas(); x++) {
		cairo_move_to(cr, x * CELL_W, 0);
		cairo_line_to(cr, x * CELL_W, proc.get_filas() * CELL_H);
		cairo_stroke(cr);
	}

	/* Selección */
	if (editor->is_selecting) {
		int min_x = std::min(editor->sel_x1, editor->sel_x2);
		int max_x = std::max(editor->sel_x1, editor->sel_x2);
		int min_y = std::min(editor->sel_y1, editor->sel_y2);
		int max_y = std::max(editor->sel_y1, editor->sel_y2);

		cairo_set_source_rgba(cr, 0.2, 0.5, 0.9, 0.15);
		cairo_rectangle(cr, min_x * CELL_W, min_y * CELL_H, (max_x - min_x + 1) * CELL_W, (max_y - min_y + 1) * CELL_H);
		cairo_fill(cr);

		cairo_set_source_rgb(cr, 0.2, 0.5, 0.9);
		cairo_set_line_width(cr, 1.0);
		cairo_rectangle(cr, min_x * CELL_W, min_y * CELL_H, (max_x - min_x + 1) * CELL_W, (max_y - min_y + 1) * CELL_H);
		cairo_stroke(cr);
	}

	/* Texto de las celdas */
	cairo_set_source_rgb(cr, 0.08, 0.08, 0.08);
	PangoLayout *layout = pango_cairo_create_layout(cr);
	if (cached_font_desc) {
		pango_layout_set_font_description(layout, cached_font_desc);
	} else {
		cached_font_desc = pango_font_description_from_string("Monospace 16");
		pango_layout_set_font_description(layout, cached_font_desc);
	}

	for (int y = 0; y < proc.get_filas(); y++) {
		for (int x = 0; x < proc.get_columnas(); x++) {
			char buf[8] = { 0 };
			gunichar c = proc.get_caracter(x, y);
			g_unichar_to_utf8(c, buf);
			pango_layout_set_text(layout, buf, -1);
			cairo_move_to(cr, x * CELL_W + 2, y * CELL_H);
			pango_cairo_show_layout(cr, layout);
		}
	}
	g_object_unref(layout);

	/* Cursor */
	if (editor->tool == Tool::LINE) {
		switch (editor->line_style) {
		case LineFamily::LIGHT: {
			cairo_set_source_rgb(cr, 0.15, 0.6, 0.25);
			break;}
		case LineFamily::DOUBLE: {
			cairo_set_source_rgb(cr, 0.7, 0.3, 0.8);
			break;}
		case LineFamily::DASHED: {
			cairo_set_source_rgb(cr, 0.85, 0.55, 0.1);
			break;}
		case LineFamily::THICK: {
			cairo_set_source_rgb(cr, 0.5, 0.1, 0.5);
			break;}
		default: {
			cairo_set_source_rgb(cr, 0.3, 0.3, 0.3);
			break;}
		}
		cairo_set_line_width(cr, 2.0);
		cairo_rectangle(cr, editor->cx * CELL_W + 1, editor->cy * CELL_H + 1, CELL_W - 2, CELL_H - 2);
		cairo_stroke(cr);
	} else if (editor->insert_mode) {
		cairo_set_source_rgb(cr, 0.9, 0.25, 0.25);
		cairo_set_line_width(cr, 2.0);
		double bar_x = editor->cx * CELL_W;
		cairo_move_to(cr, bar_x, editor->cy * CELL_H + 3);
		cairo_line_to(cr, bar_x, (editor->cy + 1) * CELL_H - 3);
		cairo_stroke(cr);
	} else {
		cairo_set_source_rgba(cr, 0.2, 0.5, 0.9, 0.25);
		cairo_rectangle(cr, editor->cx * CELL_W, editor->cy * CELL_H, CELL_W, CELL_H);
		cairo_fill(cr);
		cairo_set_source_rgb(cr, 0.2, 0.5, 0.9);
		cairo_set_line_width(cr, 1.5);
		cairo_rectangle(cr, editor->cx * CELL_W, editor->cy * CELL_H, CELL_W, CELL_H);
		cairo_stroke(cr);
	}
}

/* Paleta de caracteres */
void EditorInterfaz::on_palette_click(GtkButton *btn, gpointer user_data) {
	EditorInterfaz *editor = static_cast<EditorInterfaz*>(user_data);

	// Antirrebote 30 ms
	gint64 ahora = g_get_monotonic_time();
	if (ahora - editor->last_mouse_time < 30000)
		return;
	editor->last_mouse_time = ahora;

	gunichar c = GPOINTER_TO_UINT(g_object_get_data(G_OBJECT(btn), "char-code"));

	if (editor->is_selecting) {
		editor->procesador.rellenar_seleccion(editor->sel_x1, editor->sel_y1, editor->sel_x2, editor->sel_y2, c);
		editor->is_selecting = false;
	} else {
		editor->procesador.colocar_caracter(editor->cx, editor->cy, c, editor->insert_mode);
		if (editor->cx < editor->procesador.get_columnas() - 1)
			editor->cx++;
	}
	editor->actualizar_estado();
	gtk_widget_queue_draw(editor->drawing_area);
}

/* Acciones de filas/columnas */
void EditorInterfaz::on_row_col_action(GtkButton *btn, gpointer user_data) {
	EditorInterfaz *editor = static_cast<EditorInterfaz*>(user_data);
	const char *action = (const char*) g_object_get_data(G_OBJECT(btn), "row-col-action");
	if (!action)
		return;

	if (g_strcmp0(action, "insert_row_above") == 0) {
		editor->procesador.insertar_fila(editor->cy, false);
		editor->cy = std::max(0, editor->cy); // se mantiene, la inserción desplaza hacia abajo
	} else if (g_strcmp0(action, "insert_row_below") == 0) {
		editor->procesador.insertar_fila(editor->cy, true);
		editor->cy = std::min(editor->procesador.get_filas() - 1, editor->cy + 1);
	} else if (g_strcmp0(action, "delete_row") == 0) {
		editor->procesador.eliminar_fila(editor->cy);
		editor->cy = std::clamp(editor->cy, 0, editor->procesador.get_filas() - 1);
	} else if (g_strcmp0(action, "insert_col_left") == 0) {
		editor->procesador.insertar_columna(editor->cx, false);
		editor->cx = std::max(0, editor->cx);
	} else if (g_strcmp0(action, "insert_col_right") == 0) {
		editor->procesador.insertar_columna(editor->cx, true);
		editor->cx = std::min(editor->procesador.get_columnas() - 1, editor->cx + 1);
	} else if (g_strcmp0(action, "delete_col") == 0) {
		editor->procesador.eliminar_columna(editor->cx);
		editor->cx = std::clamp(editor->cx, 0, editor->procesador.get_columnas() - 1);
	}
	editor->redimensionar_lienzo();
	editor->actualizar_estado();
}

/* Cargar archivo */
void EditorInterfaz::on_btn_load_clicked(GtkButton *btn, gpointer user_data) {
	EditorInterfaz *editor = static_cast<EditorInterfaz*>(user_data);
	SistemaArchivos::cargar_archivo_dialogo(GTK_WINDOW(editor->ventana), callback_cargar_archivo, editor, GTK_LABEL(editor->status));
}

/* Guardar archivo */
void EditorInterfaz::on_btn_save_clicked(GtkButton *btn, gpointer user_data) {
	EditorInterfaz *editor = static_cast<EditorInterfaz*>(user_data);
	std::string contenido = editor->procesador.exportar_a_string();
	SistemaArchivos::guardar_archivo_dialogo(GTK_WINDOW(editor->ventana), contenido.c_str(), GTK_LABEL(editor->status));
}

/* Borrar (clear) */
void EditorInterfaz::on_btn_clear_clicked(GtkButton *btn, gpointer user_data) {
	EditorInterfaz *editor = static_cast<EditorInterfaz*>(user_data);
	if (editor->is_selecting) {
		editor->procesador.limpiar_seleccion(editor->sel_x1, editor->sel_y1, editor->sel_x2, editor->sel_y2);
		editor->is_selecting = false;
	} else {
		editor->procesador.borrar_en_posicion(editor->cx, editor->cy, editor->insert_mode);
	}
	editor->actualizar_estado();
	gtk_widget_queue_draw(editor->drawing_area);
}

/* Toggle INSERT/REPLACE */
void EditorInterfaz::on_toggle_insert(GtkButton *btn, gpointer user_data) {
	EditorInterfaz *editor = static_cast<EditorInterfaz*>(user_data);
	editor->insert_mode = !editor->insert_mode;
	gtk_button_set_label(btn, editor->insert_mode ? "🔴 INSERT" : "🔵 REPLACE");
	editor->actualizar_estado();
	gtk_widget_queue_draw(editor->drawing_area);
}

/* Toggle herramienta Línea / Selección */
void EditorInterfaz::on_toggle_tool(GtkToggleButton *btn, gpointer user_data) {
	EditorInterfaz *editor = static_cast<EditorInterfaz*>(user_data);
	gboolean active = gtk_toggle_button_get_active(btn);
	editor->tool = active ? Tool::LINE : Tool::SELECT;
	if (active) {
		editor->procesador.reiniciar_ultima_linea();
	}
	gtk_button_set_label(GTK_BUTTON(btn), active ? "✏️ Línea (ON)" : "✏️ Línea");
	editor->actualizar_estado();
	gtk_widget_queue_draw(editor->drawing_area);
}

/* Cambio de estilo de línea */
void EditorInterfaz::on_style_changed(GObject *self, GParamSpec *pspec, gpointer user_data) {
	EditorInterfaz *editor = static_cast<EditorInterfaz*>(user_data);
	guint idx = gtk_drop_down_get_selected(GTK_DROP_DOWN(self));
	switch (idx) {
	case 0: {
		editor->line_style = LineFamily::LIGHT;
		break;}
	case 1: {
		editor->line_style = LineFamily::DOUBLE;
		break;}
	case 2: {
		editor->line_style = LineFamily::DASHED;
		break;}
	case 3: {
		editor->line_style = LineFamily::THICK;
		break;}
	default: {
		editor->line_style = LineFamily::LIGHT;
		break;}
	}
	editor->actualizar_estado();
	gtk_widget_queue_draw(editor->drawing_area);
}

/* Arrastre: inicio */
void EditorInterfaz::on_drag_begin(GtkGestureDrag *gest, double start_x, double start_y, gpointer user_data) {
	EditorInterfaz *editor = static_cast<EditorInterfaz*>(user_data);
	editor->last_mouse_time = g_get_monotonic_time(); // permitir primer evento inmediato

	gtk_widget_grab_focus(editor->drawing_area);
	int gx = std::clamp((int) (start_x / CELL_W), 0, editor->procesador.get_columnas() - 1);
	int gy = std::clamp((int) (start_y / CELL_H), 0, editor->procesador.get_filas() - 1);
	editor->cx = gx;
	editor->cy = gy;

	if (editor->tool == Tool::LINE) {
		editor->is_selecting = false;
		editor->procesador.reiniciar_ultima_linea();
		editor->line_last_x = -1;
		editor->line_last_y = -1;
		editor->line_tool_step(gx, gy);
	} else {
		editor->is_selecting = true;
		editor->sel_x1 = editor->sel_x2 = gx;
		editor->sel_y1 = editor->sel_y2 = gy;
	}
	editor->actualizar_estado();
	gtk_widget_queue_draw(editor->drawing_area);
}

/* Arrastre: actualización (con antirrebote de 30 ms) */
void EditorInterfaz::on_drag_update(GtkGestureDrag *gest, double offset_x, double offset_y, gpointer user_data) {
	EditorInterfaz *editor = static_cast<EditorInterfaz*>(user_data);

	double start_x, start_y;
	if (!gtk_gesture_drag_get_start_point(gest, &start_x, &start_y)) {
		return;}

	int gx = std::clamp((int) ((start_x + offset_x) / CELL_W), 0, editor->procesador.get_columnas() - 1);
	int gy = std::clamp((int) ((start_y + offset_y) / CELL_H), 0, editor->procesador.get_filas() - 1);

	if (editor->tool == Tool::LINE) {
		/* Sin antirrebote: hay que procesar TODOS los eventos de movimiento,
		 * o un giro rápido dentro de la ventana de 30 ms se "aplanaría" en un
		 * único tramo recto y la esquina se perdería (había que repasar el
		 * trazo varias veces para que apareciera). */
		editor->line_tool_step(gx, gy);
		editor->cx = gx;
		editor->cy = gy;
		gtk_widget_queue_draw(editor->drawing_area);
		return;
	}

	/* Para la selección el antirrebote solo limita repintados, no pierde datos */
	gint64 ahora = g_get_monotonic_time();
	if (ahora - editor->last_mouse_time < 30000) {
		return;}
	editor->last_mouse_time = ahora;

	editor->sel_x2 = gx;
	editor->sel_y2 = gy;
	gtk_widget_queue_draw(editor->drawing_area);
}

/* Arrastre: fin */
void EditorInterfaz::on_drag_end(GtkGestureDrag *gest, double offset_x, double offset_y, gpointer user_data) {
	EditorInterfaz *editor = static_cast<EditorInterfaz*>(user_data);
	if (editor->tool == Tool::LINE) {
		editor->line_last_x = -1;
		editor->line_last_y = -1;
		editor->is_selecting = false;
		editor->procesador.reiniciar_ultima_linea();
	} else if (editor->sel_x1 == editor->sel_x2 && editor->sel_y1 == editor->sel_y2) {
		editor->is_selecting = false;
	}
	gtk_widget_queue_draw(editor->drawing_area);
}

/* Teclado */
gboolean EditorInterfaz::on_key_press(GtkEventControllerKey *ctrl, guint keyval, guint keycode, GdkModifierType mod, gpointer user_data) {
	EditorInterfaz *editor = static_cast<EditorInterfaz*>(user_data);

	if (keyval == GDK_KEY_Insert) {
		editor->insert_mode = !editor->insert_mode;
		gtk_button_set_label(GTK_BUTTON(editor->btn_insert), editor->insert_mode ? "🔴 INSERT" : "🔵 REPLACE");
		editor->actualizar_estado();
		gtk_widget_queue_draw(editor->drawing_area);
		return TRUE;
	}

	editor->is_selecting = false;

	// Teclas de dirección en modo línea
	if (editor->tool == Tool::LINE) {
		int nx = editor->cx;
		int ny = editor->cy;
		bool moved = false;
		switch (keyval) {
		case GDK_KEY_Up: {
			ny = std::max(0, editor->cy - 1);
			moved = true;
			break;}
		case GDK_KEY_Down: {
			ny = std::min(editor->procesador.get_filas() - 1, editor->cy + 1);
			moved = true;
			break;}
		case GDK_KEY_Left: {
			nx = std::max(0, editor->cx - 1);
			moved = true;
			break;}
		case GDK_KEY_Right: {
			nx = std::min(editor->procesador.get_columnas() - 1, editor->cx + 1);
			moved = true;
			break;}
		default: {
			break;}
		}
		if (moved) {
			bool nueva_horizontal = (nx != editor->cx);
			bool giro = (editor->line_last_x >= 0) && (nueva_horizontal != editor->line_last_horizontal);

			/* Al salir de la casilla actual hacia una dirección distinta a la
			 * anterior (giro/esquina), se añaden AMBAS familias en esa casilla
			 * antes de seguir, para que la esquina se forme en el mismo paso. */
			if (giro) {
				editor->procesador.colocar_linea_horizontal(editor->cx, editor->cy, editor->line_style);
				editor->procesador.colocar_linea_vertical(editor->cx, editor->cy, editor->line_style);
			}

			editor->line_last_horizontal = nueva_horizontal;
			if (nueva_horizontal) {
				editor->procesador.colocar_linea_horizontal(editor->cx, editor->cy, editor->line_style);
				editor->procesador.colocar_linea_horizontal(nx, ny, editor->line_style);
			} else {
				editor->procesador.colocar_linea_vertical(editor->cx, editor->cy, editor->line_style);
				editor->procesador.colocar_linea_vertical(nx, ny, editor->line_style);
			}
			editor->line_last_x = editor->cx;
			editor->line_last_y = editor->cy;
			editor->cx = nx;
			editor->cy = ny;
			editor->actualizar_estado();
			gtk_widget_queue_draw(editor->drawing_area);
			return TRUE;
		}
		// Borrar con espacio/supr en modo línea
		if (keyval == GDK_KEY_space || keyval == GDK_KEY_Delete) {
			editor->procesador.borrar_linea(editor->cx, editor->cy);
			if (keyval == GDK_KEY_space && editor->cx < editor->procesador.get_columnas() - 1) {
				editor->cx++;}
			editor->actualizar_estado();
			gtk_widget_queue_draw(editor->drawing_area);
			return TRUE;
		}
		if (keyval == GDK_KEY_BackSpace) {
			if (editor->cx > 0) {
				editor->cx--;}
			editor->procesador.borrar_linea(editor->cx, editor->cy);
			editor->actualizar_estado();
			gtk_widget_queue_draw(editor->drawing_area);
			return TRUE;
		}
	}

	// Movimiento del cursor en modo selección
	switch (keyval) {
	case GDK_KEY_Up: {
		editor->cy = std::max(0, editor->cy - 1);
		break;}
	case GDK_KEY_Down: {
		editor->cy = std::min(editor->procesador.get_filas() - 1, editor->cy + 1);
		break;}
	case GDK_KEY_Left: {
		editor->cx = std::max(0, editor->cx - 1);
		break;}
	case GDK_KEY_Right: {
		editor->cx = std::min(editor->procesador.get_columnas() - 1, editor->cx + 1);
		break;}
	case GDK_KEY_BackSpace: {
		if (editor->cx > 0)
			editor->cx--;
		editor->procesador.borrar_en_posicion(editor->cx, editor->cy, editor->insert_mode);
		break;}
	case GDK_KEY_Delete: {
		editor->procesador.borrar_en_posicion(editor->cx, editor->cy, editor->insert_mode);
		break;}
	case GDK_KEY_space: {
		editor->procesador.colocar_caracter(editor->cx, editor->cy, ' ', editor->insert_mode);
		if (editor->cx < editor->procesador.get_columnas() - 1)
			editor->cx++;
		break;}
	case GDK_KEY_Return:
	case GDK_KEY_KP_Enter: {
		editor->cx = 0;
		editor->cy = std::min(editor->procesador.get_filas() - 1, editor->cy + 1);
		break;}
	case GDK_KEY_Home: {
		editor->cx = 0;
		break;}
	case GDK_KEY_End: {
		editor->cx = editor->procesador.get_columnas() - 1;
		break;}
	case GDK_KEY_Tab: {
		editor->cx = std::min(editor->procesador.get_columnas() - 1, editor->cx + 1);
		break;}
	default: {
		gunichar c = gdk_keyval_to_unicode(keyval);
		if (c != 0 && !g_unichar_iscntrl(c)) {
			editor->procesador.colocar_caracter(editor->cx, editor->cy, c, editor->insert_mode);
			if (editor->cx < editor->procesador.get_columnas() - 1) {
				editor->cx++;}
		} else {
			return FALSE;
		}
		break;
	}
	}

	editor->actualizar_estado();
	gtk_widget_queue_draw(editor->drawing_area);
	return TRUE;
}
