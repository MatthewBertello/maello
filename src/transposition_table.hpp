#pragma once
#ifndef TRANSPOSITION_TABLE_HPP
#define TRANSPOSITION_TABLE_HPP
#include <cstdint>
#include <vector>

#include "utils.hpp"

namespace chess {

namespace move {
using Move = uint32_t;
}

enum TTFlags {
  kExactHashFlag = 0,
  kAlphaHashFlag = 1,
  kBetaHashFlag = 2,
  kNoHashFlag = 3,
};

struct TTEntry {
  uint64_t key = 0;
  int depth = -1;
  TTFlags flags = kNoHashFlag;
  int score = kUnknownScore;
  move::Move bestMove = 0;
};

constexpr int kTTEntrySize = sizeof(TTEntry);

class TranspositionTable {
 public:
  // The size of the table in megabytes
  unsigned long int size_ = kDefaultTranspositionTableSize;
  unsigned long int max_entries_ = (size_ * 1024 * 1024) / kTTEntrySize;
  unsigned long int num_entries_ = 0;

  std::vector<TTEntry> table_;

  TranspositionTable();

  // Sets the size of the table in megabytes
  // @param size The size of the table in megabytes
  void ChangeSize(int size);

  // Clears the table
  void Clear();

  // Stores the given entry in the table
  // @param key The key of the entry
  // @param depth The depth of the entry
  // @param flags The flags of the entry
  // @param score The score of the entry
  // @param bestMove The best move of the entry
  void Store(uint64_t key, int depth, TTFlags flags, int score,
             move::Move best_move);

  // Probes the table for the given key
  // @param key The key to probe for
  // @param alpha The alpha value
  TTFlags Probe(Key key, int depth);

  // Gets the entry with the given key
  // @param key The key to get the entry for
  // @return The entry with the given key
  TTEntry Get(Key key);

  // Gets the percentage of the table that is full (Ex. 50% = 500)
  // @return The percentage of the table that is full
  int GetFullPercentage() const;
};

inline int TranspositionTable::GetFullPercentage() const {
  return static_cast<int>((static_cast<double>(num_entries_) / max_entries_) * 1000);
}

}  // namespace chess

#endif  // TRANSPOSITION_TABLE_HPP