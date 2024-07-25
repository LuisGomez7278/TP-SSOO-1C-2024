#include "../include/entradasalida_FS.h"

void inicializar_FS()
{
    path_metadata = string_duplicate(PATH_BASE_DIALFS);
    string_append(&path_metadata, "/metadata/");

    path_bloques = string_duplicate(PATH_BASE_DIALFS);
    string_append(&path_bloques, "bloques.dat");

    path_bitmap = string_duplicate(PATH_BASE_DIALFS);
    string_append(&path_bitmap, "bitmap.dat");

    // log_debug(logger_debug, "Paths - bloques: %s, bitmap:%s", path_bloques, path_bitmap);

    inicializar_bitmap();
    inicializar_bloques();
}

void inicializar_bloques()
{
    uint32_t tam_archivo_bloques = BLOCK_COUNT*BLOCK_SIZE;

    int fd = open(path_bloques, O_RDWR | O_CREAT, 0777);

    if (fd == -1)
    {
        // perror("No se pudo abrir el archivo bloques.dat");
        log_error(logger, "No se pudo abrir el archivo bloques.dat");
        printf("path bloques: %s\n", path_bloques);
    }
    else
    {
        ftruncate(fd, tam_archivo_bloques);
        bloques = mmap(NULL, tam_archivo_bloques, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
        close(fd);
    }
    if (bloques == MAP_FAILED)
    {
        log_error(logger ,"No se pudo mapear el archivo de bloques");
        exit(1);
    }
    // else
    // {
    //     char* dump_bloques = mem_hexstring(bloques, tam_archivo_bloques);
    //     log_info(logger, "Archivo bloques.dat inicializado, hexdump: %s", dump_bloques);
    //     free(dump_bloques);
    // }
}

void inicializar_bitmap()
{
    uint32_t tam_bitmap = BLOCK_COUNT;
    
    int fd = open(path_bitmap, O_RDWR | O_CREAT, 0777);
    if (fd == -1)
    {
        // perror("No se pudo abrir el archivo bitmap.dat");
        log_error(logger, "No se pudo abrir el archivo bitmap.dat");
        printf("path bitmap: %s\n", path_bitmap);
        exit(1);
    }
    else
    {
        ftruncate(fd, tam_bitmap);
        array_bitmap = mmap(NULL, tam_bitmap, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
        close(fd);
    }
    
    bitmap_bloques = bitarray_create_with_mode(array_bitmap, tam_bitmap, LSB_FIRST);
    if (bitmap_bloques == NULL)
    {
        log_error(logger ,"No se pudo mapear el bitarray");
        exit(1);
    }
    // else
    // {
    //     char* dump_bitmap = mem_hexstring(bitmap_bloques->bitarray, tam_bitmap);
    //     log_info(logger, "Bitmap inicializado, hexdump: %s", dump_bitmap);
    //     free(dump_bitmap);
    // }
}

bool crear_archivo(char* nombre_archivo)
{
    if (existe_archivo(nombre_archivo))
    {
        log_info(logger, "Se intento crear un archivo que ya existe");
        return true;
    }
    
    int32_t bloque = buscar_bloque_libre();
    if (bloque != -1)
    {
        crear_metadata(bloque, nombre_archivo);
        log_info(logger, "Se crea metadata del archivo: %s, bloque inicial: %d", nombre_archivo, bloque);
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
    int32_t bloque_libre = -1;

    for (int32_t i = 0; i < BLOCK_COUNT; i++) {
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
    int fd = open(path_archivo_metadata, O_RDWR | O_CREAT, 0777);
    if (fd == -1)
    {
        perror("No se pudo crear metadata");
        exit(1);
    }

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
    if (existe_archivo(nombre_archivo))
    {
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

bool existe_archivo(char* nombre_archivo)
{
    char* path_archivo_metadata = string_duplicate(path_metadata);
    string_append(&path_archivo_metadata, nombre_archivo);
    int fd = open(path_archivo_metadata, O_RDONLY);
    if (fd == -1)
    {
        return false;
    }
    else
    {
        close(fd);
        return true;
    }
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
    if (!existe_archivo(nombre_archivo))
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
        int32_t diferencia_cant_bloques = nueva_cant_bloques - cant_bloques;
        log_debug(logger_debug, "cant bloques: %d, nueva cant bloques: %d, diferencia: %d", cant_bloques, nueva_cant_bloques, diferencia_cant_bloques);
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
        config_destroy(metadata);
        free(path_archivo_metadata);
        return true;
    }
}

int32_t cantidad_de_bloques(int32_t tamanio_archivo)
{
    int32_t a = tamanio_archivo/BLOCK_SIZE;
    int32_t b = tamanio_archivo%BLOCK_SIZE > 0 ? 1 : 0;//Si la cuenta da redonda es +0 si no es +1
    return a + b;
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

    for (int32_t i = 1; i <= bloques_a_asignar; ++i)
    {
        if (bitarray_test_bit(bitmap_bloques, bloque_inicial+i)) {break;}

        else {++bloques_disponibles;}
    }
    if (bloques_disponibles<bloques_a_asignar) { return false;}
    else
    {
        for (int32_t i = 1; i <= bloques_a_asignar; ++i)
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

    for (int32_t i = 0; i < BLOCK_COUNT; ++i)
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
        for (int32_t i = 0; i < cant_bloques; ++i)
        {
            bitarray_clean_bit(bitmap_bloques, bloque_inicial+i);
        }
        config_set_value(metadata, "BLOQUE_INICIAL", string_itoa(nuevo_inicio));
        config_save(metadata);

        for (int32_t i = 0; i < nueva_cant_bloques; ++i)
        {
            bitarray_set_bit(bitmap_bloques, nuevo_inicio+i);
        }
        log_info(logger_debug, "Se reasigna el archivo a la nueva posicion: %d, con: %d bloques", nuevo_inicio, nueva_cant_bloques);
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

    void* nuevos_bloques = malloc(BLOCK_COUNT*BLOCK_SIZE);

    struct dirent *de;// Puntero a la entrada (archivo) del directorio

    DIR *dr = opendir(path_metadata);// Puntero al directorio
    char* archivo_actual;
    if (dr == NULL)
    {
        log_error(logger_debug, "No se pudo abrir el directorio" );
        return;
    }

    while ((de = readdir(dr)) != NULL){
        archivo_actual = de->d_name;
        if (string_contains(archivo_actual, ".txt") && !string_equals_ignore_case(archivo_actual, nombre_archivo))
        {
            compactar_archivo(archivo_actual, nuevos_bloques);
        }
    }
    closedir(dr);

    compactar_archivo(nombre_archivo, nuevos_bloques);// El archivo que se quiere truncar se ubica al final de los bloques
    
    memcpy(bloques, nuevos_bloques, BLOCK_COUNT*BLOCK_SIZE);
    log_debug(logger_debug, "Esperando retraso compactacion");
    usleep(RETRASO_COMPACTACION*1000);
    log_info(logger, "PID: %u - Fin Compactación.", PID);
}

void limpiar_bitmap()
{
    for (int32_t i = 0; i < BLOCK_COUNT; i++)
    {
        bitarray_clean_bit(bitmap_bloques, i);
    }    
}

void compactar_archivo(char* nombre_archivo, void* nuevos_bloques)
{
    log_trace(logger_debug, "Proximo archivo a ser compactado: %s", nombre_archivo);
    char* path_archivo_metadata = string_duplicate(path_metadata);
    string_append(&path_archivo_metadata, nombre_archivo);
    t_config* metadata = config_create(path_archivo_metadata);
    
    int32_t tamanio_archivo = config_get_int_value(metadata, "TAMANIO_ARCHIVO");
    int32_t bloque_inicial = config_get_int_value(metadata, "BLOQUE_INICIAL");
    int32_t cant_bloques = cantidad_de_bloques(tamanio_archivo);
    int32_t nuevo_inicio = buscar_bloque_libre();

    // Se copian los datos en un contenedor, luego se los graba en un nuevo puntero de bloques (el cambio se hace en la funcion compactar)
    void* contenido = malloc(cant_bloques*BLOCK_SIZE);    
    FS_READ(bloques, bloque_inicial, 0, cant_bloques*BLOCK_SIZE, contenido);
    FS_WRITE(nuevos_bloques, nuevo_inicio, 0, cant_bloques*BLOCK_SIZE, contenido);

    asignar_n_bloques(nuevo_inicio, cant_bloques);
    config_set_value(metadata, "BLOQUE_INICIAL", string_itoa(nuevo_inicio));
    config_save(metadata);
    config_destroy(metadata);
    free(path_archivo_metadata);
    free(contenido);
}
