.DEFAULT_GOAL := help

.PHONY: help build test clean

help: ## Muestra los comandos disponibles
	@echo "EditorCajasEsquematicas"
	@echo
	@echo "Comandos:"
	@awk 'BEGIN {FS = ":.*##"} /^[a-zA-Z0-9_-]+:.*##/ {printf "  %-10s %s\n", $$1, $$2}' $(MAKEFILE_LIST)

build: ## Prepara o compila el proyecto
	@echo "No hay un sistema de compilación configurado todavía."

test: ## Ejecuta las pruebas del proyecto
	@echo "No hay pruebas configuradas todavía."

clean: ## Elimina archivos generados
	@echo "No hay archivos generados para limpiar."
