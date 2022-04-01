#pragma once

#ifdef DEBUG
#define SPR_DLOG(...) printf(__VA_ARGS__)
#else
#define SPR_DLOG(...) (void)(0)
#endif
