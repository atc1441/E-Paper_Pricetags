#include <Arduino.h>
#include <SPI.h>
#include "RFV3.h"
#include "main_variables.h"
#include "logger.h"
#include "cc1101_spi.h"
#include "cc1101.h"
#include "class.h"
#include "interval_timer.h"
#include "compression.h"

// Many thanks to the performance whisperer for this code https://twitter.com/fast_code_r_us :)
int compressImageNONE(uint8_t *pData, int iOffset, int iLen)
{
  int width, height;
  int x, y, offset;
  uint8_t c, cOld, *s, *d, *pBitmap, *pOut, *pTemp;
  uint8_t *pStart, *pEnd;
  uint16_t u16Check;
  int iCount; // repeat and non-repeat counts
  int pitch, bsize;
  int iSize, iCompressedSize, bTopDown = 1;

  iSize = iLen - iOffset; // size of bitmap file
  pTemp = (uint8_t *)malloc(iSize);
  Serial.printf("Allocated %d bytes for the temp bitmap\n", iSize);
  pBitmap = &pData[iOffset];
  pOut = pData; // overwrite the input HTML data with the compressed data

  if (pBitmap[0] != 'B' || pBitmap[1] != 'M' || pBitmap[14] < 0x28) {
    free(pTemp);
    Serial.println("Not a Windows BMP file!\n");
    return 0;
  }
  width = *(int32_t *)&pBitmap[18];
  height = *(int32_t *)&pBitmap[22];
  offset = *(int32_t *)&pBitmap[10]; // offset to bits
  bsize = (width + 7) / 8;
  pitch = (bsize + 3) & 0xfffc; // DWORD aligned

  if (height > 0) // bottom up
  {
    bTopDown = 0;
  }
  else
  {
    height = 0 - height;
  }
  printf("input bitmap size: %d x %d\n", width, height);

  // Fix the bitmap to be a continuous stream
  d = pTemp;
  u16Check = 0;
  for (y = 0; y < height; y++)
  {
    if (bTopDown)
      s = &pBitmap[offset + (pitch * y)];
    else
      s = &pBitmap[offset + (height - 1 - y) * pitch];
    memcpy(d, s, bsize);
    for (x = 0; x < bsize; x++) {
      u16Check += s[x]; // calculate checksum of uncompressed pixels
    }
    d += bsize;
  }
  Serial.println("Copied image to temp buffer");

  // Prepare the output data
  pOut[0] = 0x83; pOut[1] = 0x19;
  pOut[2] = 0;
  pOut[3] = (uint8_t)width;
  pOut[4] = (uint8_t)(width >> 8);
  pOut[5] = (uint8_t)height;
  pOut[6] = (uint8_t)(height >> 8);
  // 7,8,9.10 = compressed data size
  pOut[11] = pOut[12] = pOut[13] = pOut[14] = 0;
  pOut[15] = pOut[16] = pOut[17] = pOut[18] = 0;
  pOut[19] = pOut[20] = pOut[21] = 0;
  // 22 = checksum
  pOut[23] = 0; // type of compression
  pOut[24] = pOut[25] = pOut[26] = 0;
  pOut[27] = 0x84;
  // 28 = high byte of length XOR 0x80
  // 29 = low byte of length
  // 30+ compressed data

  pEnd = pTemp + (height * bsize);
  d = &pOut[30];

  pStart = s = pTemp;
  while (s < pEnd)
  {
    *d++ = *s++;
  }

  // Finalize data packet
  u16Check = 0x100 - u16Check;
  pOut[22] = (uint8_t)u16Check;
  iCompressedSize = (int)(d - &pOut[30]);
  pOut[7] = (uint8_t)(iCompressedSize >> 0);
  pOut[8] = (uint8_t)(iCompressedSize >> 8);
  pOut[9] = (uint8_t)(iCompressedSize >> 16);
  pOut[10] = (uint8_t)(iCompressedSize >> 24);
  pOut[28] = pOut[8] ^ 0x80;
  pOut[29] = pOut[7];

  // trailing 'command'
  *d++ = 0x85;
  *d++ = 0x05;
  *d++ = 0x08;
  *d++ = 0x00;
  *d++ = 0x00;
  *d++ = 0x01;
  *d++ = 0x01;

  iSize = (int)(d - pOut);
  Serial.printf("Created %d bytes of output\n", iSize);
  free(pTemp);

  return iSize;
} /* compressImage() */

