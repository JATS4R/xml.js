var test_xml = '../xpath-locator/test.xml';

var statusNode = document.querySelector('#status');

fetch(test_xml)
  .then(function(response) {
    return response.text();
  })
  .then(function(data) {

    statusNode.textContent = 'Locating XPaths…';

    var args = ['--xpath-locator', 'dummy.xml', 
      '/article[1]/front[1]/article-meta[1]/permissions[1]/copyright-你好[1]',
      '/article[1]/body[1]/p[2]/*:math[namespace-uri()=\'http://www.w3.org' +
        '/1998/Math/MathML\'][1]',
      '/article[1]/body[1]/sec[2]/p[2]',
      '/article[1]/body[1]/p[1]/sub[1]',
      '/article[1]/body[1]/p[1]/sub[1]/em[1]'];

    files = [{
        path: 'dummy.xml',
        data: data
    }];

    var output = xmltool(args, files);

    if (output.stderr) {
        statusNode.textContent = 'Errors:';
        document.getElementById('lint').textContent = output.stderr;
    } 
    else {
        statusNode.textContent = 
          'Done! Locations are marked in the document below with "v"s.';

        var loc_lines = output.stdout.split("\n");
        loc_lines.pop();
        var locations = loc_lines.map(function(loc_line) {
            return loc_line.split(":");
        })
        .sort(function(a, b) {
            if (a[0] == b[0] && a[1] == b[1]) return 0;
            if (a[0] < b[0] ||
                a[0] == b[0] && a[1] < b[1]) return 1;
            return -1;
        });

        function repeat_char(c, n) {
            for (var e = ''; e.length < n;)
                e += c;
            return e;
        }

        var lines = data.split("\n");
        locations.forEach(function(loc) {
            var line_num = loc[0];
            if (line_num > 0) {
                lines.splice(loc[0] - 1, 0, repeat_char(" ", loc[1] - 2) + "v");
            }
        })

        document.getElementById('xml').textContent = lines.join("\n");
    }
  });
