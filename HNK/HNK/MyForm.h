#pragma once
#using <System.dll>
#using <System.Drawing.dll>
#using <System.Windows.Forms.dll>

#include "GameLogic.h"

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
        literal String^ GreenShieldImagePath = L"D:\\Projects\\HNK\\Green.png";
        literal String^ RedShieldImagePath = L"D:\\Projects\\HNK\\Red.png";

        TableLayoutPanel^ tableLayoutPanel;
        Button^ passTurnButton;  // Button will be in the bottom-right cell.
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
            this->SuspendLayout();

            // Form settings.
            this->ClientSize = Drawing::Size(400, 400);
            this->Text = L"Grid GUI";

            // Create a 5×5 grid.
            this->tableLayoutPanel->ColumnCount = 5;
            for (int i = 0; i < 5; i++) {
                this->tableLayoutPanel->ColumnStyles->Add(gcnew ColumnStyle(SizeType::Percent, 20));
            }
            this->tableLayoutPanel->RowCount = 5;
            for (int i = 0; i < 5; i++) {
                this->tableLayoutPanel->RowStyles->Add(gcnew RowStyle(SizeType::Percent, 20));
            }
            this->tableLayoutPanel->Dock = DockStyle::Fill;

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
                            pb->Click += gcnew EventHandler(this, &MyForm::OnGreenShieldClick);
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

            this->Controls->Add(this->tableLayoutPanel);
            this->ResumeLayout(false);
        }

        // Event handler for green shield clicks.
        void OnGreenShieldClick(Object^ sender, EventArgs^ e) {
            PictureBox^ shield = dynamic_cast<PictureBox^>(sender);
            if (shield == nullptr || !greenShieldUI->ContainsKey(shield))
                return;

            Point pos = greenShieldUI[shield];
            int newRow;
            if (gameLogic->MoveGreenShield(pos.X, pos.Y, newRow)) {
                // Remove any control in the old cell.
                Control^ oldControl = tableLayoutPanel->GetControlFromPosition(pos.X, pos.Y);
                if (oldControl != nullptr)
                    tableLayoutPanel->Controls->Remove(oldControl);

                // Insert a placeholder panel in the old cell.
                Panel^ placeholder = gcnew Panel();
                placeholder->BackColor = Color::White;
                placeholder->Dock = DockStyle::Fill;
                tableLayoutPanel->Controls->Add(placeholder, pos.X, pos.Y);

                // Remove any control in the new target cell.
                Control^ targetControl = tableLayoutPanel->GetControlFromPosition(pos.X, newRow);
                if (targetControl != nullptr)
                    tableLayoutPanel->Controls->Remove(targetControl);

                // Remove and re-add the shield control to its new cell.
                tableLayoutPanel->Controls->Remove(shield);
                tableLayoutPanel->Controls->Add(shield, pos.X, newRow);

                // Update the dictionary to record the new cell.
                greenShieldUI[shield] = Point(pos.X, newRow);
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

                // Update the shield’s tracked position.
                redShieldUI[shield] = Point(newCol, pos.Y);
            }
        }

        // Event handler for the Pass Turn button.
        void OnPassTurnClick(Object^ sender, EventArgs^ e) {
            gameLogic->PassTurn();
        }
    };
}
