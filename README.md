# LankaJourney

LankaJourney is a Sri Lanka travel planning project designed to help users explore destinations, accommodation, tourist attractions, transportation options, and dining locations in a single desktop application. The project is based on the project outline in `Documents/Project outline.pdf` and the supporting schema and sample database files included in this repository.

## Project Overview

This project aims to provide a simple but effective travel planning experience for tourists visiting Sri Lanka. It focuses on organizing destination information by location, district, and province while allowing users to browse useful travel-related services such as:

- Tourist places and attractions
- Accommodation facilities
- Vehicle rental agencies and vehicle options
- Restaurants and local food spots
- Travel planning through a dashboard-based interface

The desktop application uses a C# WPF shell with WebView2 for its interface, a native C++17 journey-planning engine, and MySQL for travel data. Leaflet.js with public OpenStreetMap tiles displays the journey map; public OSRM routes provide driving distances without a billing account or API key.

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
- Personalized day-by-day itineraries generated from the planning form
- Starting-point selection from Katunayake International Airport or a pickup location chosen on the map

## Technology Stack

- C# WPF and WebView2 (.NET 10)
- C++17 (`g++`) native itinerary planner
- MySQL Server and MySQL Connector
- Leaflet.js, OpenStreetMap tiles and public OSRM driving routes

## Repository Structure

```text
LankaJourney/
├── main.cpp
├── Core/
├── Database/
│   ├── MySQL_Schemas/
│   └── MySQL_Sample Data/
├── Documents/
│   └── Project outline.pdf
├── Report/
│   └── report.pdf
├── UI/
│   ├── WPF_Application/
│   └── WebUI/
├── README.md
├── process.md
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

This relational design is defined in `Database/MySQL_Schemas/` and populated with seed data under `Database/MySQL_Sample Data/`.

## Project Data

The sample data files include a large set of Sri Lankan travel-related records, including:

- Location entries across multiple districts and provinces
- Accommodation records with prices and ratings
- Tourist place records for scenic and cultural destinations
- Vehicle agency and rental options
- Restaurant data with contact information

## Setup and Run

Install the .NET 10 SDK, a C++17 MinGW-w64 `g++` compiler, and MySQL Server. Import `Database/MySQL_Schemas/LankaStay_corrected_schema by GPT.sql`, then import `Database/MySQL_Sample Data/LankaStay_sample_data 2.sql` in MySQL Workbench. Use sample set 2 by itself; the sample sets reuse database IDs.

Configure `LANKAJOURNEY_MYSQL_CONNECTION` in the environment used to launch the application. An internet connection is required to load the Leaflet library, OpenStreetMap tiles and public OSRM driving routes. These public services require no API key or billing account and may be rate-limited or temporarily unavailable.

Build and run from the repository root in PowerShell:

```powershell
dotnet build .\UI\WPF_Application\WPF_Application.csproj
dotnet run --project .\UI\WPF_Application\WPF_Application.csproj
```

The build compiles `main.cpp` and bundles the WebUI for the desktop app. For the complete setup, database account instructions, request flow, map attribution and basemap/routing service usage notes, see [`process.md`](./process.md).

## Run and Usage

Open Planning, enter budget, trip length and interests, then select **Explore the journey** to generate and review the itinerary and interactive map. Database errors are reported in the app; if public routing is unavailable, the itinerary is still displayed with an explanation that driving distances could not be calculated.

## Notes

- The project is designed as an academic/training application for travel planning and database-driven UI development.
- The database seed data includes Sri Lankan towns and destinations for testing and demonstration purposes; verify its coordinates before production use.
- Restaurant, transport and activity prices are not in the current schema, so the displayed cost estimate covers accommodation only.

## Contributors

This project was developed as part of the COM1305 course work for travel and tourism system development.

## License

This project is intended for academic and educational use. Please check with the project owner or course instructor for any additional licensing or usage constraints.
