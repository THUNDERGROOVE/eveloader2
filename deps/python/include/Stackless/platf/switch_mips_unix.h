/*
 * this is the internal transfer function.
 *
 * HISTORY
 * 05-Jan-08 Thiemo Seufer  <ths@debian.org>
 *      Ported from ppc.
 */

#define STACK_REFPLUS 1

#ifdef SLP_EVAL

#define STACK_MAGIC 0

#ifdef __mips64
#define REGS_TO_SAVE "$16", "$17", "$18", "$19", "$20", "$21", "$22", \
       "$23", "$28", "$30"
#else
#define REGS_TO_SAVE "$16", "$17", "$18", "$19", "$20", "$21", "$22", \
       "$23", "$30"
#endif
    __asm__ volatile ("" : : : REGS_TO_SAVE);
/*
 * You may want to make the function static enable optimizations.
 * However, the ABI SPEC does not apply to static functions. Therefore
 * I make slp_switch a regular global function.
 */
#if 0
static
#endif
int
slp_switch(void)
{
    register int *stackref, stsizediff;
    /* __asm__ __volatile__ ("" : : : REGS_TO_SAVE); */
    __asm__ ("move %0, $29" : "=r" (stackref) : );
    {
        SLP_SAVE_STATE(stackref, stsizediff);
        __asm__ __volatile__ (
#ifdef __mips64
            "daddu $29, %0\n"
#else
            "addu $29, %0\n"
#endif
            : /* no outputs */
            : "r" (stsizediff)
            );
        SLP_RESTORE_STATE();
    }
    return 0;
}

#endif

/*
 * further self-processing support
 */

/*
 * if you want to add self-inspection tools, place them
 * here. See the x86_msvc for the necessary defines.
 * These features are highly experimental und not
 * essential yet.
 */
