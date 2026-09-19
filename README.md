# Editor de Cajas Esquemáticas

Aplicación escrita en C++ con GTK 4 para dibujar esquemas o diagramas tipo cajas, uniones, líneas y selecciones sobre una rejilla.

## ¿Cómo funciona?

La aplicación crea una ventana principal con un área de dibujo en forma de cuadrícula. Cada celda puede contener:

- líneas simples, dobles, gruesas o punteadas,
- cruces y conexiones entre celdas,
- texto o caracteres sueltos,
- selecciones para rellenar o borrar zonas,
- inserción o reemplazo de contenido en la rejilla.

El flujo principal es:

1. Se inicia la aplicación desde `EditorCajasV0.cpp`.
2. La clase `EditorInterfaz` crea la ventana, la barra de herramientas y el lienzo GTK.
3. `ProcesadorCeldas` es la parte lógica que calcula cómo se dibujan las líneas y cómo se conectan entre sí.
4. `dialogo_archivo.cpp` añade la apertura y guardado de archivos mediante diálogos del sistema.

La herramienta de línea permite trazar conexiones entre celdas. El motor de la lógica recalcula automáticamente los caracteres de unión para generar esquinas, cruces y conexiones correctas según los vecinos de la celda.

## Dependencias

Necesitas un compilador de C++ y las bibliotecas de GTK 4.

En Debian/Ubuntu:

```bash
sudo apt update
sudo apt install build-essential gcc g++ make pkg-config libgtk-4-dev
```

Esto instala lo básico para compilar con GCC y GTK 4. En muchos sistemas también se instalan automáticamente dependencias como GLib, Pango y Cairo a través de `libgtk-4-dev`.

## Compilar

Desde la raíz del proyecto:

```bash
g++ EditorCajasV0.cpp editor_interfaz.cpp procesador_celdas.cpp dialogo_archivo.cpp -o EditorCajasV3 $(pkg-config --cflags --libs gtk4) -std=c++17
```

Y luego ejecuta:

```bash
./EditorCajasV3
```

## Nota del proyecto

El repositorio incluye un `Makefile`, pero actualmente no está configurado para compilar la aplicación real; solo muestra ayuda. Por eso, la forma recomendada de compilar es usar directamente `g++` con `pkg-config`.

## Estructura principal

- `EditorCajasV0.cpp`: punto de entrada de la aplicación.
- `editor_interfaz.hpp` / `editor_interfaz.cpp`: ventana, widgets y lógica visual.
- `procesador_celdas.hpp` / `procesador_celdas.cpp`: cálculo de celdas, conexiones y rejilla.
- `dialogo_archivo.hpp` / `dialogo_archivo.cpp`: carga y guardado de archivos.

## Requisitos mínimos

- GCC o G++
- `make` (opcional, pero recomendado)
- `pkg-config`
- GTK 4 (`libgtk-4-dev`)

