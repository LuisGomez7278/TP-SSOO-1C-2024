#include "../include/entradasalida_FS.h"

void inicializar_FS()
{
    path_metadata = string_duplicate(PATH_BASE_DIALFS);
    string_append(&path_metadata, "/metadata/");

    path_bloques = string_duplicate(PATH_BASE_DIALFS);
    string_append(&path_bloques, "bloques.dat");

    path_bitmap = string_duplicate(PATH_BASE_DIALFS);
    string_append(&path_bitmap, "bitmap.dat");

    archivos_existentes = list_create();

    inicializar_bitmap();
    inicializar_bloques();
}

void inicializar_bloques()
{
    uint32_t tam_archivo_bloques = BLOCK_COUNT*BLOCK_SIZE;

    FILE* archivo_bloques = fopen(path_bloques, "rb+");

    if (archivo_bloques == NULL)
    {
        FILE* archivo_bitmap = fopen(path_bitmap, "wb+");
        if (archivo_bitmap == NULL)
        {
        	log_error(logger, "No existe y no se pudo crear el archivo bloques.dat.");
            exit(1);
        }
        truncate(path_bloques, tam_archivo_bloques);
        log_trace(logger, "Se crea el archivo bloques.dat");
    }
    else
    {
        truncate(path_bloques, tam_archivo_bloques);
        log_trace(logger, "Ya existe un archivo bloques.dat, se lo trunca al tamaño necesario");
    }

    char* dump_bloques = mem_hexstring(archivo_bloques, tam_archivo_bloques);
    log_info(logger, "Archivo bloques.dat inicializado, hexdump: %s", dump_bloques);
    fclose(archivo_bloques);
    free(dump_bloques);
}

void inicializar_bitmap()
{
    uint32_t tam_bitmap = BLOCK_COUNT/8;
    char* bloque = malloc(tam_bitmap);
    memset(bloque, 0, tam_bitmap);
    bitmap_bloques = bitarray_create_with_mode(bloque, tam_bitmap, MSB_FIRST);
    if (bitmap_bloques == NULL)
    {
        log_error(logger ,"No se pudo crear el bitmap");
        exit(1);
    }
    
    FILE* archivo_bitmap = fopen(path_bitmap, "rb+");
    if (archivo_bitmap == NULL)
    {
        FILE* archivo_bitmap = fopen(path_bitmap, "wb+");
        if (archivo_bitmap == NULL)
        {
        	log_error(logger, "No se pudo abrir el archivo bitmap.dat.");
        	bitarray_destroy(bitmap_bloques);
            exit(1);
        }
        fwrite(bitmap_bloques->bitarray, 1, tam_bitmap, archivo_bitmap);
        log_trace(logger, "Se crea el archivo bitmap.dat");
    }
    else
    {
        fread(bitmap_bloques->bitarray, 1, tam_bitmap, archivo_bitmap);
        log_trace(logger, "Se carga el archivo bitmap.dat existente");
    }

    char* dump_bitmap = mem_hexstring(bitmap_bloques->bitarray, tam_bitmap);
    log_info(logger, "Bitmap inicializado, hexdump: %s", dump_bitmap);
    fclose(archivo_bitmap);
    free(dump_bitmap);
}

void crear_archivo(char* nombre_archivo)
{
    int32_t bloque = buscar_bloque_libre();
    if (bloque != -1)
    {
        crear_metadata(bloque, nombre_archivo);
        log_info(logger, "Se crea metadata del archivo: %s", nombre_archivo);
        list_add(archivos_existentes, nombre_archivo);
    }
    else
    {
        log_error(logger, "No hay espacio disponible para crear el archivo: %s", nombre_archivo);
    }
}

