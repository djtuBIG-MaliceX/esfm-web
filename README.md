# ESFM-web

## WHAT
This is a test experiment of running leecher1337's reverse-engineered NATV.C implementation from ESFMBank (effectively the Windows MIDI driver component of the ESFM MIDI Synthesizer) pushing to ESFMu software core by Kagamiin~.

This will run on any decent, recent browser because who doesn't want to run all their apps in a browser these days...

## HOW
Have a look [here](https://www.codingchords.com/junk/webmidi/esfm-web/)

To build, all you need is Emscripten.  That is:

```
emsdk activate latest
compile.bat
```

Deploy `index.html` and `a.out.*` files to a web server.
Note:  AudioWorklet requires the following headers set on HTTPS server:
```
# Example, for Nginx
add_header Cross-Origin-Opener-Policy "same-origin" always;
add_header Cross-Origin-Embedder-Policy "require-corp" always;
```