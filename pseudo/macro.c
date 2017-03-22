#include "../nessemble.h"

/**
 * .macro pseudo instruction
 * @param {char *} string - Macro string name
 */
 void pseudo_macro(char *string) {
     int index = get_macro(string);

     if (index == -1) {
         error(_("Macro `%s` was not defined"), string);
         goto cleanup;
     }

     include_string_push(macros[index].text);

 cleanup:
     return;
 }
