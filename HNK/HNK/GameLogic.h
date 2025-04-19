#pragma once

using namespace System;
using namespace System::Collections::Generic;

// Forward declaration for Stack
template <typename T>
class Stack;

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

// Structure to represent a move
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

// Structure to represent a game state
public ref class GameState {
public:
    array<PieceType>^ board;
    Turn currentTurn;

    // Deep copy constructor
    GameState(GameState^ other) {
        board = gcnew array<PieceType>(other->board->Length);
        Array::Copy(other->board, board, other->board->Length);
        currentTurn = other->currentTurn;
    }

    GameState(array<PieceType>^ boardState, Turn turn) {
        board = gcnew array<PieceType>(boardState->Length);
        Array::Copy(boardState, board, boardState->Length);
        currentTurn = turn;
    }

    // Check if states are equal
    bool Equals(GameState^ other) {
        if (currentTurn != other->currentTurn)
            return false;

        for (int i = 0; i < board->Length; i++) {
            if (board[i] != other->board[i])
                return false;
        }
        return true;
    }
};

public ref class GameLogic {
public:
    literal int BoardSize = 5; // 5x5 board.

    // 1D array representation of the board.
    array<PieceType>^ board;
    Turn currentTurn;

    GameLogic() {
        board = gcnew array<PieceType>(BoardSize * BoardSize);

        // Set all cells to empty.
        for (int i = 0; i < board->Length; i++) {
            board[i] = PieceType::Empty;
        }

        // Place initial shields.
        // Green shields (top border): positions (1,0), (2,0), (3,0)
        board[Index(1, 0)] = PieceType::GreenShield;
        board[Index(2, 0)] = PieceType::GreenShield;
        board[Index(3, 0)] = PieceType::GreenShield;

        // Red shields (left border): positions (0,1), (0,2), (0,3)
        board[Index(0, 1)] = PieceType::RedShield;
        board[Index(0, 2)] = PieceType::RedShield;
        board[Index(0, 3)] = PieceType::RedShield;

        // Set starting turn.
        currentTurn = Turn::Red;
    }

    // Converts (col, row) into the board's 1D array index.
    int Index(int col, int row) {
        return row * BoardSize + col;
    }

    // Checks if the given coordinates are on the board.
    bool InBounds(int col, int row) {
        return (col >= 0 && col < BoardSize && row >= 0 && row < BoardSize);
    }

    // Moves a green shield downward.
    // Returns true if successful and sets newRow to the shield's new row.
    bool MoveGreenShield(int col, int row, [System::Runtime::InteropServices::Out] int% newRow) {
        newRow = row;
        if (currentTurn != Turn::Green)
            return false;

        // Ensure there is a green shield at the specified position.
        if (!InBounds(col, row) || board[Index(col, row)] != PieceType::GreenShield)
            return false;

        int targetRow = row + 1;
        if (!InBounds(col, targetRow))
            return false;

        // Check if adjacent cell is blocked.
        if (board[Index(col, targetRow)] != PieceType::Empty) {
            int jumpRow = row + 2;
            if (!InBounds(col, jumpRow) || board[Index(col, jumpRow)] != PieceType::Empty)
                return false;
            targetRow = jumpRow;
        }

        // Update board.
        board[Index(col, row)] = PieceType::Empty;
        board[Index(col, targetRow)] = PieceType::GreenShield;
        newRow = targetRow;

        // Toggle turn.
        currentTurn = Turn::Red;
        return true;
    }

    // Moves a red shield to the right.
    // Returns true if successful and sets newCol to the shield's new column.
    bool MoveRedShield(int col, int row, [System::Runtime::InteropServices::Out] int% newCol) {
        newCol = col;
        if (currentTurn != Turn::Red)
            return false;

        if (!InBounds(col, row) || board[Index(col, row)] != PieceType::RedShield)
            return false;

        int targetCol = col + 1;
        if (!InBounds(targetCol, row))
            return false;

        // Check if adjacent cell is occupied.
        if (board[Index(targetCol, row)] != PieceType::Empty) {
            int jumpCol = col + 2;
            if (!InBounds(jumpCol, row) || board[Index(jumpCol, row)] != PieceType::Empty)
                return false;
            targetCol = jumpCol;
        }

        // Update board.
        board[Index(col, row)] = PieceType::Empty;
        board[Index(targetCol, row)] = PieceType::RedShield;
        newCol = targetCol;

        // Toggle turn.
        currentTurn = Turn::Green;
        return true;
    }

    // Pass the turn without any movement.
    void PassTurn() {
        currentTurn = (currentTurn == Turn::Red) ? Turn::Green : Turn::Red;
    }

    // Check if a player has won
    bool HasPlayerWon(Turn player) {
        if (player == Turn::Red) {
            // Check if all red shields are moved off the board (reached right border)
            bool foundRed = false;
            for (int r = 0; r < BoardSize; r++) {
                for (int c = 0; c < BoardSize; c++) {
                    if (board[Index(c, r)] == PieceType::RedShield) {
                        foundRed = true;
                        break;
                    }
                }
                if (foundRed) break;
            }
            return !foundRed;
        }
        else { // Green player
            // Check if all green shields are moved off the board (reached bottom border)
            bool foundGreen = false;
            for (int r = 0; r < BoardSize; r++) {
                for (int c = 0; c < BoardSize; c++) {
                    if (board[Index(c, r)] == PieceType::GreenShield) {
                        foundGreen = true;
                        break;
                    }
                }
                if (foundGreen) break;
            }
            return !foundGreen;
        }
    }

    // Get all possible moves for the current player
    List<Move>^ GetPossibleMoves() {
        List<Move>^ moves = gcnew List<Move>();

        if (currentTurn == Turn::Red) {
            // Find all red shields and their possible moves
            for (int r = 0; r < BoardSize; r++) {
                for (int c = 0; c < BoardSize; c++) {
                    if (board[Index(c, r)] == PieceType::RedShield) {
                        // Check for normal move
                        if (c + 1 < BoardSize && board[Index(c + 1, r)] == PieceType::Empty) {
                            moves->Add(Move(c, r, c + 1, r));
                        }
                        // Check for jump move
                        if (c + 1 < BoardSize && c + 2 < BoardSize &&
                            board[Index(c + 1, r)] == PieceType::GreenShield &&
                            board[Index(c + 2, r)] == PieceType::Empty) {
                            moves->Add(Move(c, r, c + 2, r));
                        }
                    }
                }
            }
        }
        else { // Green's turn
            // Find all green shields and their possible moves
            for (int r = 0; r < BoardSize; r++) {
                for (int c = 0; c < BoardSize; c++) {
                    if (board[Index(c, r)] == PieceType::GreenShield) {
                        // Check for normal move
                        if (r + 1 < BoardSize && board[Index(c, r + 1)] == PieceType::Empty) {
                            moves->Add(Move(c, r, c, r + 1));
                        }
                        // Check for jump move
                        if (r + 1 < BoardSize && r + 2 < BoardSize &&
                            board[Index(c, r + 1)] == PieceType::RedShield &&
                            board[Index(c, r + 2)] == PieceType::Empty) {
                            moves->Add(Move(c, r, c, r + 2));
                        }
                    }
                }
            }
        }

        return moves;
    }

    // Apply a move to a game state and return the new state
    GameState^ ApplyMove(GameState^ state, Move move) {
        GameState^ newState = gcnew GameState(state);

        // Update board based on the move
        newState->board[Index(move.fromCol, move.fromRow)] = PieceType::Empty;

        if (state->currentTurn == Turn::Red) {
            newState->board[Index(move.toCol, move.toRow)] = PieceType::RedShield;
            newState->currentTurn = Turn::Green;
        }
        else {
            newState->board[Index(move.toCol, move.toRow)] = PieceType::GreenShield;
            newState->currentTurn = Turn::Red;
        }

        return newState;
    }

    // Apply a move to the current game
    bool ApplyMove(Move move) {
        if (currentTurn == Turn::Red) {
            int newCol;
            return MoveRedShield(move.fromCol, move.fromRow, newCol);
        }
        else {
            int newRow;
            return MoveGreenShield(move.fromCol, move.fromRow, newRow);
        }
    }

    // Get current game state
    GameState^ GetCurrentState() {
        return gcnew GameState(board, currentTurn);
    }

    // Check if a game state is winning for the current player
    bool IsWinningState(GameState^ state) {
        return HasPlayerWon(state->currentTurn);
    }

    // Check if a game state is losing for the current player
    bool IsLosingState(GameState^ state) {
        // Create a temporary game logic to check possible moves
        GameLogic^ tempLogic = gcnew GameLogic();
        tempLogic->board = gcnew array<PieceType>(state->board->Length);
        Array::Copy(state->board, tempLogic->board, state->board->Length);
        tempLogic->currentTurn = state->currentTurn;

        // If there are no possible moves and player hasn't won, it's a losing state
        return tempLogic->GetPossibleMoves()->Count == 0 && !HasPlayerWon(state->currentTurn);
    }

    // Backtracking Game Tree Algorithm to find the best move
    Move FindBestMove() {
        GameState^ currentState = GetCurrentState();
        List<Move>^ possibleMoves = GetPossibleMoves();

        // If no moves are possible
        if (possibleMoves->Count == 0) {
            throw gcnew InvalidOperationException("No moves available");
        }

        // Find the first good move
        for each (Move move in possibleMoves) {
            if (IsGoodMove(currentState, move)) {
                return move;
            }
        }

        // If no good moves, return the first available move
        return possibleMoves[0];
    }

    // Check if a move leads to a good state for the current player
    bool IsGoodMove(GameState^ state, Move move) {
        // Apply the move to get the new state
        GameState^ newState = ApplyMove(state, move);

        // Check if this move immediately wins the game
        if (HasPlayerWon(state->currentTurn)) {
            return true;
        }

        // Check if this move puts the opponent in a bad state
        return IsGameStateBad(newState);
    }

    // Determine if a game state is "good" using recursive backtracking
    bool IsGameStateGood(GameState^ state) {
        // Base cases
        if (HasPlayerWon(state->currentTurn)) {
            return true; // Current player has already won
        }

        GameLogic^ tempLogic = gcnew GameLogic();
        tempLogic->board = gcnew array<PieceType>(state->board->Length);
        Array::Copy(state->board, tempLogic->board, state->board->Length);
        tempLogic->currentTurn = state->currentTurn;

        List<Move>^ possibleMoves = tempLogic->GetPossibleMoves();

        // If no moves available and player hasn't won, it's a bad state
        if (possibleMoves->Count == 0) {
            return false;
        }

        // Check if any move leads to a bad state for the opponent
        for each (Move move in possibleMoves) {
            GameState^ nextState = tempLogic->ApplyMove(state, move);
            if (IsGameStateBad(nextState)) {
                return true; // Found a move leading to a bad state for opponent
            }
        }

        return false; // Couldn't find any good move
    }

    // Determine if a game state is "bad" using recursive backtracking
    bool IsGameStateBad(GameState^ state) {
        // Base cases
        if (HasPlayerWon(state->currentTurn)) {
            return false; // Current player has already won, so it's not a bad state
        }

        GameLogic^ tempLogic = gcnew GameLogic();
        tempLogic->board = gcnew array<PieceType>(state->board->Length);
        Array::Copy(state->board, tempLogic->board, state->board->Length);
        tempLogic->currentTurn = state->currentTurn;

        List<Move>^ possibleMoves = tempLogic->GetPossibleMoves();

        // If no moves available and player hasn't won, it's a bad state
        if (possibleMoves->Count == 0) {
            return true;
        }

        // If every move leads to a good state for the opponent, it's a bad state
        for each (Move move in possibleMoves) {
            GameState^ nextState = tempLogic->ApplyMove(state, move);
            if (!IsGameStateGood(nextState)) {
                return false; // Found at least one move that doesn't lead to a good state for opponent
            }
        }

        return true; // All moves lead to good states for the opponent
    }

    // Get all good moves for the current player
    List<Move>^ GetGoodMoves() {
        List<Move>^ goodMoves = gcnew List<Move>();
        List<Move>^ possibleMoves = GetPossibleMoves();
        GameState^ currentState = GetCurrentState();

        for each (Move move in possibleMoves) {
            if (IsGoodMove(currentState, move)) {
                goodMoves->Add(move);
            }
        }

        return goodMoves;
    }

    // Method to recommend the best move for current player
    String^ RecommendMove() {
        try {
            Move bestMove = FindBestMove();
            String^ player = (currentTurn == Turn::Red) ? "Red" : "Green";
            return String::Format("{0} player should move from ({1},{2}) to ({3},{4})",
                player, bestMove.fromCol, bestMove.fromRow, bestMove.toCol, bestMove.toRow);
        }
        catch (Exception^ ex) {
            return "No good moves available. Consider passing your turn.";
        }
    }
};