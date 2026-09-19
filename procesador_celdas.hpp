/*
 * procesador_celdas.hpp
 *
 * Created on: 4 ago 2026
 * Author: DjSteker
 */

#ifndef PROCESADOR_CELDAS_HPP_
#define PROCESADOR_CELDAS_HPP_

#include <vector>
#include <string>
#include <cstdint>
#include <gtk/gtk.h>

enum class LineFamily : uint8_t {
	NONE = 0, LIGHT, DOUBLE, DASHED, THICK
};

class ProcesadorCeldas {
public:
	ProcesadorCeldas() = default;
	~ProcesadorCeldas() = default;

	void inicializar(int filas, int columnas);

	int get_filas() const {
		return rows;
	}
	int get_columnas() const {
		return cols;
	}

	gunichar get_caracter(int x, int y) const;
	LineFamily get_familia_v(int x, int y) const;
	LineFamily get_familia_h(int x, int y) const;

	void colocar_linea_vertical(int x, int y, LineFamily estilo);
	void colocar_linea_horizontal(int x, int y, LineFamily estilo);
	void borrar_linea(int x, int y);

	void colocar_caracter(int x, int y, gunichar c, bool insert_mode);
	void borrar_en_posicion(int x, int y, bool insert_mode);

	void rellenar_seleccion(int x1, int y1, int x2, int y2, gunichar c);
	void limpiar_seleccion(int x1, int y1, int x2, int y2);

	void insertar_fila(int posicion, bool despues);
	void eliminar_fila(int posicion);
	void insertar_columna(int posicion, bool despues);
	void eliminar_columna(int posicion);

	std::string exportar_a_string() const;
	void importar_desde_string(const char *contenido, gsize longitud);

	void recalcular_celda(int x, int y);
	void recalcular_vecinos(int x, int y);
	void recalcular_borde(int min_x, int max_x, int min_y, int max_y);
	void recalcular_todo();

	static const char* nombre_estilo(LineFamily f);
	void  reiniciar_ultima_linea();

private:
	int cols { 0 };
	int rows { 0 };
	std::vector<std::vector<gunichar>> grid;
	std::vector<std::vector<LineFamily>> fam_v;
	std::vector<std::vector<LineFamily>> fam_h;

	static const gunichar LIGHT_TABLE[16];
	static const gunichar DOUBLE_TABLE[12];
	static const gunichar THICK_TABLE[16];

	/*
	 * Traducen las 4 conexiones booleanas (Norte, Este, Sur, Oeste) al
	 * índice de caso que usan las tablas de arriba (que NO son una máscara
	 * de bits N/E/S/O, sino una enumeración de casos: 0=nada,
	 * 1..4=un solo brazo, 5..10=rectas/esquinas, 11..14=T, 15=cruz
	 * completa para LIGHT_TABLE/THICK_TABLE; DOUBLE_TABLE es una versión
	 * reducida de 12 casos porque ║/═ se reutilizan para cualquier tramo
	 * puramente vertical u horizontal).
	 */
	static int indice_ligero_grueso(bool n, bool e, bool s, bool w);
	static int indice_doble(bool n, bool e, bool s, bool w);

	gunichar lookup_simple(LineFamily fam, bool n, bool e, bool s, bool w, bool horizontal) const;
	gunichar cross_full(LineFamily vf, LineFamily hf) const;
	gunichar t_junction(LineFamily vf, LineFamily hf, bool n, bool e, bool s, bool w) const;
	gunichar corner_junction(LineFamily vf, LineFamily hf, bool n, bool e, bool s, bool w) const;
	LineFamily fam_v_at(int x, int y) const;
	LineFamily fam_h_at(int x, int y) const;
	void recalcular_rango_filas(int y0, int y1);
	void recalcular_rango_columnas(int x0, int x1);

  // Última posición donde se colocó una línea
  int last_line_x {-1};
  int last_line_y {-1};
  bool last_line_horizontal {true};
  LineFamily last_line_style {LineFamily::NONE};
  // true = ya hay una celda de inicio memorizada pero aún no se ha
  // confirmado ninguna dirección real (todavía no se ha dibujado nada)
  bool last_line_pendiente {true};

};

#endif /* PROCESADOR_CELDAS_HPP_ */
