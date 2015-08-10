var xmlPath = 'test-xmllint.xml';
var dtdPath = 'JATS-journalpublishing1.dtd';

var statusNode = document.querySelector('#status');

var fetchFile = function(file, root) {
	return fetch(root + file).then(function(response) {
		return response.text().then(function(data) {
			return {
				path: file,
				data: data
			}
		});
	});
};

statusNode.textContent = 'Fetching DTD files…';

requests = [
  fetchFile(xmlPath, './'),
  fetchFile(dtdPath, './')
];

Promise.all(requests)
  .then(function(files) {
  	statusNode.textContent = 'Validating XML…';

  	var args = ['--noent', '--dtdvalid', dtdPath, xmlPath];

  	var output = xmltool(args, files);

  	if (output.stderr) {
  		statusNode.textContent = 'Validation errors:';
  		document.getElementById('lint').textContent = output.stderr;
  	} else {
  		statusNode.textContent = 'Valid!';
  		document.getElementById('xml').textContent = output.stdout;
  	}
  });
