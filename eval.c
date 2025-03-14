/******************************************************************************
 * @file            eval.c
 *****************************************************************************/
#include    <assert.h>
#include    <ctype.h>
#include    <stdio.h>
#include    <stdlib.h>
#include    <string.h>

#include    "as.h"
#include    "eval.h"
#include    "lex.h"
#include    "lib.h"
#include    "macro.h"
#include    "report.h"

static unsigned int eval_expr (unsigned int lhs, char *start, char **pp, int outer_prec);

static unsigned int eval_unary (unsigned int lhs, char *start, char **pp) {

    *pp = skip_whitespace (*pp);
    
    if (isdigit ((int) **pp)) {
    
        unsigned int temp, temp2;
        int ch;
        
        if ((*pp)[0] == '0' && tolower ((int) (*pp)[1]) == 'x') {
        
            unsigned int base = 16;
            *pp += 2;
            
            while (isxdigit ((int) **pp)) {
            
                temp = lhs * base;
                ch = *((*pp)++);
                
                if (ch >= '0' && ch <= '9') {
                    temp2 = ch - '0';
                } else {
                    temp2 = (ch & 0xdf) - ('A' - 10);
                }
                
                lhs = temp + temp2;
            
            }
        
        } else if ((*pp)[0] == '0') {
        
            unsigned int base = 8;
            
            while (isdigit ((int) **pp)) {
            
                temp = lhs * base;
                lhs = (temp + (*((*pp)++) - '0'));
            
            }
        
        } else {
        
            unsigned int base = 10;
            
            while (isdigit ((int) **pp)) {
            
                temp = lhs * base;
                lhs = (temp + (*((*pp)++) - '0'));
            
            }
        
        }
        
        return lhs;
    
    }
    
    if (is_name_beginner ((int) **pp)) {
   
        char *sname, *caret;
        
        struct hashtab_name *key;
        struct macro *m;
        
        caret = *pp;
        
        while (is_name_part ((int) *caret)) {
            caret++;
        }
        
        if (memcmp (*pp, "defined", caret - *pp) == 0) {
        
            caret = skip_whitespace (caret);
            *pp = caret;
            
            if (*caret == '(') {
            
                caret = skip_whitespace (caret + 1);
                *pp = caret;
                
                while (!is_end_of_line[(int) *caret]) {
                
                    if (isspace ((int) *caret) || *caret == ')') { break; }
                    caret++;
                
                }
                
                sname = xstrndup (*pp, caret - *pp);
                caret = skip_whitespace (caret);
                
                if (*caret != ')') {
                    report_line_at (get_filename (), get_line_number (), REPORT_ERROR, start, caret, "missing ')' after \"defined\"");
                }
            
            } else {
                sname = xstrndup (*pp, caret - *pp);
            }
            
            if (*caret == ')') { caret++; }
            *pp = skip_whitespace (caret);
            
            (void) sname;
            (void) key;
            
            lhs = (find_macro (sname) != NULL);
            return lhs;
        
        }
        
        sname = xstrndup (*pp, caret - *pp);
        *pp = skip_whitespace (caret);
        
        if ((key = find_macro (sname))) {
        
            if ((m = get_macro (key))) {
            
                char *temp = process_macro (start, pp, m);
                lhs = eval_unary (lhs, temp, &temp);
            
            }
        
        }
        
        return lhs;
    
    }
    
    if (**pp == '@') {
    
        char *arg, *temp = (*pp + 1);
        
        if ((arg = symname (&temp))) {
        
            if (xstrcasecmp (arg, "DataSize") == 0) {
            
                *pp = temp;
                free (arg);
                
                return state->data_size;
            
            }
            
            if (xstrcasecmp (arg, "Model") == 0) {
            
                *pp = temp;
                free (arg);
                
                return (state->model > 0 ? state->model : 1);
            
            }
            
            free (arg);
        
        }
    
    }
    
    if (**pp == '!') {
    
        unsigned int temp = 0;
        
        *pp = skip_whitespace (*pp + 1);
        temp = eval_unary (temp, start, pp);
        
        lhs = (temp == 0);
        return lhs;
    
    }
    
    if (**pp == '~') {
    
        int flip_bits = 0;
        
        while (**pp == '~') {
        
            flip_bits = !flip_bits;
            *pp = skip_whitespace (*pp + 1);
        
        }
        
        lhs = eval_unary (lhs, start, pp);
        
        if (flip_bits) {
            lhs = ~lhs;
        }
        
        return lhs;
    
    }
    
    if (**pp == '-') {
    
        int sign = 1;
        
        while (**pp == '-' || **pp == '+') {
        
            if (**pp == '-') { sign = !sign; }
            *pp = skip_whitespace (*pp + 1);
        
        }
        
        lhs = eval_unary (lhs, start, pp);
        
        if (!sign) {
            lhs = -lhs;
        }
        
        return lhs;
    
    }
    
    if (**pp == '(') {
    
        char *caret = (*pp)++;
        int depth = 0;
        
        while (!is_end_of_line[(int) **pp]) {
        
            if (**pp == '(') {
            
                (*pp)++;
                
                depth++;
                continue;
            
            }
            
            if (**pp == ')') {
            
                if (depth > 0) {
                
                    (*pp)++;
                    
                    depth--;
                    continue;
                
                }
                
                break;
            
            }
            
            (*pp)++;
        
        }
        
        if (**pp != ')') {
            report_line_at (get_filename (), get_line_number (), REPORT_WARNING, start, caret, "missing ')' in expression");
        } else {
            (*pp)++;
        }
        
        memmove (caret, caret + 1, (*pp - caret) - 1);
        *(caret + ((*pp - caret) - 1)) = '\0';
        
        lhs = eval_unary (lhs, start, &caret);
        lhs = eval_expr (lhs, start, &caret, 15);
        
        return lhs;
    
    }
    
    if (!is_end_of_line[(int) **pp]) {
    
        report_line_at (get_filename (), get_line_number (), REPORT_INTERNAL_ERROR, start, *pp, "unexpected %c character", **pp);
        
        while (!is_end_of_line[(int) **pp]) {
            (*pp)++;
        }
    
    }
    
    return lhs;

}

