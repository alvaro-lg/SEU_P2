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

#define DEBUG false
#define N_BUSES 256
#define N_SLOTS 32
#define N_FUNCS 8
#define CONFIG_DIR 0xCF8
#define CONFIG_DAT 0xCFC

uint32_t get_dir_value (int bus, int device, int function, int reg);

int main(int argc, char *argv[]) {
	
	// Variables
	int vend, prod, class, subclass, interface, class_in, 
		subclass_in, interface_in;
	uint32_t dir, dat, bar0_io, bar1_io, bar2_io, 
		bar3_io, bar4_io, bar5_mem;

	// Lectura de argumentos de entrada
	if (argc < 4) {
		perror("Indique la clase, subclase e interfaz del dispositivo que desea buscar");
		return 1;
	} else if (argc > 4) {
		perror("Demasiados argumentos");
		return 1;
	} else {
		class_in = atoi(argv[1]);
		subclass_in = atoi(argv[2]);
		interface_in = atoi(argv[3]);
		
		if (DEBUG) printf("Buscando dispositivo de clase %#x, subclase %#x e interfaz %#x...\n", 
			class_in, subclass_in, interface_in);
	}
	
	// Permiso para acceso a los 2 puertos modo usuario
	if (ioperm(CONFIG_DIR, 8, 1) < 0) {
		perror("ioperm");
		return 1;
	}

	// Recorriendo buses, slots y funciones
	for (int i = 0; i < N_BUSES; i++) {
		for (int j = 0; j < N_SLOTS; j++) {
			for (int k = 0; k < N_FUNCS; k++) {
				
				// Nueva direccion base
				dir = get_dir_value(i, j, k, 0);
				
				outl(dir, CONFIG_DIR);
				dat = inl(CONFIG_DAT);
				
				if (dat != 0xFFFFFFFF) { // Caso de que el disp existe
					
					// Extraemos el vendedor y producto
					vend = dat & 0x0000FFFF; 
					prod = dat >> 16;
					
					// Generamos direccion para el segundo registro
					dir = get_dir_value(i, j, k, 2);
					
					// Extraemos la clase, subclase e interfaz
					outl(dir, CONFIG_DIR);
					dat = inl(CONFIG_DAT);
					class = (dat & 0xFF000000) >> 24;
					subclass = (dat & 0x00FF0000) >> 16;
					interface = (dat & 0x0000FF00) >> 8;
					
					if (class == class_in && subclass == subclass_in &&
						interface == interface_in) {
					
						// Generamos direccion para el siguiente registro
						dir = get_dir_value(i, j, k, 4);
						outl(dir, CONFIG_DIR);
						bar0_io = inl(CONFIG_DAT);

						// Generamos direccion para el siguiente registro
						dir = get_dir_value(i, j, k, 5);
						outl(dir, CONFIG_DIR);
						bar1_io = inl(CONFIG_DAT);
						
						// Generamos direccion para el siguiente registro
						dir = get_dir_value(i, j, k, 6);
						outl(dir, CONFIG_DIR);
						bar2_io = inl(CONFIG_DAT);
						
						// Generamos direccion para el siguiente registro
						dir = get_dir_value(i, j, k, 7);
						outl(dir, CONFIG_DIR);
						bar3_io = inl(CONFIG_DAT);
						
						// Generamos direccion para el siguiente registro
						dir = get_dir_value(i, j, k, 8);
						outl(dir, CONFIG_DIR);
						bar4_io = inl(CONFIG_DAT);
						
						// Generamos direccion para el siguiente registro
						dir = get_dir_value(i, j, k, 9);
						outl(dir, CONFIG_DIR);
						bar5_mem = inl(CONFIG_DAT);

						// Imprimimos la info
						printf("Bus %#x Slot %#x Función %#x Vendedor %#x Producto %#x Clase %#x Subclase %#x Interfaz %#x BAR0-IO %#x BAR1-IO %#x BAR2-IO %#x BAR3-IO %#x BAR4-IO %#x BAR5-Mem %#x\n", 
							i, j, k, vend, prod, class, subclass, interface, 
							bar0_io, bar1_io, bar2_io, bar3_io, bar4_io, bar5_mem);
					}
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
