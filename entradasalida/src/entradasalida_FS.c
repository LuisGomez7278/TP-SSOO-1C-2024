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

    int fd = open(path_bloques ,O_RDWR);

    if (fd == -1)
    {
        log_error(logger, "No se pudo abrir el archivo bloques.dat");
    }
    else
    {
        ftruncate(fd, tam_archivo_bloques);
        bloques = mmap(NULL, tam_archivo_bloques, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
        close(fd);
    }
    if (bloques == MAP_FAILED)
    {
        log_error(logger ,"No se pudo inicializar el archivo de bloques");
    }
    else
    {
        char* dump_bloques = mem_hexstring(bloques, tam_archivo_bloques);
        log_info(logger, "Archivo bloques.dat inicializado, hexdump: %s", dump_bloques);
        free(dump_bloques);
    }

}

void inicializar_bitmap()
{
    uint32_t tam_bitmap = BLOCK_COUNT/8;
    
    int fd = open(path_bitmap ,O_RDWR);
    if (fd == -1)
    {
        log_error(logger, "No se pudo abrir el archivo bitmap.dat");
    }
    else
    {
        ftruncate(fd, tam_bitmap);
        array_bitmap = mmap(NULL, tam_bitmap, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
        close(fd);
    }
    
    bitmap_bloques = bitarray_create_with_mode(array_bitmap, tam_bitmap, MSB_FIRST);
    if (bitmap_bloques == NULL)
    {
        log_error(logger ,"No se pudo inicializar el bitmap");
    }
    else
    {
        char* dump_bitmap = mem_hexstring(bitmap_bloques->bitarray, tam_bitmap);
        log_info(logger, "Bitmap inicializado, hexdump: %s", dump_bitmap);
        free(dump_bitmap);
    }
}

bool crear_archivo(char* nombre_archivo)
{
    int32_t bloque = buscar_bloque_libre();
    if (bloque != -1)
    {
        crear_metadata(bloque, nombre_archivo);
        log_info(logger, "Se crea metadata del archivo: %s", nombre_archivo);
        list_add(archivos_existentes, nombre_archivo);
        return true;
    }
    else
    {
        log_error(logger, "No hay espacio disponible para crear el archivo: %s", nombre_archivo);
        return false;
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

bool eliminar_archivo(char* nombre_archivo)
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
        return true;
    }
    else
    {
        log_warning(logger, "Se trato de eliminar un archivo que no existe: %s", nombre_archivo);
        return false;
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

bool truncar_archivo(uint32_t PID, char* nombre_archivo, uint32_t nuevo_tamanio)
{
    int32_t indice;
    if (!existe_archivo(nombre_archivo, &indice))
    {
        log_error(logger, "PID: %u, trato de truncar un archivo que no existe: %s", PID, nombre_archivo);
        return false;
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
        if (diferencia_cant_bloques<=0)
        {
            config_set_value(metadata, "TAMANIO_ARCHIVO", string_itoa(nuevo_tamanio));
            liberar_n_bloques(bloque_inicial+nueva_cant_bloques, 0-diferencia_cant_bloques);
            return true;
        }
        else
        {
            bool asignacion = asignar_n_bloques(bloque_inicial+cant_bloques, diferencia_cant_bloques);//Si hay bloques contiguos disponibles, los asigna

            if (!asignacion)
            {
                asignacion = reasignar_bloques(metadata, cant_bloques, nueva_cant_bloques);//Si no, intenta reasignarle bloques al final del bitmap

                if (!asignacion)
                {
                    compactacion(PID, nombre_archivo, nueva_cant_bloques);//Si no puede, hace compactacion y lo intenta de nuevo
                    bloque_inicial = config_get_int_value(metadata, "BLOQUE_INICIAL");
                    asignacion = asignar_n_bloques(bloque_inicial+cant_bloques, diferencia_cant_bloques);
                }               
            }
            if (!asignacion)
            {
                log_warning(logger, "PID: %u - No se pudo truncar el archivo: %s por falta de espacio", PID, nombre_archivo);
                return false;
            }
            else
            {
                config_set_value(metadata, "TAMANIO_ARCHIVO", string_itoa(nuevo_tamanio));
                log_info(logger, "PID: %u - Se trunco con exito el archivo: %s, nuevo tamaño: %u", PID, nombre_archivo, nuevo_tamanio);
            }
        }        
        config_save(metadata);
        free(path_archivo_metadata);
        return true;
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

void FS_WRITE(void* bloques, uint32_t bloque_inicial, uint32_t puntero, uint32_t tamanio_total, char* datos_a_escribir)
{
    uint32_t inicio_escritura = (bloque_inicial*BLOCK_SIZE) + puntero;
    memcpy(bloques+inicio_escritura, datos_a_escribir, tamanio_total);
    log_info(logger, "Escritura exitosa");
}

void FS_READ(void* bloques, uint32_t bloque_inicial, uint32_t puntero, uint32_t tamanio_total, void* datos_leidos)
{
    uint32_t inicio_lectura = (bloque_inicial*BLOCK_SIZE) + puntero;
    memcpy(datos_leidos, bloques+inicio_lectura, tamanio_total);
    log_info(logger, "Lectura exitosa");
}

void compactacion(uint32_t PID, char* nombre_archivo, uint32_t nueva_cant_bloques)
{
    log_info(logger, "PID: %u - Inicio Compactación.", PID);
    limpiar_bitmap();
    char* archivo_actual;
    for (int32_t i = 0; i < list_size(archivos_existentes); i++)
    {
        archivo_actual = list_get(archivos_existentes, i);
        if (!string_equals_ignore_case(archivo_actual, nombre_archivo))
        {
            compactar_archivo(archivo_actual);
        }
    }
    compactar_archivo(nombre_archivo);// El archivo que se quiere truncar se ubica al final de los bloques

    sleep(RETRASO_COMPACTACION);
    log_info(logger, "PID: %u - Fin Compactación.", PID);
}

void limpiar_bitmap()
{
    for (int32_t i = 0; i < bitarray_get_max_bit(bitmap_bloques); i++)
    {
        bitarray_clean_bit(bitmap_bloques, i);
    }    
}

void compactar_archivo(char* nombre_archivo)
{
    char* path_archivo_metadata = string_duplicate(path_metadata);
    string_append(&path_archivo_metadata, nombre_archivo);
    t_config* metadata = config_create(path_archivo_metadata);
    int32_t tamanio_archivo = config_get_int_value(metadata, "TAMANIO_ARCHIVO");
    int32_t cant_bloques = cantidad_de_bloques(tamanio_archivo);
    int32_t nuevo_inicio = buscar_bloque_libre();

    asignar_n_bloques(nuevo_inicio, cant_bloques);
    config_set_value(metadata, "BLOQUE_INICIAL", string_itoa(nuevo_inicio));
    config_save(metadata);
    config_destroy(metadata);
    free(path_archivo_metadata);
}
