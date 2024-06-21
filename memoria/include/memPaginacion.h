#ifndef TP_MEMORIA_PAGINACION_H_
#define TP_MEMORIA_PAGINACION_H_

#include <commons/bitarray.h>
#include <readline/readline.h>
#include "../../utils/include/utils.h"
#include "../../utils/include/conexiones.h"
#include "extGlobales.h"

typedef struct{
    int32_t pid;
    t_list* instrs;
    t_list* paginas;
}procesoM;

typedef struct{
    int32_t marco;
    bool presencia;
}tabla_pag; //tabla de paginas en si

typedef struct {
    int32_t pid;                  
    t_list* paginas;          // lista de tablas_pag
}tabla_pag_proceso; //tabla de paginas de cada proceso

void inicializarMem();

bool crear_procesoM(char* path_instrucciones, uint32_t PID);
void eliminar_procesoM(uint32_t PID);

procesoM* buscar_proceso_por_pid(uint32_t pid);
t_list* obtener_instrs(uint32_t pid);

// tablas
void añadirTablaALista(t_list* paginas, uint32_t PID);
//t_list* crear_tabla_pag(uint32_t size);
//void crear_paginas(uint32_t PID, int32_t cantidad_paginas);
tabla_pag_proceso* obtener_tabla_pag_proceso(uint32_t PID);
tabla_pag* obtener_pagina_proceso(tabla_pag_proceso* tabla_proceso, int32_t numero_pagina);
tabla_pag* buscar_siguiente_pagina(tabla_pag_proceso* tabla_proceso, int32_t marco_actual);

int32_t asignar_marco(uint32_t PID, int32_t numero_pagina);
void liberar_frames(t_list* paginas);
uint32_t encontrar_frame(uint32_t PID, uint32_t pagina);
int32_t obtener_marco(int32_t direccion_fisica);
int32_t obtener_desplazamiento(int32_t direccion_fisica);

bool resize(uint32_t PID, int32_t size);
bool añadir_pagina_a_proceso(tabla_pag_proceso* tabla, uint32_t num_paginas, uint32_t PID);
void eliminar_pagina_de_proceso(tabla_pag_proceso* tabla, int32_t num_paginas);

bool escribir_memoria(int32_t direccion_fisica, uint32_t bytes, char* valor, uint32_t PID);
bool escribir_uint32_t_en_memoria(int32_t direccion_fisica, uint32_t bytes, uint32_t valor, uint32_t PID);

uint32_t leer_memoria_uint32_t(int32_t direccion_fisica, uint8_t bytes, uint32_t PID);
char* leer_memoria(int32_t direccion_fisica, int32_t bytes, uint32_t PID);

#endif //TP_MEMORIA_PAGINACION_H_
