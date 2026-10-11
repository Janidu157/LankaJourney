# LankaJourney

LankaJourney is a Sri Lanka travel planning project designed to help users explore destinations, accommodation, tourist attractions, transportation options, and dining locations in a single desktop application. The project is based on the project outline in `Documents/Project outline.pdf` and the supporting schema and sample database files included in this repository.

## Project Overview

This project aims to provide a simple but effective travel planning experience for tourists visiting Sri Lanka. It focuses on organizing destination information by location, district, and province while allowing users to browse useful travel-related services such as:

- Tourist places and attractions
- Accommodation facilities
- Vehicle rental agencies and vehicle options
- Restaurants and local food spots
- Travel planning through a dashboard-based interface

The application is built as a Windows desktop application using C++/CLI and Windows Forms, with data stored in a MySQL database.

## Problem Addressed

Travelers often need a centralized system to discover places, compare stay options, and plan their routes without switching between multiple sources. LankaJourney tries to simplify this by combining location data and travel services into one integrated platform.

## Features

- Dashboard-based interface for journey planning
- Location and town-based travel information
- Accommodation management with pricing and ratings
- Tourist place listings by type and accessibility
- Vehicle agency and rental information with passenger capacity
- Restaurant details with ratings and contact information
- Structured relational database design for travel data

## Technology Stack

- C++/CLI
- Windows Forms (WinForms)
- Visual Studio
- MySQL / MySQL Workbench
- SQL schema and sample data files

## Repository Structure

```text
LankaJourney/
├── Documents/
│   └── Project outline.pdf
├── MySQL_Schemas/
│   ├── LankaStay_corrected_schema by GPT.sql
│   └── Version1.sql
├── MySQL_Sample Data/
│   ├── LankaStay_sample_data 1.sql
│   └── LankaStay_sample_data 2.sql
├── Report/
│   └── report.pdf
├── UI/
│   ├── LankaJourneyUI/
│   │   ├── Dashboard.cpp
│   │   ├── Dashboard.h
│   │   ├── LankaJourneyUI.slnx
│   │   ├── LankaJourneyUI.vcxproj
│   │   └── ...
│   └── UI_Designs/
├── README.md
└── .gitignore
```

## Database Design

The database model includes a normalized structure for Sri Lanka travel data. Core tables include:

- `Location` - stores latitude, longitude, town, district, and province details
- `Accommodation` - stores accommodation type, price, rating, and food availability
- `Tourist_Places` - stores place details, climate, experience type, and difficulty
- `Vehicle_Agency` - stores agency names and contact information
- `Vehicle_Rent` - stores rental categories, passenger capacity, luggage capacity, and driver preference
- `Restaurant` - stores restaurant details and ratings

This relational design is defined in the SQL scripts under `MySQL_Schemas/` and populated with seed data under `MySQL_Sample Data/`.

## Project Data

The sample data files include a large set of Sri Lankan travel-related records, including:

- Location entries across multiple districts and provinces
- Accommodation records with prices and ratings
- Tourist place records for scenic and cultural destinations
- Vehicle agency and rental options
- Restaurant data with contact information

## Setup Instructions

### 1. Prerequisites

- Microsoft Visual Studio 2022
- MySQL Server / MySQL Workbench
- Windows operating system

### 2. Configure the Database

1. Open MySQL Workbench or a MySQL client.
2. Create a database named `LankaStay` or use the schema file to create the required tables.
3. Import the schema file:
   - `MySQL_Schemas/LankaStay_corrected_schema by GPT.sql`
4. Optionally import the sample data files from:
   - `MySQL_Sample Data/LankaStay_sample_data 1.sql`
   - `MySQL_Sample Data/LankaStay_sample_data 2.sql`

### 3. Open the Project

1. Go to the UI folder:
   - `UI/LankaJourneyUI/`
2. Open the solution file:
   - `LankaJourneyUI.slnx`
3. Build the project in Visual Studio.
4. Run the application in Debug or Release mode.

## Run and Usage

Once the application is running, the dashboard acts as the main entry point for planning a journey. The interface is intended to guide the user through travel-related sections such as destination browsing, accommodation suggestions, and planning tools in the desktop environment.

## Notes

- The project is designed as an academic/training application for travel planning and database-driven UI development.
- The database seed data includes many Sri Lankan towns and destinations for testing and demonstration purposes.
- The schema and sample files are useful references for future enhancements such as search filtering, booking flow, or map integration.

## Contributors

This project was developed as part of the COM1305 course work for travel and tourism system development.

## License

This project is intended for academic and educational use. Please check with the project owner or course instructor for any additional licensing or usage constraints.
