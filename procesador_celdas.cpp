/*
 * procesador_celdas.cpp
 *
 * Created on: 4 ago 2026
 * Author: DjSteker
 */

#include "procesador_celdas.hpp"
#include <algorithm>
#include <sstream>

const gunichar ProcesadorCeldas::LIGHT_TABLE[16] = {
/*0*/0,
/*1 W*/U'╴',
/*2 E*/U'╶',
/*3 S*/U'╷',
/*4 N*/U'╵',
/*5 EW*/U'─',
/*6 NS*/U'│',
/*7 SW*/U'┐',
/*8 NE*/U'└',
/*9 ES*/U'┌',
/*10 NW*/U'┘',
/*11 ESW*/U'┬',
/*12 NSW*/U'┤',
/*13 NEW*/U'┴',
/*14 NES*/U'├',
/*15 NESW*/U'┼' };

const gunichar ProcesadorCeldas::THICK_TABLE[16] = {
/* 0  (0b0000) Ninguna */0,
/* 1  (0b0001) Oeste   */U'╸',
/* 2  (0b0010) Este    */U'╺',
/* 3  (0b0011) S+O     */U'╻',   // Sur + Oeste
		/* 4  (0b0100) Norte   */U'╹',
		/* 5  (0b0101) E+O     */U'━',   // Este + Oeste
		/* 6  (0b0110) N+S     */U'┃',   // Norte + Sur
		/* 7  (0b0111) S+E+O   */U'┓',   // Sur + Este + Oeste
		/* 8  (0b1000) N+E     */U'┗',   // Norte + Este
		/* 9  (0b1001) E+S     */U'┏',   // Este + Sur
		/* 10 (0b1010) N+O     */U'┛',   // Norte + Oeste
		/* 11 (0b1011) E+S+O   */U'┳',   // Este + Sur + Oeste
		/* 12 (0b1100) N+S+O   */U'┫',   // Norte + Sur + Oeste
		/* 13 (0b1101) N+E+O   */U'┻',   // Norte + Este + Oeste
		/* 14 (0b1110) N+E+S   */U'┣',   // Norte + Este + Sur
		/* 15 (0b1111) Todas   */U'╋'    // Norte + Este + Sur + Oeste
		};

const gunichar ProcesadorCeldas::DOUBLE_TABLE[12] = {
/*0*/0,
/*1 NS*/U'║',
/*2 W*/U'═',
/*3 ES*/U'╔',
/*4 NE*/U'╚',
/*5 NW*/U'╝',
/*6 SW*/U'╗',
/*7 ESW*/U'╦',
/*8 NSW*/U'╣',
/*9 NEW*/U'╩',
/*10 NES*/U'╠',
/*11 NESW*/U'╬' };

/*
 * IMPORTANTE: ninguna de las 3 tablas de arriba está indexada por una
 * máscara de bits N=8,E=4,S=2,W=1. Son listas de "casos" con su propio
 * orden. LIGHT_TABLE y THICK_TABLE comparten el mismo orden de 16 casos
 * (los comentarios en 0b... de THICK_TABLE son incorrectos, pero los
 * glifos coinciden exactamente con el orden de LIGHT_TABLE). DOUBLE_TABLE
 * usa un orden reducido de 12 casos porque ║ y ═ representan cualquier
 * conexión puramente vertical u horizontal (una rama sola, o las dos
 * rectas). Estas dos funciones calculan el índice correcto según ese
 * orden a partir de las 4 conexiones reales de la celda.
 */
int ProcesadorCeldas::indice_ligero_grueso(bool n, bool e, bool s, bool w) {
	int count = (n ? 1 : 0) + (e ? 1 : 0) + (s ? 1 : 0) + (w ? 1 : 0);
	if (count == 0) {
		return 0;
	}
	if (count == 1) {
		if (w) {return 1;}
		if (e) {return 2;}
		if (s) {return 3;}
		return 4; /* n */
	}
	if (count == 2) {
		if (e && w) {return 5;}  /* recta horizontal */
		if (n && s) {return 6;}  /* recta vertical */
		if (s && w) {return 7;}  /* esquina ┐ */
		if (n && e) {return 8;}  /* esquina └ */
		if (e && s) {return 9;}  /* esquina ┌ */
		if (n && w) {return 10;} /* esquina ┘ */
	}
	if (count == 3) {
		if (!n) {return 11;} /* T ┬ (falta Norte) */
		if (!e) {return 12;} /* T ┤ (falta Este)  */
		if (!s) {return 13;} /* T ┴ (falta Sur)   */
		return 14;          /* T ├ (falta Oeste) */
	}
	return 15; /* count == 4, cruz completa */
}

