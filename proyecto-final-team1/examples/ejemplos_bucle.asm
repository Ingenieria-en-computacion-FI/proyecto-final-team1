SECTION .data
    VALORES: DB 5, 10, 15, 20

SECTION .text
GLOBAL _START

_START:
    MOV EBX, VALORES   ; EBX apunta al primer dato en memoria
    MOV ECX, 4         ; ECX es el contador (4 vueltas)
    MOV EAX, 0         ; EAX acumulara la suma total

SUMAR:
    ADD EAX, [EBX]     ; Suma a EAX el valor apuntado por EBX
    INC EBX            ; Mueve el puntero a la siguiente direccion
    DEC ECX            ; Resta 1 al contador de vueltas
    JNE SUMAR          ; Si ECX no es cero, salta a SUMAR

    NOP                ; EAX terminara valiendo 50
