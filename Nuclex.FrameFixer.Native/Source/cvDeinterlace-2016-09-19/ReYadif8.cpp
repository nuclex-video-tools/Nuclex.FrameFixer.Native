#include <vector>
#include <memory>
#include <cstring>
#include <cstdint>

#define MIN(a,b) ((a) > (b) ? (b) : (a))
#define MAX(a,b) ((a) < (b) ? (b) : (a))
#define ABS(a) ((a) > 0 ? (a) : (-(a)))

#define MIN3(a,b,c) MIN(MIN(a,b),c)
#define MAX3(a,b,c) MAX(MAX(a,b),c)

// it suppose calc_spatialDif and calc_spatialScore will be optimize
// by compiler that use SIMD instruction
void calc_spatialDif(
  const std::uint8_t *cur_prev,    
  const std::uint8_t *cur_next,
  std::int16_t *dst,
  int width,
  int shift
) {
  std::int16_t b = ABS(shift);
  cur_prev += shift;
  cur_next -= shift;
  dst += b;

  for(int x = b; x < width - b; ++x) {
    *dst = ABS((*cur_prev) - (*cur_next));
    ++dst;
    ++cur_prev; 
    ++cur_next;
  }
}

void calc_spatialDif(
  const std::uint16_t *cur_prev,    
  const std::uint16_t *cur_next,
  std::int32_t *dst,
  int width,
  int shift
) {
  std::int32_t b = ABS(shift);
  cur_prev += shift;
  cur_next -= shift;
  dst += b;

  for(int x = b; x < width - b; ++x) {
    *dst = ABS((*cur_prev) - (*cur_next));
    ++dst;
    ++cur_prev; 
    ++cur_next;
  }
}

// dif       0 1 2 3 4 5 6 7
// dif+1     1 2 3 4 5 6 7
// dif+2     2 3 4 5 6 7
// score   s s s s s s s s
void calc_spatialScore(
  std::int16_t *dif,
  std::int16_t *score,
  int width
) {
  // CHECK: Doesn't integer promotion leave dif1 as std::int16_t and dif2, dif3 as int?
  auto dif1 = dif;
  auto dif2 = dif + 1;
  auto dif3 = dif + 2;
  score += 1;
    
  for(int i = 1; i < width - 1; ++i) {
    *score = ((*dif1) + (*dif2) + (*dif3));

    ++score;
    ++dif1;
    ++dif2;
    ++dif3;
  }
}

void calc_spatialScore(
  std::int32_t *dif,
  std::int32_t *score,
  int width
) {
  // CHECK: Doesn't integer promotion leave dif1 as std::int32_t and dif2, dif3 as int?
  auto dif1 = dif;
  auto dif2 = dif + 1;
  auto dif3 = dif + 2;
  score += 1;
    
  for(int i = 1; i < width - 1; ++i) {
    *score = ((*dif1) + (*dif2) + (*dif3));

    ++score;
    ++dif1;
    ++dif2;
    ++dif3;
  }
}

// TODO: Gobal variables. Goodbye thread safety?
int spatialScoreValue_MaxWidth = 0;

std::unique_ptr<std::int16_t[]> spatial16DifN2; // -2
std::unique_ptr<std::int16_t[]> spatial16DifN1; // -1
std::unique_ptr<std::int16_t[]> spatial16Dif0;  //  0
std::unique_ptr<std::int16_t[]> spatial16DifP1; //  1
std::unique_ptr<std::int16_t[]> spatial16DifP2; //  2

std::unique_ptr<std::int16_t[]> spatial16ScoreN2; // -2
std::unique_ptr<std::int16_t[]> spatial16ScoreN1; // -1
std::unique_ptr<std::int16_t[]> spatial16Score0;  //  0
std::unique_ptr<std::int16_t[]> spatial16ScoreP1; //  1
std::unique_ptr<std::int16_t[]> spatial16ScoreP2; //  2

