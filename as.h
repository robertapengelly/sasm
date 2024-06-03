/******************************************************************************
 * @file            as.h
 *****************************************************************************/
#ifndef     _AS_H
#define     _AS_H

#include    <stdio.h>

#include    "list.h"
#include    "symbol.h"
#include    "vector.h"

struct proc {

    struct vector regs, args;
    char *name;
    
    char *filename;
    unsigned long line_number;

};

#define     AS_OUTPUT_OBJ               0x00
#define     AS_OUTPUT_BIN               0x01

struct as_state {

    const char *ifile, *ofile, *lfile;
    FILE *ofp;
    
    struct list *pplist;
    int model, data_size;
    
    struct vector procs;
    char *ext;
    
    struct symbol *end_symbol;
    int format;

};

extern struct as_state *state;
extern const char *program_name;

#endif      /* _AS_H */
