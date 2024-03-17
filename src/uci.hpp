#pragma once
#include <atomic>
#include <iostream>
#include <thread>

#include "search.hpp"
namespace chess {

namespace move {
using Move = uint32_t;
}

class Position;  // Forward declaration
class Uci {
 public:
  std::thread search_thread_;
  std::atomic<bool> stop_search_;

  // The function that runs the search thread
  void SearchThreadFunction();

  // Stops the search thread and blocks until it is stopped
  void StopSearchThread();

  SearchEngine search_engine_ = SearchEngine(stop_search_);
  Position position_;
  bool uses_Ucinewgame_ = false;
  bool initialized_ = false;

  // Returns the first word of the given string
  // @param command The string to get the first word of
  // @return The first word of the string
  static std::string GetFirstWord(std::string command);

  // Removes the first word of the given string
  // @param str The string to remove the first word of
  // @return The string with the first word removed
  static std::string RemoveFirstWord(const std::string& str);

  // Initializes any necessary data. Called once after the uci command is
  // received or as necessary if the uci command is not received.
  void Init();

  // Starts the uci loop
  void Start();

  // The uci loop
  void UciLoop();

  // Processes the given command
  // @param command The command to process
  void ProcessCommand(const std::string& command);

  // Parses the Perft command
  // @param command The command to parse
  void ParsePerft(std::string command);

  // Command to tell the engine that the next position is from a new game.
  // Resets the search.
  void Ucinewgame();

  // Parses the Go command. Reads in the search parameters and starts the search.
  // @param command The command to parse
  void ParseGo(std::string command);

  // Parses the setoption command
  // @param command The command to parse
  void ParseOption(std::string command);

  // Parses a move string and returns the move for the current position
  // @param moveString The move string to parse
  // @return The move
  move::Move ParseMove(std::string moveString);

  // Parses the position command
  // @param command The command to parse
  void ParsePosition(std::string command);
};

}  // namespace chess