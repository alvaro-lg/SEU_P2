/**
 * Programa desarrollado en el primer apartado de la practica de
 * Programacion de Dispositivos en Linux de la asignatura Sistemas
 * Empotrados y Ubicuos.
 * 
 * @author Álvaro López García <alvaro.lopezgar@alumnos.upm.es>
 * @date Dic-2022
 */
 #include <stdio.h>
 #include <string.h>
 
 #define E2BIG 7
 
int main(int argc, char *argv[]) {
	
	if (argc < 2) {
		perror("Indique la dirección base del dispositivo");
		return 1;
	} else if (argc > 2) {
		perror("Demasiados argumentos");
		return 1;
	} else {
		printf("%s", argv[1]);
	}
	return 0;
}