int32_t buscar_bloque_libre()
{
    int32_t cant_bloques = bitarray_get_max_bit(bitmap_bloques);
    int32_t bloque_libre = -1;

    for (int i = 0; i < cant_bloques; i++) {
        if (!bitarray_test_bit(bitmap_bloques, i)) { // Si el bit está en 0, el bloque está libre
            bloque_libre = i;
            bitarray_set_bit(bitmap_bloques, i); // Marcar el bit como ocupado
            break;
        }
    }
    
    return bloque_libre; // No hay bloques libres
}

void crear_metadata(int32_t bloque, char* nombre_archivo)
{
    char* path_archivo_metadata = string_duplicate(path_metadata);
    string_append(&path_archivo_metadata, nombre_archivo);

    t_config* metadata = config_create(path_archivo_metadata);
    char* bloque_inicial = string_itoa(bloque);
    config_set_value(metadata, "BLOQUE_INICIAL", bloque_inicial);
    config_set_value(metadata, "TAMANIO_ARCHIVO", "0");
    free(bloque_inicial);

    config_save_in_file(metadata, path_archivo_metadata);
    config_destroy(metadata);
    free(path_archivo_metadata);
}

void eliminar_archivo(char* nombre_archivo)
{
    int32_t indice;
    if (existe_archivo(nombre_archivo, &indice))
    {
        list_remove(archivos_existentes, indice); //Eliminar de la lista de archivos existentes
        char* path_archivo_metadata = string_duplicate(path_metadata);
        string_append(&path_archivo_metadata, nombre_archivo);

        liberar_bloques(path_archivo_metadata);

        remove(path_archivo_metadata); //Eliminar archivo de metadata
        free(path_archivo_metadata);
        log_info(logger, "Se elimino el archivo: %s con exito", nombre_archivo);
    }
    else
    {
        log_warning(logger, "Se trato de eliminar un archivo que no existe: %s", nombre_archivo);
    }
    
}

bool existe_archivo(char* nombre_archivo, int32_t* indice)
{
    char* elem;
    for (int i = 0; i < list_size(archivos_existentes); i++)
    {
        elem = list_get(archivos_existentes, i);
        if (string_equals_ignore_case(elem, nombre_archivo)){
            *indice = i;
            return true;
            }
    }
    return false;
}

void liberar_bloques(char* path_archivo_metadata)
{
    t_config* metadata = config_create(path_archivo_metadata);
    int32_t tamanio_archivo = config_get_int_value(metadata, "TAMANIO_ARCHIVO");
    int32_t bloque_inicial = config_get_int_value(metadata, "BLOQUE_INICIAL");
    int32_t cant_bloques = cantidad_de_bloques(tamanio_archivo);

    for (int32_t i = 0; i < cant_bloques; i++)
    {
        bitarray_clean_bit(bitmap_bloques, bloque_inicial+i);
    }
    config_destroy(metadata);
}

void truncar_archivo(char* nombre_archivo, uint32_t nuevo_tamanio)
{
    int32_t indice;
    if (!existe_archivo(nombre_archivo, &indice))
    {
        log_error(logger, "Se trato de truncar un archivo que no existe: %s", nombre_archivo);
    }
    else
    {
        char* path_archivo_metadata = string_duplicate(path_metadata);
        string_append(&path_archivo_metadata, nombre_archivo);
        t_config* metadata = config_create(path_archivo_metadata);
        int32_t bloque_inicial = config_get_int_value(metadata, "BLOQUE_INICIAL");
        int32_t tamanio_archivo = config_get_int_value(metadata, "TAMANIO_ARCHIVO");

        int32_t cant_bloques = cantidad_de_bloques(tamanio_archivo);
        int32_t nueva_cant_bloques = cantidad_de_bloques(nuevo_tamanio);
        int32_t diferencia_cant_bloques = cant_bloques - nueva_cant_bloques;
        config_set_value(metadata, "TAMANIO_ARCHIVO", string_itoa(nuevo_tamanio));
        if (diferencia_cant_bloques<=0)
        {
            liberar_n_bloques(bloque_inicial+nueva_cant_bloques, 0-diferencia_cant_bloques);
        }
        else
        {
        bool asignacion = asignar_n_bloques(bloque_inicial+cant_bloques, diferencia_cant_bloques);//Si hay bloques contiguos disponibles

        if (!asignacion)//Si no, intenta reasignarle bloques al final del bitmap
        {
            asignacion = reasignar_bloques(metadata, cant_bloques, nueva_cant_bloques);

            if (!asignacion)//Si no puede, hace compactacion y lo intenta de nuevo
            {
                // compactacion debe dejar el archivo actual al final del bitmap
                // tambien debe asignar el nuevo bloque inicial a la metadata o devolverlo como int32
                // compactacion();
                bloque_inicial = config_get_int_value(metadata, "BLOQUE_INICIAL");
                asignacion = asignar_n_bloques(bloque_inicial+cant_bloques, diferencia_cant_bloques);
            }               
        }            
        if (!asignacion)
        {
            log_warning(logger, "No se pudo truncar el archivo: %s por falta de espacio", nombre_archivo);
        }
        else
        {
            log_info(logger, "Se trunco con exito el archivo: %s, nuevo tamaño: %u", nombre_archivo, nuevo_tamanio);
        }
        }
        
        config_save(metadata);
    }
}

