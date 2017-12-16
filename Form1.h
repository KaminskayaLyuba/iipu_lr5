#pragma once
#include "DeviceFinder.h"
namespace iipu_lr5_t3 {

	using namespace System;
	using namespace System::ComponentModel;
	using namespace System::Collections;
	using namespace System::Windows::Forms;
	using namespace System::Data;
	using namespace System::Drawing;

	/// <summary>
	/// Zusammenfassung für Form1
	/// </summary>
	public ref class Form1 : public System::Windows::Forms::Form
	{
	public:
		Form1(void)
		{
			EnumDevices();
			InitializeComponent();
			for (int i = 0; i < allClasses.size(); i++)
				listBoxClasses->Items->Add(gcnew String(allClasses[i].text));
			//reShow();
			//
			//TODO: Konstruktorcode hier hinzufügen.
			//
		}

	protected:
		/// <summary>
		/// Verwendete Ressourcen bereinigen.
		/// </summary>
		~Form1()
		{
			if (components)
			{
				delete components;
			}
		}
	private: System::Windows::Forms::ListBox^  listBoxClasses;
	protected:
	private: System::Windows::Forms::ListBox^  listBoxDevices;
	private: System::Windows::Forms::Button^  buttonInfo;
	private: System::Windows::Forms::Button^  rejectButton;

	private:
		/// <summary>
		/// Erforderliche Designervariable.
		/// </summary>
		System::ComponentModel::Container ^components;

#pragma region Windows Form Designer generated code
		/// <summary>
		/// Erforderliche Methode für die Designerunterstützung.
		/// Der Inhalt der Methode darf nicht mit dem Code-Editor geändert werden.
		/// </summary>
		void InitializeComponent(void)
		{
			this->listBoxClasses = (gcnew System::Windows::Forms::ListBox());
			this->listBoxDevices = (gcnew System::Windows::Forms::ListBox());
			this->buttonInfo = (gcnew System::Windows::Forms::Button());
			this->rejectButton = (gcnew System::Windows::Forms::Button());
			this->SuspendLayout();
			// 
			// listBoxClasses
			// 
			this->listBoxClasses->FormattingEnabled = true;
			this->listBoxClasses->Location = System::Drawing::Point(13, 13);
			this->listBoxClasses->Name = L"listBoxClasses";
			this->listBoxClasses->Size = System::Drawing::Size(219, 433);
			this->listBoxClasses->TabIndex = 0;
			this->listBoxClasses->SelectedIndexChanged += gcnew System::EventHandler(this, &Form1::listBoxClasses_SelectedIndexChanged);
			// 
			// listBoxDevices
			// 
			this->listBoxDevices->FormattingEnabled = true;
			this->listBoxDevices->Location = System::Drawing::Point(239, 13);
			this->listBoxDevices->Name = L"listBoxDevices";
			this->listBoxDevices->Size = System::Drawing::Size(224, 433);
			this->listBoxDevices->TabIndex = 1;
			// 
			// buttonInfo
			// 
			this->buttonInfo->Location = System::Drawing::Point(239, 457);
			this->buttonInfo->Name = L"buttonInfo";
			this->buttonInfo->Size = System::Drawing::Size(224, 23);
			this->buttonInfo->TabIndex = 2;
			this->buttonInfo->Text = L"Info";
			this->buttonInfo->UseVisualStyleBackColor = true;
			this->buttonInfo->Click += gcnew System::EventHandler(this, &Form1::buttonInfo_Click);
			// 
			// rejectButton
			// 
			this->rejectButton->Location = System::Drawing::Point(13, 457);
			this->rejectButton->Name = L"rejectButton";
			this->rejectButton->Size = System::Drawing::Size(219, 23);
			this->rejectButton->TabIndex = 3;
			this->rejectButton->Text = L"Reject";
			this->rejectButton->UseVisualStyleBackColor = true;
			this->rejectButton->Click += gcnew System::EventHandler(this, &Form1::rejectButton_Click);
			// 
			// Form1
			// 
			this->AutoScaleDimensions = System::Drawing::SizeF(6, 13);
			this->AutoScaleMode = System::Windows::Forms::AutoScaleMode::Font;
			this->ClientSize = System::Drawing::Size(477, 492);
			this->Controls->Add(this->rejectButton);
			this->Controls->Add(this->buttonInfo);
			this->Controls->Add(this->listBoxDevices);
			this->Controls->Add(this->listBoxClasses);
			this->Name = L"Form1";
			this->Text = L"Form1";
			this->ResumeLayout(false);

		}
#pragma endregion
	private: System::Void buttonInfo_Click(System::Object^  sender, System::EventArgs^  e) {
		System::Windows::Forms::MessageBox::Show( "Name: " + gcnew String(allClasses[listBoxClasses->SelectedIndex].devices[listBoxDevices->SelectedIndex].name) + "\n" +
			"Manufacturer: " + gcnew String(allClasses[listBoxClasses->SelectedIndex].devices[listBoxDevices->SelectedIndex].mfg) + "\n" +
			"Provider: " + gcnew String(allClasses[listBoxClasses->SelectedIndex].devices[listBoxDevices->SelectedIndex].provider) + "\n" +
			"HardwareID: " + gcnew String(allClasses[listBoxClasses->SelectedIndex].devices[listBoxDevices->SelectedIndex].HID) + "\n" +
			".SYS path: " + gcnew String(allClasses[listBoxClasses->SelectedIndex].devices[listBoxDevices->SelectedIndex].sys) + "\n" +
			"Driver description: " + gcnew String(allClasses[listBoxClasses->SelectedIndex].devices[listBoxDevices->SelectedIndex].driver) + "\n" +
			"Device path: " + gcnew String(allClasses[listBoxClasses->SelectedIndex].devices[listBoxDevices->SelectedIndex].devicePath) + "\n" +
			"GUID: " + (gcnew Guid(allClasses[listBoxClasses->SelectedIndex].devices[listBoxDevices->SelectedIndex].guid.Data1, 
				allClasses[listBoxClasses->SelectedIndex].devices[listBoxDevices->SelectedIndex].guid.Data2,
				allClasses[listBoxClasses->SelectedIndex].devices[listBoxDevices->SelectedIndex].guid.Data3,
				allClasses[listBoxClasses->SelectedIndex].devices[listBoxDevices->SelectedIndex].guid.Data4[0],
				allClasses[listBoxClasses->SelectedIndex].devices[listBoxDevices->SelectedIndex].guid.Data4[1],
				allClasses[listBoxClasses->SelectedIndex].devices[listBoxDevices->SelectedIndex].guid.Data4[2],
				allClasses[listBoxClasses->SelectedIndex].devices[listBoxDevices->SelectedIndex].guid.Data4[3],
				allClasses[listBoxClasses->SelectedIndex].devices[listBoxDevices->SelectedIndex].guid.Data4[4],
				allClasses[listBoxClasses->SelectedIndex].devices[listBoxDevices->SelectedIndex].guid.Data4[5],
				allClasses[listBoxClasses->SelectedIndex].devices[listBoxDevices->SelectedIndex].guid.Data4[6],
				allClasses[listBoxClasses->SelectedIndex].devices[listBoxDevices->SelectedIndex].guid.Data4[7]))->ToString() + "\n" +
			"Alive: " + allClasses[listBoxClasses->SelectedIndex].devices[listBoxDevices->SelectedIndex].alive
		);
	}
	private: System::Void listBoxClasses_SelectedIndexChanged(System::Object^  sender, System::EventArgs^  e) {
		listBoxDevices->Items->Clear();
		for (int i = 0; i < allClasses[listBoxClasses->SelectedIndex].devices.size(); i++)		
			listBoxDevices->Items->Add(gcnew String( allClasses[listBoxClasses->SelectedIndex].devices[i].name));		
	}
	private: System::Void rejectButton_Click(System::Object^  sender, System::EventArgs^  e) {
		if (deviceChangeStatus(allClasses[listBoxClasses->SelectedIndex].devices[listBoxDevices->SelectedIndex]))
			System::Windows::Forms::MessageBox::Show("Success");
		else
			System::Windows::Forms::MessageBox::Show("Fail");
		allClasses[listBoxClasses->SelectedIndex].devices[listBoxDevices->SelectedIndex].alive = isEnabled(allClasses[listBoxClasses->SelectedIndex].devices[listBoxDevices->SelectedIndex]);
	}
	void reShow()
	{
		listBoxClasses->Items->Clear();
		for (int i = 0; i < allClasses.size(); i++)
			listBoxClasses->Items->Add(gcnew String(allClasses[i].text));
	}
};
}
