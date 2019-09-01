#ifndef _PCENV_H
#define _PCENV_H

namespace PcEnv {

// This constant makes it possible to write code that the compiler compiles
// and checks for syntax errors but discards if not enabled.
// If using the PCENV define directly the compiler never sees the code that
// is conditionally disabled.
#if defined PCENV
static constexpr bool enabled = true;
#else
static constexpr bool enabled = false;
#endif
}

#endif // _PCENV_H_
