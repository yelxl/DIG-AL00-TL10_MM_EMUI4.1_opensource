

#ifndef _FPUCW_H
#define _FPUCW_H



/* This header file provides the following facilities:
     fpucw_t                        integral type holding the value of 'fctrl'
     FPU_PC_MASK                    bit mask denoting the precision control
     FPU_PC_DOUBLE                  precision control for 53 bits mantissa
     FPU_PC_EXTENDED                precision control for 64 bits mantissa
     GET_FPUCW ()                   yields the current FPU control word
     SET_FPUCW (word)               sets the FPU control word
     DECL_LONG_DOUBLE_ROUNDING      variable declaration for
                                    BEGIN/END_LONG_DOUBLE_ROUNDING
     BEGIN_LONG_DOUBLE_ROUNDING ()  starts a sequence of instructions with
                                    'long double' safe operation precision
     END_LONG_DOUBLE_ROUNDING ()    ends a sequence of instructions with
                                    'long double' safe operation precision
 */

/* Inline assembler like this works only with GNU C.  */
#if (defined __i386__ || defined __x86_64__) && defined __GNUC__

typedef unsigned short fpucw_t; /* glibc calls this fpu_control_t */

# define FPU_PC_MASK 0x0300
# define FPU_PC_DOUBLE 0x200    /* glibc calls this _FPU_DOUBLE */
# define FPU_PC_EXTENDED 0x300  /* glibc calls this _FPU_EXTENDED */

# define GET_FPUCW() \
  ({ fpucw_t _cw;                                               \
     __asm__ __volatile__ ("fnstcw %0" : "=m" (*&_cw));         \
     _cw;                                                       \
   })
# define SET_FPUCW(word) \
  (void)({ fpucw_t _ncw = (word);                               \
           __asm__ __volatile__ ("fldcw %0" : : "m" (*&_ncw));  \
         })

# define DECL_LONG_DOUBLE_ROUNDING \
  fpucw_t oldcw;
# define BEGIN_LONG_DOUBLE_ROUNDING() \
  (void)(oldcw = GET_FPUCW (),                                  \
         SET_FPUCW ((oldcw & ~FPU_PC_MASK) | FPU_PC_EXTENDED))
# define END_LONG_DOUBLE_ROUNDING() \
  SET_FPUCW (oldcw)

#else

typedef unsigned int fpucw_t;

# define FPU_PC_MASK 0
# define FPU_PC_DOUBLE 0
# define FPU_PC_EXTENDED 0

# define GET_FPUCW() 0
# define SET_FPUCW(word) (void)(word)

# define DECL_LONG_DOUBLE_ROUNDING
# define BEGIN_LONG_DOUBLE_ROUNDING()
# define END_LONG_DOUBLE_ROUNDING()

#endif

#endif /* _FPUCW_H */
