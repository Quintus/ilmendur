#include "i18n.hpp"
#include "os.hpp"

#ifdef ILMENDUR_DEBUG_BUILD
#include <iostream>
#endif

using namespace std;
namespace fs = std::filesystem;

void I18n::setup()
{
    char* userlocale = setlocale(LC_ALL, "");

    fs::path transdir;
#ifdef ILMENDUR_DEBUG_BUILD
    // Support running from the build directory in debug mode
    if (fs::exists(OS::exePath().parent_path() / fs::u8path("CMakeCache.txt"))) {
        transdir = fs::current_path() / fs::u8path("translations");
    } else {
#endif
        transdir = OS::gameDataDir() / fs::u8path("translations");
#ifdef ILMENDUR_DEBUG_BUILD
    }
#endif

    /* The below call, which sets the catalogue search path, is hairy,
     * because `transdir' can contain Unicode characters; they must be
     * converted to the native filesystem encoding, which on POSIX
     * systems is dictated by the environment variable $LANG, or more
     * precisely, by the value the respective libc assigns to LC_CTYPE
     * (or its override value LC_ALL), which on Unix is usually
     * initialized from the $LANG environment variable (see locale(7)
     * for details). On Windows (starting with Windows 2000 I think),
     * the native file system encoding is *fixed* to UTF-16LE
     * regardless of the environment settings, but bindtextdomain() is
     * unable to deal with an environment-independent file system
     * encoding. Therefore, on Windows, wbindtextdomain() is provided
     * as an alternative by libintl that deals with that specific
     * problem by accepting Windows' UTF16-LE in the directory
     * parameter.
     *
     * There is no defined `native wide encoding' on POSIX systems,
     * they only have a `native narrow encoding' defined as explained
     * above by $LANG; today it is usually UTF-8.
     *
     * Note that the actual catalogue bindtextdomain() resolves to
     * is of the following format (§ 11.2.3 of the Gettext manual):
     *
     *    <transdir>/<locale>/LC_<category>/<ILMENDUR_GETTEXT_DOMAIN>.mo
     *
     * (Where `category' usually is literally `MESSAGES'.)
     */
#if defined(__unix__)
    const char* actual_dir = bindtextdomain(ILMENDUR_GETTEXT_DOMAIN, transdir.string().c_str());
    transdir = fs::path(actual_dir); // < Conversion from native narrow         ^ Conversion to native narrow
#elif defined(_WIN32)
    const wchar_t* actual_dir = wbindtextdomain(ILMENDUR_GETTEXT_DOMAIN, transdir.wstring().c_str());
    transdir = fs::path(actual_dir); // < Conversion from native wide = UTF16-LE     ^ Conversion to native wide = UTF-16LE
#else
#error unsupported system
#endif

    // Please always output UTF-8 from the message catalogues
    bind_textdomain_codeset(ILMENDUR_GETTEXT_DOMAIN, "UTF-8");

    // Activate the correct message catalogue
    textdomain(ILMENDUR_GETTEXT_DOMAIN);

#ifdef ILMENDUR_DEBUG_BUILD
    cout << "I18n information:" << endl
         << "  Locale:    " << userlocale << endl
         << "  Domain:    " << ILMENDUR_GETTEXT_DOMAIN << endl
         << "  Directory: " << transdir.string() << endl;
#endif
}
