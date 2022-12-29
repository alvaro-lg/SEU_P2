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
#define N_DEVICES 32
#define N_FUNCS 8
#define CONFIG_DIR 0xCF8
#define CONFIG_DAT 0xCFC
#define STANDARD_HEADER 0x0
#define PCI_TO_PCI_HEADER 0x1
#define CARD_BUS_HEADER 0x2
#define PCI_TO_PCI_CLASS 0x6
#define PCI_TO_PCI_SUBCLASS 0x4

uint32_t get_dir_value (int bus, int device, int func, int reg);
int get_vend_id(int bus, int device, int func);
int get_prod_id(int bus, int device, int func);
int get_class(int bus, int device, int func);
int get_subclass(int bus, int device, int func);
int get_interface(int bus, int device, int func);
int get_header_type(int bus, int device, int func);
int get_secondary_bus(int bus, int device, int func);
bool is_multifunc(int bus, int device, int func);
uint32_t get_barx(int bus, int device, int func, int x);
void check_function(int bus, int device, int func, int class_in, int subclass_in, int interface_in);
void check_device(int bus, int device, int class_in, int subclass_in, int interface_in);
void check_bus(int bus, int class_in, int subclass_in, int interface_in);

int main(int argc, char *argv[]) {
	
	// Variables
	int class_in, subclass_in, interface_in, bus, prod, header_type,
	vend;

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
	
	// Escaneo recursivo
	header_type = get_header_type(0, 0, 0);
	if (header_type == STANDARD_HEADER) { // Single PCI host controller
		check_bus(0, class_in, subclass_in, interface_in);
	} else { // Multiple PCI host controllers
		for (int func = 0; func < N_FUNCS; func++) {
			vend = get_vend_id(0, 0, func);
			prod = get_prod_id(0, 0, func);
     
			// Caso de que el disp no existe
			if (vend == 0xFFFF && prod == 0xFFFF)
				break;
				
			// Escaneamos los demas buses
			bus = func;
			check_bus(bus, class_in, subclass_in, interface_in);
		}
	}
	return 0;
}

uint32_t get_dir_value (int bus,  int device, int func, int reg) {
	
	int bus_, device_, func_, reg_;
	uint32_t dir = 0x80000000;
	
	bus_ = bus << 16;
	device_ = device << 11;
	func_ = func << 8;
	reg_ = reg << 2;
	
	return dir | bus_ | device_ | func_ | reg_;
}

int get_vend_id(int bus, int device, int func) {
	
	// Valor del registro y direccion
	uint32_t dat, dir;
	
	// Nueva direccion base
	dir = get_dir_value(bus, device, func, 0);
				
	outl(dir, CONFIG_DIR);
	dat = inl(CONFIG_DAT);
	
	return dat & 0x0000FFFF; 
}

int get_prod_id(int bus, int device, int func) {
	
	// Valor del registro y direccion
	uint32_t dat, dir;
	
	// Nueva direccion base
	dir = get_dir_value(bus, device, func, 0);
				
	outl(dir, CONFIG_DIR);
	dat = inl(CONFIG_DAT);
	
	return dat >> 16;
}

int get_class(int bus, int device, int func) {
	
	// Valor del registro y direccion
	uint32_t dat, dir;
	
	// Nueva direccion base
	dir = get_dir_value(bus, device, func, 2);
				
	outl(dir, CONFIG_DIR);
	dat = inl(CONFIG_DAT);
	
	return (dat & 0xFF000000) >> 24;
}

int get_subclass(int bus, int device, int func) {
	
	// Valor del registro y direccion
	uint32_t dat, dir;
	
	// Nueva direccion base
	dir = get_dir_value(bus, device, func, 2);
				
	outl(dir, CONFIG_DIR);
	dat = inl(CONFIG_DAT);
	
	return (dat & 0x00FF0000) >> 16;
}

int get_interface(int bus, int device, int func) {
	
	// Valor del registro y direccion
	uint32_t dat, dir;
	
	// Nueva direccion base
	dir = get_dir_value(bus, device, func, 2);
				
	outl(dir, CONFIG_DIR);
	dat = inl(CONFIG_DAT);
	
	return (dat & 0x0000FF00) >> 8;
}

int get_header_type(int bus, int device, int func) {
	
	// Valor del registro y direccion
	uint32_t dat, dir;
	
	// Generamos direccion para el tercer registro
	dir = get_dir_value(bus, device, func, 3);
	
	// Averiguamos si el dispositivo es multifunción
	outl(dir, CONFIG_DIR);
	dat = inl(CONFIG_DAT);
	
	return (dat & 0x00EF0000) >> 16;
}

