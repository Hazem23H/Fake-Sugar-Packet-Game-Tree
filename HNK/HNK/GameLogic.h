#pragma once

using namespace System;

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
};
