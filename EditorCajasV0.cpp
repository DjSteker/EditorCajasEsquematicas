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

