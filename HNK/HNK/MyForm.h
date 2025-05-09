﻿#pragma once
#using <System.dll>
#using <System.Drawing.dll>
#using <System.Windows.Forms.dll>

#include "GameLogic.h"
#include "Stack.h"

using namespace System;
using namespace System::ComponentModel;
using namespace System::Drawing;
using namespace System::Windows::Forms;
using namespace System::Collections::Generic;

namespace HNK {

    public ref class MyForm : public Form {
    public:
        MyForm(void) {
            // Create game logic and UI dictionary mappings.
            gameLogic = gcnew GameLogic();
            greenShieldUI = gcnew Dictionary<PictureBox^, Point>();
            redShieldUI = gcnew Dictionary<PictureBox^, Point>();

            InitializeComponent();
        }

    protected:
        ~MyForm() {
            if (components)
                delete components;
        }

    private:
        // Paths to shield images. Adjust these paths as needed.
        literal String^ GreenShieldImagePath = L"C:\\Users\\kanzy\\Desktop\\Green.png";
        literal String^ RedShieldImagePath = L"C:\\Users\\kanzy\\Desktop\\Red.png";

        TableLayoutPanel^ tableLayoutPanel;
        Button^ passTurnButton;      // Button will be in the bottom-right cell.
        Button^ recommendButton;     // Button to recommend the next move
        TextBox^ infoTextBox;        // TextBox to display move recommendations
        System::ComponentModel::IContainer^ components;

        // Dictionaries to track which PictureBox is in which grid cell.
        Dictionary<PictureBox^, Point>^ greenShieldUI;
        Dictionary<PictureBox^, Point>^ redShieldUI;

        // Instance of the game logic.
        GameLogic^ gameLogic;

        // Timer for AI move
        Timer^ aiTimer;

        // Game over flag
        bool gameOver;

        // Initialize the UI components.
        void InitializeComponent(void) {
            this->components = gcnew System::ComponentModel::Container();
            this->tableLayoutPanel = gcnew TableLayoutPanel();
            this->infoTextBox = gcnew TextBox();
            this->recommendButton = gcnew Button();
            this->aiTimer = gcnew Timer(this->components);
            this->gameOver = false;
            this->SuspendLayout();

            // Form settings.
            this->ClientSize = Drawing::Size(600, 500);
            this->Text = L"Game Tree Demo";

            // Setup AI timer
            this->aiTimer->Interval = 500;
            this->aiTimer->Tick += gcnew EventHandler(this, &MyForm::OnAITimerTick);

            // Create a 5×5 grid.
            this->tableLayoutPanel->ColumnCount = 5;
            for (int i = 0; i < 5; i++) {
                this->tableLayoutPanel->ColumnStyles->Add(gcnew ColumnStyle(SizeType::Percent, 20));
            }
            this->tableLayoutPanel->RowCount = 5;
            for (int i = 0; i < 5; i++) {
                this->tableLayoutPanel->RowStyles->Add(gcnew RowStyle(SizeType::Percent, 20));
            }
            this->tableLayoutPanel->Dock = DockStyle::Top;
            this->tableLayoutPanel->Height = 400;

            // Populate the grid with controls.
            for (int r = 0; r < 5; r++) {
                for (int c = 0; c < 5; c++) {
                    Control^ cell = nullptr;

                    // Bottom-right cell gets the Pass Turn button.
                    if (r == 4 && c == 4) {
                        passTurnButton = gcnew Button();
                        passTurnButton->Text = L"Pass Turn";
                        passTurnButton->Dock = DockStyle::Fill;
                        passTurnButton->Click += gcnew EventHandler(this, &MyForm::OnPassTurnClick);
                        cell = passTurnButton;
                    }
                    // For border cells, add shields or panels.
                    else if (r == 0 || r == 4 || c == 0 || c == 4) {
                        // Top border: add green shields in columns 1 to 3.
                        if (r == 0 && c >= 1 && c <= 3 &&
                            gameLogic->board[gameLogic->Index(c, r)] == PieceType::GreenShield) {

                            PictureBox^ pb = gcnew PictureBox();
                            pb->Dock = DockStyle::Fill;
                            pb->SizeMode = PictureBoxSizeMode::Zoom;
                            pb->Image = Image::FromFile(GreenShieldImagePath);
                            greenShieldUI[pb] = Point(c, r);
                            cell = pb;
                        }
                        // Left border: add red shields in rows 1 to 3.
                        else if (c == 0 && r >= 1 && r <= 3 &&
                            gameLogic->board[gameLogic->Index(c, r)] == PieceType::RedShield) {

                            PictureBox^ pb = gcnew PictureBox();
                            pb->Dock = DockStyle::Fill;
                            pb->SizeMode = PictureBoxSizeMode::Zoom;
                            pb->Image = Image::FromFile(RedShieldImagePath);
                            pb->Click += gcnew EventHandler(this, &MyForm::OnRedShieldClick);
                            redShieldUI[pb] = Point(c, r);
                            cell = pb;
                        }
                        else {
                            // For other border cells, display a colored panel.
                            cell = gcnew Panel();
                            if (c == 4 && r >= 1 && r <= 3)
                                cell->BackColor = Color::LightPink;
                            else if (r == 4 && c >= 1 && c <= 3)
                                cell->BackColor = Color::LightGreen;
                            else
                                cell->BackColor = Color::LightGray;
                        }
                    }
                    else {
                        // Inner cells.
                        cell = gcnew Panel();
                        cell->BackColor = Color::White;
                    }

                    cell->Dock = DockStyle::Fill;
                    this->tableLayoutPanel->Controls->Add(cell, c, r);
                }
            }

            // Setup recommendation button
            this->recommendButton->Location = Point(20, 420);
            this->recommendButton->Size = Drawing::Size(150, 30);
            this->recommendButton->Text = "Recommend Move";
            this->recommendButton->Click += gcnew EventHandler(this, &MyForm::OnRecommendClick);

            // Setup info text box
            this->infoTextBox->Location = Point(180, 420);
            this->infoTextBox->Size = Drawing::Size(400, 60);
            this->infoTextBox->Multiline = true;
            this->infoTextBox->ReadOnly = true;
            this->infoTextBox->ScrollBars = ScrollBars::Vertical;

            // Add controls to form
            this->Controls->Add(this->tableLayoutPanel);
            this->Controls->Add(this->recommendButton);
            this->Controls->Add(this->infoTextBox);

            this->ResumeLayout(false);
        }

        // Timer tick event for AI move
        void OnAITimerTick(Object^ sender, EventArgs^ e) {
            aiTimer->Stop();
            if (gameLogic->currentTurn == Turn::Green && !gameOver) {
                MakeGreenAIMove();
            }
        }

        // Make AI move for Green player
        void MakeGreenAIMove() {
            if (gameLogic->currentTurn != Turn::Green) return;

            try {
                // First see if there are any green shields left
                bool foundGreen = false;
                for each (KeyValuePair<PictureBox^, Point> ^ pair in greenShieldUI) {
                    foundGreen = true;
                    break;
                }

                if (!foundGreen) {
                    infoTextBox->Text = "No Green shields left.";
                    return;
                }

                // Try to move each Green shield
                bool madeMoveSuccessfully = false;

                for each (KeyValuePair<PictureBox^, Point> ^ pair in greenShieldUI) {
                    PictureBox^ shield = pair->Key;
                    Point pos = pair->Value;
                    int newRow;

                    // Skip shields that are already at the bottom row
                    if (pos.Y == 4) continue;

                    // Try to move this shield
                    if (gameLogic->MoveGreenShield(pos.X, pos.Y, newRow)) {
                        // Remove shield from old position
                        tableLayoutPanel->Controls->Remove(shield);

                        // Add placeholder to old position
                        Panel^ placeholder = gcnew Panel();
                        placeholder->BackColor = Color::White;
                        placeholder->Dock = DockStyle::Fill;
                        tableLayoutPanel->Controls->Add(placeholder, pos.X, pos.Y);

                        // If there was anything in the target position, remove it
                        Control^ targetControl = tableLayoutPanel->GetControlFromPosition(pos.X, newRow);
                        if (targetControl != nullptr) {
                            tableLayoutPanel->Controls->Remove(targetControl);
                        }

                        // Move shield to new position
                        tableLayoutPanel->Controls->Add(shield, pos.X, newRow);

                        // Update dictionary
                        greenShieldUI[shield] = Point(pos.X, newRow);

                        // Update info text
                        infoTextBox->Text = "Green AI moved from (" + pos.X + "," + pos.Y + ") to (" + pos.X + "," + newRow.ToString() + ")";

                        madeMoveSuccessfully = true;

                        // Check for win after moving
                        CheckWinCondition();

                        break;  // We made one move, that's enough
                    }
                }

                if (!madeMoveSuccessfully) {
                    // No valid moves found, pass turn
                    infoTextBox->Text = "AI could not find a valid move. Passing turn.";
                    gameLogic->PassTurn();
                }
            }
            catch (Exception^ ex) {
                infoTextBox->Text = "AI error: " + ex->Message;
                gameLogic->PassTurn();
            }
        }

        // Event handler for red shield clicks.
        void OnRedShieldClick(Object^ sender, EventArgs^ e) {
            if (gameOver) return;  // Prevent moves after game is over

            PictureBox^ shield = dynamic_cast<PictureBox^>(sender);
            if (shield == nullptr || !redShieldUI->ContainsKey(shield))
                return;

            Point pos = redShieldUI[shield];

            // Skip if this shield is already at right edge
            if (pos.X == 4) return;

            int newCol;
            if (gameLogic->MoveRedShield(pos.X, pos.Y, newCol)) {
                // Remove control from the old cell.
                Control^ oldControl = tableLayoutPanel->GetControlFromPosition(pos.X, pos.Y);
                if (oldControl != nullptr)
                    tableLayoutPanel->Controls->Remove(oldControl);

                // Insert a placeholder in the old cell.
                Panel^ placeholder = gcnew Panel();
                placeholder->BackColor = Color::White;
                placeholder->Dock = DockStyle::Fill;
                tableLayoutPanel->Controls->Add(placeholder, pos.X, pos.Y);

                // Remove any control already present in the target cell.
                Control^ targetControl = tableLayoutPanel->GetControlFromPosition(newCol, pos.Y);
                if (targetControl != nullptr)
                    tableLayoutPanel->Controls->Remove(targetControl);

                // Remove and re-add the shield to its new location.
                tableLayoutPanel->Controls->Remove(shield);
                tableLayoutPanel->Controls->Add(shield, newCol, pos.Y);

                // Update the shield's tracked position.
                redShieldUI[shield] = Point(newCol, pos.Y);

                // Update info text
                infoTextBox->Text = "Red shield moved from (" + pos.X + "," + pos.Y + ") to (" + newCol + "," + pos.Y + ")";

                // Check for win condition after every move
                CheckWinCondition();

                // After Red's move, trigger the AI move for Green (if game is still ongoing)
                if (gameLogic->currentTurn == Turn::Green && !gameOver) {
                    // Start timer for AI move
                    aiTimer->Start();
                }
            }
        }

        // Event handler for the Pass Turn button.
        void OnPassTurnClick(Object^ sender, EventArgs^ e) {
            if (gameOver) return;  // Prevent pass turn after game is over

            gameLogic->PassTurn();
            infoTextBox->Text = "Turn passed. " +
                ((gameLogic->currentTurn == Turn::Red) ? "Red" : "Green") +
                "'s turn now.";

            // If it's Green's turn after passing, start the AI timer
            if (gameLogic->currentTurn == Turn::Green) {
                aiTimer->Start();
            }
        }

        // Event handler for the Recommend Move button
        void OnRecommendClick(Object^ sender, EventArgs^ e) {
            if (gameOver) {
                infoTextBox->Text = "Game is over. No more moves available.";
                return;
            }

            try {
                String^ recommendation = gameLogic->RecommendMove();
                infoTextBox->Text = recommendation;
            }
            catch (Exception^ ex) {
                infoTextBox->Text = "Error finding recommendation: " + ex->Message;
            }
        }

        // Count how many shields have reached their destination
        int CountShieldsAtDestination(Dictionary<PictureBox^, Point>^ shieldDict, bool isRed) {
            int count = 0;
            for each (KeyValuePair<PictureBox^, Point> ^ pair in shieldDict) {
                Point pos = pair->Value;
                if ((isRed && pos.X == 4) || (!isRed && pos.Y == 4)) {
                    count++;
                }
            }
            return count;
        }

        // Check if any player has won
        void CheckWinCondition() {
            // Count how many red shields have reached the right edge (column 4)
            int redShieldsAtDestination = CountShieldsAtDestination(redShieldUI, true);

            // Count how many green shields have reached the bottom edge (row 4)
            int greenShieldsAtDestination = CountShieldsAtDestination(greenShieldUI, false);

            // Update progress in info text box
            String^ progressInfo = "Progress: Red " + redShieldsAtDestination + "/3 shields at goal, Green " +
                greenShieldsAtDestination + "/3 shields at goal.";

            // Win condition: All 3 shields of a player have reached their destination
            if (redShieldsAtDestination == 3) {
                gameOver = true;
                MessageBox::Show("Red player has won the game!\nAll 3 red shields reached the right edge!", "Game Over",
                    MessageBoxButtons::OK, MessageBoxIcon::Information);
                infoTextBox->Text = "GAME OVER: Red player wins! All 3 red shields reached the right edge!";
                DisableGameInteractions();
            }
            else if (greenShieldsAtDestination == 3) {
                gameOver = true;
                MessageBox::Show("Green player has won the game!\nAll 3 green shields reached the bottom edge!", "Game Over",
                    MessageBoxButtons::OK, MessageBoxIcon::Information);
                infoTextBox->Text = "GAME OVER: Green player wins! All 3 green shields reached the bottom edge!";
                DisableGameInteractions();
            }
            else {
                // If no winner yet, update progress info
                infoTextBox->Text += "\n" + progressInfo;
            }
        }

        // Disable game interactions after game is over
        void DisableGameInteractions() {
            // Disable red shield clicks
            for each (KeyValuePair<PictureBox^, Point> ^ pair in redShieldUI) {
                pair->Key->Enabled = false;
            }

            // Disable pass turn button
            passTurnButton->Enabled = false;

            // Disable recommend button
            recommendButton->Enabled = false;

            // Optional: Change form title to indicate game over
            this->Text = L"Game Tree Demo - GAME OVER";
        }
    };
}