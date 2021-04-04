#pragma once

int compressImageRLE(uint8_t *pData, int iOffset, int iLen);
int compressBufferRLE(uint8_t *pIn, int iSize, uint8_t *pOut);
