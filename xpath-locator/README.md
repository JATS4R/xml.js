This is a tool to take the XPath expressions produced by the Schematron output,
and locate them within the XML documents, by column and line number.

The validator will continue to use xmllint in the first pass, to do DTD validation
and entity resolution.

The problem is that both my utility and xmllint will use libxml, but both are designed
as main programs. So, with what I know how to do so far with emscripted (static linking),
that would mean two separate >4Meg javascript libraries, the bulk of both being the libxml2
library.

So I need to create just one main module that does both functions. 
I don't want to hack the libxml2 version of xmllint. So instead, I will copy it to my
repo, and change the name of `main` to `xmllint`. I'll put my utility in another file,
and have it either run the xpath locator function, or else delegate to xmllint.

Strategy:

* ✓Fork Alf's xml.js, make sure building and testing works, as is.
    - Forked it to jats4r: https://github.com/JATS4R/xml.js
* ✓Create a new branch, xpath-locator.
* Make a copy of xmllint.c, that's out of libxml2, and use this to build the library
* Rename it's `main` to `xmllint`, and create another, separate main program that
  just delegates.
* Add my xpath locator function.


# Development

Right now, I'm developing xpath_locator.c by compiling it and running it as a binary
(instead of JS), using `gcc`.

First download, build, and install a working copy of libxml2. Then put its `bin`
directory in the front of your PATH.

Build xpath_locator with:

```
gcc `xml2-config --cflags --libs` -o xpath_locator xpath_locator.c
```

Test:

```
./xpath_locator '/foo[1]/bar[1]' '/foo[2]/bar[2]'
```


# To do

* ✓Allocate storage for the array of XPathFinders
* (✓)For each xpath expression passed in:
  * ✓strcpy, and save the original
  * ✓Count how many segments
  * ✓Allocation storage for each of the XPathSegFinder
  * ✓Split it on '/'
  * (✓)For each segment:
      * ✓strcpy, and save the original
      * extract and save the element_local_name
      * extract and save the namespace_uri, if there is one
      * extract and save the position

