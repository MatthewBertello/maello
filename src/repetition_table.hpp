#pragma once
#ifndef REPETITION_TABLE_HPP
#define REPETITION_TABLE_HPP

#include <array>
#include <cstdint>

#include "utils.hpp"

namespace chess {

class RepetitionTable {
 public:
  std::array<Key, kRepetitionTableSize> table;
  int head = 0;
  int tail = 0;
  bool full = false;

  // Adds the given key to the table.
  // @param key The key to add.
  inline void Add(Key key);

  // Removes the last key from the table.
  inline void RemoveLast();

  // Returns whether the table is empty.
  // @return Whether the table is empty.
  inline bool IsEmpty();

  // Returns the key at the given index.
  // @param index The index to get the key from.
  // @return The key at the given index.
  inline Key Get(int index);

  // Returns the size of the table.
  // @return The size of the table.
  inline int size();

  // Returns whether the table contains the given key within the last 100 moves.
  // Only 100 moves are checked because it is impossible to have a repetition
  // before then. This is because a position cannot be repeated after a pawn
  // move or capture occurs and if no pawn moves or captures occured in the last
  // 50 moves the game would be drawn.
  // @param key The key to check for.
  // @return Whether the table contains the given key.
  inline bool HasRepetition(Key key);

  // Returns the number of times the given key appears in the table.
  // Only the most recent 100 moves are checked.
  // @param key The key to count.
  // @return The number of times the given key appears in the table.
  inline int CountRepetitions(Key key);

  // Clears the table.
  inline void Clear();
};

inline void RepetitionTable::Add(Key key) {
  table[head] = key;
  head = (head + 1) % kRepetitionTableSize;
  if (full) tail = (tail + 1) % kRepetitionTableSize;
  full = head == tail;
}

inline void RepetitionTable::RemoveLast() {
  if (IsEmpty()) return;
  head = (head - 1 + kRepetitionTableSize) % kRepetitionTableSize;
  full = false;
}

inline bool RepetitionTable::IsEmpty() { return !full && (head == tail); }

inline Key RepetitionTable::Get(int index) {
  return table[(tail + index) % kRepetitionTableSize];
}

inline int RepetitionTable::size() {
  if (full) {
    return kRepetitionTableSize;
  } else {
    return (head + kRepetitionTableSize - tail) % kRepetitionTableSize;
  }
}

inline bool RepetitionTable::HasRepetition(Key key) {
  int recent_positions = std::min(size(), 100);
  for (int i = 0; i < recent_positions; i++) {
    if (Get(size() - i - 1) == key) {
      return true;
    }
  }
  return false;
}

inline int RepetitionTable::CountRepetitions(Key key) {
  int count = 0;
  int recent_positions = std::min(size(), 100);
  for (int i = 0; i < recent_positions; i++) {
    if (Get(size() - i - 1) == key) {
      count++;
    }
  }
  return count;
}

inline void RepetitionTable::Clear() {
  head = tail = 0;
  full = false;
}

}  // namespace chess

#endif  // REPETITION_TABLE_HPP