#define     OP_MUL                      0
#define     OP_DIV                      1
#define     OP_MOD                      2
#define     OP_PLUS                     3
#define     OP_MINUS                    4
#define     OP_LT                       5
#define     OP_GT                       6
#define     OP_LTEQ                     7
#define     OP_GTEQ                     8
#define     OP_EQEQ                     9
#define     OP_NOTEQ                    11
#define     OP_AND                      12
#define     OP_XOR                      13
#define     OP_OR                       14
#define     OP_ANDAND                   15
#define     OP_OROR                     16
#define     OP_LSHIFT                   17
#define     OP_RSHIFT                   18
#define     OP_QUEST                    19
#define     OP_MAX                      20

struct op {

    char *word;
    int kind;

};

static struct op *get_op (char **pp) {

    static struct op kws[] = {
    
        {   "<=",   OP_LTEQ     },
        {   ">=",   OP_GTEQ     },
        {   "==",   OP_EQEQ     },
        {   "!=",   OP_NOTEQ    },
        {   "&&",   OP_ANDAND   },
        {   "||",   OP_OROR     },
        {   "<<",   OP_LSHIFT   },
        {   ">>",   OP_RSHIFT   },
        
        {   "*",    OP_MUL      },
        {   "/",    OP_DIV      },
        {   "%",    OP_MOD      },
        {   "+",    OP_PLUS     },
        {   "-",    OP_MINUS    },
        {   "<",    OP_LT       },
        {   ">",    OP_GT       },
        {   "&",    OP_AND      },
        {   "^",    OP_XOR      },
        {   "|",    OP_OR       },
        {   "?",    OP_QUEST    }
    
    };
    
    struct op *kw;
    unsigned int i;
    
    for (i = 0; i < (sizeof (kws) / sizeof (*kws)); i++) {
    
        kw = &kws[i];
        
        if (strncmp (*pp, kw->word, strlen (kw->word)) == 0) {
        
            *pp += strlen (kw->word);
            return kw;
        
        }
    
    }
    
    return 0;

}

