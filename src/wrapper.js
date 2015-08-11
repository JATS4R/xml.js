var xmltool = function (args, files) {

  function make_utf8_converter() {
    var bytes_left = 0;
    var acc;

    return function(code) {
      code &= 0xFF;
      if (!bytes_left) {
        if (code < 128) {
          acc = code;
        }
        else {
          while ( (code = (code << 1) & 0xFF) >= 128 ) bytes_left++;
          acc = code >> (bytes_left + 1);
        }
      }
      else {
        bytes_left--;
        acc = (acc << 6) | (code & 0x3F);
      }
      //console.log("code = 0x" + code.toString(16) + 
      //            ", bytes_left = " + bytes_left + 
      //            ", acc = 0x" + acc.toString(16));
      return bytes_left ? '' : String.fromCharCode(acc);
    };
  }

  var stdout = '';
  var stdout_converter = make_utf8_converter();
  var stderr = '';
  var stderr_converter = make_utf8_converter();



  /* XMLLINT.RAW.JS */

  return {
    stdout: stdout,
    stderr: stderr,
  };
}
