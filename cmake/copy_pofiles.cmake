# This script exists for the sole purpose of allowing to run the
# `ilmendur' executable from the build directory without calling
# `make install' before. It copies all .gmo files generated by
# gettext_process_po_files into the directory "gettext-catalogues"
# in the format required by bindtextdomain(3). Ensure it is called
# after the "pofiles" target provided by gettext_process_po_files.
#
# Pass these parameters:
#   - ILMENDUR_GETTEXT_DOMAIN
#   - PO_FILES
foreach(pofile ${PO_FILES})
  get_filename_component(lang ${pofile} NAME_WE)
  message(STATUS "Copying ${lang}.gmo to gettext-catalogue directory structure for ${ILMENDUR_GETTEXT_DOMAIN} domain")

  file(COPY ${CMAKE_BINARY_DIR}/${lang}.gmo
    DESTINATION "${CMAKE_BINARY_DIR}/gettext-catalogues/${lang}/LC_MESSAGES")
  file(RENAME "${CMAKE_BINARY_DIR}/gettext-catalogues/${lang}/LC_MESSAGES/${lang}.gmo"
    "${CMAKE_BINARY_DIR}/gettext-catalogues/${lang}/LC_MESSAGES/${ILMENDUR_GETTEXT_DOMAIN}.mo")
endforeach()
