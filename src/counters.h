#pragma once

#define xstr(s) str(s)
#define str(s) #s

#define COUNTER(x) uint32_t _ctr_##x; 
#define EXT_COUNTER(x) extern uint32_t _ctr_##x; 
#define INC(x) _ctr_##x++
#define COUNT(x) _ctr_##x
#define CLEAR(x) _ctr_##x = 0
#define JSON(js, x) js[str(x)] = COUNT(x)

