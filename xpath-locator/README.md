This is a tool to take the XPath expressions produced by the Schematron output,
and locate them within the XML documents, by column and line number.

Usage:

```
./xpath_locator file.xml xpath1 xpath2 ...
```

The XPath expressions are restricted to a very rigid form; the form that is produced
by the Schematron validation output. They are used to identify a specific element
in the document tree


# Integraton with JATS4R validator


The validator will continue to use xmllint in the first pass, to do DTD validation
and entity resolution.

So, both this utility and xmllint will use libxml2. However, both need to be called as
main programs. So, with what I know how to do so far with emscripted (static linking),
that would mean two separate, rather large, javascript libraries, the bulk of both being 
the libxml2 library -- some pretty nasty redundancy.

So, I created one main module, in xmltool.c, that delegates to either xmllint or to
xpath_locator, depending on the first command-line option. 
I didn't want to hack the libxml2 version of xmllint, so instead, I copied it out of
libxml2, and change the name of the `main` function to `xmllint`. xpath_locator is
in its own C module, and can be compiled either with a `main` function, or with
`xpath_locator` (see Development, below).

The output of all this is a JavaScript library, xmltool.js, which works as follows:

* If the very first command line argument is `xpath_locator`, then it is stripped
  out, and the xpath_locator function is called with the rest of the arguments
* Otherwise, `xmllint` is called.

So, you can use `xmltool` exactly the same as you currently use `xmllint`, and 
never be the wiser.



# Development

Here are the steps to compile xpath_locator as a stand-alone binary executable.

It might or might not be necessary to first download, build, and install a working copy 
of libxml2. For example, download the hourly snapshot from 
[here](http://www.xmlsoft.org/downloads.html), for example,
[ftp://xmlsoft.org/libxml2/libxml2-git-snapshot.tar.gz](). Cloning the git repo would
require extra steps, because it doesn't include a *configure* script. Unzip it, and
cd into the directory. Decide where you want to install it, e.g. "/home/user/libxml2/install", 
and then:

```
./configure --prefix=/home/user/libxml2/install
make
make install
```

To set your environment to use this, add the install/bin directory to the front of your PATH.
Then, confirm with `which xml2-config`.

Next, change to the xpath_locator project directory, and build xpath_locator as a 
stand-alone program, with:

```
gcc -DXPATH_LOCATOR_MAIN `xml2-config --cflags --libs` -o xpath_locator xpath_locator.c
```

Test with the following. The first four should find a match, the fifth, not.

```
./xpath_locator test.xml \
  '/article[1]/front[1]/article-meta[1]/permissions[1]/copyright-你好[1]' \
  '/article[1]/body[1]/p[2]/*:math[namespace-uri()='"'"'http://www.w3.org/1998/Math/MathML'"'"'][1]' \
  '/article[1]/body[1]/sec[2]/p[2]' \
  '/article[1]/body[1]/p[1]/sub[1]' \
  '/article[1]/body[1]/p[1]/sub[1]/em[1]'
```

# To do


* Create a new test program for it. I want it to demonstrate a simple interface for
  displaying the original XML file, with arrows pointing to the identified locations


* xpath_locator needs to read the DTD.  Test it with each of the
  test files in validator/samples.

