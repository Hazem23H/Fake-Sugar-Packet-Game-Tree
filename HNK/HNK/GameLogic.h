#pragma once

using namespace System;
using namespace System::Collections::Generic;

// Represents what occupies each cell.
public enum class PieceType {
    Empty,
    GreenShield,
    RedShield
};

// Represents whose turn it is.
public enum class Turn {
    Red,
    Green
};

// Represents a move on the board
public value struct Move {
    int fromCol;
    int fromRow;
    int toCol;
    int toRow;

    Move(int fCol, int fRow, int tCol, int tRow) {
        fromCol = fCol;
        fromRow = fRow;
        toCol = tCol;
        toRow = tRow;
    }

    virtual String^ ToString() override {
        return String::Format("Move from ({0},{1}) to ({2},{3})",
            fromCol, fromRow, toCol, toRow);
    }
};

// Snapshot of game state
public ref class GameState {
public:
    array<PieceType>^ board;
    Turn currentTurn;

    GameState(GameState^ other) {
        board = gcnew array<PieceType>(other->board->Length);
        Array::Copy(other->board, board, board->Length);
        currentTurn = other->currentTurn;
    }

    GameState(array<PieceType>^ boardState, Turn turn) {
        board = gcnew array<PieceType>(boardState->Length);
        Array::Copy(boardState, board, board->Length);
        currentTurn = turn;
    }
};