int compressImageRLE(uint8_t *pData, int iOffset, int iLen)
{
  int width, height;
  int x, y, offset;
  uint8_t c, cOld, *s, *d, *pBitmap, *pOut, *pTemp;
  uint8_t *pStart, *pEnd;
  uint16_t u16Check;
  int iCount; // repeat and non-repeat counts
  int pitch, bsize;
  int iSize, iCompressedSize, bTopDown = 1;

  iSize = iLen - iOffset; // size of bitmap file
  pTemp = (uint8_t *)malloc(iSize);
  Serial.printf("Allocated %d bytes for the temp bitmap\n", iSize);
  pBitmap = &pData[iOffset];
  pOut = pData; // overwrite the input HTML data with the compressed data

  if (pBitmap[0] != 'B' || pBitmap[1] != 'M' || pBitmap[14] < 0x28) {
    free(pTemp);
    Serial.println("Not a Windows BMP file!\n");
    return 0;
  }
  width = *(int32_t *)&pBitmap[18];
  height = *(int32_t *)&pBitmap[22];
  offset = *(int32_t *)&pBitmap[10]; // offset to bits
  bsize = (width + 7) / 8;
  pitch = (bsize + 3) & 0xfffc; // DWORD aligned

  if (height > 0) // bottom up
  {
    bTopDown = 0;
  }
  else
  {
    height = 0 - height;
  }
  printf("input bitmap size: %d x %d\n", width, height);

  // Fix the bitmap to be a continuous stream
  d = pTemp;
  u16Check = 0;
  for (y = 0; y < height; y++)
  {
    if (bTopDown)
      s = &pBitmap[offset + (pitch * y)];
    else
      s = &pBitmap[offset + (height - 1 - y) * pitch];
    memcpy(d, s, bsize);
    for (x = 0; x < bsize; x++) {
      u16Check += s[x]; // calculate checksum of uncompressed pixels
    }
    d += bsize;
  }
  Serial.println("Copied image to temp buffer");
  // Prepare the output data
  pOut[0] = 0x83; pOut[1] = 0x19;
  pOut[2] = 0;
  pOut[3] = (uint8_t)width;
  pOut[4] = (uint8_t)(width >> 8);
  pOut[5] = (uint8_t)height;
  pOut[6] = (uint8_t)(height >> 8);
  // 7,8,9.10 = compressed data size
  pOut[11] = pOut[12] = pOut[13] = pOut[14] = 0;
  pOut[15] = pOut[16] = pOut[17] = pOut[18] = 0;
  pOut[19] = pOut[20] = pOut[21] = 0;
  // 22 = checksum
  pOut[23] = 1; // type of compression
  pOut[24] = pOut[25] = pOut[26] = 0;
  pOut[27] = 0x84;
  // 28 = high byte of length XOR 0x80
  // 29 = low byte of length
  // 30+ compressed data

  pEnd = pTemp + (height * bsize);
  d = &pOut[30];
  // Compress the data
  iCount = 1;
  pStart = s = pTemp;
  cOld = *s++;
  while (s < pEnd)
  {
    c = *s++;
    if (c == cOld)
    {
      iCount++;
      continue;
    }
    // count consecutive non-repeats
    if (iCount == 1) /* Any repeats? */
    {
      s = pStart;
      goto encpb1; /* Look for consecutive, non-repeats */
    }
    s--; // back up over the non-repeating byte
    while (iCount > 126)
    {
      *d++ = 0xff; /* Store max count */
      *d++ = cOld; /* Store the repeating byte */
      iCount -= 127;
    }
    if (iCount) /* any remaining count? */
    {
      *d++ = (unsigned char)(iCount | 0x80);
      *d++ = cOld;
    }
    pStart = s;
encpb1:
    /* Find # of consec non-repeats */
    if (s == pEnd) /* Done with the data? */
      break;
    /* Count non-repeats */
    cOld = *s++; /* Get the compare byte */
    iCount = 1; /* Assume 1 non-repeat to start */
    while (s < pEnd)
    {
      c = *s++;
      if (c == cOld)
        break;
      cOld = c;
      iCount++;
    }
    iCount--;
    s--;
    // encode non-repeats
    while (iCount > 127)
    {
      *d++ = 127;
      memcpy(d, pStart, 127);
      d += 127;
      pStart += 127;
      iCount -= 127;
    }
    if (iCount)
    {
      *d++ = (uint8_t)iCount;
      memcpy(d, pStart, iCount);
      d += iCount;
      pStart += iCount;
    }
    iCount = 1;
  } // while encoding
  // finish storing last repeat
  while (iCount > 126)
  {
    *d++ = 0xff; /* Store max count */
    *d++ = cOld; /* Store the repeating byte */
    iCount -= 127;
  }
  if (iCount) /* any remaining count? */
  {
    *d++ = (unsigned char)(iCount | 0x80);
    *d++ = cOld;
  }

  // Finalize data packet
  u16Check = 0x100 - u16Check;
  pOut[22] = (uint8_t)u16Check;
  iCompressedSize = (int)(d - &pOut[30]);
  pOut[7] = (uint8_t)(iCompressedSize >> 0);
  pOut[8] = (uint8_t)(iCompressedSize >> 8);
  pOut[9] = (uint8_t)(iCompressedSize >> 16);
  pOut[10] = (uint8_t)(iCompressedSize >> 24);
  pOut[28] = pOut[8] ^ 0x80;
  pOut[29] = pOut[7];

  // trailing 'command'
  *d++ = 0x85;
  *d++ = 0x05;
  *d++ = 0x08;
  *d++ = 0x00;
  *d++ = 0x00;
  *d++ = 0x01;
  *d++ = 0x01;

  iSize = (int)(d - pOut);
  Serial.printf("Created %d bytes of output\n", iSize);
  free(pTemp);
  return iSize;
} /* compressImageRLE() */
