/*
    ChibiOS/RT - Copyright (C) 2013-2014 Uladzimir Pylinsky aka barthess

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
    You may obtain a copy of the License at

        http://www.apache.org/licenses/LICENSE-2.0

    Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    See the License for the specific language governing permissions and
    limitations under the License.
*/

#include <cstdint>
#include <cstddef>
#include <cstdlib>
#include <cstring>

#include "memtest.h"

static unsigned int prng_seed = 42;

/*
 *
 */
template <typename T>
class Generator {
public:
  Generator(void) : pattern(0) {;}
  virtual T get(void) = 0;
  virtual testtype get_type(void) = 0;
  virtual void init(T seed) {
    pattern = seed;
  }
protected:
  T pattern;
};

/*
 *
 */
template <typename T>
class GeneratorWalkingOne : public Generator<T> {
  T get(void) {
    T ret = this->pattern;

    this->pattern <<= 1;
    if (0 == this->pattern)
      this->pattern = 1;

    return ret;
  }

  testtype get_type(void) {
    return MEMTEST_WALKING_ONE;
  }
};

/*
 *
 */
template <typename T>
class GeneratorWalkingZero : public Generator<T> {
  T get(void) {
    T ret = ~this->pattern;

    this->pattern <<= 1;
    if (0 == this->pattern)
      this->pattern = 1;

    return ret;
  }

  testtype get_type(void) {
    return MEMTEST_WALKING_ZERO;
  }
};

/*
 *
 */
template <typename T>
class GeneratorOwnAddress : public Generator<T> {
  T get(void) {
    T ret = this->pattern;
    this->pattern++;
    return ret;
  }

  testtype get_type(void) {
    return MEMTEST_OWN_ADDRESS;
  }
};

/*
 *
 */
template <typename T>
class GeneratorMovingInv : public Generator<T> {
  T get(void) {
    T ret = this->pattern;
    this->pattern = ~this->pattern;
    return ret;
  }

  testtype get_type(void) {
    if ((this->pattern == 0) || ((this->pattern & 0xFF) == 0xFF))
      return MEMTEST_MOVING_INVERSION_ZERO;
    else
      return MEMTEST_MOVING_INVERSION_55AA;
  }
};

/*
 *
 */
template <typename T>
class GeneratorMovingInvRand : public Generator<T> {
public:
  GeneratorMovingInvRand(void) : step(0), prev(0){;}
  void init(T seed) {
    srand(seed);
    step = 0;
    prev = 0;
  }

  T get(void) {
    T ret;

    if ((step & 1) == 0) {
      ret  = 0;
      ret |= rand();
      // for uint64_t we need to call rand() twice
      if (8 == sizeof(T)) {
        // multiplication used instead of 32 bit shift for warning avoidance
        ret *= 0x100000000;
        ret |= rand();
      }
      prev = ret;
    }
    else {
      ret = ~prev;
    }
    step++;

    return ret;
  }

  testtype get_type(void) {
    return MEMTEST_MOVING_INVERSION_RAND;
  }

private:
  size_t step;
  T prev;
};

/*
 *
 */
template <typename T>
static void memtest_sequential(memtest_t *testp, Generator<T> &generator, T seed) {
  const size_t steps = testp->size / sizeof(T);
  size_t i;
  T *mem = static_cast<T *>(testp->start);
  T got;
  T expect;

  /* fill ram */
  generator.init(seed);
  for (i=0; i<steps; i++)
    mem[i] = generator.get();

  /* read back and compare */
  generator.init(seed);
  for (i=0; i<steps; i++) {
    got = mem[i];
    expect = generator.get();
    if ((got != expect) && (nullptr != testp->errcb)) {
      testp->errcb(testp, generator.get_type(), i, sizeof(T), got, expect);
      return;
    }
  }
}

template <typename T>
static void walking_one(memtest_t *testp) {
  GeneratorWalkingOne<T> generator;
  memtest_sequential<T>(testp, generator, 1);
}

template <typename T>
static void walking_zero(memtest_t *testp) {
  GeneratorWalkingZero<T> generator;
  memtest_sequential<T>(testp, generator, 1);
}

template <typename T>
static void own_address(memtest_t *testp) {
  GeneratorOwnAddress<T> generator;
  memtest_sequential<T>(testp, generator, 0);
}

template <typename T>
static void moving_inversion_zero(memtest_t *testp) {
  GeneratorMovingInv<T> generator;
  T seed;
  seed = 0;
  memtest_sequential<T>(testp, generator, seed);
  seed = ~seed;
  memtest_sequential<T>(testp, generator, seed);
}

template <typename T>
static void moving_inversion_55aa(memtest_t *testp) {
  GeneratorMovingInv<T> generator;
  T seed;
  memset(&seed, 0x55, sizeof(seed));
  memtest_sequential<T>(testp, generator, seed);
  seed = ~seed;
  memtest_sequential<T>(testp, generator, seed);
}

template <typename T>
static void moving_inversion_rand(memtest_t *testp) {
  GeneratorMovingInvRand<T> generator;
  T mask = -1;
  prng_seed++;
  memtest_sequential<T>(testp, generator, prng_seed & mask);
}

/*
 *
 */
static void memtest_wrapper(memtest_t *testp,
                            void (*p_u8) (memtest_t *testp),
                            void (*p_u16)(memtest_t *testp),
                            void (*p_u32)(memtest_t *testp),
                            void (*p_u64)(memtest_t *testp)) {

  if (testp->width_mask & MEMTEST_WIDTH_8)
    p_u8(testp);

  if (testp->width_mask & MEMTEST_WIDTH_16)
    p_u16(testp);

  if (testp->width_mask & MEMTEST_WIDTH_32)
    p_u32(testp);

  if (testp->width_mask & MEMTEST_WIDTH_64)
    p_u64(testp);
}

/*
 *
 */
void memtest_run(memtest_t *testp, uint32_t testmask) {

  if (testmask & MEMTEST_WALKING_ONE) {
    memtest_wrapper(testp,
        walking_one<uint8_t>,
        walking_one<uint16_t>,
        walking_one<uint32_t>,
        walking_one<uint64_t>);
  }

  if (testmask & MEMTEST_WALKING_ZERO) {
    memtest_wrapper(testp,
        walking_zero<uint8_t>,
        walking_zero<uint16_t>,
        walking_zero<uint32_t>,
        walking_zero<uint64_t>);
  }

  if (testmask & MEMTEST_OWN_ADDRESS) {
    memtest_wrapper(testp,
        own_address<uint8_t>,
        own_address<uint16_t>,
        own_address<uint32_t>,
        own_address<uint64_t>);
  }

  if (testmask & MEMTEST_MOVING_INVERSION_ZERO) {
    memtest_wrapper(testp,
        moving_inversion_zero<uint8_t>,
        moving_inversion_zero<uint16_t>,
        moving_inversion_zero<uint32_t>,
        moving_inversion_zero<uint64_t>);
  }

  if (testmask & MEMTEST_MOVING_INVERSION_55AA) {
    memtest_wrapper(testp,
        moving_inversion_55aa<uint8_t>,
        moving_inversion_55aa<uint16_t>,
        moving_inversion_55aa<uint32_t>,
        moving_inversion_55aa<uint64_t>);
  }

  if (testmask & MEMTEST_MOVING_INVERSION_RAND) {
    memtest_wrapper(testp,
        moving_inversion_rand<uint8_t>,
        moving_inversion_rand<uint16_t>,
        moving_inversion_rand<uint32_t>,
        moving_inversion_rand<uint64_t>);
  }
}

