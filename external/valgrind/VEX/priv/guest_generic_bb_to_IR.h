
/*--------------------------------------------------------------------*/
/*--- begin                               guest_generic_bb_to_IR.h ---*/
/*--------------------------------------------------------------------*/



#ifndef __VEX_GUEST_GENERIC_BB_TO_IR_H
#define __VEX_GUEST_GENERIC_BB_TO_IR_H

#include "libvex_basictypes.h"
#include "libvex_ir.h"              // IRJumpKind
#include "libvex.h"                 // VexArch

/* This defines stuff needed by the guest insn disassemblers.
   It's a bit circular; is imported by
   - the guest-specific toIR.c files (guest-{x86,amd64,ppc,arm}/toIR.c)
   - the generic disassembly driver (bb_to_IR.c)
   - vex_main.c
*/


/* ---------------------------------------------------------------
   Result of disassembling an instruction
   --------------------------------------------------------------- */

/* The results of disassembling an instruction.  There are three
   possible outcomes.  For Dis_Resteer, the disassembler _must_
   continue at the specified address.  For Dis_StopHere, the
   disassembler _must_ terminate the BB.  For Dis_Continue, we may at
   our option either disassemble the next insn, or terminate the BB;
   but in the latter case we must set the bb's ->next field to point
   to the next instruction.  */

typedef

   struct {

      /* The disassembled insn has this length.  Must always be
         set. */
      UInt len;

      /* What happens next?
         Dis_StopHere:  this insn terminates the BB; we must stop.
         Dis_Continue:  we can optionally continue into the next insn
         Dis_ResteerU:  followed an unconditional branch; continue at 
                        'continueAt'
         Dis_ResteerC:  (speculatively, of course) followed a
                        conditional branch; continue at 'continueAt'
      */
      enum { Dis_StopHere, Dis_Continue, 
             Dis_ResteerU, Dis_ResteerC } whatNext;

      /* For Dis_StopHere, we need to end the block and create a
         transfer to whatever the NIA is.  That will have presumably
         been set by the IR generated for this insn.  So we need to
         know the jump kind to use.  Should Ijk_INVALID in other Dis_
         cases. */
      IRJumpKind jk_StopHere;

      /* For Dis_Resteer, this is the guest address we should continue
         at.  Otherwise ignored (should be zero). */
      Addr   continueAt;

   }

   DisResult;


/* ---------------------------------------------------------------
   The type of a function which disassembles one instruction.
   C's function-type syntax is really astonishing bizarre.
   --------------------------------------------------------------- */



typedef

   DisResult (*DisOneInstrFn) ( 

      /* This is the IRSB to which the resulting IR is to be appended. */
      /*OUT*/ IRSB*        irbb,

      /* Return True iff resteering to the given addr is allowed (for
         branches/calls to destinations that are known at JIT-time) */
      /*IN*/  Bool         (*resteerOkFn) ( /*opaque*/void*, Addr ),

      /* Should we speculatively resteer across conditional branches?
         (Experimental and not enabled by default).  The strategy is
         to assume that backward branches are taken and forward
         branches are not taken. */
      /*IN*/  Bool         resteerCisOk,

      /* Vex-opaque data passed to all caller (valgrind) supplied
         callbacks. */
      /*IN*/  void*        callback_opaque,

      /* Where is the guest code? */
      /*IN*/  const UChar* guest_code,

      /* Where is the actual insn?  Note: it's at &guest_code[delta] */
      /*IN*/  Long         delta,

      /* What is the guest IP of the insn? */
      /*IN*/  Addr         guest_IP,

      /* Info about the guest architecture */
      /*IN*/  VexArch      guest_arch,
      /*IN*/  const VexArchInfo* archinfo,

      /* ABI info for both guest and host */
      /*IN*/  const VexAbiInfo*  abiinfo,

      /* The endianness of the host */
      /*IN*/  VexEndness   host_endness,

      /* Should diagnostics be printed for illegal instructions? */
      /*IN*/  Bool         sigill_diag

   );


/* ---------------------------------------------------------------
   Top-level BB to IR conversion fn.
   --------------------------------------------------------------- */

/* See detailed comment in guest_generic_bb_to_IR.c. */
extern
IRSB* bb_to_IR ( 
         /*OUT*/VexGuestExtents* vge,
         /*OUT*/UInt*            n_sc_extents,
         /*OUT*/UInt*            n_guest_instrs, /* stats only */
         /*MOD*/VexRegisterUpdates* pxControl,
         /*IN*/ void*            callback_opaque,
         /*IN*/ DisOneInstrFn    dis_instr_fn,
         /*IN*/ const UChar*     guest_code,
         /*IN*/ Addr             guest_IP_bbstart,
         /*IN*/ Bool             (*chase_into_ok)(void*,Addr),
         /*IN*/ VexEndness       host_endness,
         /*IN*/ Bool             sigill_diag,
         /*IN*/ VexArch          arch_guest,
         /*IN*/ const VexArchInfo* archinfo_guest,
         /*IN*/ const VexAbiInfo*  abiinfo_both,
         /*IN*/ IRType           guest_word_type,
         /*IN*/ UInt             (*needs_self_check)
                                    (void*, /*MB_MOD*/VexRegisterUpdates*,
                                            const VexGuestExtents*),
         /*IN*/ Bool             (*preamble_function)(void*,IRSB*),
         /*IN*/ Int              offB_GUEST_CMSTART,
         /*IN*/ Int              offB_GUEST_CMLEN,
         /*IN*/ Int              offB_GUEST_IP,
         /*IN*/ Int              szB_GUEST_IP
      );


#endif /* ndef __VEX_GUEST_GENERIC_BB_TO_IR_H */

/*--------------------------------------------------------------------*/
/*--- end                                 guest_generic_bb_to_IR.h ---*/
/*--------------------------------------------------------------------*/
