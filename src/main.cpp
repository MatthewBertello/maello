#include <iostream>

#include "evaluator.hpp"
#include "move_gen.hpp"
#include "position.hpp"
#include "precomputed_data.hpp"
#include "uci.hpp"
#include "utils.hpp"

#define DEBUG false

using namespace chess;

int main() {
  if (DEBUG) {
    // Debug
  } else {
    Uci uci;
    uci.Start();
  }

  return 0;
}