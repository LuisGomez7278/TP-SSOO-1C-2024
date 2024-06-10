#ifndef TP_MEMORIA_PAGINACION_H_
#define TP_MEMORIA_PAGINACION_H_

#include <commons/bitarray.h>
#include <readline/readline.h>
#include "../../utils/include/utils.h"
#include "../../utils/include/conexiones.h"
#include "extGlobales.h"

typedef struct{
    int pid;
    t_list* instrs;
    t_list* paginas;
}procesoM;

typedef struct{
    int marco;
    bool presencia;
}tabla_pag; //tabla de paginas en si

typedef struct {
    int pid;                  
    t_list* paginas;          // lista de tablas_pag
}tabla_pag_proceso; //tabla de paginas de cada proceso

void inicializarMem();

procesoM* crear_proceso(char* path_instrucciones, uint32_t PID);

// tablas
void añadirTablaALista(t_list* paginas, uint32_t PID);
//t_list* crear_tabla_pag(uint32_t size);
//void crear_paginas(uint32_t PID, int cantidad_paginas);
tabla_pag_proceso* obtener_tabla_pag_proceso(uint32_t PID);
tabla_pag* obtener_pagina_proceso(tabla_pag_proceso* tabla_proceso, int numero_pagina);
tabla_pag* buscar_siguiente_pagina(tabla_pag_proceso* tabla_proceso, int marco_actual);

int asignar_marco(uint32_t PID, int numero_pagina);
int obtener_marco(int direccion_fisica);
int obtener_desplazamiento(int direccion_fisica);

void resize(uint32_t PID, int size);
void añadir_pagina_a_proceso(tabla_pag_proceso* tabla, uint32_t num_paginas, uint32_t PID);
void eliminar_pagina_de_proceso(tabla_pag_proceso* tabla, int num_paginas);

bool escribir_memoria(int direccion_fisica, int bytes, char* valor, uint32_t PID);
char* leer_memoria(int direccion_fisica, int bytes, uint32_t PID);

#endif //TP_MEMORIA_PAGINACION_H_
