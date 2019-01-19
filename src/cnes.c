// cnes.c
// Author: hyan23
// Date: 2017.08.20

#include <stdio.h>
#include "nes/nes.h"

int main(int argc, char* argv[])
{
    if (argc != 2) {
        printf("Usage: %s <filename>\n", argv[0]);
        printf("-e -- edit emulator config(usr vi)\n");
        printf("-E -- edit emulator config(use GUI text editor)\n");
        return 0;
    }
    
    if (STREQU(argv[1], "-e")) {
        cnes_edit_ini(FALSE);
        return 0;
    }
    if (STREQU(argv[1], "-E")) {
        cnes_edit_ini(TRUE);
        return 0;
    }
    
    cnes_conf_s conf;
    cnes_conf_init(&conf);
    strcpy(conf.emulator.file, argv[1]);
    if (FAILED(cnes_read_ini(&conf))) {
        cnes_touch_ini();
    }
    
    cnes_nes_s nes;
    error_t result = cnes_nes_init(&nes, &conf);
    if (FAILED(result)) {
        printf("Init Nes Error%04x\n", result);
        return -1;
    }
    cnes_nes_run(&nes);
    cnes_nes_close(&nes);
    return 0;
}