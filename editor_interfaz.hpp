/*
 * editor_interfaz.hpp
 *
 *  Created on: 4 ago 2026
 *      Author: DjSteker
 */

#ifndef EDITOR_INTERFAZ_HPP_
#define EDITOR_INTERFAZ_HPP_

#include <gtk/gtk.h>
#include "procesador_celdas.hpp"
#include "dialogo_archivo.hpp"

enum class Tool {
	SELECT, LINE
};

/**
 * @brief Clase que encapsula la interfaz gráfica del editor.
 */
class EditorInterfaz {
public:
	EditorInterfaz();
	~EditorInterfaz();

	/**
	 * @brief Inicializa la ventana principal y todos los widgets.
	 */
	void inicializar(GtkApplication *app);

	/**
	 * @brief Actualiza el texto de la barra de estado.
	 */
	void actualizar_estado();

	/**
	 * @brief Redimensiona el lienzo según el tamaño de la rejilla.
	 */
	void redimensionar_lienzo();

	// Callback para cargar archivo (se usa desde SistemaArchivos)
	static void callback_cargar_archivo(const char *contenido, gsize longitud, gpointer user_data);

private:
	// --- Widgets principales ---
	GtkWidget *ventana;
	GtkWidget *drawing_area;
	GtkWidget *status;
	GtkWidget *btn_insert;
	GtkWidget *btn_tool_line;
	GtkDropDown *style_dropdown;

	// --- Estado de la interfaz ---
	int cx;                            // Posición X del cursor
	int cy;                            // Posición Y del cursor
	bool is_selecting;
	bool insert_mode;                  // Modo INSERT/REPLACE
	int sel_x1, sel_y1, sel_x2, sel_y2; // Coordenadas de selección
	Tool tool;

	// --- Estado de la herramienta Línea ---
	LineFamily line_style;             // Estilo de línea actual
	int line_last_x;
	int line_last_y;
	bool line_last_horizontal;

	// --- Antirrebote del ratón ---
	gint64 last_mouse_time;

	// --- Instancia de la lógica de negocio ---
	ProcesadorCeldas procesador;

	// --- Constantes de diseño ---
	static constexpr int CELL_W = 16;
	static constexpr int CELL_H = 24;
	static constexpr int INIT_COLS = 80;
	static constexpr int INIT_ROWS = 40;

	// --- Cache de fuente Pango ---
	static PangoFontDescription *cached_font_desc;

	// --- Métodos privados de utilidad ---
	void construir_paleta(GtkWidget *box);
	GtkWidget* crear_boton_paleta(const char *label, gunichar ch);
	GtkWidget* crear_seccion_paleta(const char *title, const char *chars);
	void line_tool_step(int gx, int gy);

public:
	// --- Callbacks estáticos de la interfaz ---
	static void dibujar_lienzo(GtkDrawingArea *area, cairo_t *cr, int width, int height, gpointer user_data);
	static void on_palette_click(GtkButton *btn, gpointer user_data);
	static void on_row_col_action(GtkButton *btn, gpointer user_data);
	static void on_btn_load_clicked(GtkButton *btn, gpointer user_data);
	static void on_btn_clear_clicked(GtkButton *btn, gpointer user_data);
	static void on_btn_save_clicked(GtkButton *btn, gpointer user_data);
	static void on_toggle_insert(GtkButton *btn, gpointer user_data);
	static void on_toggle_tool(GtkToggleButton *btn, gpointer user_data);
	static void on_style_changed(GObject *self, GParamSpec *pspec, gpointer user_data);
	static void on_drag_begin(GtkGestureDrag *gest, double start_x, double start_y, gpointer user_data);
	static void on_drag_update(GtkGestureDrag *gest, double offset_x, double offset_y, gpointer user_data);
	static void on_drag_end(GtkGestureDrag *gest, double offset_x, double offset_y, gpointer user_data);
	static gboolean on_key_press(GtkEventControllerKey *ctrl, guint keyval, guint keycode, GdkModifierType mod, gpointer user_data);
};

#endif /* EDITOR_INTERFAZ_HPP_ */
