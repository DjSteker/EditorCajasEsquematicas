/*
 * dialogo_archivo.hpp
 *
 *  Created on: 4 ago 2026
 *      Author: DjSteker
 */

#ifndef DIALOGO_ARCHIVO_HPP_
#define DIALOGO_ARCHIVO_HPP_

#include <gtk/gtk.h>
#include <cstring>

/**
 * @brief Clase de utilidad para gestionar la apertura y guardado de archivos
 *        de texto mediante diálogos nativos del sistema en GTK4 (GtkFileDialog).
 */
class SistemaArchivos {
public:
	/**
	 * @brief Callback invocado al finalizar la carga de un archivo.
	 * @param contenido Bytes leídos del archivo (nullptr si hubo error o se canceló).
	 * @param longitud Número de bytes en "contenido" (0 si hubo error o se canceló).
	 * @param user_data Puntero de contexto proporcionado por el llamador.
	 *
	 * @note El puntero "contenido" solo es válido durante la ejecución del callback;
	 *       si se necesita conservarlo, debe copiarse dentro del propio callback.
	 */
	using CargarArchivoCallback = void (*)(const char *contenido, gsize longitud, gpointer user_data);

	/**
	 * @brief Muestra el selector del sistema para GUARDAR un archivo.
	 * @param ventana_padre Ventana GTK sobre la que se muestra el diálogo (puede ser nullptr).
	 * @param contenido Texto a escribir (cadena terminada en NUL).
	 * @param etiqueta_estado GtkLabel donde se reporta el resultado (puede ser nullptr).
	 */
	static void guardar_archivo_dialogo(GtkWindow *ventana_padre, const char *contenido, GtkLabel *etiqueta_estado);

	/**
	 * @brief Muestra el selector del sistema para ABRIR un archivo.
	 * @param ventana_padre Ventana GTK sobre la que se muestra el diálogo (puede ser nullptr).
	 * @param callback Función invocada con el contenido leído.
	 * @param user_data Puntero de contexto que se reenvía tal cual al callback.
	 * @param etiqueta_estado GtkLabel donde se reporta el resultado (puede ser nullptr).
	 */
	static void cargar_archivo_dialogo(GtkWindow *ventana_padre, CargarArchivoCallback callback, gpointer user_data, GtkLabel *etiqueta_estado);

private:
	/* --- Estructuras de contexto interno para operaciones asíncronas --- */

	struct ContextoGuardado {
		gchar *contenido;
		gsize longitud;
		GtkLabel *etiqueta_estado;
	};

	struct ContextoCarga {
		CargarArchivoCallback callback;
		gpointer user_data;
		GtkLabel *etiqueta_estado;
	};

	/* --- Métodos auxiliares y callbacks internos --- */

	/**
	 * @brief Actualiza el texto de la etiqueta de estado si existe; de lo contrario, usa g_message.
	 */
	static void mostrar_estado(GtkLabel *etiqueta, const char *texto) {
		if (etiqueta != nullptr) {
			gtk_label_set_text(etiqueta, texto);
		} else {
			g_message("%s", texto);
		}
	}

	/**
	 * @brief Callback de finalización para la operación de guardado de GtkFileDialog.
	 */
	static void on_guardar_terminado(GObject *origen, GAsyncResult *resultado, gpointer ud);

	/**
	 * @brief Callback de finalización para la operación de carga de GtkFileDialog.
	 */
	static void on_cargar_terminado(GObject *origen, GAsyncResult *resultado, gpointer ud);
};

#endif /* DIALOGO_ARCHIVO_HPP_ */

/*
 * dialogo_archivo.hpp
 *
 *  Created on: 4 ago 2026
 *      Author: usuario001
 */
