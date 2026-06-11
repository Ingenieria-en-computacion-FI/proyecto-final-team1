#!/bin/bash

echo "=== Iniciando mantenimiento profundo del repositorio ==="

# Ejecutar el clean nativo del Makefile principal si existe
if [ -f "./Makefile" ]; then
    echo "Llamando instrucciones del Makefile..."
    make clean > /dev/null 2>&1
fi

# Limpieza exhaustiva de archivos binarios y objetos huérfanos
echo "Removiendo archivos objeto y volcados de memoria..."
rm -f *.obj *.o *.bin salida.obj ensamblador
rm -f src/*.o src/*.obj
rm -f examples/*.obj examples/*.o
rm -f /tmp/test_run.log

echo "=== Repositorio impecable y listo para ser empaquetado (.zip) ==="