public ref class GameLogic {
public:
    literal int BoardSize = 5;
    array<PieceType>^ board;
    Turn currentTurn;

    // Constructor
    GameLogic() {
        board = gcnew array<PieceType>(BoardSize * BoardSize);
        for (int i = 0; i < board->Length; i++)
            board[i] = PieceType::Empty;

        // Place initial shields
        board[Index(1, 0)] = PieceType::GreenShield;
        board[Index(2, 0)] = PieceType::GreenShield;
        board[Index(3, 0)] = PieceType::GreenShield;
        board[Index(0, 1)] = PieceType::RedShield;
        board[Index(0, 2)] = PieceType::RedShield;
        board[Index(0, 3)] = PieceType::RedShield;

        currentTurn = Turn::Red;
    }

    // Converts 2D coords to 1D index
    int Index(int col, int row) {
        return row * BoardSize + col;
    }

    // Returns true if within board bounds
    bool InBounds(int col, int row) {
        return col >= 0 && col < BoardSize&& row >= 0 && row < BoardSize;
    }

    // Toggle turn between Red and Green
    void SwitchTurn() {
        currentTurn = (currentTurn == Turn::Red) ? Turn::Green : Turn::Red;
    }

    // Move green shield down or jump (for the computer)
    bool MoveGreenShield(int col, int row, [System::Runtime::InteropServices::Out] int% newRow) {
        newRow = row;
        if (currentTurn != Turn::Green) return false;  // Ensure it's the computer's turn
        if (!InBounds(col, row) || board[Index(col, row)] != PieceType::GreenShield) return false;

        // Look for a valid move (simple version: move down one step or jump)
        int target = row + 1;
        if (InBounds(col, target) && board[Index(col, target)] == PieceType::Empty) {
            // Move down
            board[Index(col, row)] = PieceType::Empty;
            board[Index(col, target)] = PieceType::GreenShield;
            newRow = target;
            SwitchTurn();
            return true;
        }

        // Check for jump
        int jump = row + 2;
        if (InBounds(col, jump) && board[Index(col, target)] == PieceType::RedShield && board[Index(col, jump)] == PieceType::Empty) {
            // Make a jump over the red shield
            board[Index(col, row)] = PieceType::Empty;
            board[Index(col, jump)] = PieceType::GreenShield;
            newRow = jump;
            SwitchTurn();
            return true;
        }

        return false;  // No valid move found
    }


    // Move red shield right or jump
    bool MoveRedShield(int col, int row, [System::Runtime::InteropServices::Out] int% newCol) {
        newCol = col;
        if (currentTurn != Turn::Red) return false;
        if (!InBounds(col, row) || board[Index(col, row)] != PieceType::RedShield) return false;

        int target = col + 1;
        if (!InBounds(target, row)) return false;
        if (board[Index(target, row)] != PieceType::Empty) {
            int jump = col + 2;
            if (!InBounds(jump, row) || board[Index(jump, row)] != PieceType::Empty) return false;
            target = jump;
        }

        board[Index(col, row)] = PieceType::Empty;
        board[Index(target, row)] = PieceType::RedShield;
        newCol = target;
        SwitchTurn();
        return true;
    }

    // Pass turn manually
    void PassTurn() {
        SwitchTurn();
    }

    // Check if all of player's shields are gone
    bool HasPlayerWon(Turn player) {
        PieceType pType = (player == Turn::Red) ? PieceType::RedShield : PieceType::GreenShield;
        for (int i = 0; i < board->Length; i++) {
            if (board[i] == pType) return false;
        }
        return true;
    }

    // Get all legal moves
    List<Move>^ GetPossibleMoves() {
        List<Move>^ moves = gcnew List<Move>();
        if (currentTurn == Turn::Red) {
            for (int r = 0; r < BoardSize; r++) {
                for (int c = 0; c < BoardSize; c++) {
                    if (board[Index(c, r)] == PieceType::RedShield) {
                        if (c + 1 < BoardSize && board[Index(c + 1, r)] == PieceType::Empty)
                            moves->Add(Move(c, r, c + 1, r));
                        if (c + 2 < BoardSize && board[Index(c + 1, r)] == PieceType::GreenShield && board[Index(c + 2, r)] == PieceType::Empty)
                            moves->Add(Move(c, r, c + 2, r));
                    }
                }
            }
        }
        else {
            for (int r = 0; r < BoardSize; r++) {
                for (int c = 0; c < BoardSize; c++) {
                    if (board[Index(c, r)] == PieceType::GreenShield) {
                        if (r + 1 < BoardSize && board[Index(c, r + 1)] == PieceType::Empty)
                            moves->Add(Move(c, r, c, r + 1));
                        if (r + 2 < BoardSize && board[Index(c, r + 1)] == PieceType::RedShield && board[Index(c, r + 2)] == PieceType::Empty)
                            moves->Add(Move(c, r, c, r + 2));
                    }
                }
            }
        }
        return moves;
    }

    // Apply a move to a snapshot
    GameState^ ApplyMove(GameState^ state, Move move) {
        array<PieceType>^ newBoard = gcnew array<PieceType>(state->board->Length);
        Array::Copy(state->board, newBoard, state->board->Length);
        GameState^ result = gcnew GameState(newBoard, state->currentTurn);

        result->board[Index(move.fromCol, move.fromRow)] = PieceType::Empty;
        if (state->currentTurn == Turn::Red) {
            result->board[Index(move.toCol, move.toRow)] = PieceType::RedShield;
            result->currentTurn = Turn::Green;
        }
        else {
            result->board[Index(move.toCol, move.toRow)] = PieceType::GreenShield;
            result->currentTurn = Turn::Red;
        }
        return result;
    }

    // Apply a move to the live board
    bool ApplyMove(Move move) {
        int dummy;
        if (currentTurn == Turn::Red) return MoveRedShield(move.fromCol, move.fromRow, dummy);
        return MoveGreenShield(move.fromCol, move.fromRow, dummy);
    }

    // Snapshot current state
    GameState^ GetCurrentState() {
        return gcnew GameState(board, currentTurn);
    }

    // Determine win/lose on a given state
    bool IsWinningState(GameState^ state) {
        PieceType pType = (state->currentTurn == Turn::Red) ? PieceType::RedShield : PieceType::GreenShield;
        for (int i = 0; i < state->board->Length; i++) {
            if (state->board[i] == pType) return false;
        }
        return true;
    }

    bool IsLosingState(GameState^ state) {
        GameLogic^ temp = gcnew GameLogic();
        temp->board = gcnew array<PieceType>(state->board->Length);
        Array::Copy(state->board, temp->board, state->board->Length);
        temp->currentTurn = state->currentTurn;
        List<Move>^ moves = temp->GetPossibleMoves();
        return moves->Count == 0 && !IsWinningState(state);
    }

    // Backtracking to find best move
    Move FindBestMove() {
        GameState^ state = GetCurrentState();
        List<Move>^ moves = GetPossibleMoves();
        if (moves->Count == 0) throw gcnew InvalidOperationException("No moves available");
        for each (Move m in moves) {
            GameState^ next = ApplyMove(state, m);
            if (!IsLosingState(next)) return m;
        }
        return moves[0];
    }

    // Recommend move for UI
    String^ RecommendMove() {
        try {
            Move m = FindBestMove();
            String^ p = (currentTurn == Turn::Red) ? "Red" : "Green";
            return String::Format("{0} player should move from ({1},{2}) to ({3},{4})",
                p, m.fromCol, m.fromRow, m.toCol, m.toRow);
        }
        catch (Exception^ ex) {
            return "No good moves available.";
        }
    }
};
