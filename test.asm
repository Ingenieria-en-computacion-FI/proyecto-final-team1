;;; =========================================
;;; PRUEBA DE ESTRÉS - MÓDULO LEXER (IA-32)
;;; =========================================

    SECTION .data
_var_Global1    db    100       ; Identificador válido con guion bajo

    SECTION .text
    global _start

_start:
    mOv    EaX,  [ _var_Global1 ]    ; Prueba de mayúsculas/minúsculas y corchetes SIB
    aDd    EbX, -450                 ; Prueba de números negativos
    
    ; --- AQUÍ VIENE LA BASURA LÉXICA ---
    MOV    ECX, @simbolo_ilegal      ; ERROR 1: Carácter especial prohibido
    JMP    ?etiqueta_rota            ; ERROR 2: Símbolo de interrogación
    ADD    EDX, 10$USD               ; ERROR 3: Símbolo de moneda en un número
    ; -----------------------------------

etiqueta_valida:                     ; Etiqueta correcta terminada en dos puntos
    INC EDI;ComentarioPegadoSinEspacio
    NOP