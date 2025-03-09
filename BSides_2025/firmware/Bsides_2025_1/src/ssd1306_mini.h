#pragma once

#ifdef __cplusplus
extern "C" {
#endif

void oledDrawText(int x, int y, const char* str, int color, int scale);
void oledInit();
void oledClearBuffer(int color);
void oledRefresh();

#ifdef __cplusplus
}
#endif
