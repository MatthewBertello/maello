#include "transposition_table.hpp"
namespace chess {
TranspositionTable::TranspositionTable() { ChangeSize(size_); }

void TranspositionTable::ChangeSize(int size) {
  table_.clear();
  size_ = size;
  max_entries_ = (size * 1024 * 1024) / kTTEntrySize;
  table_.resize(max_entries_);
  num_entries_ = 0;
}

void TranspositionTable::Clear() {
  ChangeSize(size_);
}

void TranspositionTable::Store(uint64_t key, int depth, TTFlags flags,
                               int score, move::Move bestMove) {
  TTEntry *entry = &table_[key % max_entries_];
  if (entry->depth == -1) num_entries_++;
  entry->key = key;
  entry->depth = depth;
  entry->flags = flags;
  entry->score = score;
  entry->bestMove = bestMove;
}

TTFlags TranspositionTable::Probe(Key key, int depth) {
  TTEntry *entry = &table_[key % max_entries_];

  if (entry->key == key) {
    if (entry->depth >= depth) {
      return entry->flags;
    }
  }

  return kNoHashFlag;
}

TTEntry TranspositionTable::Get(Key key) {
  TTEntry *entry = &table_[key % max_entries_];
  return *entry;
}

}  // namespace chess