/// <summary>Deinterlaces a single scan line with the Yadif algorithm</summary>
/// <param name="mode">
///   Unclear, is only checked for being less than 2. Possible meanings:
///   0 = single frame rate, temporal and spatial interlacing check (default).
///   1 = double frame rate, temporal and spatial interlacing check.
///   2 = single frame rate, skips spatial interlacing check.
///   3 = double frame rate, skips spatial interlacing check. 
/// </param>
/// <param name="dst">Buffer in which the deinterlaced scanline is deposited</param>
/// <param name="prev">Scanline that is preceding the scanline being deinterlaced</param>
/// <param name="cur">Scanline that is to be deinterlaced (and contains garbage)</param>
/// <param name="next">Scanline that is followed the scanline being deinterlaced</param>
/// <param name="w">Width of the scanline in pixels, not bytes</param>
/// <param name="step1">Number of bytes to go forward to reach the next pixel</param>
/// <param name="parity">Whether this is an even or an odd scanline</param>
void ReYadif1Row(
  int mode,
  std::uint8_t *dst,
  const std::uint8_t *prev, const std::uint8_t *cur, const std::uint8_t *next,
  int w, int step1, int parity
) {
  const std::uint8_t *prev2 = parity ? prev : cur;
  const std::uint8_t *next2 = parity ? cur : next;

  if(w > spatialScoreValue_MaxWidth) {
    spatialScoreValue_MaxWidth = w;
    spatial16DifN2.reset(new std::int16_t[w]);
    spatial16DifN1.reset(new std::int16_t[w]);
    spatial16Dif0.reset(new std::int16_t[w]);
    spatial16DifP1.reset(new std::int16_t[w]);
    spatial16DifP2.reset(new std::int16_t[w]);
    spatial16ScoreN2.reset(new std::int16_t[w]);
    spatial16ScoreN1.reset(new std::int16_t[w]);
    spatial16Score0.reset(new std::int16_t[w]);
    spatial16ScoreP1.reset(new std::int16_t[w]);
    spatial16ScoreP2.reset(new std::int16_t[w]);
  }

  // pre-calculate spatial score, can be optimize by SIMD
  calc_spatialDif((std::uint8_t*)cur - step1, (std::uint8_t*)cur + step1, spatial16DifN2.get(), w, -2);
  calc_spatialDif((std::uint8_t*)cur - step1, (std::uint8_t*)cur + step1, spatial16DifN1.get(), w, -1);
  calc_spatialDif((std::uint8_t*)cur - step1, (std::uint8_t*)cur + step1, spatial16Dif0.get(),  w,  0);
  calc_spatialDif((std::uint8_t*)cur - step1, (std::uint8_t*)cur + step1, spatial16DifP1.get(), w, +1);
  calc_spatialDif((std::uint8_t*)cur - step1, (std::uint8_t*)cur + step1, spatial16DifP2.get(), w, +2);
  calc_spatialScore(spatial16DifN2.get(), spatial16ScoreN2.get(), w);
  calc_spatialScore(spatial16DifN1.get(), spatial16ScoreN1.get(), w);
  calc_spatialScore(spatial16Dif0.get(),  spatial16Score0.get(),  w);
  calc_spatialScore(spatial16DifP1.get(), spatial16ScoreP1.get(), w);
  calc_spatialScore(spatial16DifP2.get(), spatial16ScoreP2.get(), w);

  // CHECK: Off by 1? Shouldn't this start at 2?
  for(int x = 3; x < w-3; x++) {
    int minScore = spatial16Score0.get()[x];
    int spatial_pred = (cur[-step1 + 0] + cur[+step1 + 0]) / 2;

    if(minScore > spatial16ScoreN1.get()[x]) {
      minScore = spatial16ScoreN1.get()[x];
      spatial_pred = (cur[-step1 + -1] + cur[+step1 + -1]) / 2;

      if(minScore > spatial16ScoreN2.get()[x]) {
        minScore = spatial16ScoreN2.get()[x];
        spatial_pred = (cur[-step1 + -2] + cur[+step1 + -2]) / 2;
      }
    }

    if(minScore > spatial16ScoreP1.get()[x]) {
      minScore = spatial16ScoreP1.get()[x];
      spatial_pred = (cur[-step1 + 1] + cur[+step1 + 1]) / 2;
      if(minScore > spatial16ScoreP2.get()[x]) {
        minScore = spatial16ScoreP2.get()[x];
        spatial_pred = (cur[-step1 + 2] + cur[+step1 + 2]) / 2;
      }
    }

    int c = cur[-step1];
    int d = (prev2[0] + next2[0]) / 2;
    int e = cur[+step1];
    int temporal_diff0 = ABS(prev2[0] - next2[0]) / 2;
    int temporal_diff1 = (ABS(prev[-step1] - cur[-step1]) + ABS(prev[+step1] - cur[+step1])) / 2;
    int temporal_diff2 = (ABS(next[-step1] - cur[-step1]) + ABS(next[+step1] - cur[+step1])) / 2;
    int diff = MAX3(temporal_diff0, temporal_diff1, temporal_diff2);

    if(mode < 2) {
      int b = (prev2[-2 * step1] + next2[-2 * step1]) >> 1;
      int f = (prev2[+2 * step1] + next2[+2 * step1]) >> 1;
#if 0
      int a = cur[-3 * step1];
      int g = cur[+3 * step1];
      int max = MAX3(d - e, d - c, MIN3(MAX(b - c, f - e), MAX(b - c, b - a), MAX(f - g, f - e)));
      int min = MIN3(d - e, d - c, MAX3(MIN(b - c, f - e), MIN(b - c, b - a), MIN(f - g, f - e)));
#else
      int max = MAX3(d - e, d - c, MIN(b - c, f - e));
      int min = MIN3(d - e, d - c, MAX(b - c, f - e));
#endif

      diff = MAX3(diff, min, -max);
    }
    if(spatial_pred > d + diff) {
      spatial_pred = d + diff;
    } else if (spatial_pred < d - diff) {
      spatial_pred = d - diff;
    }

    *dst = spatial_pred;

    ++dst;
    ++cur;
    ++prev;
    ++next;
    ++prev2;
    ++next2;
  }
}

