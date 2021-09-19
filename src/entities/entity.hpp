#ifndef ILMENDUR_ENTITY_HPP
#define ILMENDUR_ENTITY_HPP

/**
 * This is the list of valid values for the `il_entity` field in
 * Blender objects. Please always assign an explicit number, because
 * this numbering has to be stable in order to stay in sync with what
 * is used in the Blender scene files. If a type is to be deleted,
 * leave the corresponding number out unless you are absolutely sure
 * no existing Blender scene uses that number anymore.
 */
enum class entity_type {
    invalid = 0, // Default value to indicate no type is set
    sign = 1,    // A sign with text.

    fin // Keep this value last and do not assign a number to it
};

#endif /* ILMENDUR_ENTITY_HPP */