int get_secondary_bus(int bus, int device, int func) {
	
	// Valor del registro y direccion
	uint32_t dat, dir;
	
	// Generamos direccion para el tercer registro
	dir = get_dir_value(bus, device, func, 6);
	
	// Averiguamos si el dispositivo es multifunción
	outl(dir, CONFIG_DIR);
	dat = inl(CONFIG_DAT);
	
	return (dat & 0x0000FF00) >> 8;
}

bool is_multifunc(int bus, int device, int func) {
	
	// Valor del registro y direccion
	uint32_t dat, dir;
	
	// Generamos direccion para el tercer registro
	dir = get_dir_value(bus, device, func, 3);
	
	// Averiguamos si el dispositivo es multifunción
	outl(dir, CONFIG_DIR);
	dat = inl(CONFIG_DAT);
	
	return ((dat & 0x00FF0000) >> 23) > 0;
}

uint32_t get_barx(int bus, int device, int func, int x) {
	
	// Direccion del registro
	uint32_t dir;
	
	// Generamos direccion para el registro y obtenemos su valor
	dir = get_dir_value(bus, device, func, x + 4);
	outl(dir, CONFIG_DIR);
	return inl(CONFIG_DAT);
}

void check_function(int bus, int device, int func, int class_in, int subclass_in, int interface_in) {
	
	int vend, prod, class, subclass, new_bus, interface;
	uint32_t bar0_io, bar1_io, bar2_io, 
		bar3_io, bar4_io, bar5_mem;
    
    class = get_class(bus, device, func);
    subclass = get_subclass(bus, device, func);
    
    if ((class == PCI_TO_PCI_CLASS) && 
		(subclass == PCI_TO_PCI_SUBCLASS)) {
		new_bus = get_secondary_bus(bus, device, func);
		check_bus(new_bus, class_in, subclass_in, interface_in);
	} else {
		
		// Comprobamos que es como el que buscamos
		class = get_class(bus, device, func);
		subclass = get_subclass(bus, device, func);
		interface = get_interface(bus, device, func);
		
		if (class == class_in && subclass == subclass_in &&
						interface == interface_in) {
			// Obtenemos la info del dispositivo
			vend = get_vend_id(bus, device, func);
			prod = get_prod_id(bus, device, func);
			bar0_io = get_barx(bus, device, func, 0);
			bar1_io = get_barx(bus, device, func, 1);
			bar2_io = get_barx(bus, device, func, 2);
			bar3_io = get_barx(bus, device, func, 3);
			bar4_io = get_barx(bus, device, func, 4);
			bar5_mem = get_barx(bus, device, func, 5);

			// Imprimimos la info
			printf("Bus %#x Slot %#x Función %#x Vendedor %#x Producto %#x Clase %#x Subclase %#x Interfaz %#x BAR0-IO %#x BAR1-IO %#x BAR2-IO %#x BAR3-IO %#x BAR4-IO %#x BAR5-Mem %#x\n", 
				bus, device, func, vend, prod, class, subclass, interface, 
				bar0_io, bar1_io, bar2_io, bar3_io, bar4_io, bar5_mem);	
		}
	}
}

void check_device(int bus, int device, int class_in, int subclass_in, int interface_in) {
	
     int vend, prod, func = 0;
 
     vend = get_vend_id(bus, device, func);
     prod = get_prod_id(bus, device, func);
     
     // Caso de que el disp no existe
     if (vend == 0xFFFF && prod == 0xFFFF)
		return;
	
     check_function(bus, device, func, class_in, subclass_in, interface_in);
     
     if(is_multifunc(bus, device, func)) {
		 // It's a multi-function device, so check remaining functions
		 for (func = 1; func < N_FUNCS; func++) {
			 vend = get_vend_id(bus, device, func);
			 prod = get_prod_id(bus, device, func);
     
             if (vend != 0xFFFF && prod != 0xFFFF) {
                 check_function(bus, device, func, class_in, subclass_in, interface_in);
             }
         }
     }
 }

void check_bus(int bus, int class_in, int subclass_in, int interface_in) {
	for (int dev = 0; dev < N_DEVICES; dev++) {
		check_device(bus, dev, class_in, subclass_in, interface_in);
	}
}