int ProcesadorCeldas::indice_doble(bool n, bool e, bool s, bool w) {
	int count = (n ? 1 : 0) + (e ? 1 : 0) + (s ? 1 : 0) + (w ? 1 : 0);
	if (count == 0) {
		return 0;
	}
	if (!e && !w) {
		return 1; /* solo vertical: N, S, o N+S */
	}
	if (!n && !s) {
		return 2; /* solo horizontal: E, W, o E+W */
	}
	if (count == 2) {
		if (e && s) {return 3;} /* esquina ╔ */
		if (n && e) {return 4;} /* esquina ╚ */
		if (n && w) {return 5;} /* esquina ╝ */
		return 6;              /* s && w -> esquina ╗ */
	}
	if (count == 3) {
		if (!n) {return 7;}  /* T ╦ */
		if (!e) {return 8;}  /* T ╣ */
		if (!s) {return 9;}  /* T ╩ */
		return 10;          /* !w -> T ╠ */
	}
	return 11; /* count == 4, cruz completa */
}

void ProcesadorCeldas::inicializar(int filas, int columnas) {
	this->rows = filas;
	this->cols = columnas;
	grid.assign(rows, std::vector<gunichar>(cols, ' '));
	fam_v.assign(rows, std::vector<LineFamily>(cols, LineFamily::NONE));
	fam_h.assign(rows, std::vector<LineFamily>(cols, LineFamily::NONE));
	last_line_x = -1;
	last_line_y = -1;
	last_line_horizontal = true;
	last_line_style = LineFamily::NONE;
	last_line_pendiente = true;
}

gunichar ProcesadorCeldas::get_caracter(int x, int y) const {
	if (x < 0 || x >= cols || y < 0 || y >= rows) {
		return ' ';
	}
	return grid[y][x];
}

LineFamily ProcesadorCeldas::get_familia_v(int x, int y) const {
	if (x < 0 || x >= cols || y < 0 || y >= rows) {
		return LineFamily::NONE;
	}
	return fam_v[y][x];
}

LineFamily ProcesadorCeldas::get_familia_h(int x, int y) const {
	if (x < 0 || x >= cols || y < 0 || y >= rows) {
		return LineFamily::NONE;
	}
	return fam_h[y][x];
}

const char* ProcesadorCeldas::nombre_estilo(LineFamily f) {
	switch (f) {
	case LineFamily::LIGHT: {
		return "Sencilla";
	}
	case LineFamily::DOUBLE: {
		return "Doble";
	}
	case LineFamily::DASHED: {
		return "Punteada";
	}
	case LineFamily::THICK: {
		return "Gruesa";
	}
	default: {
		return "-";
	}
	}
}

LineFamily ProcesadorCeldas::fam_v_at(int x, int y) const {
	if (x < 0 || x >= cols || y < 0 || y >= rows) {
		return LineFamily::NONE;
	}
	return fam_v[y][x];
}

LineFamily ProcesadorCeldas::fam_h_at(int x, int y) const {
	if (x < 0 || x >= cols || y < 0 || y >= rows) {
		return LineFamily::NONE;
	}
	return fam_h[y][x];
}

gunichar ProcesadorCeldas::lookup_simple(LineFamily fam, bool n, bool e, bool s, bool w, bool horizontal) const {
	if (fam == LineFamily::DASHED) {
		int idx = indice_ligero_grueso(n, e, s, w);
		bool solo_horizontal = !n && !s && (e || w);
		bool solo_vertical = !e && !w && (n || s);
		if (idx == 0) {
			return horizontal ? U'┄' : U'┆';
		}
		if (solo_horizontal) {
			return U'┄';
		}
		if (solo_vertical) {
			return U'┆';
		}
		/* esquina/T/cruz: no hay glifo punteado, se reutiliza el de la familia sencilla */
		return LIGHT_TABLE[idx];
	}
	if (fam == LineFamily::LIGHT) {
		int idx = indice_ligero_grueso(n, e, s, w);
		if (idx == 0) {
			return horizontal ? U'─' : U'│';
		}
		return LIGHT_TABLE[idx];
	} else if (fam == LineFamily::DOUBLE) {
		int idx = indice_doble(n, e, s, w);
		if (idx == 0) {
			return horizontal ? U'═' : U'║';
		}
		return DOUBLE_TABLE[idx];
	} else if (fam == LineFamily::THICK) {
		int idx = indice_ligero_grueso(n, e, s, w);
		if (idx == 0) {
			return horizontal ? U'━' : U'┃';
		}
		return THICK_TABLE[idx];
	}
	return ' ';
}