int32_t cantidad_de_bloques(int32_t tamanio_archivo)
{
    return floor(tamanio_archivo / BLOCK_SIZE) + (tamanio_archivo%BLOCK_SIZE > 0); //Si la cuenta da redonda es +0 si no es +1
}

void liberar_n_bloques(int32_t bloque_inicial, int32_t bloques_a_liberar)
{
    for (int32_t i = 0; i < bloques_a_liberar; i++)
    {
        bitarray_clean_bit(bitmap_bloques, bloque_inicial+i);
    }
    log_info(logger_debug, "Se liberaron %d bloques del bitmap", bloques_a_liberar);
}

bool asignar_n_bloques(int32_t bloque_inicial, int32_t bloques_a_asignar)
{
    int32_t bloques_disponibles = 0;

    for (int32_t i = 1; i <= bloques_a_asignar; i++)
    {
        if (bitarray_test_bit(bitmap_bloques, bloque_inicial+i)) {break;}

        else {bloques_disponibles++;}
    }
    if (bloques_disponibles<bloques_a_asignar) { return false;}
    else
    {
        for (int32_t i = 1; i <= bloques_a_asignar; i++)
        {
            bitarray_set_bit(bitmap_bloques, bloque_inicial+i);
        }
        return true;
    }
}

bool reasignar_bloques(t_config* metadata, int32_t cant_bloques, int32_t nueva_cant_bloques)
{
    int32_t bloques_disponibles = 0;
    int32_t nuevo_inicio = 0;

    for (int32_t i = 0; i < bitarray_get_max_bit(bitmap_bloques); i++)
    {
        if (!bitarray_test_bit(bitmap_bloques, i))
        {
            bloques_disponibles++;
            if (bloques_disponibles>=nueva_cant_bloques)
                {break;}
        }
        else {
            bloques_disponibles=0;
            nuevo_inicio=i+1;
        }
    }
    if (bloques_disponibles < nueva_cant_bloques)
    {
        log_info(logger_debug, "No hay suficiente espacio para asignar al archivo");
        return false;
    }
    else
    {
        int32_t bloque_inicial = config_get_int_value(metadata, "BLOQUE_INICIAL");
        for (int32_t i = 0; i < cant_bloques; i++)
        {
            bitarray_clean_bit(bitmap_bloques, bloque_inicial+i);
        }
        config_set_value(metadata, "BLOQUE_INICIAL", string_itoa(nuevo_inicio));
        config_save(metadata);

        for (int32_t i = 0; i < nueva_cant_bloques; i++)
        {
            bitarray_set_bit(bitmap_bloques, bloque_inicial+i);
        }
        log_info(logger_debug, "Se reasigna el archivo a la nueva posicion: %d", nuevo_inicio);
        return true;
    }
}
