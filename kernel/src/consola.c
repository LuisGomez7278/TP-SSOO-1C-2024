#include "../include/consola.h"
 
 
void iniciar_consola_interactiva(){
    char* leido=NULL;
    bool validacion_leido=false;

    while(1){
        
        leido=readline (">");
        
        if( leido != NULL && strcmp(leido, "") != 0){
            validacion_leido = validacion_de_ingreso_por_consola (leido);
            
            if(validacion_leido){
                atender_instruccion_validada(leido);
                log_trace(logger_debug, "Comando VALIDO");          
            }else{                
                 log_error(logger_debug, "Comando NO reconocido");
            }
            free(leido);
        }
    }
}
 

bool validacion_de_ingreso_por_consola(char* leido){
    bool comando_validado=false;
    char** array_de_comando= string_split (leido, " ");

    log_trace(logger_debug,"El comando tiene: %d de algo.",string_array_size(array_de_comando));

    if(strcmp(array_de_comando[0],"EJECUTAR_SCRIPT")==0 && string_array_size(array_de_comando)==2 ){                   //string_array_size(char** array);
        comando_validado=true;
    }else if (strcmp(array_de_comando[0],"INICIAR_PROCESO")==0 && string_array_size(array_de_comando)==2){
        comando_validado=true;
    }else if (strcmp(array_de_comando[0],"FINALIZAR_PROCESO")==0 && string_array_size(array_de_comando)==2){
        comando_validado=true;
    }else if (strcmp(array_de_comando[0],"DETENER_PLANIFICACION")==0 && string_array_size(array_de_comando)==1){
        comando_validado=true;
    }else if (strcmp(array_de_comando[0],"INICIAR_PLANIFICACION")==0 && string_array_size(array_de_comando)==1){
        comando_validado=true;
    }else if (strcmp(array_de_comando[0],"MULTIPROGRAMACION")==0 && string_array_size(array_de_comando)==2){
        comando_validado=true;
    }else if (strcmp(array_de_comando[0],"PROCESO_ESTADO")==0 && string_array_size(array_de_comando)==1){
        comando_validado=true;
    }

    return comando_validado;
}