gunichar ProcesadorCeldas::cross_full(LineFamily vf, LineFamily hf) const {
	auto norm = [](LineFamily f) {
		if (f == LineFamily::DASHED || f == LineFamily::NONE) {
			return LineFamily::LIGHT;
		}
		return f;
	};
	LineFamily v = norm(vf);
	LineFamily h = norm(hf);
	if (v == LineFamily::THICK || h == LineFamily::THICK) {
		return U'╋';
	}
	if (v == LineFamily::DOUBLE && h == LineFamily::DOUBLE) {
		return U'╬';
	}
	if (v == LineFamily::LIGHT && h == LineFamily::DOUBLE) {
		return U'╪';
	}
	if (v == LineFamily::DOUBLE && h == LineFamily::LIGHT) {
		return U'╫';
	}
	return U'┼';
}

gunichar ProcesadorCeldas::t_junction(LineFamily vf, LineFamily hf, bool n, bool e, bool s, bool w) const {
	auto norm = [](LineFamily f) {
		if (f == LineFamily::DASHED || f == LineFamily::NONE) {
			return LineFamily::LIGHT;
		}
		return f;
	};
	LineFamily v = norm(vf);
	LineFamily h = norm(hf);
	bool same = (v == h);

	if (!n) { /* faltan Norte: E+S+O -> ┬ */
		if (same && v == LineFamily::LIGHT) {return U'┬';}
		if (same && v == LineFamily::DOUBLE) {return U'╦';}
		if (same && v == LineFamily::THICK) {return U'┳';}
		if (v == LineFamily::DOUBLE && h == LineFamily::LIGHT) {return U'╥';}
		if (v == LineFamily::LIGHT && h == LineFamily::DOUBLE) {return U'╤';}
		return U'┳';
	}
	if (!s) { /* faltan Sur: N+E+O -> ┴ */
		if (same && v == LineFamily::LIGHT) {return U'┴';}
		if (same && v == LineFamily::DOUBLE) {return U'╩';}
		if (same && v == LineFamily::THICK) {return U'┻';}
		if (v == LineFamily::DOUBLE && h == LineFamily::LIGHT) {return U'╨';}
		if (v == LineFamily::LIGHT && h == LineFamily::DOUBLE) {return U'╧';}
		return U'┻';
	}
	if (!w) { /* faltan Oeste: N+E+S -> ├ */
		if (same && v == LineFamily::LIGHT) {return U'├';}
		if (same && v == LineFamily::DOUBLE) {return U'╠';}
		if (same && v == LineFamily::THICK) {return U'┣';}
		if (v == LineFamily::DOUBLE && h == LineFamily::LIGHT) {return U'╟';}
		if (v == LineFamily::LIGHT && h == LineFamily::DOUBLE) {return U'╞';}
		return U'┣';
	}
	if (!e) { /* faltan Este: N+S+O -> ┤ */
		if (same && v == LineFamily::LIGHT) {return U'┤';}
		if (same && v == LineFamily::DOUBLE) {return U'╣';}
		if (same && v == LineFamily::THICK) {return U'┫';}
		if (v == LineFamily::DOUBLE && h == LineFamily::LIGHT) {return U'╢';}
		if (v == LineFamily::LIGHT && h == LineFamily::DOUBLE) {return U'╡';}
		return U'┫';
	}
	return U'┼';
}

/*
 * Esquina en ángulo recto (solo 2 direcciones perpendiculares conectadas:
 * una del eje vertical -N o S- y otra del eje horizontal -E u O-).
 */
