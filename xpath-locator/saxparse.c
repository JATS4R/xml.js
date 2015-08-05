#include <libxml/SAX2.h>


void my_startDocument(void *ctx) {
    printf("startDocument, line number = %d\n", xmlSAX2GetLineNumber(ctx));
}

void my_startElementNs(void *ctx,
                       const xmlChar *localname,
                       const xmlChar *prefix,
                       const xmlChar *URI,
                       int nb_namespaces,
                       const xmlChar **namespaces,
                       int nb_attributes,
                       int nb_defaulted,
                       const xmlChar **attributes) 
{
    printf("start element: loc = %d:%d: localname='%s', prefix='%s', URI='%s'\n",
        xmlSAX2GetLineNumber(ctx),
        xmlSAX2GetColumnNumber(ctx),
        localname, prefix, URI);
}


static xmlSAXHandler handlers_struct;

static xmlSAXHandlerPtr handlers = &handlers_struct;

int main(int argc, char **argv) {
    if (argc != 2) {
        return(1);
    }

    // Initialize default handler structure for SAX 2
    xmlSAXVersion(handlers, 2);
    handlers->startDocument = my_startDocument;
    handlers->startElementNs = my_startElementNs;


    int res = xmlSAXUserParseFile(handlers, NULL, argv[1]);
}

