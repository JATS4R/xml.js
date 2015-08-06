#include <stdio.h>
#include <string.h>
#include <libxml/SAX2.h>


struct XPathSegFinder_struct {
    /* What we're looking for: */
    xmlChar *original;   /* pointer to the original string */
    xmlChar *local_name;
    xmlChar *namespace_uri;
    int position;
    /* Where we are in the process of finding it: */
    int count;
};
typedef struct XPathSegFinder_struct XPathSegFinder;

struct XPathFinder_struct {
    /* What we're looking for: */
    xmlChar *original;
    int num_segs;         /* number of xpath segments in this xpath */
    XPathSegFinder *seg_finders;
    /* Where we are in th process of finding it: */
    int current_level;
    /* What we found: */
    int line_number;
    int column_number;
};
typedef struct XPathFinder_struct XPathFinder;

typedef int bool;
static bool TRUE = 1;
static bool FALSE = 0;


/* 
  xmlChar versions of common library functions. The purpose of these is
  to get rid of compiler warnings.
*/

static int
xstrlen(const xmlChar *s) {
    return strlen((const char *) s);
}

static const xmlChar *
xstrchr(const xmlChar *str, int c) {
    return (const xmlChar *) strchr((const char *) str, c);
}

static xmlChar *
xstrcpy(xmlChar *dest, const xmlChar *src) {
    return (xmlChar *) strcpy((char *) dest, (const char *) src);
}

static xmlChar *
xstrncpy(xmlChar *dest, const xmlChar *src, size_t num) {
    return (xmlChar *) strncpy((char *) dest, (const char *) src, num);
}

static xmlChar *
xstrtok(xmlChar *str, const char *delims) {
    return (xmlChar *) strtok((char *) str, delims);
}

static int
xstrcmp(const xmlChar *str1, const xmlChar *str2) {
    return strcmp((const char *) str1, (const char *) str2);
}

static int 
xstrncmp(const xmlChar *str1, const xmlChar *str2, size_t num) {
    return strncmp((const char *) str1, (const char *) str2, num);
}

/* Count the number of times a character occurs in a string. */
static int 
count_chars(const xmlChar* string, char ch) {
    int count = 0;
    int length = xstrlen(string);
    for (int i = 0; i < length; ++i) {
        if (string[i] == ch) count++;
    }
    return count;
}


/* 
  This function converts every '/' that appears inside square brackets into '^'.
  This is a hack necessary because '/' is used to delimit segments, but only when
  they appear outside square brackets.
  If escape is TRUE, then we escape: '/' -> '^'. If FALSE, then unescape.
*/

static void 
escape_uri_slashes(xmlChar *str, bool escape) {
    char from = escape ? '/' : '^';
    char to = escape ? '^' : '/';
    int length = xstrlen(str);
    int bracket_depth = 0;
    for (int i = 0; i < length; ++i) {
        if (str[i] == '[') bracket_depth++;
        else if (str[i] == ']') bracket_depth--;
        else if (bracket_depth && str[i] == from) {
            str[i] = to;
        }
    }
}

static void *
mmalloc(unsigned long size) {
    void *m = malloc(size);
    // printf("malloc 0x%lx, %lu bytes\n", (unsigned long) m, size);
    return m;
}

static void
ffree(void *m) {
    // printf("free 0x%lx\n", (unsigned long) m);
    free(m);
}

static bool 
starts_with(const xmlChar *str, const xmlChar *prefix)
{
    size_t prefix_len = xstrlen(prefix),
           str_len = xstrlen(str);
    return str_len < prefix_len ? FALSE : 
              xstrncmp(prefix, str, prefix_len) == 0;
}

static xmlChar *
new_string(const xmlChar *src) {
    xmlChar *ns = (xmlChar *) mmalloc(xstrlen(src) + 1);
    xstrcpy(ns, src);
    return ns;
}

static xmlChar *
new_string_n(const xmlChar *src, size_t len) {
    xmlChar *ns = (xmlChar *) mmalloc(len + 1);
    xstrncpy(ns, src, len);
    ns[len] = 0;
    return ns;
}

