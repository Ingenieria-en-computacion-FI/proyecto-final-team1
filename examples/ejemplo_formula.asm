SECTION .text
GLOBAL _START

_START:
    ; Simular: EAX = (100 + 50) - (20 * 2)
    
    ; Parte 1: (100 + 50)
    MOV EAX, 100
    MOV EBX, 50
    ADD EAX, EBX       ; EAX ahora vale 150
    
    ; Parte 2: (20 * 2) simulado con sumas
    MOV ECX, 20
    MOV EDX, 20
    ADD ECX, EDX       ; ECX ahora vale 40
    
    ; Parte 3: Resta final
    SUB EAX, ECX       ; EAX = 150 - 40 = 110
    
    NOP                ; Fin de la operacion