static int get_prec (int kind) {

    switch (kind) {
    
        case OP_MUL:    case OP_DIV:    case OP_MOD:
        
            return 3;
        
        case OP_PLUS:   case OP_MINUS:
        
            return 4;
        
        case OP_LSHIFT:                 case OP_RSHIFT:
        
            return 5;
        
        case OP_LT:     case OP_GT:     case OP_LTEQ:       case OP_GTEQ:
        
            return 6;
        
        case OP_EQEQ:   case OP_NOTEQ:
        
            return 7;
        
        case OP_AND:
        
            return 8;
        
        case OP_XOR:
        
            return 9;
        
        case OP_OR:
        
            return 10;
        
        case OP_ANDAND:
        
            return 11;
        
        case OP_OROR:
        
            return 12;
        
        case OP_QUEST:
        
            return 13;
        
        default:
        
            break;
    
    } 
    
    return 100;

}

static unsigned int eval_expr (unsigned int lhs, char *start, char **pp, int outer_prec) {

    struct op *op1, *op2;
    unsigned int rhs;
    
    int prec, look_ahead;
    
    for (;;) {
    
        *pp = skip_whitespace (*pp);
        
        if (is_end_of_line[(int) **pp]) {
            break;
        }
        
        op1 = get_op (pp);
        
        if (!op1 || (prec = get_prec (op1->kind)) > outer_prec) {
        
            if (op1) { *pp -= strlen (op1->word); }
            break;
        
        }
        
        *pp = skip_whitespace (*pp);
        
        if (op1->kind == OP_QUEST) {
        
            unsigned int left = 0, right = 0;
            
            left = eval_unary (left, start, pp);
            left = eval_expr (left, start, pp, 14);
            
            *pp = skip_whitespace (*pp);
            assert (**pp == ':');
            *pp = skip_whitespace (*pp + 1);
            
            right = eval_unary (right, start, pp);
            right = eval_expr (right, start, pp, 14);
            
            if (lhs != 0) {
                lhs = left;
            } else {
                lhs = right;
            }
            
            continue;
        
        }
        
        if (is_end_of_line[(int) **pp]) {
            break;
        }
        
        rhs = 0;
        rhs = eval_unary (rhs, start, pp);
        
        for (;;) {
        
            *pp = skip_whitespace (*pp);
            
            if (is_end_of_line[(int) **pp]) {
                break;
            }
            
            op2 = get_op (pp);
            
            if (!op2 || (look_ahead = get_prec (op2->kind)) > prec) {
            
                if (op2) { *pp -= strlen (op2->word); }
                break;
            
            }
            
            *pp = skip_whitespace (*pp);
            rhs = eval_expr (rhs, start, pp, look_ahead);
        
        }
        
        switch (op1->kind) {
        
            case OP_MUL:
            
                lhs *= rhs;
                break;
            
            case OP_DIV:
            
                lhs /= rhs;
                break;
            
            case OP_MOD:
            
                lhs %= rhs;
                break;
            
            case OP_PLUS:
            
                lhs += rhs;
                break;
            
            case OP_MINUS:
            
                lhs -= rhs;
                break;
            
            case OP_LT:
            
                lhs = (lhs < rhs);
                break;
            
            case OP_GT:
            
                lhs = (lhs > rhs);
                break;
            
            case OP_LTEQ:
            
                lhs = (lhs <= rhs);
                break;
            
            case OP_GTEQ:
            
                lhs = (lhs >= rhs);
                break;
            
            case OP_EQEQ:
            
                lhs = (lhs == rhs);
                break;
            
            case OP_NOTEQ:
            
                lhs = (lhs != rhs);
                break;
            
            case OP_AND:
            
                lhs &= rhs;
                break;
            
            case OP_XOR:
            
                lhs ^= rhs;
                break;
            
            case OP_OR:
            
                lhs |= rhs;
                break;
            
            case OP_ANDAND:
            
                lhs = ((lhs != 0) && (rhs != 0));
                break;
            
            case OP_OROR:
            
                lhs = ((lhs != 0) || (rhs != 0));
                break;
            
            case OP_LSHIFT:
            
                lhs <<= rhs;
                break;
            
            case OP_RSHIFT:
            
                lhs >>= rhs;
                break;
            
            default:
            
                report_at (__FILE__, __LINE__, REPORT_INTERNAL_ERROR, "unimplemented");
                break;
        
        }
    
    }
    
    return lhs;

}

int eval (char *start, char **pp) {

    unsigned int lhs = 0;
    
    lhs = eval_unary (lhs, start, pp);
    lhs = eval_expr (lhs, start, pp, 15);
    
    return (lhs != 0);

}
