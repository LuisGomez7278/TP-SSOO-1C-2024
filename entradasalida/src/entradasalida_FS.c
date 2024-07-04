#include "../include/entradasalida_FS.h"

void inicializar_FS()
{
    path_metadata = string_duplicate(PATH_BASE_DIALFS);
    string_append(&path_metadata, "/metadata/");

    path_bloques = string_duplicate(PATH_BASE_DIALFS);
    string_append(&path_bloques, "bloques.dat");

    path_bitmap = string_duplicate(PATH_BASE_DIALFS);
    string_append(&path_bitmap, "bitmap.dat");

    crear_bitmap();
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

void crear_bitmap()
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
