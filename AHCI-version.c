/**
 * Programa desarrollado en el primer apartado de la practica de
 * Programacion de Dispositivos en Linux de la asignatura Sistemas
 * Empotrados y Ubicuos.
 * 
 * @author Álvaro López García <alvaro.lopezgar@alumnos.upm.es>
 * @date Dic-2022
 */

#include <stdio.h>
#include <fcntl.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include <sys/mman.h>

#define DEBUG true
#define SIZE 4096

// Struct para leer info del dispositivo
struct ahci_info {
	uint32_t CAP; 
	uint32_t GHC; 
	uint32_t IS; 
	uint32_t PI;
	uint32_t VS; 
	// ... no usados por el programa
};

/**
 * Funcion auxiliar para extraer los valores de major y minor del 
 * registro MMIO de la version.
 * 
 * @param struct ahci_info volatile estructura con los valores de los
 * registros de control del ahci.
 * @return float con el numero de version.
 */
float get_version(struct ahci_info volatile *ahci);
 
int main(int argc, char *argv[]) {
	
	// Variables
	int fd;
	unsigned int base_dir;
	struct ahci_info volatile *ahci;
	
	// Lectura de argumentos de entrada
	if (argc < 2) {
		perror("Indique la dirección base del dispositivo");
		return 1;
	} else if (argc > 2) {
		perror("Demasiados argumentos");
		return 1;
	} else {
		base_dir = strtoul(argv[1], NULL, 16);
	}
	
	// De lAPIC.c
	if ((fd = open("/dev/mem", O_RDONLY|O_DSYNC)) < 0) { 
		// O_DSYNC: accesos no usan cache
		perror("open"); 
		return 1; 
	}
	
	// Traduccion dir. lógicas a dir. físicas de los regs MMIO
	if ((ahci = mmap(NULL, SIZE, PROT_READ, MAP_SHARED, fd,
			base_dir))==MAP_FAILED) {
		perror("mmap"); 
		return 1; 
	}
		
	if (DEBUG) printf("Acceso dir física %x usando %p\n", 
		base_dir, ahci);
		
	// Imprimiendo numero de version
	printf("Versión %.1f\n", get_version(ahci));
	
	return 0;
}

float get_version(struct ahci_info volatile *ahci) {
	
	uint32_t VS = ahci -> VS; // Valor del registro de la version
	float major, minor; // Variables locales

	// Extraccion de los valores del major y el minor
	major = (VS & 0xFFFF0000) >> 16;
	minor = ((VS & 0x0000FF00) >> 8) * 10 + (VS & 0x000000FF);
	
	return major + (minor / 100);
}
