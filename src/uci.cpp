#include "uci.hpp"

#include <atomic>
#include <iostream>
#include <sstream>
#include <string>
#include <thread>

#include "evaluator.hpp"
#include "precomputed_data.hpp"

namespace chess {

std::string Uci::GetFirstWord(std::string str) {
  std::istringstream iss(str);
  std::string word;
  iss >> word;
  return word;
}

std::string Uci::RemoveFirstWord(const std::string &str) {
  size_t firstSpace = str.find(' ');
  if (firstSpace != std::string::npos) {
    return str.substr(firstSpace + 1);
  } else {
    return "";
  }
}

void Uci::Init() {
  if (initialized_) {
    return;
  }
  chess::precomputed_data::Init();
  chess::zobrist::Init();
  InitEvalTables();
  initialized_ = true;
}

void Uci::Start() {
  StopSearchThread();
  ParsePosition("startpos");
  UciLoop();
}

void Uci::UciLoop() {
  std::string input;
  while (true) {
    std::getline(std::cin, input);
    std::istringstream iss(input);
    std::string command;
    while (std::getline(iss, command)) {
      if (command.empty()) {
        continue;
      }
      try {
        ProcessCommand(command);
      } catch (const std::exception &e) {
        std::cout << "Error: " << e.what() << std::endl;
      }
    }
  }
}

void Uci::ProcessCommand(const std::string &command) {
  // Get the first word of the command
  std::string firstWord = GetFirstWord(command);

  // Remove the first word from the command
  std::string remainingCommand = RemoveFirstWord(command);

  if (firstWord == "quit") {
    StopSearchThread();
    exit(0);
  } else if (firstWord == "stop") {
    StopSearchThread();
  } else if (firstWord == "uci") {
    std::cout << "id name " << "Maello" << std::endl;
    std::cout << "id author " << "Matthew Bertello" << std::endl;
    std::cout << std::endl;
    std::cout << "option name Hash type spin default " << kDefaultTranspositionTableSize << " min 1 max 1024" << std::endl;
    std::cout << "uciok" << std::endl;
    Init();
  } else if (firstWord == "isready") {
    std::cout << "readyok" << std::endl;
  } else if (firstWord == "ucinewgame") {
    uses_Ucinewgame_ = true;
    Ucinewgame();
  } else if (firstWord == "position") {
    Init();
    StopSearchThread();
    if (!uses_Ucinewgame_) {
      Ucinewgame();
    }
    ParsePosition(remainingCommand);
  } else if (firstWord == "go") {
    Init();
    StopSearchThread();
    ParseGo(remainingCommand);
  } else if (firstWord == "setoption") {
    StopSearchThread();
    ParseOption(remainingCommand);
  } else if (firstWord == "d") {
    StopSearchThread();
    std::cout << position_ << std::endl;
  } else if (firstWord == "perft") {
    Init();
    ParsePerft(remainingCommand);
  } else if (firstWord == "eval") {
    Init();
    int eval = Evaluate(position_);

    if (position_.state_.side_to_move == kWhite) {
      std::cout << eval << std::endl;
    } else {
      std::cout << -eval << std::endl;
    }
  } else {
    std::cout << "Unknown command: " << command << std::endl;
  }
}
// perft <depth> <fen>
void Uci::ParsePerft(std::string command) {
  StopSearchThread();

  // get the first word of the command
  std::string firstWord = GetFirstWord(command);

  // remove the first word from the command
  command = RemoveFirstWord(command);

  bool debug = false;
  if (firstWord == "debug") {
    debug = true;
    firstWord = GetFirstWord(command);
    command = RemoveFirstWord(command);
  }

  int depth = 0;
  try {
    depth = std::stoi(firstWord);
  } catch (const std::exception &e) {
    std::cout << "Invalid depth" << std::endl;
    return;
  }

  // get the first word of the command
  firstWord = GetFirstWord(command);

  if (firstWord != "") {
    if (firstWord == "startpos") {
      ParsePosition(command);
    } else {
      ParsePosition("fen " + command);
    }
  }

  if (debug) {
    Perft(position_, depth);
  } else {
    uint64_t nodes = PerftHelper(position_, depth);
    std::cout << nodes << std::endl;
  }
}

void Uci::Ucinewgame() {
  StopSearchThread();
  position_.Reset();
  position_.transposition_table_.Clear();
  position_.repetition_table_.Clear();
}

void Uci::ParseGo(std::string command) {
  search_engine_.ResetSearchParameters();

  int depth = -1;
  uint64_t whiteTime = 0;
  uint64_t blackTime = 0;
  uint64_t whiteIncrement = 0;
  uint64_t blackIncrement = 0;
  int movesToGo = 0;
  uint64_t nodes = 0;
  uint64_t moveTime = 0;
  bool infinite = false;

  std::string firstWord;

  // parse the command
  while (command.length() > 0) {
    // get the first word of the command
    firstWord = GetFirstWord(command);

    // remove the first word from the command
    command = RemoveFirstWord(command);

    if (firstWord == "depth") {
      depth = std::stoi(GetFirstWord(command));
      command = RemoveFirstWord(command);
    } else if (firstWord == "wtime") {
      whiteTime = std::stoi(GetFirstWord(command));
      command = RemoveFirstWord(command);
    } else if (firstWord == "btime") {
      blackTime = std::stoi(GetFirstWord(command));
      command = RemoveFirstWord(command);
    } else if (firstWord == "winc") {
      whiteIncrement = std::stoi(GetFirstWord(command));
      command = RemoveFirstWord(command);
    } else if (firstWord == "binc") {
      blackIncrement = std::stoi(GetFirstWord(command));
      command = RemoveFirstWord(command);
    } else if (firstWord == "movestogo") {
      movesToGo = std::stoi(GetFirstWord(command));
      command = RemoveFirstWord(command);
    } else if (firstWord == "nodes") {
      nodes = std::stoi(GetFirstWord(command));
      command = RemoveFirstWord(command);
    } else if (firstWord == "movetime") {
      moveTime = std::stoi(GetFirstWord(command));
      command = RemoveFirstWord(command);
    } else if (firstWord == "infinite") {
      infinite = true;
    }
  }

  if (!infinite) {
    if (depth != -1) {
      search_engine_.search_depth_ = depth;
    }
    if (nodes != 0) {
      search_engine_.max_nodes_ = nodes;
    }
    if (moveTime != 0) {
      search_engine_.end_time_ = GetTime() + moveTime;
    } else {
      if (movesToGo != 0) {
        search_engine_.moves_to_go_ = movesToGo;
      }
      if (whiteTime != 0) {
        search_engine_.white_time_ = whiteTime;
      }
      if (blackTime != 0) {
        search_engine_.black_time_ = blackTime;
      }
      if (whiteIncrement != 0) {
        search_engine_.white_inc_ = whiteIncrement;
      }
      if (blackIncrement != 0) {
        search_engine_.black_inc_ = blackIncrement;
      }
      search_engine_.engine_decides_search_params_ = true;
    }
  }

  stop_search_ = false;
  search_thread_ = std::thread(&Uci::SearchThreadFunction, this);
}

void Uci::ParseOption(std::string command) {
  // get the first word of the command
  std::string firstWord = GetFirstWord(command);

  // remove the first word from the command
  command = RemoveFirstWord(command);

  if (firstWord == "name") {
    std::string name = GetFirstWord(command);
    command = RemoveFirstWord(command);
    if (name == "Hash") {
      std::string value = GetFirstWord(command);
      command = RemoveFirstWord(command);
      if (value == "value") {
        std::string size = GetFirstWord(command);
        command = RemoveFirstWord(command);
        position_.transposition_table_.ChangeSize(std::stoi(size));
      }
    }
  }
}

move::Move Uci::ParseMove(std::string moveString) {
  move::MoveList moves;
  GenerateMoves(position_, moves);
  for (const move::Move &move : moves) {
    std::string test = move::ToString(move);
    if (move::ToString(move) == moveString) {
      return move;
    }
  }
  return 0;
}

void Uci::ParsePosition(std::string command) {
  StopSearchThread();

  // get the first word of the command
  std::string firstWord = GetFirstWord(command);

  // remove the first word from the command
  command = RemoveFirstWord(command);

  if (firstWord == "startpos") {
    position_.Set(kStartingPositionFen);
  } else if (firstWord == "fen") {
    position_.Set(command);
  } else {
    return;
  }
  while (command.length() > 0) {
    // get the first word of the command
    firstWord = GetFirstWord(command);

    // remove the first word from the command
    command = RemoveFirstWord(command);

    if (firstWord == "moves") {
      while (command.length() > 0) {
        // get the first word of the command
        firstWord = GetFirstWord(command);

        // remove the first word from the command
        command = RemoveFirstWord(command);

        move::Move move = ParseMove(firstWord);
        if (move == 0) {
          break;
        }
        position_.MakeMove(move, false);
        position_.repetition_table_.Add(position_.state_.key);
      }
    }
  }
}

void Uci::SearchThreadFunction() { search_engine_.Search(position_); }

// Stops the search thread and blocks until it is stopped
void Uci::StopSearchThread() {
  stop_search_ = true;
  if (search_thread_.joinable()) {
    search_thread_.join();
  }
}

}  // namespace chess