std::unique_ptr<std::int32_t[]> spatial32DifN2; // -2
std::unique_ptr<std::int32_t[]> spatial32DifN1; // -1
std::unique_ptr<std::int32_t[]> spatial32Dif0;  //  0
std::unique_ptr<std::int32_t[]> spatial32DifP1; //  1
std::unique_ptr<std::int32_t[]> spatial32DifP2; //  2

std::unique_ptr<std::int32_t[]> spatial32ScoreN2; // -2
std::unique_ptr<std::int32_t[]> spatial32ScoreN1; // -1
std::unique_ptr<std::int32_t[]> spatial32Score0;  //  0
std::unique_ptr<std::int32_t[]> spatial32ScoreP1; //  1
std::unique_ptr<std::int32_t[]> spatial32ScoreP2; //  2

void ReYadif1Row(
  int mode,
  std::uint16_t *dst,
  const std::uint16_t *prev, const std::uint16_t *cur, const std::uint16_t *next,
  int w, int step1, int parity
) {
  const std::uint16_t *prev2 = parity ? prev : cur;
  const std::uint16_t *next2 = parity ? cur : next;

  if(w > spatialScoreValue_MaxWidth) {
    spatialScoreValue_MaxWidth = w;
    spatial32DifN2.reset(new std::int32_t[w]);
    spatial32DifN1.reset(new std::int32_t[w]);
    spatial32Dif0.reset(new std::int32_t[w]);
    spatial32DifP1.reset(new std::int32_t[w]);
    spatial32DifP2.reset(new std::int32_t[w]);
    spatial32ScoreN2.reset(new std::int32_t[w]);
    spatial32ScoreN1.reset(new std::int32_t[w]);
    spatial32Score0.reset(new std::int32_t[w]);
    spatial32ScoreP1.reset(new std::int32_t[w]);
    spatial32ScoreP2.reset(new std::int32_t[w]);
  }

  // pre-calculate spatial score, can be optimize by SIMD
  calc_spatialDif((std::uint16_t *)cur - step1, (std::uint16_t *)cur + step1, spatial32DifN2.get(), w, -2);
  calc_spatialDif((std::uint16_t *)cur - step1, (std::uint16_t *)cur + step1, spatial32DifN1.get(), w, -1);
  calc_spatialDif((std::uint16_t *)cur - step1, (std::uint16_t *)cur + step1, spatial32Dif0.get(),  w,  0);
  calc_spatialDif((std::uint16_t *)cur - step1, (std::uint16_t *)cur + step1, spatial32DifP1.get(), w, +1);
  calc_spatialDif((std::uint16_t *)cur - step1, (std::uint16_t *)cur + step1, spatial32DifP2.get(), w, +2);
  calc_spatialScore(spatial32DifN2.get(), spatial32ScoreN2.get(), w);
  calc_spatialScore(spatial32DifN1.get(), spatial32ScoreN1.get(), w);
  calc_spatialScore(spatial32Dif0.get(),  spatial32Score0.get(),  w);
  calc_spatialScore(spatial32DifP1.get(), spatial32ScoreP1.get(), w);
  calc_spatialScore(spatial32DifP2.get(), spatial32ScoreP2.get(), w);

  // CHECK: Off by 1? Shouldn't this start at 2?
  for(int x = 3; x < w-3; x++) {
    int minScore = spatial32Score0.get()[x];
    int spatial_pred = (cur[-step1 + 0] + cur[+step1 + 0]) / 2;

    if(minScore > spatial32ScoreN1.get()[x]) {
      minScore = spatial32ScoreN1.get()[x];
      spatial_pred = (cur[-step1 + -1] + cur[+step1 + -1]) / 2;

      if(minScore > spatial32ScoreN2.get()[x]) {
        minScore = spatial32ScoreN2.get()[x];
        spatial_pred = (cur[-step1 + -2] + cur[+step1 + -2]) / 2;
      }
    }

    if(minScore > spatial32ScoreP1.get()[x]) {
      minScore = spatial32ScoreP1.get()[x];
      spatial_pred = (cur[-step1 + 1] + cur[+step1 + 1]) / 2;
      if(minScore > spatial32ScoreP2.get()[x]) {
        minScore = spatial32ScoreP2.get()[x];
        spatial_pred = (cur[-step1 + 2] + cur[+step1 + 2]) / 2;
      }
    }

    int c = cur[-step1];
    int d = (prev2[0] + next2[0]) / 2;
    int e = cur[+step1];
    int temporal_diff0 = ABS(prev2[0] - next2[0]) / 2;
    int temporal_diff1 = (ABS(prev[-step1] - cur[-step1]) + ABS(prev[+step1] - cur[+step1])) / 2;
    int temporal_diff2 = (ABS(next[-step1] - cur[-step1]) + ABS(next[+step1] - cur[+step1])) / 2;
    int diff = MAX3(temporal_diff0, temporal_diff1, temporal_diff2);

    if(mode < 2) {
      int b = (prev2[-2 * step1] + next2[-2 * step1]) >> 1;
      int f = (prev2[+2 * step1] + next2[+2 * step1]) >> 1;
#if 0
      int a = cur[-3 * step1];
      int g = cur[+3 * step1];
      int max = MAX3(d - e, d - c, MIN3(MAX(b - c, f - e), MAX(b - c, b - a), MAX(f - g, f - e)));
      int min = MIN3(d - e, d - c, MAX3(MIN(b - c, f - e), MIN(b - c, b - a), MIN(f - g, f - e)));
#else
      int max = MAX3(d - e, d - c, MIN(b - c, f - e));
      int min = MIN3(d - e, d - c, MAX(b - c, f - e));
#endif

      diff = MAX3(diff, min, -max);
    }
    if(spatial_pred > d + diff) {
      spatial_pred = d + diff;
    } else if (spatial_pred < d - diff) {
      spatial_pred = d - diff;
    }

    *dst = spatial_pred;

    ++dst;
    ++cur;
    ++prev;
    ++next;
    ++prev2;
    ++next2;
  }
}

