
Online demo at http://syssgx.github.com/xml.js/

This package exports the `xmllint` object which is an Emscripten port of
libxml2's `xmllint` command for use in the browser or node.

##### API #####

```javascript

xmllint(args, files);

```

`args` is an array of arguments to pass to xmllint.

`files` is an array of objects, each with a `path` and `data` property. These will be turned into pseudo-filesystem objects for xmllint to access.

The return value Object has two properties: 'stdout' and 'stderr':

```javascript

if (!xmllint(args, files).stderr) {
	//there were no errors.
}

```

#### Building xmllint from source ####

[Install the Emscripten 
SDK](https://kripken.github.io/emscripten-site/docs/getting_started/downloads.html). When that's
done, make sure that you `source` the *emsdk_env.sh* script, to set up your environment to
use it.

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
gulp test   # currently failing with "./test/test.xml fails to validate"
```

To test in a browser, make sure your repo is being served by an HTTP server, and open
test/test.html in it.

