/******************************************************************************
 * @file            obj.c
 *****************************************************************************/
#include    <stdio.h>
#include    <stdlib.h>
#include    <string.h>

#include    "as.h"
#include    "fixup.h"
#include    "frag.h"
#include    "obj.h"
#include    "report.h"
#include    "section.h"
#include    "symbol.h"

static void write_to_byte_array (unsigned char *arr, unsigned long value, int size) {

    int i;
    
    for (i = 0; i < size; i++) {
        arr[i] = (value >> (8 * i)) & 0xff;
    }

}

static int output_relocation (struct fixup *fixup, unsigned long start_address_of_section, FILE *fp) {

    struct relocation_info reloc;
    
    long log2_of_size, size;
    unsigned long r_symbolnum;
    
    write_to_byte_array (reloc.r_address, fixup->frag->address + fixup->where - start_address_of_section, 4);
    
    if (symbol_is_section_symbol (fixup->add_symbol)) {
    
        if (symbol_get_section (fixup->add_symbol) == text_section) {
            r_symbolnum = N_TEXT;
        } else if (symbol_get_section (fixup->add_symbol) == data_section) {
            r_symbolnum = N_DATA;
        } else if (symbol_get_section (fixup->add_symbol) == bss_section) {
            r_symbolnum = N_BSS;
        } else {
        
            report_at (__FILE__, __LINE__, REPORT_INTERNAL_ERROR, "invalid section %s", section_get_name (symbol_get_section (fixup->add_symbol)));
            exit (EXIT_FAILURE);
        
        }
    
    } else {
    
        struct symbol *symbol;
        long symbol_number;
        
        for (symbol = symbols, symbol_number = 0; symbol && (symbol != fixup->add_symbol); symbol = symbol->next) {
        
            if (symbol_is_external (symbol) || symbol_is_undefined (symbol)) {
                symbol_number++;
            }
        
        }
        
        r_symbolnum  = symbol_number;
        r_symbolnum |= (1LU << 31);
    
    }
    
    if (fixup->pcrel) {
        r_symbolnum |= (1LU << 28);
    }
    
    for (log2_of_size = -1, size = fixup->size; size; size >>= 1, log2_of_size++);
    r_symbolnum |= ((unsigned long) log2_of_size << 29);
    
    if (fixup->reloc_type == RELOC_TYPE_FAR_CALL) {
        r_symbolnum |= (1LU << 27);
    }
    
    write_to_byte_array (reloc.r_symbolnum, r_symbolnum, 4);
    
    if (fwrite (&reloc, sizeof (reloc), 1, fp) != 1) {
    
        report_at (program_name, 0, REPORT_ERROR, "error writing text relocations");
        return 1;
    
    }
    
    return 0;

}

