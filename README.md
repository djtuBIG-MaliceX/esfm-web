# ESFM-web

## WHAT
This is a test experiment of running leecher1337's reverse-engineered NATV.C implementation from ESFMBank (effectively the Windows MIDI driver component of the ESFM Software Synthesizer) pushing to ESFMu software core.

This will run on any decent, recent browser because who doesn't want to run all their apps in a browser these days...

## HOW
Have a look [here](https://www.codingchords.com/junk/esfm-web)

To build, all you need is Emscripten.  That is:

``` 
emsdk activate latest
compile.bat
```

Deploy index.html, a.out.js and a.out.wasm to a web server.
