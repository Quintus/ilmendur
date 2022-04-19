#ifndef ILMENDUR_I18N_HPP
#define ILMENDUR_I18N_HPP
// Note: buildconfig.hpp contains the ENABLE_NLS define and has thus to be included before gettext.h.
#include <buildconfig.hpp>
#include "gettext.h"

// This file contains translation macros for use with GNU Gettext.
// See section 1.5 of the GNU Gettext manual.

/// Translate the passed string with Gettext
#define _(String) gettext (String)
/// Helper referenced by N_() macro
#define gettext_noop(String) String
/// Do not translate the passed string, but do detect it
#define N_(String) gettext_noop (String)

#endif /* ILMENDUR_I18N_HPP */