gunichar ProcesadorCeldas::corner_junction(LineFamily vf, LineFamily hf, bool n, bool e, bool s, bool w) const {
	auto norm = [](LineFamily f) {
		if (f == LineFamily::DASHED || f == LineFamily::NONE) {
			return LineFamily::LIGHT;
		}
		return f;
	};
	LineFamily v = norm(vf);
	LineFamily h = norm(hf);

	if (v == LineFamily::THICK || h == LineFamily::THICK) {
		return THICK_TABLE[indice_ligero_grueso(n, e, s, w)];
	}

	bool same = (v == h);
	if (same && v == LineFamily::LIGHT) {
		return LIGHT_TABLE[indice_ligero_grueso(n, e, s, w)];
	}
	if (same && v == LineFamily::DOUBLE) {
		return DOUBLE_TABLE[indice_doble(n, e, s, w)];
	}

	/* Combinaciones mixtas sencilla/doble */
	if (e && s) { /* forma "┌" */
		return (v == LineFamily::DOUBLE) ? U'╓' : U'╒';
	}
	if (s && w) { /* forma "┐" */
		return (v == LineFamily::DOUBLE) ? U'╖' : U'╕';
	}
	if (n && e) { /* forma "└" */
		return (v == LineFamily::DOUBLE) ? U'╙' : U'╘';
	}
	if (n && w) { /* forma "┘" */
		return (v == LineFamily::DOUBLE) ? U'╜' : U'╛';
	}
	return LIGHT_TABLE[indice_ligero_grueso(n, e, s, w)];
}

void ProcesadorCeldas::recalcular_celda(int x, int y) {
	if (x < 0 || x >= cols || y < 0 || y >= rows) {
		return;
	}

	LineFamily vf = fam_v[y][x];
	LineFamily hf = fam_h[y][x];

	if (vf == LineFamily::NONE && hf == LineFamily::NONE) {
		return;
	}

	bool conn_n = false;
	bool conn_e = false;
	bool conn_s = false;
	bool conn_w = false;

	// Detección de conexiones verticales
	if (vf != LineFamily::NONE) {
		if (y > 0 && fam_v[y - 1][x] == vf) {
			conn_n = true;
		}
		if (y < rows - 1 && fam_v[y + 1][x] == vf) {
			conn_s = true;
		}
	}

	// Detección de conexiones horizontales
	if (hf != LineFamily::NONE) {
		if (x > 0 && fam_h[y][x - 1] == hf) {
			conn_w = true;  // Izquierda = Oeste
		}
		if (x < cols - 1 && fam_h[y][x + 1] == hf) {
			conn_e = true;  // Derecha = Este
		}
	}

	int count = (conn_n ? 1 : 0) + (conn_e ? 1 : 0) + (conn_s ? 1 : 0) + (conn_w ? 1 : 0);
	bool is_cross = (vf != LineFamily::NONE && hf != LineFamily::NONE);

	if (is_cross) {
		if (count == 4) {
			grid[y][x] = cross_full(vf, hf);
			return;
		}
		if (count == 3) {
			grid[y][x] = t_junction(vf, hf, conn_n, conn_e, conn_s, conn_w);
			return;
		}
		if (count == 2) {
			bool recta = (conn_n && conn_s) || (conn_e && conn_w);
			if (recta) {
				/* Recta en un eje + la familia perpendicular presente en la
				 * misma celda (aunque no se prolongue a los vecinos): sigue
				 * siendo un cruce real entre dos líneas, no una recta suelta. */
				grid[y][x] = cross_full(vf, hf);
			} else {
				grid[y][x] = corner_junction(vf, hf, conn_n, conn_e, conn_s, conn_w);
			}
			return;
		}
		/* count == 1 o count == 0: ambas familias ocupan la celda aunque una
		 * de ellas (o las dos) no tenga aún vecinos con los que conectar;
		 * se sigue mostrando el cruce combinado en vez de descartar una de
		 * las dos líneas. */
		grid[y][x] = cross_full(vf, hf);
		return;
	}

	if (vf != LineFamily::NONE && hf == LineFamily::NONE) {
		grid[y][x] = lookup_simple(vf, conn_n, false, conn_s, false, false);
	} else if (vf == LineFamily::NONE && hf != LineFamily::NONE) {
		grid[y][x] = lookup_simple(hf, false, conn_e, false, conn_w, true);
	} else {
		grid[y][x] = cross_full(vf, hf);
	}
}

