@echo off
:: requires Emscripten emsdk with emcc already activated
emcc esfm.c esfm_registers.c esfmu_helper.c natv.c emscripten_wrapper.c -std=c11 -s EXPORTED_RUNTIME_METHODS=ccall,cwrap   -s EXPORTED_FUNCTIONS="['_Initialize', '_Reset', '_WriteMidiData', '_GetSample', '_GetSampleTwo', '_PlaySysex', '_CloseInstance', '_malloc']" --embed-file banks