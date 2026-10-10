#pragma once

namespace LankaJourneyUI {

	using namespace System;
	using namespace System::ComponentModel;
	using namespace System::Collections;
	using namespace System::Windows::Forms;
	using namespace System::Data;
	using namespace System::Drawing;

	/// <summary>
	/// Summary for Dashboard
	/// </summary>
	public ref class Dashboard : public System::Windows::Forms::Form
	{
	public:
		Dashboard(void)
		{
			InitializeComponent();
			//
			//TODO: Add the constructor code here
			//
		}

	protected:
		/// <summary>
		/// Clean up any resources being used.
		/// </summary>
		~Dashboard()
		{
			if (components)
			{
				delete components;
			}
		}

	protected:



	private: System::Windows::Forms::PictureBox^ pictureBox1;
	private: System::Windows::Forms::Button^ button1;
	private: System::Windows::Forms::Panel^ PanelSlidebar;
	private: System::Windows::Forms::Button^ button9;

	private: System::Windows::Forms::Button^ button7;
	private: System::Windows::Forms::Button^ button6;
	private: System::Windows::Forms::Button^ btnAccommodatiion;

	private: System::Windows::Forms::Button^ button4;
	private: System::Windows::Forms::Button^ btnPlanning;

	private: System::Windows::Forms::Button^ btnDashboard;
	private: System::Windows::Forms::PictureBox^ horizontalLine;
	private: System::Windows::Forms::Panel^ pnlForButton;
	private: System::Windows::Forms::Panel^ destinationsPanel;
	private: System::Windows::Forms::Button^ nextButton;
	private: System::Windows::Forms::Button^ previousButton;
	private: System::Windows::Forms::PictureBox^ destinationPictureBox;








	protected:

	private:
		/// <summary>
		/// Required designer variable.
		/// </summary>
		System::ComponentModel::Container ^components;

#pragma region Windows Form Designer generated code
		/// <summary>
		/// Required method for Designer support - do not modify
		/// the contents of this method with the code editor.
		/// </summary>
		void InitializeComponent(void)
		{
			System::ComponentModel::ComponentResourceManager^ resources = (gcnew System::ComponentModel::ComponentResourceManager(Dashboard::typeid));
			this->pictureBox1 = (gcnew System::Windows::Forms::PictureBox());
			this->button1 = (gcnew System::Windows::Forms::Button());
			this->PanelSlidebar = (gcnew System::Windows::Forms::Panel());
			this->horizontalLine = (gcnew System::Windows::Forms::PictureBox());
			this->button9 = (gcnew System::Windows::Forms::Button());
			this->button7 = (gcnew System::Windows::Forms::Button());
			this->button6 = (gcnew System::Windows::Forms::Button());
			this->btnAccommodatiion = (gcnew System::Windows::Forms::Button());
			this->button4 = (gcnew System::Windows::Forms::Button());
			this->btnPlanning = (gcnew System::Windows::Forms::Button());
			this->btnDashboard = (gcnew System::Windows::Forms::Button());
			this->pnlForButton = (gcnew System::Windows::Forms::Panel());
			this->destinationsPanel = (gcnew System::Windows::Forms::Panel());
			this->destinationPictureBox = (gcnew System::Windows::Forms::PictureBox());
			this->previousButton = (gcnew System::Windows::Forms::Button());
			this->nextButton = (gcnew System::Windows::Forms::Button());
			(cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->pictureBox1))->BeginInit();
			this->PanelSlidebar->SuspendLayout();
			(cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->horizontalLine))->BeginInit();
			this->pnlForButton->SuspendLayout();
			this->destinationsPanel->SuspendLayout();
			(cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->destinationPictureBox))->BeginInit();
			this->SuspendLayout();
			// 
			// pictureBox1
			// 
			this->pictureBox1->BackgroundImageLayout = System::Windows::Forms::ImageLayout::Stretch;
			this->pictureBox1->Dock = System::Windows::Forms::DockStyle::Fill;
			this->pictureBox1->Image = (cli::safe_cast<System::Drawing::Image^>(resources->GetObject(L"pictureBox1.Image")));
			this->pictureBox1->Location = System::Drawing::Point(0, 0);
			this->pictureBox1->Name = L"pictureBox1";
			this->pictureBox1->Size = System::Drawing::Size(1262, 673);
			this->pictureBox1->SizeMode = System::Windows::Forms::PictureBoxSizeMode::StretchImage;
			this->pictureBox1->TabIndex = 2;
			this->pictureBox1->TabStop = false;
			this->pictureBox1->Click += gcnew System::EventHandler(this, &Dashboard::pictureBox1_Click);
			// 
			// button1
			// 
			this->button1->BackColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(51)), static_cast<System::Int32>(static_cast<System::Byte>(2)),
				static_cast<System::Int32>(static_cast<System::Byte>(6)));
			this->button1->BackgroundImage = (cli::safe_cast<System::Drawing::Image^>(resources->GetObject(L"button1.BackgroundImage")));
			this->button1->BackgroundImageLayout = System::Windows::Forms::ImageLayout::Stretch;
			this->button1->FlatAppearance->BorderSize = 0;
			this->button1->FlatStyle = System::Windows::Forms::FlatStyle::Flat;
			this->button1->Font = (gcnew System::Drawing::Font(L"Microsoft YaHei UI", 16.2F, System::Drawing::FontStyle::Bold, System::Drawing::GraphicsUnit::Point,
				static_cast<System::Byte>(0)));
			this->button1->ForeColor = System::Drawing::Color::Black;
			this->button1->Location = System::Drawing::Point(-3, -3);
			this->button1->Name = L"button1";
			this->button1->Size = System::Drawing::Size(396, 77);
			this->button1->TabIndex = 3;
			this->button1->Text = L"Plan My Journey";
			this->button1->UseVisualStyleBackColor = false;
			this->button1->Click += gcnew System::EventHandler(this, &Dashboard::button1_Click);
			// 
			// PanelSlidebar
			// 
			this->PanelSlidebar->BackgroundImage = (cli::safe_cast<System::Drawing::Image^>(resources->GetObject(L"PanelSlidebar.BackgroundImage")));
			this->PanelSlidebar->BackgroundImageLayout = System::Windows::Forms::ImageLayout::Stretch;
			this->PanelSlidebar->Controls->Add(this->horizontalLine);
			this->PanelSlidebar->Controls->Add(this->button9);
			this->PanelSlidebar->Controls->Add(this->button7);
			this->PanelSlidebar->Controls->Add(this->button6);
			this->PanelSlidebar->Controls->Add(this->btnAccommodatiion);
			this->PanelSlidebar->Controls->Add(this->button4);
			this->PanelSlidebar->Controls->Add(this->btnPlanning);
			this->PanelSlidebar->Controls->Add(this->btnDashboard);
			this->PanelSlidebar->Location = System::Drawing::Point(21, 86);
			this->PanelSlidebar->Name = L"PanelSlidebar";
			this->PanelSlidebar->Padding = System::Windows::Forms::Padding(0, 10, 0, 155);
			this->PanelSlidebar->Size = System::Drawing::Size(225, 575);
			this->PanelSlidebar->TabIndex = 4;
			// 
			// horizontalLine
			// 
			this->horizontalLine->BackgroundImage = (cli::safe_cast<System::Drawing::Image^>(resources->GetObject(L"horizontalLine.BackgroundImage")));
			this->horizontalLine->BackgroundImageLayout = System::Windows::Forms::ImageLayout::Stretch;
			this->horizontalLine->Dock = System::Windows::Forms::DockStyle::Top;
			this->horizontalLine->Location = System::Drawing::Point(0, 310);
			this->horizontalLine->Name = L"horizontalLine";
			this->horizontalLine->Size = System::Drawing::Size(225, 50);
			this->horizontalLine->TabIndex = 5;
			this->horizontalLine->TabStop = false;
			// 
			// button9
			// 
			this->button9->BackgroundImage = (cli::safe_cast<System::Drawing::Image^>(resources->GetObject(L"button9.BackgroundImage")));
			this->button9->BackgroundImageLayout = System::Windows::Forms::ImageLayout::Stretch;
			this->button9->Dock = System::Windows::Forms::DockStyle::Bottom;
			this->button9->FlatStyle = System::Windows::Forms::FlatStyle::Flat;
			this->button9->Location = System::Drawing::Point(0, 370);
			this->button9->Name = L"button9";
			this->button9->Size = System::Drawing::Size(225, 50);
			this->button9->TabIndex = 12;
			this->button9->UseVisualStyleBackColor = true;
			// 
			// button7
			// 
			this->button7->BackgroundImage = (cli::safe_cast<System::Drawing::Image^>(resources->GetObject(L"button7.BackgroundImage")));
			this->button7->BackgroundImageLayout = System::Windows::Forms::ImageLayout::Stretch;
			this->button7->Dock = System::Windows::Forms::DockStyle::Top;
			this->button7->FlatStyle = System::Windows::Forms::FlatStyle::Flat;
			this->button7->Location = System::Drawing::Point(0, 260);
			this->button7->Name = L"button7";
			this->button7->Size = System::Drawing::Size(225, 50);
			this->button7->TabIndex = 10;
			this->button7->UseVisualStyleBackColor = true;
			// 
			// button6
			// 
			this->button6->BackgroundImage = (cli::safe_cast<System::Drawing::Image^>(resources->GetObject(L"button6.BackgroundImage")));
			this->button6->BackgroundImageLayout = System::Windows::Forms::ImageLayout::Stretch;
			this->button6->Dock = System::Windows::Forms::DockStyle::Top;
			this->button6->FlatStyle = System::Windows::Forms::FlatStyle::Flat;
			this->button6->Location = System::Drawing::Point(0, 210);
			this->button6->Name = L"button6";
			this->button6->Size = System::Drawing::Size(225, 50);
			this->button6->TabIndex = 9;
			this->button6->UseVisualStyleBackColor = true;
			// 
			// btnAccommodatiion
			// 
			this->btnAccommodatiion->BackgroundImage = (cli::safe_cast<System::Drawing::Image^>(resources->GetObject(L"btnAccommodatiion.BackgroundImage")));
			this->btnAccommodatiion->BackgroundImageLayout = System::Windows::Forms::ImageLayout::Stretch;
			this->btnAccommodatiion->Dock = System::Windows::Forms::DockStyle::Top;
			this->btnAccommodatiion->FlatStyle = System::Windows::Forms::FlatStyle::Flat;
			this->btnAccommodatiion->Location = System::Drawing::Point(0, 160);
			this->btnAccommodatiion->Name = L"btnAccommodatiion";
			this->btnAccommodatiion->Size = System::Drawing::Size(225, 50);
			this->btnAccommodatiion->TabIndex = 8;
			this->btnAccommodatiion->UseVisualStyleBackColor = true;
			// 
			// button4
			// 
			this->button4->BackgroundImage = (cli::safe_cast<System::Drawing::Image^>(resources->GetObject(L"button4.BackgroundImage")));
			this->button4->BackgroundImageLayout = System::Windows::Forms::ImageLayout::Stretch;
			this->button4->Dock = System::Windows::Forms::DockStyle::Top;
			this->button4->FlatStyle = System::Windows::Forms::FlatStyle::Flat;
			this->button4->Location = System::Drawing::Point(0, 110);
			this->button4->Name = L"button4";
			this->button4->Padding = System::Windows::Forms::Padding(15, 0, 0, 0);
			this->button4->Size = System::Drawing::Size(225, 50);
			this->button4->TabIndex = 7;
			this->button4->UseVisualStyleBackColor = true;
			// 
			// btnPlanning
			// 
			this->btnPlanning->BackgroundImage = (cli::safe_cast<System::Drawing::Image^>(resources->GetObject(L"btnPlanning.BackgroundImage")));
			this->btnPlanning->BackgroundImageLayout = System::Windows::Forms::ImageLayout::Stretch;
			this->btnPlanning->Dock = System::Windows::Forms::DockStyle::Top;
			this->btnPlanning->FlatStyle = System::Windows::Forms::FlatStyle::Flat;
			this->btnPlanning->Location = System::Drawing::Point(0, 60);
			this->btnPlanning->Name = L"btnPlanning";
			this->btnPlanning->Size = System::Drawing::Size(225, 50);
			this->btnPlanning->TabIndex = 6;
			this->btnPlanning->UseVisualStyleBackColor = true;
			// 
			// btnDashboard
			// 
			this->btnDashboard->BackgroundImage = (cli::safe_cast<System::Drawing::Image^>(resources->GetObject(L"btnDashboard.BackgroundImage")));
			this->btnDashboard->BackgroundImageLayout = System::Windows::Forms::ImageLayout::Stretch;
			this->btnDashboard->Dock = System::Windows::Forms::DockStyle::Top;
			this->btnDashboard->FlatStyle = System::Windows::Forms::FlatStyle::Popup;
			this->btnDashboard->Location = System::Drawing::Point(0, 10);
			this->btnDashboard->Name = L"btnDashboard";
			this->btnDashboard->Padding = System::Windows::Forms::Padding(15, 5, 5, 5);
			this->btnDashboard->Size = System::Drawing::Size(225, 50);
			this->btnDashboard->TabIndex = 5;
			this->btnDashboard->UseVisualStyleBackColor = true;
			// 
			// pnlForButton
			// 
			this->pnlForButton->Controls->Add(this->button1);
			this->pnlForButton->Location = System::Drawing::Point(562, 286);
			this->pnlForButton->Name = L"pnlForButton";
			this->pnlForButton->Size = System::Drawing::Size(396, 77);
			this->pnlForButton->TabIndex = 5;
			// 
			// destinationsPanel
			// 
			this->destinationsPanel->Controls->Add(this->nextButton);
			this->destinationsPanel->Controls->Add(this->previousButton);
			this->destinationsPanel->Controls->Add(this->destinationPictureBox);
			this->destinationsPanel->Location = System::Drawing::Point(298, 438);
			this->destinationsPanel->Name = L"destinationsPanel";
			this->destinationsPanel->Size = System::Drawing::Size(935, 187);
			this->destinationsPanel->TabIndex = 6;
			// 
			// destinationPictureBox
			// 
			this->destinationPictureBox->BackgroundImage = (cli::safe_cast<System::Drawing::Image^>(resources->GetObject(L"destinationPictureBox.BackgroundImage")));
			this->destinationPictureBox->BackgroundImageLayout = System::Windows::Forms::ImageLayout::Stretch;
			this->destinationPictureBox->Dock = System::Windows::Forms::DockStyle::Fill;
			this->destinationPictureBox->Location = System::Drawing::Point(0, 0);
			this->destinationPictureBox->Name = L"destinationPictureBox";
			this->destinationPictureBox->Size = System::Drawing::Size(935, 187);
			this->destinationPictureBox->TabIndex = 0;
			this->destinationPictureBox->TabStop = false;
			// 
			// previousButton
			// 
			this->previousButton->Location = System::Drawing::Point(733, 110);
			this->previousButton->Name = L"previousButton";
			this->previousButton->Size = System::Drawing::Size(43, 23);
			this->previousButton->TabIndex = 1;
			this->previousButton->Text = L"<-";
			this->previousButton->UseVisualStyleBackColor = true;
			// 
			// nextButton
			// 
			this->nextButton->Location = System::Drawing::Point(810, 110);
			this->nextButton->Name = L"nextButton";
			this->nextButton->Size = System::Drawing::Size(43, 23);
			this->nextButton->TabIndex = 2;
			this->nextButton->Text = L"->";
			this->nextButton->UseVisualStyleBackColor = true;
			// 
			// Dashboard
			// 
			this->AutoScaleDimensions = System::Drawing::SizeF(8, 16);
			this->AutoScaleMode = System::Windows::Forms::AutoScaleMode::Font;
			this->BackColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(3)), static_cast<System::Int32>(static_cast<System::Byte>(7)),
				static_cast<System::Int32>(static_cast<System::Byte>(30)));
			this->ClientSize = System::Drawing::Size(1262, 673);
			this->Controls->Add(this->destinationsPanel);
			this->Controls->Add(this->pnlForButton);
			this->Controls->Add(this->PanelSlidebar);
			this->Controls->Add(this->pictureBox1);
			this->MaximizeBox = false;
			this->MinimizeBox = false;
			this->Name = L"Dashboard";
			this->StartPosition = System::Windows::Forms::FormStartPosition::CenterScreen;
			this->Text = L"LankaJourney";
			(cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->pictureBox1))->EndInit();
			this->PanelSlidebar->ResumeLayout(false);
			(cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->horizontalLine))->EndInit();
			this->pnlForButton->ResumeLayout(false);
			this->destinationsPanel->ResumeLayout(false);
			(cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->destinationPictureBox))->EndInit();
			this->ResumeLayout(false);

		}
#pragma endregion
	private: System::Void pictureBox1_Click(System::Object^ sender, System::EventArgs^ e) {
	}
private: System::Void button1_Click(System::Object^ sender, System::EventArgs^ e) {
}
};
}
