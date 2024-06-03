/******************************************************************************
 * @file            bin.c
 *****************************************************************************/
#include    "limits.h"
#include    "stdio.h"
#include    "stdlib.h"
#include    "string.h"

#include    "as.h"
#include    "fixup.h"
#include    "frag.h"
#include    "lib.h"
#include    "report.h"
#include    "section.h"
#include    "symbol.h"

static unsigned long text_size = 0, data_size = 0, bss_size = 0;
static void *output = 0;

void output_binary (FILE *fp) {

    unsigned long i = 0;
    struct frag *frag;
    
    section_set (text_section);
    
    for (frag = current_frag_chain->first_frag; frag; frag = frag->next) {
    
        if (frag->fixed_size == 0) {
            continue;
        }
        
        text_size += frag->fixed_size;
    
    }
    
    section_set (data_section);
    
    for (frag = current_frag_chain->first_frag; frag; frag = frag->next) {
    
        if (frag->fixed_size == 0) {
            continue;
        }
        
        data_size += frag->fixed_size;
    
    }
    
    section_set (bss_section);
    
    for (frag = current_frag_chain->first_frag; frag; frag = frag->next) {
    
        if (frag->fixed_size == 0) {
            continue;
        }
        
        bss_size += frag->fixed_size;
    
    }
    
    output = xmalloc (text_size + data_size);
    section_set (text_section);
    
    for (frag = current_frag_chain->first_frag; frag; frag = frag->next) {
    
        if (frag->fixed_size == 0) {
            continue;
        }
        
        memcpy ((unsigned char *) output + i, frag->buf, frag->fixed_size);
        i += frag->fixed_size;
    
    }
    
    section_set (data_section);
    
    for (frag = current_frag_chain->first_frag; frag; frag = frag->next) {
    
        if (frag->fixed_size == 0) {
            continue;
        }
        
        memcpy ((unsigned char *) output + i, frag->buf, frag->fixed_size);
        i += frag->fixed_size;
    
    }
    
    if (get_error_count () > 0) {
        exit (EXIT_FAILURE);
    }
    
    if (fwrite (output, text_size + data_size, 1, fp) != 1) {
    
        report_at (program_name, 0, REPORT_ERROR, "failed to write data to '%s'", state->ofile);
        exit (EXIT_FAILURE);
    
    }

}
