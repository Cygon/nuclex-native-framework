/*
MIT License

Copyright (c) 2017 James Edward Anhalt III - https://github.com/jeaiii/itoa

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#include <cstddef> // for std::size_t
#include <cstdint> // for std::std::uint32_t, std::std::uint64_t

struct pair { char t, o; };
#define P(T) T, '0',  T, '1', T, '2', T, '3', T, '4', T, '5', T, '6', T, '7', T, '8', T, '9'
static const pair s_pairs[] = { P('0'), P('1'), P('2'), P('3'), P('4'), P('5'), P('6'), P('7'), P('8'), P('9') };

#define W(N, I) *(pair*)&b[N] = s_pairs[I]
#define A(N) t = (std::uint64_t(1) << (32 + N / 5 * N * 53 / 16)) / std::uint32_t(1e##N) + 1 + N/6 - N/8, t *= u, t >>= N / 5 * N * 53 / 16, t += N / 6 * 4, W(0, t >> 32)
#define S(N) b[N] = char(std::uint64_t(10) * std::uint32_t(t) >> 32) + '0'
#define D(N) t = std::uint64_t(100) * std::uint32_t(t), W(N, t >> 32)

#define C0 b[0] = char(u) + '0'
#define C1 W(0, u)
#define C2 A(1), S(2)
#define C3 A(2), D(2)
#define C4 A(3), D(2), S(4)
#define C5 A(4), D(2), D(4)
#define C6 A(5), D(2), D(4), S(6)
#define C7 A(6), D(2), D(4), D(6)
#define C8 A(7), D(2), D(4), D(6), S(8)
#define C9 A(8), D(2), D(4), D(6), D(8)

#define L09(F) u < 100        ? L01(F) : L29(F)
#define L29(F) u < 1000000    ? L25(F) : L69(F)
#define L25(F) u < 10000      ? L23(F) : L45(F)
#define L69(F) u < 100000000  ? L67(F) : L89(F)
#define L03(F) u < 100        ? L01(F) : L23(F)

#define L01(F) u < 10         ? F(0) : F(1)
#define L23(F) u < 1000       ? F(2) : F(3)
#define L45(F) u < 100000     ? F(4) : F(5)
#define L67(F) u < 10000000   ? F(6) : F(7)
#define L89(F) u < 1000000000 ? F(8) : F(9)

#define POS(N) (N < length ? C##N, N + 1 : N + 1)
#define NEG(N) (N + 1 < length ? *b++ = '-', C##N, N + 2 : N + 2)

std::size_t to_chars_jeaiii(char *b, std::size_t length, std::uint32_t u) {
  std::uint64_t t;
  return L09(POS);
}

std::size_t to_chars_jeaiii(char *b, std::size_t length, std::int32_t i) {
  std::uint64_t t;
  std::uint32_t u = i;
  return i < 0 ? u = 0 - u, L09(NEG) : L09(POS);
}

std::size_t to_chars_jeaiii(char *b, std::size_t length, std::uint64_t n) {
  std::size_t count;
  std::uint32_t u = std::uint32_t(n);
  std::uint64_t t;

  if(u == n)
    return L09(POS);

  std::uint64_t a = n / 100000000;

  if(std::uint32_t(a) == a) {
    u = std::uint32_t(a);
    b += count = L09(POS);
    count += 8;
    if(count > length)
      return count;
  } else {
    u = std::uint32_t(a / 100000000);
    b += count = L03(POS);
    count += 16;
    if(count > length)
      return count;

    u = a % 100000000;
    C7;
    b += 8;
  }

  u = n % 100000000;
  C7;
  return count;
}

std::size_t to_chars_jeaiii(char *b, std::size_t length, std::int64_t i) {
  return i < 0
    ? to_chars_jeaiii(b + 1, length > 0 ? b[0] = '-', length - 1 : 0, std::uint64_t(0) - std::uint64_t(i)) + 1
    : to_chars_jeaiii(b, length, std::uint64_t(i));
}
