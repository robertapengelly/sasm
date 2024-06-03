/******************************************************************************
 * @file            frag.h
 *****************************************************************************/
#ifndef     _FRAG_H
#define     _FRAG_H

#define     RELAX_TYPE_NONE_NEEDED                          0
#define     RELAX_TYPE_ALIGN                                1
#define     RELAX_TYPE_ALIGN_CODE                           2
#define     RELAX_TYPE_ORG                                  3
#define     RELAX_TYPE_SPACE                                4
#define     RELAX_TYPE_MACHINE_DEPENDENT                    5

struct frag {

    unsigned long fixed_size, address, size;
    unsigned int relax_type, relax_subtype;
    
    struct symbol *symbol;
    unsigned char *buf;
    
    signed long offset;
    unsigned long opcode_offset_in_buf;
    
    const char *filename;
    unsigned long line_number;
    
    int relax_marker;
    struct frag *next;

};

extern struct frag zero_address_frag;
extern struct frag *current_frag;

#define     FRAG_BUF_REALLOC_STEP       16

struct frag *frag_alloc (void);

int frags_is_greater_than_offset (unsigned long offset2, struct frag *frag2, unsigned long offset1, struct frag *frag1, signed long *offset_p);
int frags_offset_is_fixed (struct frag *frag1, struct frag *frag2, signed long *offset_p);

unsigned char *finished_frag_increase_fixed_size_by_frag_offset (struct frag *frag);
unsigned char *frag_alloc_space (unsigned long space);
unsigned char *frag_increase_fixed_size (unsigned long increase);

void frag_align (signed long alignment, int fill_char, signed long max_bytes_to_skip);
void frag_align_code (signed long alignment, signed long max_bytes_to_skip);

void frag_append_1_char (unsigned char ch);
void frag_new (void);
void frag_set_as_variant (int relax_type, int relax_subtype, struct symbol *symbol, signed long offset, unsigned long opcode_offset_in_buf);

#endif      /* _FRAG_H */
