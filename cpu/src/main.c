#include <stdlib.h>
#include <stdio.h>
#include <utils/utils.h>

int main(int argc, char *argv[])
{
    t_log *logger;
    t_config *config;

    logger = iniciar_logger("cpu");
    log_info(logger, "Hola soy cpu");
    
    return 0;
}
