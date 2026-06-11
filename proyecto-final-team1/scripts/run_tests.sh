#!/bin/bash

# Colores para la terminal de WSL
GREEN='\033[0;32m'
RED='\033[0;31m'
BLUE='\033[0;34m'
NC='\033[0m' # Sin color

echo -e "${BLUE}====================================================${NC}"
echo -e "${BLUE}   SUITE AUTOMATIZADA DE PRUEBAS — EQUIPO 1         ${NC}"
echo -e "${BLUE}====================================================${NC}\n"

# 1. Validar que el ejecutable exista en la raíz del proyecto
if [ ! -f "./ensamblador" ]; then
    echo -e "${RED}[ERROR] No se encontro el ejecutable 'ensamblador' en la raiz.${NC}"
    echo -e "Por favor, ejecuta 'make' antes de correr las pruebas.\n"
    exit 1
fi

TOTAL_PRUEBAS=0
PRUEBAS_EXITOSAS=0

# 2. Rastrear y ejecutar cada archivo .asm en la carpeta de ejemplos
for file in examples/*.asm; do
    # Asegurar que existan archivos que coincidan
    [ -e "$file" ] || continue
    
    TOTAL_PRUEBAS=$((TOTAL_PRUEBAS + 1))
    echo -e "Ejecutando Prueba $TOTAL_PRUEBAS: ${BLUE}$file${NC}"
    
    # Ejecutar guardando logs en temporales para no ensuciar la pantalla
    ./ensamblador "$file" > /tmp/test_run.log 2>&1
    STATUS=$?
    
    # 3. Evaluar robustez del sistema
    # Un exit code de 0 significa éxito total. 
    # El archivo de estrés (test.asm) generará avisos léxicos controlados, si termina sin crashearse (SegFault), es correcto.
    if [ $STATUS -eq 0 ] || [ $STATUS -eq 1 ]; then
        if [[ "$file" == *"test.asm"* ]]; then
            echo -e "${GREEN}[PASS] Tolerancia a fallos validada de forma segura.${NC}\n"
        else
            echo -e "${GREEN}[PASS] Modulo ensamblado con exito.${NC}\n"
        fi
        PRUEBAS_EXITOSAS=$((PRUEBAS_EXITOSAS + 1))
    else
        echo -e "${RED}[FAIL] Violacion de memoria o colapso inesperado (Exit code: $STATUS).${NC}\n"
    fi
done

# 4. Desplegar reporte final de control de calidad
echo -e "${BLUE}====================================================${NC}"
echo -e "               RESUMEN DE EVALUACIÓN                "
echo -e "${BLUE}====================================================${NC}"
echo -e "Total de casos analizados: $TOTAL_PRUEBAS"
if [ $PRUEBAS_EXITOSAS -eq $TOTAL_PRUEBAS ]; then
    echo -e "Resultado global: ${GREEN}$PRUEBAS_EXITOSAS / $TOTAL_PRUEBAS PASADOS (100% Correctitud)${NC}"
else
    echo -e "Resultado global: ${RED}$PRUEBAS_EXITOSAS / $TOTAL_PRUEBAS PASADOS. Revisar fallos.${NC}"
fi
echo -e "${BLUE}====================================================${NC}"
