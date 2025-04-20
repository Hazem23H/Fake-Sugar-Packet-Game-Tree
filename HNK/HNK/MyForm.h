#pragma once
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

        // Initialize the UI components.
        void InitializeComponent(void) {
            this->components = gcnew System::ComponentModel::Container();
            this->tableLayoutPanel = gcnew TableLayoutPanel();
            this->infoTextBox = gcnew TextBox();
            this->recommendButton = gcnew Button();
            this->SuspendLayout();

            // Form settings.
            this->ClientSize = Drawing::Size(600, 500);
            this->Text = L"Game Tree Demo";

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
                            //pb->Click += gcnew EventHandler(this, &MyForm::OnGreenShieldClick);
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
        void MyForm::MoveGreenShieldAI() {
            // Example AI logic (you can make this more complex later)
            for (int col = 0; col < 5; col++) {
                // Check if there's a Green shield in the first row
                Control^ control = tableLayoutPanel->GetControlFromPosition(col, 0);  // First row
                PictureBox^ shield = dynamic_cast<PictureBox^>(control);

                if (shield != nullptr && shield->BackColor == Color::Green) {
                    // Move the Green shield down if possible
                    for (int row = 1; row < 5; row++) {
                        PictureBox^ target = dynamic_cast<PictureBox^>(tableLayoutPanel->GetControlFromPosition(col, row));

                        if (target != nullptr && target->BackColor == Color::Transparent) {
                            // Move the shield to the new position
                            tableLayoutPanel->Controls->Remove(shield);
                            tableLayoutPanel->Controls->Add(shield, col, row);
                            infoTextBox->Text = "AI moved green shield to (" + col + ", " + row + ")";
                            return;  // AI moves one shield per turn
                        }
                    }
                }
            }
        }

        // Event handler for green shield clicks.
     // Event handler for when a Green shield is clicked
        void MyForm::OnGreenShieldClick(Object^ sender, EventArgs^ e) {
            PictureBox^ clickedShield = dynamic_cast<PictureBox^>(sender);

            if (clickedShield != nullptr && gameLogic->currentTurn == Turn::Green) {
                // Only allow the Green shield to be clicked if it's the AI's turn
                int col = clickedShield->Location.X / clickedShield->Width;  // Calculate the column
                int row = clickedShield->Location.Y / clickedShield->Height;  // Calculate the row

                // Debug message: Show current Green shield position
                infoTextBox->Text = "Green shield clicked at (" + col + ", " + row + ")";

                // If needed, you can add logic to make the AI move the shield here.
                // But if you're just allowing the AI to move the shield automatically, this click event is probably unnecessary.
            }
        }


        // Event handler for red shield clicks.
        void OnRedShieldClick(Object^ sender, EventArgs^ e) {
            PictureBox^ shield = dynamic_cast<PictureBox^>(sender);
            if (shield == nullptr || !redShieldUI->ContainsKey(shield))
                return;

            Point pos = redShieldUI[shield];
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

                // Check if any player has won
                CheckWinCondition();
            }
        }

        // Event handler for the Pass Turn button.
        void OnPassTurnClick(Object^ sender, EventArgs^ e) {
            gameLogic->PassTurn();
            infoTextBox->Text = "Turn passed. " +
                ((gameLogic->currentTurn == Turn::Red) ? "Red" : "Green") +
                "'s turn now.";
            if (gameLogic->currentTurn == Turn::Red) {
            }
            if (gameLogic->currentTurn == Turn::Green) {
                MoveGreenShieldAI();  // Let the AI make the move
            }
        }

        // Event handler for the Recommend Move button
        void OnRecommendClick(Object^ sender, EventArgs^ e) {
            try {
                String^ recommendation = gameLogic->RecommendMove();
                infoTextBox->Text = recommendation;
            }
            catch (Exception^ ex) {
                infoTextBox->Text = "Error finding recommendation: " + ex->Message;
            }
        }

        // Check if any player has won
        void CheckWinCondition() {
            if (gameLogic->HasPlayerWon(Turn::Red)) {
                MessageBox::Show("Red player has won the game!", "Game Over",
                    MessageBoxButtons::OK, MessageBoxIcon::Information);
            }
            else if (gameLogic->HasPlayerWon(Turn::Green)) {
                MessageBox::Show("Green player has won the game!", "Game Over",
                    MessageBoxButtons::OK, MessageBoxIcon::Information);
            }
        }
    };
}