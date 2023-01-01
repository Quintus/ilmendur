#ifndef ILMENDUR_I18N_HPP
#define ILMENDUR_I18N_HPP
#include "buildconfig.hpp" /* Always include this before gettext.h, it defines ENABLE_NLS */
#include "gettext.h"

/* Gettext translation keywords. This must match the `--keyword' arguments
 * passed to xgettext(1). The gettext(3), ngettext(3), and pgettext(3)
 * functions are provided by glibc on Linux, otherwise by libintl. */

/// Normal translation
#define _(String) gettext(String)
/// Translation with singular and plural variants
#define PL_(Singular, Plural, Num) ngettext((Singular), (Plural), (Num))
/// Context-sensitive normal translation
#define C_(Context, String) pgettext(Context, String)

namespace I18n {
    void setup();
}

#endif /* ILMENDUR_I18N_HPP */
