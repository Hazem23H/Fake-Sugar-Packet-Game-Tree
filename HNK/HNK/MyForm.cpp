#include "MyForm.h"

using namespace HNK;
using namespace System;
using namespace System::Windows::Forms;
using namespace System::Drawing;


[STAThread]
void Main(array<String^>^ args) {
    Application::EnableVisualStyles();
    Application::SetCompatibleTextRenderingDefault(false);
    Application::Run(gcnew MyForm());
}
 
