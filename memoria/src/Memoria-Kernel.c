#include "../include/Memoria-Kernel.h"


// void atender_conexion_KERNEL_MEMORIA(){
//     bool continuarIterando=1;
//      t_list* lista;   
//         while (continuarIterando) {
//             int cod_op = recibir_operacion(fd_kernel);
//             switch (cod_op) {
//             case HANDSHAKE:
//                 recibir_mensaje(fd_kernel,logger_memoria);
//                 break;
//             case PAQUETE:
//                 lista = recibir_paquete(fd_kernel);
//                 log_info(logger_memoria, "Me llegaron los siguientes valores:\n");
//                 list_logger(lista,logger_memoria_db);
//                 // list_iterate(lista, (void*) iterator);
// 			break;
//             case -1:
//                 log_error(logger_memoria_db, "el MODULO DE KERNEL SE DESCONECTO. Terminando servidor");
//                 continuarIterando=0;
//                 break;
//             default:
//                 log_warning(logger_memoria_db,"Operacion desconocida de KERNEL. No quieras meter la pata");
//                 break;
//             }
//         }


// }