void output_obj (FILE *fp) {

    unsigned long start_address_of_data;
    struct fixup *fixup;
    
    unsigned long symbol_table_size;
    struct symbol *symbol;
    
    unsigned long string_table_pos;
    struct frag *frag;
    
    unsigned long text_size, data_size, bss_size;
    unsigned long tr_size, dr_size;
    
    struct exec header;
    memset (&header, 0, sizeof (header));
    
    write_to_byte_array (header.a_info, 0x00640000 | MAGIC, 4);
    
    if ((symbol = state->end_symbol)) {
        write_to_byte_array (header.a_entry, symbol_get_value (symbol), 4);
    }
    
    if (fseek (fp, sizeof (header), SEEK_SET)) {
    
        report_at (program_name, 0, REPORT_ERROR, "failed whilst seeking passed header");
        return;
    
    }
    
    section_set (text_section);
    text_size = 0;
    
    for (frag = current_frag_chain->first_frag; frag; frag = frag->next) {
    
        if (frag->fixed_size == 0) {
            continue;
        }
        
        if (fwrite (frag->buf, frag->fixed_size, 1, fp) != 1) {
        
            report_at (program_name, 0, REPORT_ERROR, "failed whilst writing text");
            return;
        
        }
        
        text_size += frag->fixed_size;
    
    }
    
    write_to_byte_array (header.a_text, text_size, 4);
    
    section_set (data_section);
    data_size = 0;
    
    for (frag = current_frag_chain->first_frag; frag; frag = frag->next) {
    
        if (frag->fixed_size == 0) {
            continue;
        }
        
        if (fwrite (frag->buf, frag->fixed_size, 1, fp) != 1) {
        
            report_at (program_name, 0, REPORT_ERROR, "failed whilst writing data");
            return;
        
        }
        
        data_size += frag->fixed_size;
    
    }
    
    write_to_byte_array (header.a_data, data_size, 4);
    
    section_set (bss_section);
    bss_size = 0;
    
    for (frag = current_frag_chain->first_frag; frag; frag = frag->next) {
    
        if (frag->fixed_size == 0) {
            continue;
        }
        
        bss_size += frag->fixed_size;
    
    }
    
    write_to_byte_array (header.a_bss, bss_size, 4);
    
    section_set (text_section);
    tr_size = 0;
    
    start_address_of_data = 0;
    
    for (fixup = current_frag_chain->first_fixup; fixup; fixup = fixup->next) {
    
        if (fixup->done) {
            continue;
        }
        
        if (output_relocation (fixup, start_address_of_data, fp)) {
            return;
        }
        
        tr_size += sizeof (struct relocation_info);
    
    }
    
    write_to_byte_array (header.a_trsize, tr_size, 4);
    
    section_set (data_section);
    dr_size = 0;
    
    start_address_of_data = current_frag_chain->first_frag->address;
    
    for (fixup = current_frag_chain->first_fixup; fixup; fixup = fixup->next) {
    
        if (fixup->done) {
            continue;
        }
        
        if (output_relocation (fixup, start_address_of_data, fp)) {
            return;
        }
        
        dr_size += sizeof (struct relocation_info);
    
    }
    
    write_to_byte_array (header.a_drsize, dr_size, 4);
    
    symbol_table_size = 0;
    string_table_pos = 4;
    
    for (symbol = symbols; symbol; symbol = symbol->next) {
    
        if (symbol_is_external (symbol) || symbol_is_undefined (symbol)) {
        
            struct nlist symbol_entry;
            memset (&symbol_entry, 0, sizeof (symbol_entry));
            
            write_to_byte_array (symbol_entry.n_strx, string_table_pos, 4);
            string_table_pos += strlen (symbol->name) + 1;
            
            if (symbol->section == undefined_section) {
                symbol_entry.n_type = N_UNDF;
            } else if (symbol->section == text_section) {
                symbol_entry.n_type = N_TEXT;
            } else if (symbol->section == data_section) {
                symbol_entry.n_type = N_DATA;
            } else if (symbol->section == bss_section) {
                symbol_entry.n_type = N_BSS;
            } else if (symbol->section == absolute_section) {
                symbol_entry.n_type = N_ABS;
            } else {
            
                report_at (__FILE__, __LINE__, REPORT_INTERNAL_ERROR, "invalid section %s", section_get_name (symbol->section));
                exit (EXIT_FAILURE);
            
            }
            
            write_to_byte_array (symbol_entry.n_value, symbol_get_value (symbol), 4);
            symbol_entry.n_type |= N_EXT;
            
            if (fwrite (&symbol_entry, sizeof (symbol_entry), 1, fp) != 1) {
            
                report_at (program_name, 0, REPORT_ERROR, "error writing symbol table");
                return;
            
            }
            
            symbol_table_size += sizeof (symbol_entry);
        
        }
    
    }
    
    write_to_byte_array (header.a_syms, symbol_table_size, 4);
    
    if (fwrite (&string_table_pos, 4, 1, fp) != 1) {
    
        report_at (program_name, 0, REPORT_ERROR, "failed to write string table");
        return;
    
    }
    
    for (symbol = symbols; symbol; symbol = symbol->next) {
    
        if (symbol_is_external (symbol) || symbol_is_undefined (symbol)) {
        
            if (fwrite (symbol->name, strlen (symbol->name) + 1, 1, fp) != 1) {
            
                report_at (program_name, 0, REPORT_ERROR, "failed to write string table");
                return;
            
            }
        
        }
    
    }
    
    rewind (fp);
    
    if (fwrite (&header, sizeof (header), 1, fp) != 1) {
    
        report_at (program_name, 0, REPORT_ERROR, "failed to write header");
        return;
    
    }

}
