'use strict';

function generateBmp(canvas, depth) {
  depth = depth || 1; 
  var ctx = canvas.getContext('2d');
  var imagedata = ctx.getImageData(0, 0, canvas.width, canvas.height).data;
  var b = []; 

  for (var i = canvas.height - 1; i >= 0; i--) {
    
    var rowArr = [];
    for (var j = 0; j < canvas.width; j++) {
      
      var index = i * canvas.width * 4 + j * 4; 
      
      if (depth <= 8) {
        var gap = Math.floor(255 / (Math.pow(2, depth) - 1));
        
        var paletteIndex = Math.floor((imagedata[index] + imagedata[index + 1] + imagedata[index + 2]) / gap / 3);
        rowArr.push(paletteIndex);
      }
      
      else if (depth == 16) {
        var red = Math.ceil(imagedata[index] / 255 * 0x1f);
        var green = Math.ceil(imagedata[index + 1] / 255 * 0x1f);
        var blue = Math.ceil(imagedata[index + 2] / 255 * 0x1f);

        rowArr.push(((green & 0x07) << 5) + blue);
        rowArr.push(0x00 + (red << 2) + (green >> 3));
      }
      
      else {
        var alpha = (depth == 32 ? imagedata[index + 3] : 255) / 255;
        
        rowArr.push(imagedata[index + 2] * alpha + (1 - alpha) * 255);
        rowArr.push(imagedata[index + 1] * alpha + (1 - alpha) * 255);
        rowArr.push(imagedata[index] * alpha + (1 - alpha) * 255);
        depth == 32 && rowArr.push(imagedata[index + 3]); 
      }
    }
    
    if (depth <= 8) {
      var tmpArr = [];
      var colorIn8Bits = Math.floor(8 / depth);
      for (var k = 0, len = rowArr.length; k < len; k += colorIn8Bits) {
        var _byte = 0;
        for (var l = 0; l < colorIn8Bits; l += 1) {
          _byte += rowArr[k + l] << (colorIn8Bits - l - 1);
        }
        tmpArr.push(_byte);
      }
      rowArr = tmpArr;
    }

    var totalRowArrLength = Math.floor((depth * canvas.width + 31) / 32) * 4; 
    
    while (totalRowArrLength > rowArr.length) {
      rowArr.push(0);
    }
    b = b.concat(rowArr);
  }

  return createBmpHeader(canvas.height, depth, canvas.width, b);
}

function createBmpHeader(height, depth, width, arr) {
    var offset, height, data;

    function conv(size) {
        return String.fromCharCode(size & 0xff, (size >> 8) & 0xff, (size >> 16) & 0xff, (size >> 24) & 0xff);
    }

    offset = depth <= 8 ? 54 + Math.pow(2, depth) * 4 : 54;

    data = 'BM';
    data += conv(offset + Math.ceil(width * height * depth / 8));
    data += conv(0);
    data += conv(offset);

    data += conv(40);
    data += conv(width);
    data += conv(height);
    data += String.fromCharCode(1, 0);
    data += String.fromCharCode(depth, 0);
    data += conv(0);
    data += conv(arr.length);
    data += conv(0);
    data += conv(0);
    data += conv(0);
    data += conv(0);

    if (depth <= 8) {
        data += conv(0);

        for (var s = Math.floor(255 / (Math.pow(2, depth) - 1)), i = s; i < 256; i += s) {
            data += conv(i + i * 256 + i * 65536);
        }
    }

    for (var i = 0, len = arr.length; i < len; i++) {
        data += String.fromCharCode(arr[i]);
    }
    return data;
}

function createDataUri(data) {
  return 'data:image/bmp;base64,' + btoa(data);
}
