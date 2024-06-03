/******************************************************************************
 * @file            section.h
 *****************************************************************************/
#ifndef     _SECTION_H
#define     _SECTION_H

struct frag_chain {

    struct fixup *first_fixup, *last_fixup;
    struct frag *first_frag, *last_frag;
    
    struct frag_chain *next;

};

#define     SECTION_IS_NORMAL(section)  \
    ((section != undefined_section) && (section != absolute_section) && (section != expr_section) && (section != reg_section))

extern struct section *undefined_section;
extern struct section *absolute_section;
extern struct section *expr_section;
extern struct section *reg_section;

extern struct section *text_section;
extern struct section *data_section;
extern struct section *bss_section;

extern struct section *current_section;

extern struct frag_chain *current_frag_chain;
extern struct section *sections;

struct section *section_get_next_section (struct section *section);
struct section *section_set (struct section *section);
struct section *section_set_by_name (const char *name);

struct symbol *section_symbol (struct section *section);
const char *section_get_name (struct section *section);

void section_record_alignment_power (struct section *section, int alignment_power);

#endif      /* _SECTION_H */
