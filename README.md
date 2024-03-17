# Maello
Maello is a UCI chess engine written in c++.

You can play against Maello on lichess [here](https://lichess.org/@/maello_bot).

## Features
* UCI protocol
* Piece-Square table evaluation with piece activity and king safety modifiers
* Magic Bitboard move generation
* Transposition table using Zobrist Hashing
* Draw detection for 3-fold repetition and 50-move rule
* Negamax search with Alpha-Beta pruning
* Quiexcence search
* Principle Variation search
* Killer move heuristic
* History heuristic
* Search extension when in check
* Null Move Pruning
* Late Move Reductions
* MVV-LVA move ordering

## Acknowledgements
Learning how to code a chess engine can be a daunting task. Below are some resources that I found helpful.
* The [Chess Programming Wiki](https://www.chessprogramming.org/Main_Page)
* Code Monkey King's [Chess Programming](https://www.youtube.com/@chessprogramming591) youtube channel
* Bruce Moreland's [Chess Programming Topics](https://web.archive.org/web/20071026090003/http://www.brucemo.com/compchess/programming/index.htm)
* [Bluefever Software's](https://www.youtube.com/@BlueFeverSoft) youtube [videos](https://www.youtube.com/watch?v=bGAfaepBco4&list=PLZ1QII7yudbc-Ky058TEaOstZHVbT-2hg) on programming a chess engine
* [Sebastion Lague's](https://www.youtube.com/@SebastianLague) youtube [videos](https://www.youtube.com/watch?v=_vqlIPDR2TU&list=PLFt_AvWsXl0cvHyu32ajwh2qU1i6hl77c) about chess engines
* [PeSTO's Piece Square Tables](https://www.chessprogramming.org/PeSTO%27s_Evaluation_Function)
* The [Stockfish](https://github.com/official-stockfish/Stockfish) chess engine

Additionally these resources were invaluable for the testing and use of the engine.
* [lichess.org](https://lichess.org/)
* The [lichess bot client](https://github.com/lichess-bot-devs/lichess-bot)
* The [Arena Chess GUI](http://www.playwitharena.de/)
