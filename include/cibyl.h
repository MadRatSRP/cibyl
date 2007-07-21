/*********************************************************************
 *
 * Copyright (C) 2006,  Blekinge Institute of Technology
 *
 * Filename:      cibyl.h
 * Author:        Simon Kagstrom <ska@bth.se>
 * Description:   Cibyl defs
 *
 * $Id: cibyl.h 13453 2007-02-05 16:28:37Z ska $
 *
 ********************************************************************/
#ifndef __CIBYL_H__
#define __CIBYL_H__

/* Stack size */
#ifndef NOPH_STACK_SIZE
# define NOPH_STACK_SIZE  8192 /* 8KB stack */
#endif

#ifndef __ASSEMBLER__

# define NOPH_SECTION(x) __attribute__((section (x)))

/* Convenience */
typedef int bool_t;

/* Special object! */
typedef int NOPH_Exception_t;

/* Include the generated syscall definitions
 *
 * Thanks to Ian Lance Taylor for the use of .set push / .set pop.
 */
# include "cibyl-syscall_defs.h"

extern void __NOPH_try(void (*callback)(NOPH_Exception_t exception, void *arg), void *arg);

/**
 * Begin a block of catching Java exceptions. Java exceptions will be
 * catched within the block after NOPH_try (ended by NOPH_catch). The
 * exception handling works as in Java, so the handler will return to
 * the statement after the NOPH_catch() statement. The block must be
 * closed by a @a NOPH_catch.
 *
 * @param callback the callback to call if an exception occurs in the block
 * @param arg the argument to pass to the callback
 */
#define NOPH_try(callback, arg) do { \
  asm volatile("");                  \
  __NOPH_try(callback, arg);         \
} while(0); do

/**
 * End a Java exception-catching block, catching all exceptions. Must
 * be preceeded by a @a NOPH_try
 *
 * @see NOPH_try
 */
extern void __NOPH_catch(void);
#define NOPH_catch() while(0); do {  \
  asm volatile("");                  \
  __NOPH_catch();                    \
} while(0)

/**
 * End a Java exception-catching block, catching a specific class of
 * exceptions. Must be preceeded by a @a NOPH_try.
 *
 * @warning This is not yet implemented and will now to the same as @a
 * NOPH_catch
 *
 * @see NOPH_try, NOPH_catch
 */
#define NOPH_catch_exception(exceptionClass) NOPH_catch()

#endif /* __ASSEMBLER__ */

#endif /* !__CIBYL_H__ */
