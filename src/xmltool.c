#include <string.h>
#include <stdio.h>

extern int xmllint(int argc, char **argv);
extern int xpath_locator(int argc, char **argv);

int
main(int argc, char **argv) {
    if (!strcmp(argv[1], "--xpath-locator")) {
        for (int i = 1; i < argc - 1; ++i) {
            argv[i] = argv[i + 1];
        }
        return xpath_locator(argc - 1, argv);
    }

    return xmllint(argc, argv);
}