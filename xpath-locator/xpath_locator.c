#include <stdio.h>
#include <string.h>
#include <libxml/SAX2.h>


struct XPathSegFinder_struct {
    xmlChar *original;   /* pointer to the original string */
    xmlChar *element_local_name;
    xmlChar *namespace_uri;
    int position;
    int current_num;
};
typedef struct XPathSegFinder_struct XPathSegFinder;

struct XPathFinder_struct {
    xmlChar *original;
    int num_segs;         /* number of xpath segments in this xpath */
    XPathSegFinder *seg_finders;
    int current_level;
};
typedef struct XPathFinder_struct XPathFinder;

static XPathFinder *xpath_finders;

static
int count_chars(const char* string, char ch) {
    int count = 0;
    int length = strlen(string);
    for (int i = 0; i < length; ++i) {
        if (string[i] == ch) count++;
    }
    return count;
}

int
main(int argc, char **argv) {
    int num_xpaths = argc - 1;
    printf("num_xpaths == %d\n", num_xpaths);
    xpath_finders = (XPathFinder *) malloc(num_xpaths * sizeof(XPathFinder));

    for (int xpath_num = 0; xpath_num < num_xpaths; ++xpath_num) {
        char *xpath_expr = argv[xpath_num + 1];
        XPathFinder *path_finder = &xpath_finders[xpath_num];
        path_finder->original = (xmlChar *) malloc(strlen(xpath_expr) + 1);
        strcpy((char *) path_finder->original, xpath_expr);

        printf("Got xpath expression %s\n", path_finder->original);
        int num_segs = path_finder->num_segs = count_chars(xpath_expr, '/');
        printf("num_segs = %d\n", num_segs);

        XPathSegFinder *seg_finders = 
            (XPathSegFinder *) malloc(num_segs * sizeof(XPathSegFinder));
        path_finder->seg_finders = seg_finders;

        /* Extract each XPath segment */
        char *seg = strtok(xpath_expr, "/");
        int seg_num = 0;
        while (seg != NULL) {
            XPathSegFinder *seg_finder = &seg_finders[seg_num];
            seg_finder->original = (xmlChar *) malloc(strlen(seg));
            strcpy((char *) seg_finder->original, seg);

            printf("  Segment '%s'\n", seg_finder->original);


            seg = strtok(NULL, "/");
            seg_num++;
        }
    }


    for (int xpath_num = 0; xpath_num < num_xpaths; ++xpath_num) {
        XPathFinder *path_finder = &xpath_finders[xpath_num];
        int num_segs = path_finder->num_segs;
        XPathSegFinder *seg_finders = path_finder->seg_finders;
        for (int seg_num = 0; seg_num < num_segs; ++seg_num) {
            XPathSegFinder *seg_finder = &seg_finders[seg_num];
            free(seg_finder->original);
        }

        free(path_finder->seg_finders);
        free(path_finder->original);
    }
    free(xpath_finders);
}

