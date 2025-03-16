/******************************************************************************
 * @file            intel.h
 *****************************************************************************/
#ifndef     _INTEL_H
#define     _INTEL_H

#define     MAX_OPERANDS                3
#define     MAX_REG_NAME_SIZE           8

struct template {

    const char *name;
    int operands;
    
#define     NO_BSUF                     (1LU << 6)          /* 0x00000040 */
#define     NO_WSUF                     (1LU << 7)          /* 0x00000080 */
#define     NO_SSUF                     (1LU << 8)          /* 0x00000100 */
#define     NO_LSUF                     (1LU << 21)         /* 0x00200000 */
#define     NO_QSUF                     (1LU << 22)         /* 0x00400000 */
#define     NO_INTELSUF                 (1LU << 23)         /* 0x00800000 */
    
    unsigned int base_opcode;
    unsigned int extension_opcode;

#define     NONE                        (~0U)
    
    unsigned long opcode_modifier;
    
#define     W                           (1LU << 0)          /* 0x00000001 */
#define     D                           (1LU << 1)          /* 0x00000002 */
    
#define     MODRM                       (1LU << 2)          /* 0x00000004 */
#define     SHORT_FORM                  (1LU << 3)          /* 0x00000008 */
    
#define     JUMP                        (1LU << 4)          /* 0x00000010 */
#define     CALL                        (1LU << 5)          /* 0x00000020 */
    
#define     IGNORE_SIZE                 (1LU << 9)          /* 0x00000200 */
#define     DEFAULT_SIZE                (1LU << 24)         /* 0x01000000 */
#define     SEGSHORTFORM                (1LU << 18)         /* 0x00040000 */
    
#define     JUMPINTERSEGMENT            (1LU << 11)         /* 0x00000800 */
#define     JUMPBYTE                    (1LU << 12)         /* 0x00001000 */
    
#define     SIZE16                      (1LU << 13)         /* 0x00002000 */
#define     SIZE32                      (1LU << 14)         /* 0x00004000 */
    
#define     IS_PREFIX                   (1LU << 15)         /* 0x00008000 */
#define     IS_STRING                   (1LU << 16)         /* 0x00010000 */
    
#define     REG_DUPLICATION             (1LU << 17)         /* 0x00020000 */
    
    unsigned long operand_types[MAX_OPERANDS];
    
#define     REG8                        0x00000001
#define     REG16                       0x00000002
#define     REG32                       0x00000004
    
#define     REG                         (REG8 | REG16 | REG32)
#define     WORD_REG                    (REG16 | REG32)
    
#define     SEGMENT1                    0x00000008
#define     SEGMENT2                    0x00020000
#define     CONTROL                     0x00000010
#define     DEBUG                       0x00100000
#define     TEST                        0x00200000

#define     IMM8                        0x00000020
#define     IMM8S                       0x00000040
#define     IMM16                       0x00000080
#define     IMM32                       0x00000100
    
#define     IMM                         (IMM8 | IMM8S | IMM16 | IMM32)
#define     ENCODABLEIMM                (IMM8 | IMM16 | IMM32)
    
#define     DISP8                       0x00000200
#define     DISP16                      0x00000400
#define     DISP32                      0x00000800
    
#define     DISP                        (DISP8 | DISP16 | DISP32)
#define     BASE_INDEX                  0x00001000
    
/**
 * INV_MEM is for instruction with modrm where general register
 * encoding is allowed only in modrm.regmem (control register move).
 */
#define     INV_MEM                     0x00040000
#define     ANY_MEM                     (DISP8 | DISP16 | DISP32 | BASE_INDEX | INV_MEM)
    
#define     ACC                         0x00002000
#define     PORT                        0x00004000
#define     SHIFT_COUNT                 0x00008000
#define     JUMP_ABSOLUTE               0x00010000
    
#define     IMPLICIT_REGISTER           (SHIFT_COUNT | ACC)
    
    unsigned int cpu_flags;
    
#define     CPU_8086                    (1U << 0)
#define     CPU_186                     (1U << 1)
#define     CPU_286                     (1U << 2)
#define     CPU_386                     (1U << 3)
#define     CPU_486                     (1U << 4)
#define     CPU_586                     (1U << 5)
#define     CPU_686                     (1U << 6)
    
#define     CPU_8087                    (1U << 7)
#define     CPU_287                     (1U << 8)
#define     CPU_387                     (1U << 9)
#define     CPU_687                     (1U << 10)
    
#define     CPU_CMOV                    (1U << 12)

};

#define     REG_FLAT_NUMBER             (~0U)

struct reg_entry {

    const char *name;
    
    unsigned long type;
    unsigned int number;

};

#endif      /* _INTEL_H */
