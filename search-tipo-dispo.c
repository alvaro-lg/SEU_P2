/**
 * Programa desarrollado en el segundo apartado de la practica de
 * Programacion de Dispositivos en Linux de la asignatura Sistemas
 * Empotrados y Ubicuos.
 * 
 * @author Álvaro López García <alvaro.lopezgar@alumnos.upm.es>
 * @date Dic-2022
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <fcntl.h>
#include <stdint.h>
#include <sys/io.h>

#define DEBUG true
#define N_BUSES 256
#define N_SLOTS 32
#define N_FUNCS 8
#define CONFIG_DIR 0xCF8
#define CONFIG_DAT 0xCFC

uint32_t get_dir_value (int bus, int device, int function, int reg);

int main(int argc, char *argv[]) {
	
	// Variables
	int vend, prod, class, subclass, interface;
	uint32_t dir, dat;

	// Lectura de argumentos de entrada
	if (argc < 4) {
		perror("Indique la clase, subclase e interfaz del dispositivo que desea buscar");
		return 1;
	} else if (argc > 4) {
		perror("Demasiados argumentos");
		return 1;
	} else {
		class = atoi(argv[1]);
		subclass = atoi(argv[2]);
		interface = atoi(argv[3]);
		
		if (DEBUG) printf("Buscando dispositivo de clase %d, subclase %d e interfaz %d\n", class, subclass, interface);
	}
	
	// Permiso para acceso a los 2 puertos modo usuario
	// TODO: Revisar
	if (ioperm(CONFIG_DIR, 8, 1) < 0) {
		perror("ioperm");
		return 1;
	}
	
	for (int i = 0; i < N_BUSES; i++) {
		for (int j = 0; j < N_SLOTS; j++) {
			for (int k = 0; k < N_FUNCS; k++) {
				dir = get_dir_value(i, j, k, 0);
				
				outl(dir, CONFIG_DIR);
				dat = inl(CONFIG_DAT);
				
				if (dat != 0xFFFFFFFF) { // Caso de que el disp existe
					
					// Extraemos el vendedor y producto
					vend = dat & 0x0000FFFF; 
					prod = dat >> 16;
					
					// Extraemos la clase y subclase
					// TODO: Revisar
					ioperm(CONFIG_DIR + 0x8, 8, 1);
					outl(dir, CONFIG_DIR + 0x8);
					dat = inl(CONFIG_DAT + 0x8);
					class = (dat & 0xFF000000) >> 24;
					subclass = (dat & 0x00FF0000) >> 16;

					printf("Bus %x Slot %x Función %x Vendedor %x Producto %x Clase %x Subclase %x\n", 
						i, j, k, vend, prod, class, subclass);
				}
			}
		}
	}

	return 0;
}

uint32_t get_dir_value (int bus, int device, int function, int reg) {
	
	int bus_, device_, function_, reg_;
	uint32_t dir = 0x80000000;
	
	bus_ = bus << 16;
	device_ = device << 11;
	function_ = function << 8;
	reg_ = reg << 2;
	
	return dir | bus_ | device_ | function_ | reg_;
}
