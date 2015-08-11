This is a fork of [hubgit/xml.js](https://github.com/hubgit/xml.js), which is
a fork of [kripken/xml.js](https://github.com/kripken/xml.js).

In this particular fork, work has been done to adapt libxml2 to the specific
needs of the [JATS4R validator](http://jats4r.org/validator/), which is a
client-side validation tool.


This package exports the `xmltool` program, which can do one of two things:

* Under normal circumstances, it acts exactly the same as libxml2's xmllint
  command;
* If the first argument is `--xpath-locator`, then this performs a specialized
  task of determining the exact file locations of a set of XPath expressions.




## API


### xmllint

```javascript
var args = ['--noent', '--dtdvalid', dtdPath, xmlPath];
var files = [
  {
    path: dtdPath,
    data: dtdContents
  },
  {
    path: xmlPath,
    data: xmlContents
  }
];
var ret = xmltool(args, files);
```

`args` is an array of arguments to pass to the program.

`files` is an array of objects, each with a `path` and `data` property. These will be turned into pseudo-filesystem objects for the program to access.

The return value object has two properties: 'stdout' and 'stderr'. For example:

```javascript
if (!ret.stderr) {
  // there were no errors.
}
```


### xpath-locator

This is a tool to take the XPath expressions produced by the Schematron output,
and locate them within the XML documents, by column and line number.

Invoke it with:

```javascript
var args = ['--xpath-locator', xmlPath, xpath1, xpath2];
var files = [{
  path: xmlPath,
  data: xmlContents
}];
var ret = xmltool(args, files);
if (!ret.stderr) {
  // no errors
}
```

The output will be a set of line and column numbers, one per line. The line and
the column number separated by a colon. For example, if there were three XPath
expressions, then the output might look like this:

```
30:34
44:19
28:12
```

The XPath expressions are restricted to a very rigid form; the form that is produced
by the Schematron validation output. They are used to identify a specific element
in the document tree. Here are some examples:

```
/article[1]/body[1]/p[2]/*:math[namespace-uri()='http://www.w3.org/1998/Math/MathML'][1]
/article[1]/body[1]/sec[2]/p[2]
```

# Integration with JATS4R validator

The validator will continue to use xmllint in the first pass, to do DTD validation
and entity resolution.

Both xmllint and xpath-locator use the libxml2 library. However, both need to be 
called as main programs. So, to avoid redundantly including all of the libxml2
code in two separate JavaScript libraries, we created one main module, in 
xmltool.c, that delegates to either xmllint.c or to
xpath_locator.c, depending on the first command-line option. 

To allow xmllint to be called as a subroutine to something else. 
Rather than hack the libxml2 version of xmllint, we copied it out of
libxml2, and changed it in the copy.

The output of all this is a JavaScript library, xmltool.js, which works as follows:

* If the very first command line argument is `--xpath-locator`, then it is stripped
  out, and the `xpath_locator` function is called with the rest of the arguments
* Otherwise, `xmllint` is called.


# Building from source

[Install the Emscripten 
SDK](https://kripken.github.io/emscripten-site/docs/getting_started/downloads.html). 
When that's done, make sure that you `source` the *emsdk_env.sh* script, to set up 
your environment to use it.

```
git clone https://github.com/JATS4R/xml.js.git
cd xml.js
git checkout xpath-locator
git submodule init
git submodule update

npm install
npm install -g gulp

gulp clean
gulp libxml2
gulp compile
gulp test     # currently failing with "./test/test.xml fails to validate"
```

To test in a browser, make sure your repo is being served by an HTTP server, and open
up the following files in a browser:

* test/test-xmllint.html
* test/test-xpath-locator.html


# C-language development

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

Next, change to the *src* project directory, and build xpath_locator as a 
stand-alone program, with:

```
gcc -DXPATH_LOCATOR_MAIN `xml2-config --cflags --libs` -o xpath_locator xpath_locator.c
```

Test with the following. The first four should find a match, the fifth, not.

```
./xpath_locator ../test/test-xpath-locator.xml \
  '/article[1]/front[1]/article-meta[1]/permissions[1]/copyright-你好[1]' \
  '/article[1]/body[1]/p[2]/*:math[namespace-uri()='"'"'http://www.w3.org/1998/Math/MathML'"'"'][1]' \
  '/article[1]/body[1]/sec[2]/p[2]' \
  '/article[1]/body[1]/p[1]/sub[1]' \
  '/article[1]/body[1]/p[1]/sub[1]/em[1]'
```