//
//#ifndef DIALOGO_ARCHIVO_HPP_
//#define DIALOGO_ARCHIVO_HPP_
//
//#include <gtk/gtk.h>
//#include <cstring>
//
//class SistemaArchivos {
//private:
//	/* --- Contexto interno para la operación de guardado --- */
//	struct ContextoGuardado {
//		gchar *contenido;
//		gsize longitud;
//		GtkLabel *etiqueta_estado;
//	};
//
//	/* --- Contexto interno para la operación de carga --- */
//	struct ContextoCarga {
//		CargarArchivoCallback callback;
//		gpointer user_data;
//		GtkLabel *etiqueta_estado;
//	};
//
//	/* --- Actualiza la etiqueta de estado si existe --- */
//	static void mostrar_estado(GtkLabel *etiqueta, const char *texto) {
//		if (etiqueta != nullptr) {
//			gtk_label_set_text(etiqueta, texto);
//		} else {
//			g_message("%s", texto);
//		}
//	}
//
//	/*
//	 * Módulo de diálogos de sistema para guardar y cargar archivos de texto
//	 * usando GtkFileDialog (API asíncrona nativa de GTK4, sustituye a
//	 * GtkFileChooserDialog). El usuario elige el directorio/archivo mediante
//	 * el selector propio del sistema operativo.
//	 */
//
//	/*
//	 * Callback invocado al finalizar la carga de un archivo.
//	 *   contenido: bytes leídos del archivo (nullptr si hubo error o se canceló)
//	 *   longitud:  número de bytes en "contenido" (0 si hubo error o se canceló)
//	 *   user_data: puntero de contexto proporcionado por el llamador, reenviado tal cual
//	 *
//	 * El puntero "contenido" solo es válido durante la ejecución del callback;
//	 * si se necesita conservarlo, debe copiarse dentro del propio callback.
//	 */
//	using CargarArchivoCallback = void (*)(const char *contenido, gsize longitud, gpointer user_data);
//public:
//	/*
//	 * Muestra el selector de sistema para GUARDAR y escribe "contenido" en el
//	 * archivo/directorio que elija el usuario.
//	 *   ventana_padre:   ventana GTK sobre la que se muestra el diálogo (puede ser nullptr)
//	 *   contenido:       texto a escribir (cadena terminada en NUL)
//	 *   etiqueta_estado: GtkLabel donde se reporta el resultado (puede ser nullptr)
//	 */
//	static void guardar_archivo_dialogo(GtkWindow *ventana_padre, const char *contenido, GtkLabel *etiqueta_estado);
//	/*
//	 * Muestra el selector de sistema para ABRIR y, si el usuario elige un
//	 * archivo, lee su contenido y lo entrega mediante "callback".
//	 *   ventana_padre:   ventana GTK sobre la que se muestra el diálogo (puede ser nullptr)
//	 *   callback:        función invocada con el contenido leído
//	 *   user_data:       puntero de contexto que se reenvía tal cual al callback
//	 *   etiqueta_estado: GtkLabel donde se reporta el resultado (puede ser nullptr)
//	 */
//	static void cargar_archivo_dialogo(GtkWindow *ventana_padre, CargarArchivoCallback callback, gpointer user_data, GtkLabel *etiqueta_estado);
//
//};
//
//#endif /* DIALOGO_ARCHIVO_HPP_ */
/*
 * dialogo_archivo.hpp
 */

//#ifndef DIALOGO_ARCHIVO_HPP_
//#define DIALOGO_ARCHIVO_HPP_
//
//#include <gtk/gtk.h>
//#include <cstring>
//
//class SistemaArchivos {
//public:
//	/*
//	 * Callback invocado al finalizar la carga de un archivo.
//	 */
//	using CargarArchivoCallback = void (*)(const char *contenido, gsize longitud, gpointer user_data);
//
//	/*
//	 * Métodos públicos para invocar los diálogos
//	 */
//	static void guardar_archivo_dialogo(GtkWindow *ventana_padre, const char *contenido, GtkLabel *etiqueta_estado);
//	static void cargar_archivo_dialogo(GtkWindow *ventana_padre, CargarArchivoCallback callback, gpointer user_data, GtkLabel *etiqueta_estado);
//
//private:
//	/* --- Contexto interno para la operación de guardado --- */
//	struct ContextoGuardado {
//		gchar *contenido;
//		gsize longitud;
//		GtkLabel *etiqueta_estado;
//	};
//
//	/* --- Contexto interno para la operación de carga --- */
//	struct ContextoCarga {
//		CargarArchivoCallback callback;
//		gpointer user_data;
//		GtkLabel *etiqueta_estado;
//	};
//
//	/* --- Actualiza la etiqueta de estado si existe --- */
//	static void mostrar_estado(GtkLabel *etiqueta, const char *texto) {
//		if (etiqueta != nullptr) {
//			gtk_label_set_text(etiqueta, texto);
//		} else {
//			g_message("%s", texto);
//		}
//	}
//
//	/* --- Callbacks internos de GtkFileDialog --- */
//	/*
//	 * Muestra el selector de sistema para GUARDAR y escribe "contenido" en el
//	 * archivo/directorio que elija el usuario.
//	 *   ventana_padre:   ventana GTK sobre la que se muestra el diálogo (puede ser nullptr)
//	 *   contenido:       texto a escribir (cadena terminada en NUL)
//	 *   etiqueta_estado: GtkLabel donde se reporta el resultado (puede ser nullptr)
//	 */
//	static void on_guardar_terminado(GObject *origen, GAsyncResult *resultado, gpointer ud);
//	/*
//	 * Muestra el selector de sistema para ABRIR y, si el usuario elige un
//	 * archivo, lee su contenido y lo entrega mediante "callback".
//	 *   ventana_padre:   ventana GTK sobre la que se muestra el diálogo (puede ser nullptr)
//	 *   callback:        función invocada con el contenido leído
//	 *   user_data:       puntero de contexto que se reenvía tal cual al callback
//	 *   etiqueta_estado: GtkLabel donde se reporta el resultado (puede ser nullptr)
//	 */
//	static void on_cargar_terminado(GObject *origen, GAsyncResult *resultado, gpointer ud);
//};
//
//#endif /* DIALOGO_ARCHIVO_HPP_ */
//