/// <summary>Interpolates two scan lines and writes the result into a third</summary>
/// <param name="dst">Buffer into which the interpolated pixels will be written</param>
/// <param name="src1">Buffer containing of the first input scanline</param>
/// <param name="src2">Buffer containing of the second input scanline</param>
/// <param name="width">Width of the scanline in bytes</param>
/// <remarks>
///   Interpolates based on bytes, so only 8 bit pixel formats will work.
/// </remarks>
static void interpolate(std::uint8_t *dst, const std::uint8_t *src1, const std::uint8_t *src2, int width) {    
  for(int i = 0; i < width; ++i) {
    *dst = ((*src1) + (*src2)) / 2;
    ++dst;
    ++src1;
    ++src2;
  }
}

void ReYadif1Channel(
  int mode,
  std::uint8_t *dst,
  const std::uint8_t *prev0, const std::uint8_t *cur0, const std::uint8_t *next0, int step1,
  int w, int h,
  int parity, int tff
) {
  memset(dst, 0, step1 * 2); // first two line fill blank

  dst += step1 * 2;    
  prev0 += step1 * 2;
  cur0  += step1 * 2;
  next0 += step1 * 2;

  for(int y = 2; y < h - 2; y++) {
    if(((y ^ parity) & 1)) {
      ReYadif1Row(mode, dst, prev0, cur0, next0, w, step1, (parity ^ tff));
    } else {
      memcpy(dst, cur0, w); // copy original
    }

    dst += step1;
    prev0 += step1;
    cur0 += step1;
    next0 += step1;
  }

  memset(dst + ((h - 2)*step1), 0, step1 * 2); // last two line fill blank
}