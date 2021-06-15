#pragma once

#ifndef SKLIB_PRELOADED_COMMON_HEADERS
#define SKLIB_PRELOADED_COMMON_HEADERS

#include <cstdio>
#include <iostream>
#include <fstream>
#include <algorithm>
#include <string>
#include <vector>
#include <utility>
#include <type_traits>
#include <mutex>
#include <thread>
#include <chrono>

#else
#error SkLib: This cannot happen. Common Headers are loaded twice despite pragma once.
#endif