// Global variables:

static XPathFinder *xpath_finders;
int num_xpaths;
static int parser_level;

static void
debug_out(const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    char f[strlen(fmt) + parser_level * 2 + 1];
    *f = 0;
    for (int i = 0; i < parser_level; ++i) 
        strcat(f, "  ");
    strcat(f, fmt);
    //vprintf(f, args);
}

// FIXME: need error checking throughout this.
static void 
my_startElementNs(void *ctx,
                  const xmlChar *localname,
                  const xmlChar *prefix,
                  const xmlChar *URI,
                  int nb_namespaces,
                  const xmlChar **namespaces,
                  int nb_attributes,
                  int nb_defaulted,
                  const xmlChar **attributes) 
{
    debug_out("%d:%d: start %s:%s (%s)\n",
        xmlSAX2GetLineNumber(ctx),
        xmlSAX2GetColumnNumber(ctx),
        prefix, localname, URI);

    for (int xpath_num = 0; xpath_num < num_xpaths; ++xpath_num) 
    {
        XPathFinder *xpath_finder = &xpath_finders[xpath_num];
        debug_out("  Checking xpath finder #%d; line_number = %d\n", 
            xpath_num, xpath_finder->line_number);
        int xpf_level = xpath_finder->current_level;
        if (xpath_finder->line_number == 0 &&
            xpf_level == parser_level) 
        {
            XPathSegFinder *seg_finder = &xpath_finder->seg_finders[xpf_level];
            debug_out("    Checking seg_finder[%d]\n", xpf_level);
            const xmlChar *seg_uri = seg_finder->namespace_uri;
            if ( !xstrcmp(localname, seg_finder->local_name) ) {
                debug_out("      local_name matches\n");
                if ( ( (URI == NULL && seg_uri == NULL) ||
                   (URI != NULL && seg_uri != NULL && !xstrcmp(URI, seg_uri)) ) ) 
                {
                    debug_out("      element match!\n");
                    seg_finder->count++;
                    if (seg_finder->count == seg_finder->position) {
                        debug_out("      position match! parser_level = %d, "
                            "num_segs = %d\n", parser_level, xpath_finder->num_segs);
                        if (parser_level == xpath_finder->num_segs - 1) {
                            // Found!
                            int line_number = xmlSAX2GetLineNumber(ctx);
                            debug_out("      line_number <- %d\n", line_number);
                            xpath_finder->line_number = line_number;
                            xpath_finder->column_number = xmlSAX2GetColumnNumber(ctx);
                        }
                        else {
                            xpath_finder->current_level++;
                        }
                    }
                }
            }
        }
    }

    parser_level++;
}

static void
my_endElementNs(void *ctx,
                const xmlChar *localname,
                const xmlChar *prefix,
                const xmlChar *URI)
{
    parser_level--;
    debug_out("%d:%d: end %s:%s (%s)\n",
        xmlSAX2GetLineNumber(ctx),
        xmlSAX2GetColumnNumber(ctx),
        prefix, localname, URI);
}

static xmlSAXHandler handlers_struct;
static xmlSAXHandlerPtr handlers = &handlers_struct;

