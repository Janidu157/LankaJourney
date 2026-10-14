# LankaJourney Planning Process

This document describes how the planning flow works, how to configure the database and OpenStreetMap routing, and how to run the WPF application on Windows.

## Architecture and request flow

1. The user enters an accommodation, food, transport and activity budget, trip length, interests, and a starting point on `UI/WebUI/planning.html`. Katunayake International Airport is the default; a user can instead open the map picker and set a home or other pickup location.
2. The **Explore the journey** button validates the form, keeps the preferences in browser storage, and sends them through the WebView2 message bridge. The WPF host only accepts planning messages from its bundled local planning page.
3. `UI/WPF_Application/JourneyPlanningService.cs` validates the request and starts the native `planner.exe`. The WPF build creates this executable from the repository-root `main.cpp`; it does not use or overwrite the separate root-level `planner.exe` file.
4. The native program constructs a `user` object from the submitted preferences and pickup coordinates **before** it asks the WPF host for database records. This request/response handshake ensures the requirements are held by the C++ planner before database retrieval begins.
5. The WPF host uses the MySQL connection string from the environment to read tourist places, accommodation and restaurants, including each record's linked location and coordinates. It passes this catalog to the waiting C++ process.
6. `main.cpp` matches places to the selected experience/region preferences, then orders equally matched destinations by distance from the chosen starting point. It ranks stays by budget fit, proximity to the selected itinerary (using saved coordinates where available), rating and price. If the catalog has fewer matching places than trip days, the remaining days are left flexible rather than repeating destinations. It suggests the highest-rated restaurant found in the destination's town or district.
7. The WPF host sends the starting point and mapped destinations, in itinerary order, to the public OSRM driving router. It returns a drivable route, route geometry for the map, and distances/durations for each leg. Day 1 starts at the selected pickup point; subsequent days start at the previous mapped destination. The suggested accommodation is shown on the map but is not inserted as a route stop.
8. The itinerary and route are sent back to WebView2 and displayed on `UI/WebUI/journey.html` using Leaflet.js and OpenStreetMap tiles. **Adjust your requirements** returns the user to the planning form.

The desktop WPF app loads the WebUI files bundled beside the built application; it does not depend on GitHub Pages being current. Opening the WebUI directly in a normal browser does not run the native planner or make database/routing calls: the planning page displays a message directing the user to the WPF desktop app.

## What is and is not estimated

- Accommodation prices in the sample schema are treated as a **per-night** price. The displayed accommodation total is that price multiplied by the requested number of nights.
- When the accommodation budget is zero, the planner treats it as unspecified and ranks stays by rating and price. Otherwise, it first prefers options within the accommodation budget divided by the number of nights. If none fit, it still suggests the best available option and marks that it exceeded the budget.
- The supplied schema does not include restaurant prices, activity fees, or transport prices. The planner does not invent those costs; it shows the user's budget allocations and clearly labels the accommodation-only estimate.
- Katunayake International Airport is the default starting point. Users who start elsewhere should choose **Choose a pickup point** and set their location on the map; the planner uses these coordinates to order similarly matched destinations and the route begins there.
- Distance routing requires valid latitude/longitude values in `Location`. The sample coordinates are development data; verify them against the actual premises before relying on directions.
- Leaflet renders raster map tiles from the public `tile.openstreetmap.de` endpoint and requires an internet connection. Attribution for OpenStreetMap contributors is shown below the map and in Leaflet's controls. CARTO's Voyager endpoint is not used because it requires a key.
- Driving distances use the public OSRM demo routing service at `router.project-osrm.org`; this service is independent of OpenStreetMap's tile servers, requires no API key or billing account, and may be unavailable or rate-limited. If routing fails, the itinerary still appears with an explicit route-unavailable message. For production or heavy use, host OSRM yourself or choose a routing provider with an appropriate service agreement.
- Use the public OpenStreetMap tile endpoint for normal interactive map viewing only; do not bulk-download or prefetch tiles. Public tile servers are community services and are not a guaranteed production service. Observe the OSRM demo server's fair-use limits as well.

## Requirements

- Windows 10/11.
- .NET 10 SDK and Windows Desktop/WPF support.
- A C++17 MinGW-w64 `g++` compiler on `PATH`. The WPF project builds the native planner automatically.
- A running MySQL 8 server. MySQL Workbench is optional but convenient for importing the scripts.
- Internet access to load Leaflet, OpenStreetMap map tiles, and public OSRM driving routes.

## Prepare MySQL

1. Start MySQL Server locally or note the address of the MySQL server you will use. The WPF app does not install or start a database server automatically.
2. In MySQL Workbench, open and execute [`Database/MySQL_Schemas/LankaStay_corrected_schema by GPT.sql`](./Database/MySQL_Schemas/LankaStay_corrected_schema%20by%20GPT.sql). It creates the `LankaStay` database and the tables used by the planner. The accommodation table includes the `Website` column required by the sample data import.
3. Open and execute [`Database/MySQL_Sample Data/LankaStay_sample_data 2.sql`](./Database/MySQL_Sample%20Data/LankaStay_sample_data%202.sql) to insert the sample records. It contains `USE LankaStay`. Use this sample set by itself: sample set 1 reuses primary keys and should not be loaded on top of sample set 2.
4. For a safer application account, create a MySQL user that can read the database but cannot change it. For a local server, run the following in Workbench and replace the example password with your own:

   ```sql
   CREATE USER 'lankajourney_app'@'localhost' IDENTIFIED BY 'choose-a-local-password';
   GRANT SELECT ON LankaStay.* TO 'lankajourney_app'@'localhost';
   ```

   If MySQL is remote, create the account for the appropriate client host instead of `localhost` and configure the server firewall accordingly.
