// https://kripken.github.io/emscripten-site/docs/api_reference/module.html

var Module = {
	arguments: args || [],
	preRun: function () {
		files.forEach(function(file) {
			// create the folder if needed
			var parts = file.path.split('/');

			if (parts.length > 1) {
				FS.createPath('/', parts.slice(0, -1).join('/'), true, true);
			}

			// create the file
			FS.createDataFile('/', file.path, 
        Module['intArrayFromString'](file.data), true, true);
		});
	},

  // This function converts the UTF-8 byte stream to a string
  stdout: function(code) {
    stdout += stdout_converter(code);
  },

	stderr: function (code) {
    stderr += stderr_converter(code);
	}
};