int
main(int argc, char **argv) {
    num_xpaths = argc - 1;
    // printf("num_xpaths == %d\n", num_xpaths);

    // printf("allocating xpath_finders: ");
    xpath_finders = (XPathFinder *) mmalloc(num_xpaths * sizeof(XPathFinder));

    for (int xpath_num = 0; xpath_num < num_xpaths; ++xpath_num) {
        // FIXME: translate into xmlChars here:
        xmlChar *xpath_expr = (xmlChar *) argv[xpath_num + 1];
        XPathFinder *xpath_finder = &xpath_finders[xpath_num];
        // printf("Initializing xpath_finder at 0x%lx\n", (unsigned long) xpath_finder);
        xpath_finder->current_level = 0;
        xpath_finder->line_number = 0;
        xpath_finder->column_number = 0;

        // printf("allocating xpath_finder->original: ");
        xpath_finder->original = new_string(xpath_expr);

        // printf("Got xpath expression '%s'\n", xpath_finder->original);
        escape_uri_slashes(xpath_expr, TRUE);
        // printf("After escaping:      '%s'\n", xpath_expr);
        int num_segs = xpath_finder->num_segs = count_chars(xpath_expr, '/');
        // printf("num_segs = %d\n", num_segs);

        // printf("allocating seg_finders: ");
        XPathSegFinder *seg_finders = 
            (XPathSegFinder *) mmalloc(num_segs * sizeof(XPathSegFinder));
        xpath_finder->seg_finders = seg_finders;

        /* Extract each XPath segment */
        xmlChar *seg = xstrtok(xpath_expr, "/");
        int seg_num = 0;
        while (seg != NULL) {
            XPathSegFinder *seg_finder = &seg_finders[seg_num];
            seg_finder->count = 0;


            // printf("initializing seg_finder at 0x%lx\n", (unsigned long) seg_finder);
            // printf("allocating seg_finder->original: ");
            escape_uri_slashes(seg, FALSE);
            seg_finder->original = new_string(seg);

            // printf("  Segment '%s'\n", seg_finder->original);

            // Get the element local name
            xmlChar *lns = starts_with(seg, (const xmlChar *) "*:") ? seg + 2 : seg;
            const xmlChar *bracket = xstrchr(seg, '[');
            int local_name_len = bracket - lns;
            xmlChar *local_name = seg_finder->local_name = 
                new_string_n(lns, local_name_len);
            // printf("local_name = '%s'\n", local_name);

            if (starts_with(bracket + 1, (const xmlChar *) "namespace-uri()=")) {
                const xmlChar *ns_start = bracket + 18;
                const xmlChar *ns_end = xstrchr(ns_start, '\'');
                seg_finder->namespace_uri = new_string_n(ns_start, ns_end - ns_start);
                // printf("found namespace '%s'\n", seg_finder->namespace_uri);
                bracket = xstrchr(ns_end, '[');
            }
            else {
                seg_finder->namespace_uri = NULL;
            }

            const xmlChar *pos_start = bracket + 1;
            const xmlChar *pos_end = xstrchr(pos_start, ']');
            size_t pos_str_len = pos_end - pos_start;
            char pos_str[10];
            strncpy(pos_str, (const char *) pos_start, pos_str_len);
            pos_str[pos_str_len] = 0;
            seg_finder->position = atoi(pos_str);
            // printf("position: %d\n", seg_finder->position);

            seg = xstrtok(NULL, "/");
            seg_num++;
        }
    }


    // Initialize default handler structure for SAX 2
    xmlSAXVersion(handlers, 2);
    handlers->startElementNs = my_startElementNs;
    handlers->endElementNs = my_endElementNs;

    parser_level = 0;
    int res = xmlSAXUserParseFile(handlers, NULL, "test.xml");

    // Output the results
    for (int xpath_num = 0; xpath_num < num_xpaths; ++xpath_num) {
        XPathFinder *xpath_finder = &xpath_finders[xpath_num];
        printf("%d:%d\n", xpath_finder->line_number, xpath_finder->column_number);
    }    


    for (int xpath_num = 0; xpath_num < num_xpaths; ++xpath_num) {
        XPathFinder *xpath_finder = &xpath_finders[xpath_num];
        int num_segs = xpath_finder->num_segs;
        XPathSegFinder *seg_finders = xpath_finder->seg_finders;
        for (int seg_num = 0; seg_num < num_segs; ++seg_num) {
            XPathSegFinder *seg_finder = &seg_finders[seg_num];
            ffree(seg_finder->original);
            ffree(seg_finder->local_name);
            ffree(seg_finder->namespace_uri);
        }

        ffree(xpath_finder->seg_finders);
        ffree(xpath_finder->original);
    }
    ffree(xpath_finders);
}