5. In the PowerShell window that will launch the app, configure the connection string. The environment variable is read by the WPF process; do not put a real password in source files or commit it:

   ```powershell
   $env:LANKAJOURNEY_MYSQL_CONNECTION = "Server=127.0.0.1;Port=3306;Database=LankaStay;User ID=lankajourney_app;Password=choose-a-local-password;"
   ```

   Use the correct server, port, username and password for your MySQL installation.

The planner reads `Tourist_Places`, `Accommodation`, `Restaurant`, and their related `Location` records. Its database account only needs `SELECT` access.

If you created the database using an older copy of the schema, check whether `Accommodation.Website` exists before importing sample set 2. If it does not, add it once with `ALTER TABLE Accommodation ADD COLUMN Website VARCHAR(200);`. Do not re-run the full `CREATE TABLE` schema against an already-created database.

## OpenStreetMap and routing

No API key, Google Cloud project, or billing card is required. When a journey is generated, the WPF host makes one HTTPS request to the public OSRM route endpoint. Leaflet displays the itinerary locations and returned road route on OpenStreetMap tiles from `tile.openstreetmap.de`. The map credits OpenStreetMap contributors.

The basemap tiles and public routing service need internet access. If the tile provider blocks requests or OSRM is unavailable or cannot find a continuous route, the journey remains visible and the page explains why its map or driving distance is unavailable. Public basemap and OSRM demo endpoints are for development/light use, not a production availability guarantee; move to a self-hosted tile/routing service or a suitably licensed provider if usage grows.

## Build and run

From the repository root in PowerShell:

```powershell
dotnet build .\UI\WPF_Application\WPF_Application.csproj
dotnet run --project .\UI\WPF_Application\WPF_Application.csproj
```

The first build restores the WPF and MySQL Connector dependencies, compiles `main.cpp` into the WPF output directory, and copies the WebUI assets there. Keep the MySQL connection environment variable set in the same PowerShell session when running the app. Submit the planning form and the journey result opens inside the desktop application.

If using Visual Studio, open `UI/WPF_Application/WPF_Application.slnx`, ensure `g++` is available to the Visual Studio process, and run the WPF project. Set the MySQL connection environment variable in the Visual Studio launch environment. A missing MySQL setting, failed MySQL connection, or missing native executable is reported on the planning page instead of being presented as a successful plan; a routing-service problem is shown alongside the generated itinerary.

## Main files

- `main.cpp` — C++17 `user` model, preference matching, accommodation ranking, day-by-day itinerary and JSON protocol.
- `UI/WPF_Application/MainWindow.xaml.cs` — local WebView2 hosting, request validation and browser/native message bridge.
- `UI/WPF_Application/JourneyPlanningService.cs` — MySQL catalog reads, native process lifecycle, OSRM route request and result handling.
- `UI/WebUI/planning.html` and `UI/WebUI/js/planner.js` — journey preference form and WPF bridge client.
- `UI/WebUI/journey.html` and `UI/WebUI/js/journey.js` — generated itinerary and Leaflet/OpenStreetMap map display.
- `Database/MySQL_Schemas/LankaStay_corrected_schema by GPT.sql` — schema used by the app and sample-data import.

## Project folder structure

The tree below lists the project files and folders, excluding Git metadata, generated build output (`bin`/`obj`), and machine-specific Visual Studio user settings.

```text
LankaJourney/
|-- .gitignore
|-- README.md
|-- main.cpp
|-- planner.exe
|-- process.md
|-- Core/
|   |-- accommodation.cpp
|   |-- accommodation.h
|   |-- destination.cpp
|   |-- destinations.h
|   |-- restaurants.cpp
|   `-- restaurants.h
|-- Database/
|   |-- MySQL_Sample Data/
|   |   |-- LankaStay_sample_data 1.sql
|   |   `-- LankaStay_sample_data 2.sql
|   `-- MySQL_Schemas/
|       |-- LankaStay_corrected_schema by GPT.sql
|       `-- Version1.sql
|-- Documents/
|   `-- Project outline.pdf
|-- Report/
|   `-- report.pdf
|-- Test/
|   `-- test1.txt
`-- UI/
    |-- WebUI/
    |   |-- README.txt
    |   |-- accommodations.html
    |   |-- bookings.html
    |   |-- destinations.html
    |   |-- index.html
    |   |-- journey.html
    |   |-- login.html
    |   |-- planning.html
    |   |-- restaurants.html
    |   |-- settings.html
    |   |-- vehicles.html
    |   |-- css/
    |   |   `-- style.css
    |   `-- js/
    |       |-- app.js
    |       |-- journey.js
    |       `-- planner.js
    `-- WPF_Application/
        |-- App.xaml
        |-- App.xaml.cs
        |-- AssemblyInfo.cs
        |-- JourneyPlanningService.cs
        |-- MainWindow.xaml
        |-- MainWindow.xaml.cs
        |-- WPF_Application.csproj
        |-- WPF_Application.slnx
        `-- Properties/
            `-- PublishProfiles/
                |-- ClickOnceProfile.pubxml
                `-- FolderProfile.pubxml
```
