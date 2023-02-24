# msteg :floppy_disk: :mag:
A mini steganography program for hiding data in files (Currently only BMP files).

## Installing

Use `make` to compile from source.
```
git clone https://github.com/buzzqrd/msteg
cd msteg/
make
```
After making, install msteg with
```
sudo make install
```

## Usage
To list that stats of a file, run the following command:
```
msteg --stat [file]
```
Write a string into a file:
```
msteg -w [file] "string"
```
Read string out of file:
```
msteg -r [file]
```

### Writing a file into the media
To embed the file:
```
msteg -w -i [input-file] [media]
```
To extract the file:
```
msteg -r -o [output] [media]
```


## Updates
msteg is an ongoing, longterm project. In time, this project aims to support all forms of media that can encapsulate data. In the future, this project should be updated to include a better and wider range of steganographic methods. 
The current supported media types are:
- BMP (Bitmap Images)

Upcoming updates will soon include a man page and help page, as well as better support for stdin and stdout.

### Future Plans
- PNG support
- Variation in bit usage (not just the lowest order bit)
- Passcodes to change the encoding pattern

## Contributing
Contributions are welcome to this repository. The main goal of this project is to make a simple, lightweight, portable C implementation of a steganographic tool. Any contributions that help to fulfill this goal are greatly appreciated.

## License
This project is licenced under The Unlicense.
```
This is free and unencumbered software released into the public domain.

Anyone is free to copy, modify, publish, use, compile, sell, or
distribute this software, either in source code form or as a compiled
binary, for any purpose, commercial or non-commercial, and by any
means.

In jurisdictions that recognize copyright laws, the author or authors
of this software dedicate any and all copyright interest in the
software to the public domain. We make this dedication for the benefit
of the public at large and to the detriment of our heirs and
successors. We intend this dedication to be an overt act of
relinquishment in perpetuity of all present and future rights to this
software under copyright law.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
IN NO EVENT SHALL THE AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR
OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
OTHER DEALINGS IN THE SOFTWARE.

For more information, please refer to <http://unlicense.org/>
```


