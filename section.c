/******************************************************************************
 * @file            section.c
 *****************************************************************************/
#include    <string.h>

#include    "frag.h"
#include    "lib.h"
#include    "section.h"
#include    "symbol.h"

struct section {

    const char *name;
    
    struct frag_chain frag_chain;
    struct symbol *symbol;
    
    int alignment_power;
    struct section *next;

};

static struct section internal_sections[4];
static struct symbol section_symbols[4];

struct section *undefined_section;
struct section *absolute_section;
struct section *expr_section;
struct section *reg_section;

struct section *text_section;
struct section *data_section;
struct section *bss_section;

struct section *current_section;

struct frag_chain *current_frag_chain = 0;
struct section *sections = 0;

static struct section *find_or_make_section_by_name (const char *name) {

    struct section *section, **p_next;
    
    for (p_next = &sections, section = sections; section; p_next = &(section->next), section = *p_next) {
        
        if (strcmp (name, section->name) == 0) {
            break;
        }
    
    }
    
    if (!section) {
    
        section = xmalloc (sizeof (*section));
        section->name = xstrdup (name);
        
        section->symbol = symbol_create (name, section, 0, &zero_address_frag);
        section->symbol->flags |= SYMBOL_FLAG_SECTION_SYMBOL;
        
        symbol_add_to_chain (section->symbol);
        *p_next = section;
    
    }
    
    return section;

}

struct section *section_get_next_section (struct section *section) {
    return section->next;
}

struct section *section_set (struct section *section) {

    current_section = section;
    
    current_frag_chain = &current_section->frag_chain;
    current_frag       = current_frag_chain->last_frag;
    
    return section;

}

struct section *section_set_by_name (const char *name) {
    return section_set (find_or_make_section_by_name (name));
}

struct symbol *section_symbol (struct section *section) {
    return section->symbol;
}

const char *section_get_name (struct section *section) {
    return section->name;
}

void section_record_alignment_power (struct section *section, int alignment_power) {

    if (alignment_power > section->alignment_power) {
        section->alignment_power = alignment_power;
    }

}


#define CREATE_INTERNAL_SECTION(section_var, section_name, section_index) \
    (section_var) = &internal_sections[(section_index)];        \
    (section_var)->name = (section_name);                       \
    (section_var)->symbol = &section_symbols[(section_index)];  \
    (section_var)->symbol->name    = (section_name);            \
    (section_var)->symbol->section = (section_var);             \
    (section_var)->symbol->frag    = &zero_address_frag;        \
    symbol_set_value ((section_var)->symbol, 0);                \
    (section_var)->symbol->flags  |= SYMBOL_FLAG_SECTION_SYMBOL

void sections_init (void) {

    CREATE_INTERNAL_SECTION (undefined_section, "*UND*",    0);
    CREATE_INTERNAL_SECTION (absolute_section,  "*ABS*",    1);
    CREATE_INTERNAL_SECTION (expr_section,      "*EXPR*",   2);
    CREATE_INTERNAL_SECTION (reg_section,       "*REG*",    3);
    
    text_section = section_set_by_name (".text");
    text_section->frag_chain.last_frag  = text_section->frag_chain.first_frag  = frag_alloc ();
    text_section->frag_chain.last_fixup = text_section->frag_chain.first_fixup = 0;
    
    data_section = section_set_by_name (".data");
    data_section->frag_chain.last_frag  = data_section->frag_chain.first_frag  = frag_alloc ();
    data_section->frag_chain.last_fixup = data_section->frag_chain.first_fixup = 0;
    
    bss_section = section_set_by_name (".bss");
    bss_section->frag_chain.last_frag  = bss_section->frag_chain.first_frag  = frag_alloc ();
    bss_section->frag_chain.last_fixup = bss_section->frag_chain.first_fixup = 0;
    
    /* .text section is the default section. */
    section_set (text_section);

}

#undef CREATE_INTERNAL_SECTION