void ProcesadorCeldas::recalcular_vecinos(int x, int y) {
	recalcular_celda(x - 1, y);
	recalcular_celda(x + 1, y);
	recalcular_celda(x, y - 1);
	recalcular_celda(x, y + 1);
	recalcular_celda(x, y);
}

void ProcesadorCeldas::recalcular_borde(int min_x, int max_x, int min_y, int max_y) {
	for (int x = min_x - 1; x <= max_x + 1; x++) {
		recalcular_celda(x, min_y - 1);
		recalcular_celda(x, max_y + 1);
	}
	for (int y = min_y - 1; y <= max_y + 1; y++) {
		recalcular_celda(min_x - 1, y);
		recalcular_celda(max_x + 1, y);
	}
}

void ProcesadorCeldas::recalcular_todo() {
	for (int y = 0; y < rows; y++) {
		for (int x = 0; x < cols; x++) {
			if (fam_v[y][x] != LineFamily::NONE || fam_h[y][x] != LineFamily::NONE) {
				recalcular_celda(x, y);
			}
		}
	}
}

void ProcesadorCeldas::colocar_linea_vertical(int x, int y, LineFamily estilo) {
	if (x < 0 || x >= cols || y < 0 || y >= rows) {
		return;
	}

	if (last_line_x < 0) {
		/* Primer punto de un trazo nuevo: solo se memoriza, no se dibuja
		 * nada todavía. Así un simple clic sin arrastre no añade ninguna
		 * familia "por defecto" que luego provoque cruces falsos. */
		last_line_x = x;
		last_line_y = y;
		last_line_horizontal = false;
		last_line_style = estilo;
		last_line_pendiente = true;
		return;
	}

	if (last_line_pendiente) {
		if (last_line_x == x && last_line_y == y) {
			/* Seguimos en la celda de inicio, aún sin dirección real */
			last_line_style = estilo;
			return;
		}
		/* Primer movimiento real: confirma la celda de inicio Y la nueva
		 * como un tramo vertical. */
		fam_v[last_line_y][last_line_x] = estilo;
		fam_v[y][x] = estilo;
		recalcular_vecinos(last_line_x, last_line_y);
		last_line_x = x;
		last_line_y = y;
		last_line_horizontal = false;
		last_line_style = estilo;
		last_line_pendiente = false;
		recalcular_vecinos(x, y);
		return;
	}

	bool hay_giro = last_line_horizontal && !(last_line_x == x && last_line_y == y);
	if (hay_giro) {
		/* Veníamos dibujando en horizontal y ahora salimos en vertical:
		 * la celda que se abandona recibe también la familia vertical,
		 * para que la esquina se forme justo al salir de ella. */
		fam_v[last_line_y][last_line_x] = estilo;
		recalcular_vecinos(last_line_x, last_line_y);
	}

	fam_v[y][x] = estilo;

	last_line_x = x;
	last_line_y = y;
	last_line_horizontal = false;
	last_line_style = estilo;

	recalcular_vecinos(x, y);
}

void ProcesadorCeldas::colocar_linea_horizontal(int x, int y, LineFamily estilo) {
	if (x < 0 || x >= cols || y < 0 || y >= rows) {
		return;
	}

	if (last_line_x < 0) {
		last_line_x = x;
		last_line_y = y;
		last_line_horizontal = true;
		last_line_style = estilo;
		last_line_pendiente = true;
		return;
	}

	if (last_line_pendiente) {
		if (last_line_x == x && last_line_y == y) {
			last_line_style = estilo;
			return;
		}
		fam_h[last_line_y][last_line_x] = estilo;
		fam_h[y][x] = estilo;
		recalcular_vecinos(last_line_x, last_line_y);
		last_line_x = x;
		last_line_y = y;
		last_line_horizontal = true;
		last_line_style = estilo;
		last_line_pendiente = false;
		recalcular_vecinos(x, y);
		return;
	}

	bool hay_giro = !last_line_horizontal && !(last_line_x == x && last_line_y == y);
	if (hay_giro) {
		/* Veníamos dibujando en vertical y ahora salimos en horizontal:
		 * la celda que se abandona recibe también la familia horizontal,
		 * para que la esquina se forme justo al salir de ella. */
		fam_h[last_line_y][last_line_x] = estilo;
		recalcular_vecinos(last_line_x, last_line_y);
	}

	fam_h[y][x] = estilo;

	last_line_x = x;
	last_line_y = y;
	last_line_horizontal = true;
	last_line_style = estilo;

	recalcular_vecinos(x, y);
}

