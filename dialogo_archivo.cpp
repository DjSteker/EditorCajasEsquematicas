/*
 * dialogo_archivo.cpp
 *
 *  Created on: 4 ago 2026
 *      Author: DjSteker
 */

#include "dialogo_archivo.hpp"
#include <cstring>

void SistemaArchivos::on_guardar_terminado(GObject *origen, GAsyncResult *resultado, gpointer ud) {
	ContextoGuardado *ctx = static_cast<ContextoGuardado*>(ud);
	GtkFileDialog *dialogo = GTK_FILE_DIALOG(origen);

	GError *err = nullptr;
	GFile *archivo = gtk_file_dialog_save_finish(dialogo, resultado, &err);

	if (archivo == nullptr) {
		if (err != nullptr) {
			if (!g_error_matches(err, G_IO_ERROR, G_IO_ERROR_CANCELLED)) {
				gchar *msg = g_strdup_printf("❌ Error al guardar: %s", err->message);
				mostrar_estado(ctx->etiqueta_estado, msg);
				g_free(msg);
			}
			g_error_free(err);
		}
	} else {
		GError *err_escritura = nullptr;
		if (g_file_replace_contents(archivo, ctx->contenido, ctx->longitud, nullptr, FALSE, G_FILE_CREATE_NONE, nullptr, nullptr, &err_escritura)) {
			gchar *ruta = g_file_get_path(archivo);
			gchar *msg = g_strdup_printf("✅ Guardado en %s", ruta != nullptr ? ruta : "el archivo seleccionado");
			mostrar_estado(ctx->etiqueta_estado, msg);
			g_free(msg);
			g_free(ruta);
		} else {
			gchar *msg = g_strdup_printf("❌ Error al guardar: %s", err_escritura->message);
			mostrar_estado(ctx->etiqueta_estado, msg);
			g_error_free(err_escritura);
			g_free(msg);
		}
		g_object_unref(archivo);
	}

	g_free(ctx->contenido);
	g_free(ctx);
}

void SistemaArchivos::guardar_archivo_dialogo(GtkWindow *ventana_padre, const char *contenido, GtkLabel *etiqueta_estado) {
	GtkFileDialog *dialogo = gtk_file_dialog_new();
	gtk_file_dialog_set_title(dialogo, "Guardar diagrama");
	gtk_file_dialog_set_initial_name(dialogo, "diagrama.txt");

	ContextoGuardado *ctx = g_new0(ContextoGuardado, 1);
	ctx->longitud = std::strlen(contenido);
	ctx->contenido = g_strndup(contenido, ctx->longitud);
	ctx->etiqueta_estado = etiqueta_estado;

	gtk_file_dialog_save(dialogo, ventana_padre, nullptr, on_guardar_terminado, ctx);
	g_object_unref(dialogo);
}

void SistemaArchivos::on_cargar_terminado(GObject *origen, GAsyncResult *resultado, gpointer ud) {
	ContextoCarga *ctx = static_cast<ContextoCarga*>(ud);
	GtkFileDialog *dialogo = GTK_FILE_DIALOG(origen);

	GError *err = nullptr;
	GFile *archivo = gtk_file_dialog_open_finish(dialogo, resultado, &err);

	if (archivo == nullptr) {
		if (err != nullptr) {
			if (!g_error_matches(err, G_IO_ERROR, G_IO_ERROR_CANCELLED)) {
				gchar *msg = g_strdup_printf("❌ Error al abrir: %s", err->message);
				mostrar_estado(ctx->etiqueta_estado, msg);
				g_free(msg);
			}
			g_error_free(err);
		}
		if (ctx->callback != nullptr) {
			ctx->callback(nullptr, 0, ctx->user_data);
		}
	} else {
		gchar *contenido = nullptr;
		gsize longitud = 0;
		GError *err_lectura = nullptr;

		if (g_file_load_contents(archivo, nullptr, &contenido, &longitud, nullptr, &err_lectura)) {
			gchar *ruta = g_file_get_path(archivo);
			gchar *msg = g_strdup_printf("✅ Cargado desde %s", ruta != nullptr ? ruta : "el archivo seleccionado");
			mostrar_estado(ctx->etiqueta_estado, msg);
			g_free(msg);
			g_free(ruta);

			if (ctx->callback != nullptr) {
				ctx->callback(contenido, longitud, ctx->user_data);
			}
			g_free(contenido);
		} else {
			gchar *msg = g_strdup_printf("❌ Error al leer: %s", err_lectura->message);
			mostrar_estado(ctx->etiqueta_estado, msg);
			g_error_free(err_lectura);
			g_free(msg);
			if (ctx->callback != nullptr) {
				ctx->callback(nullptr, 0, ctx->user_data);
			}
		}
		g_object_unref(archivo);
	}

	g_free(ctx);
}

void SistemaArchivos::cargar_archivo_dialogo(GtkWindow *ventana_padre, CargarArchivoCallback callback, gpointer user_data, GtkLabel *etiqueta_estado) {
	GtkFileDialog *dialogo = gtk_file_dialog_new();
	gtk_file_dialog_set_title(dialogo, "Cargar diagrama");

	ContextoCarga *ctx = g_new0(ContextoCarga, 1);
	ctx->callback = callback;
	ctx->user_data = user_data;
	ctx->etiqueta_estado = etiqueta_estado;

	gtk_file_dialog_open(dialogo, ventana_padre, nullptr, on_cargar_terminado, ctx);
	g_object_unref(dialogo);
}
