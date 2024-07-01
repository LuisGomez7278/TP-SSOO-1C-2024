#include "../include/entradasalida_FS.h"

void inicializar_FS()
{
    path_metadata = string_duplicate(PATH_BASE_DIALFS);
    string_append(&path_metadata, "/metadata/");

    path_bloques = string_duplicate(PATH_BASE_DIALFS);
    string_append(&path_metadata, "bloques.dat");

    path_bloques = string_duplicate(PATH_BASE_DIALFS);
    string_append(&path_metadata, "bitmap.dat");

    archivo_bloques = fopen("");
}