void ProcesadorCeldas::reiniciar_ultima_linea() {
	last_line_x = -1;
	last_line_y = -1;
	last_line_horizontal = true;
	last_line_style = LineFamily::NONE;
	last_line_pendiente = true;
}

void ProcesadorCeldas::borrar_linea(int x, int y) {
	if (x < 0 || x >= cols || y < 0 || y >= rows) {
		return;
	}
	fam_v[y][x] = LineFamily::NONE;
	fam_h[y][x] = LineFamily::NONE;
	grid[y][x] = ' ';
	recalcular_vecinos(x, y);
}

void ProcesadorCeldas::colocar_caracter(int x, int y, gunichar c, bool insert_mode) {
	if (x < 0 || x >= cols || y < 0 || y >= rows) {
		return;
	}
	if (insert_mode) {
		for (int i = cols - 1; i > x; i--) {
			grid[y][i] = grid[y][i - 1];
			fam_v[y][i] = fam_v[y][i - 1];
			fam_h[y][i] = fam_h[y][i - 1];
		}
	}
	grid[y][x] = c;
	fam_v[y][x] = LineFamily::NONE;
	fam_h[y][x] = LineFamily::NONE;
	recalcular_vecinos(x, y);
}

void ProcesadorCeldas::borrar_en_posicion(int x, int y, bool insert_mode) {
	if (x < 0 || x >= cols || y < 0 || y >= rows) {
		return;
	}
	if (insert_mode) {
		for (int i = x; i < cols - 1; i++) {
			grid[y][i] = grid[y][i + 1];
			fam_v[y][i] = fam_v[y][i + 1];
			fam_h[y][i] = fam_h[y][i + 1];
		}
		grid[y][cols - 1] = ' ';
		fam_v[y][cols - 1] = LineFamily::NONE;
		fam_h[y][cols - 1] = LineFamily::NONE;
		recalcular_todo();
	} else {
		grid[y][x] = ' ';
		fam_v[y][x] = LineFamily::NONE;
		fam_h[y][x] = LineFamily::NONE;
		recalcular_vecinos(x, y);
	}
}

void ProcesadorCeldas::rellenar_seleccion(int x1, int y1, int x2, int y2, gunichar c) {
	int min_x = std::clamp(std::min(x1, x2), 0, cols - 1);
	int max_x = std::clamp(std::max(x1, x2), 0, cols - 1);
	int min_y = std::clamp(std::min(y1, y2), 0, rows - 1);
	int max_y = std::clamp(std::max(y1, y2), 0, rows - 1);
	for (int y = min_y; y <= max_y; y++) {
		for (int x = min_x; x <= max_x; x++) {
			grid[y][x] = c;
			fam_v[y][x] = LineFamily::NONE;
			fam_h[y][x] = LineFamily::NONE;
		}
	}
	recalcular_borde(min_x, max_x, min_y, max_y);
}

void ProcesadorCeldas::limpiar_seleccion(int x1, int y1, int x2, int y2) {
	int min_x = std::clamp(std::min(x1, x2), 0, cols - 1);
	int max_x = std::clamp(std::max(x1, x2), 0, cols - 1);
	int min_y = std::clamp(std::min(y1, y2), 0, rows - 1);
	int max_y = std::clamp(std::max(y1, y2), 0, rows - 1);
	for (int y = min_y; y <= max_y; y++) {
		for (int x = min_x; x <= max_x; x++) {
			grid[y][x] = ' ';
			fam_v[y][x] = LineFamily::NONE;
			fam_h[y][x] = LineFamily::NONE;
		}
	}
	recalcular_borde(min_x, max_x, min_y, max_y);
}

void ProcesadorCeldas::recalcular_rango_filas(int y0, int y1) {
	int start_y = std::max(0, y0);
	int end_y = std::min(rows - 1, y1);
	for (int y = start_y; y <= end_y; y++) {
		for (int x = 0; x < cols; x++) {
			if (fam_v[y][x] != LineFamily::NONE || fam_h[y][x] != LineFamily::NONE) {
				recalcular_celda(x, y);
			}
		}
	}
}

void ProcesadorCeldas::recalcular_rango_columnas(int x0, int x1) {
	int start_x = std::max(0, x0);
	int end_x = std::min(cols - 1, x1);
	for (int y = 0; y < rows; y++) {
		for (int x = start_x; x <= end_x; x++) {
			if (fam_v[y][x] != LineFamily::NONE || fam_h[y][x] != LineFamily::NONE) {
				recalcular_celda(x, y);
			}
		}
	}
}

void ProcesadorCeldas::insertar_fila(int posicion, bool despues) {
	int pos = std::clamp(despues ? posicion + 1 : posicion, 0, rows);
	grid.insert(grid.begin() + pos, std::vector<gunichar>(cols, ' '));
	fam_v.insert(fam_v.begin() + pos, std::vector<LineFamily>(cols, LineFamily::NONE));
	fam_h.insert(fam_h.begin() + pos, std::vector<LineFamily>(cols, LineFamily::NONE));
	rows++;
	recalcular_rango_filas(pos - 1, pos + 1);
}

void ProcesadorCeldas::eliminar_fila(int posicion) {
	if (rows <= 1) {
		return;
	}
	int at = std::clamp(posicion, 0, rows - 1);
	grid.erase(grid.begin() + at);
	fam_v.erase(fam_v.begin() + at);
	fam_h.erase(fam_h.begin() + at);
	rows--;
	recalcular_rango_filas(at - 1, at + 1);
}

void ProcesadorCeldas::insertar_columna(int posicion, bool despues) {
	int pos = std::clamp(despues ? posicion + 1 : posicion, 0, cols);
	for (int y = 0; y < rows; y++) {
		grid[y].insert(grid[y].begin() + pos, ' ');
		fam_v[y].insert(fam_v[y].begin() + pos, LineFamily::NONE);
		fam_h[y].insert(fam_h[y].begin() + pos, LineFamily::NONE);
	}
	cols++;
	recalcular_rango_columnas(pos - 1, pos + 1);
}

void ProcesadorCeldas::eliminar_columna(int posicion) {
	if (cols <= 1) {
		return;
	}
	int at = std::clamp(posicion, 0, cols - 1);
	for (int y = 0; y < rows; y++) {
		grid[y].erase(grid[y].begin() + at);
		fam_v[y].erase(fam_v[y].begin() + at);
		fam_h[y].erase(fam_h[y].begin() + at);
	}
	cols--;
	recalcular_rango_columnas(at - 1, at + 1);
}

std::string ProcesadorCeldas::exportar_a_string() const {
	std::string resultado;
	for (int y = 0; y < rows; y++) {
		for (int x = 0; x < cols; x++) {
			char buf[8] = { 0 };
			g_unichar_to_utf8(grid[y][x], buf);
			resultado += buf;
		}
		resultado += "\n";
	}
	return resultado;
}

void ProcesadorCeldas::importar_desde_string(const char *contenido, gsize longitud) {
	if (contenido == nullptr || longitud == 0) {
		return;
	}
	std::vector < std::vector < gunichar >> nueva_grid;
	std::vector<gunichar> fila_actual;
	const char *ptr = contenido;
	const char *end = contenido + longitud;
	while (ptr < end) {
		gunichar c = g_utf8_get_char(ptr);
		if (c == '\n') {
			nueva_grid.push_back(fila_actual);
			fila_actual.clear();
		} else if (c != '\r') {
			fila_actual.push_back(c);
		}
		ptr = g_utf8_next_char(ptr);
	}
	if (!fila_actual.empty()) {
		nueva_grid.push_back(fila_actual);
	}
	if (nueva_grid.empty()) {
		return;
	}
	rows = static_cast<int>(nueva_grid.size());
	cols = 0;
	for (const auto &f : nueva_grid) {
		cols = std::max(cols, static_cast<int>(f.size()));
	}
	grid.assign(rows, std::vector<gunichar>(cols, ' '));
	fam_v.assign(rows, std::vector<LineFamily>(cols, LineFamily::NONE));
	fam_h.assign(rows, std::vector<LineFamily>(cols, LineFamily::NONE));
	for (int y = 0; y < rows; y++) {
		for (size_t x = 0; x < nueva_grid[y].size(); x++) {
			if (x < static_cast<size_t>(cols)) {
				grid[y][x] = nueva_grid[y][x];
			}
		}
	}

	// Resetear estado de última línea tras importar
	reiniciar_ultima_linea();
}
