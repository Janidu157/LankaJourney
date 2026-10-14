using System.Diagnostics;
using System.Globalization;
using System.IO;
using System.Net.Http;
using System.Text;
using System.Text.Json;
using System.Text.Json.Nodes;
using MySqlConnector;

namespace WPF_Application;

public sealed class PlanningRequest
{
    public decimal AccommodationBudget { get; init; }
    public decimal FoodBudget { get; init; }
    public decimal TransportBudget { get; init; }
    public decimal ActivityBudget { get; init; }
    public decimal TotalBudget { get; init; }
    public decimal? BudgetPerDay { get; init; }
    public int Days { get; init; }
    public int Nights { get; init; }
    public string[] Destinations { get; init; } = [];
    public StartingPointRequest StartingPoint { get; init; } = new();
}

public sealed class StartingPointRequest
{
    public string Name { get; init; } = string.Empty;
    public double? Latitude { get; init; }
    public double? Longitude { get; init; }
}

public sealed class JourneyPlanningService
{
    private const string OsrmRouteUrl = "https://router.project-osrm.org/route/v1/driving/";
    private static readonly JsonSerializerOptions JsonOptions = new(JsonSerializerDefaults.Web);
    private readonly HttpClient httpClient = new();

    public async Task<JsonObject> CreateJourneyAsync(PlanningRequest requirements, CancellationToken cancellationToken = default)
    {
        Validate(requirements);

        string connectionString = Environment.GetEnvironmentVariable("LANKAJOURNEY_MYSQL_CONNECTION")
            ?? throw new InvalidOperationException(
                "MySQL is not configured. Set the LANKAJOURNEY_MYSQL_CONNECTION environment variable; see process.md.");

        string plannerPath = Path.Combine(AppContext.BaseDirectory, "planner.exe");
        if (!File.Exists(plannerPath))
        {
            throw new FileNotFoundException(
                "The native journey planner was not built. Rebuild the WPF project with g++ available on PATH.",
                plannerPath);
        }

        var startInfo = new ProcessStartInfo(plannerPath)
        {
            UseShellExecute = false,
            CreateNoWindow = true,
            RedirectStandardInput = true,
            RedirectStandardOutput = true,
            RedirectStandardError = true,
            StandardInputEncoding = new UTF8Encoding(false),
            StandardOutputEncoding = new UTF8Encoding(false),
            StandardErrorEncoding = new UTF8Encoding(false)
        };

        using var process = new Process { StartInfo = startInfo };
        if (!process.Start())
        {
            throw new InvalidOperationException("The native journey planner could not be started.");
        }

        Task<string> errorTask = process.StandardError.ReadToEndAsync(cancellationToken);
        try
        {
            await process.StandardInput.WriteLineAsync(JsonSerializer.Serialize(requirements, JsonOptions));
            await process.StandardInput.FlushAsync(cancellationToken);

            string? handshake = await process.StandardOutput.ReadLineAsync(cancellationToken);
            if (handshake is null)
            {
                await process.WaitForExitAsync(cancellationToken);
                string plannerError = await errorTask;
                throw new InvalidOperationException(string.IsNullOrWhiteSpace(plannerError)
                    ? $"The native journey planner exited with code {process.ExitCode} before requesting database data."
                    : $"The native journey planner rejected the requirements: {plannerError.Trim()}");
            }

            using (JsonDocument handshakeDocument = JsonDocument.Parse(handshake))
            {
                if (!handshakeDocument.RootElement.TryGetProperty("type", out JsonElement type) ||
                    type.GetString() != "catalogRequest")
                {
                    throw new InvalidOperationException("The native journey planner returned an unexpected database request.");
                }
            }

            var (places, accommodations, restaurants) = await LoadCatalogAsync(connectionString, cancellationToken);
            var catalog = new { places, accommodations, restaurants };
            Task<string> outputTask = process.StandardOutput.ReadToEndAsync(cancellationToken);
            await process.StandardInput.WriteLineAsync(JsonSerializer.Serialize(catalog, JsonOptions));
            process.StandardInput.Close();
            await process.WaitForExitAsync(cancellationToken);

            string output = await outputTask;
            string error = await errorTask;
            if (process.ExitCode != 0)
            {
                throw new InvalidOperationException(string.IsNullOrWhiteSpace(error)
                    ? $"The native journey planner exited with code {process.ExitCode}."
                    : $"The native journey planner failed: {error.Trim()}");
            }

            if (JsonNode.Parse(output) is not JsonObject journey)
            {
                throw new InvalidOperationException("The native journey planner returned an invalid response.");
            }

            await AddOpenStreetMapRouteAsync(journey, cancellationToken);
            return journey;
        }
        finally
        {
            if (!process.HasExited)
            {
                process.Kill(entireProcessTree: true);
            }
        }
    }

    private static void Validate(PlanningRequest request)
    {
        if (request.Days is < 1 or > 30)
        {
            throw new InvalidOperationException("Choose a trip length between 1 and 30 days.");
        }
        if (request.Nights < 0 || request.Nights > request.Days)
        {
            throw new InvalidOperationException("The number of nights must be between 0 and the number of days.");
        }
        if (request.AccommodationBudget < 0 || request.FoodBudget < 0 || request.TransportBudget < 0 ||
            request.ActivityBudget < 0 || request.TotalBudget < 0 ||
            (request.BudgetPerDay.HasValue && request.BudgetPerDay.Value < 0))
        {
            throw new InvalidOperationException("Budgets cannot be negative.");
        }

        string[] allowedDestinations =
        [
            "Upcountry", "Down South", "Beach", "Historical", "Cultural",
            "Religious", "Wildlife", "Adventure", "Shopping", "Nature"
        ];
        if (request.Destinations.Length == 0 ||
            request.Destinations.Any(destination => !allowedDestinations.Contains(destination, StringComparer.Ordinal)))
        {
            throw new InvalidOperationException("Select at least one valid destination or experience.");
        }
        if (string.IsNullOrWhiteSpace(request.StartingPoint.Name) ||
            !request.StartingPoint.Latitude.HasValue ||
            !double.IsFinite(request.StartingPoint.Latitude.Value) ||
            request.StartingPoint.Latitude.Value is < -90 or > 90 ||
            !request.StartingPoint.Longitude.HasValue ||
            !double.IsFinite(request.StartingPoint.Longitude.Value) ||
            request.StartingPoint.Longitude.Value is < -180 or > 180)
        {
            throw new InvalidOperationException("Choose a valid starting point for your journey.");
        }
    }

    private static async Task<(List<PlaceRow>, List<AccommodationRow>, List<RestaurantRow>)> LoadCatalogAsync(
        string connectionString,
        CancellationToken cancellationToken)
    {
        await using var connection = new MySqlConnection(connectionString);
        await connection.OpenAsync(cancellationToken);

        const string placesSql = """
            SELECT p.Place_ID, p.Place_Name, p.Place_Type, p.Visiting_Experience, p.Climate,
                   p.Accessibility, p.Diff_Level, l.Town, l.District, l.Latitude, l.Longitude
            FROM Tourist_Places p
            LEFT JOIN Location l ON l.Location_ID = p.Location_ID
            ORDER BY p.Place_Name
            """;
        var places = new List<PlaceRow>();
        await using (var command = new MySqlCommand(placesSql, connection))
        await using (var reader = await command.ExecuteReaderAsync(cancellationToken))
        {
            while (await reader.ReadAsync(cancellationToken))
            {
                places.Add(new PlaceRow(
                    reader.GetString(0), reader.GetString(1), ReadText(reader, 2), ReadText(reader, 3),
                    ReadText(reader, 4), ReadText(reader, 5), ReadText(reader, 6), ReadText(reader, 7),
                    ReadText(reader, 8), ReadCoordinate(reader, 9, -90, 90), ReadCoordinate(reader, 10, -180, 180)));
            }
        }

        const string accommodationsSql = """
            SELECT a.Accom_ID, a.Accom_Name, a.Accom_Type, a.Accom_Price, a.Rating,
                   l.Town, l.District, l.Latitude, l.Longitude
            FROM Accommodation a
            LEFT JOIN Location l ON l.Location_ID = a.Location_ID
            ORDER BY a.Accom_Name
            """;
        var accommodations = new List<AccommodationRow>();
        await using (var command = new MySqlCommand(accommodationsSql, connection))
        await using (var reader = await command.ExecuteReaderAsync(cancellationToken))
        {
            while (await reader.ReadAsync(cancellationToken))
            {
                accommodations.Add(new AccommodationRow(
                    reader.GetString(0), ReadText(reader, 1), ReadText(reader, 2), ReadNumber(reader, 3) ?? 0,
                    ReadNumber(reader, 4) ?? 0, ReadText(reader, 5), ReadText(reader, 6),
                    ReadCoordinate(reader, 7, -90, 90), ReadCoordinate(reader, 8, -180, 180)));
            }
        }

        const string restaurantsSql = """
            SELECT r.Restaurant_ID, r.Restaurant_Name, r.Rating, l.Town, l.District, l.Latitude, l.Longitude
            FROM Restaurant r
            LEFT JOIN Location l ON l.Location_ID = r.Location_ID
            ORDER BY r.Restaurant_Name
            """;
        var restaurants = new List<RestaurantRow>();
        await using (var command = new MySqlCommand(restaurantsSql, connection))
        await using (var reader = await command.ExecuteReaderAsync(cancellationToken))
        {
            while (await reader.ReadAsync(cancellationToken))
            {
                restaurants.Add(new RestaurantRow(
                    reader.GetString(0), reader.GetString(1), ReadNumber(reader, 2) ?? 0,
                    ReadText(reader, 3), ReadText(reader, 4), ReadCoordinate(reader, 5, -90, 90),
                    ReadCoordinate(reader, 6, -180, 180)));
            }
        }

        return (places, accommodations, restaurants);
    }

    private async Task AddOpenStreetMapRouteAsync(JsonObject journey, CancellationToken cancellationToken)
    {
        JsonArray days = journey["days"]?.AsArray()
            ?? throw new InvalidOperationException("The native journey planner did not return any days.");

        JsonObject? startingPoint = journey["startingPoint"] as JsonObject;
        var routeStops = new List<(JsonObject? Day, double Latitude, double Longitude)>();
        if (TryCoordinates(startingPoint, out double startLatitude, out double startLongitude))
        {
            routeStops.Add((null, startLatitude, startLongitude));
        }

        var destinationDays = new List<JsonObject>();
        foreach (JsonNode? dayNode in days)
        {
            if (dayNode is JsonObject day && day["destination"] is JsonObject destination &&
                TryCoordinates(destination, out double latitude, out double longitude))
            {
                routeStops.Add((day, latitude, longitude));
                destinationDays.Add(day);
            }
            else if (dayNode is JsonObject dayWithoutCoordinates &&
                     dayWithoutCoordinates["destination"] is JsonObject)
            {
                dayWithoutCoordinates["routeNotice"] = "This destination does not have coordinates in the database.";
            }
        }

        if (routeStops.Count < 2 || routeStops[0].Day is not null)
        {
            journey["mapMessage"] =
                "OpenStreetMap shows available locations, but a driving route needs coordinates for your starting point and at least one destination.";
            return;
        }

        string coordinates = string.Join(";", routeStops.Select(stop =>
            $"{stop.Longitude.ToString(CultureInfo.InvariantCulture)},{stop.Latitude.ToString(CultureInfo.InvariantCulture)}"));
        string routeUri = $"{OsrmRouteUrl}{Uri.EscapeDataString(coordinates)}?overview=full&geometries=geojson&steps=false";

        try
        {
            using var request = new HttpRequestMessage(HttpMethod.Get, routeUri);
            request.Headers.UserAgent.ParseAdd("LankaJourney/1.0");
            using HttpResponseMessage response = await httpClient.SendAsync(request, cancellationToken);
            if (!response.IsSuccessStatusCode)
            {
                SetRouteUnavailable(journey, destinationDays,
                    $"The itinerary was created, but the OpenStreetMap routing service returned HTTP {(int)response.StatusCode}. Driving distances are unavailable.");
                return;
            }

            using JsonDocument routeDocument = await JsonDocument.ParseAsync(
                await response.Content.ReadAsStreamAsync(cancellationToken), cancellationToken: cancellationToken);
            JsonElement root = routeDocument.RootElement;
            if (!root.TryGetProperty("code", out JsonElement code) || code.GetString() != "Ok" ||
                !root.TryGetProperty("routes", out JsonElement routes) ||
                routes.ValueKind != JsonValueKind.Array || routes.GetArrayLength() == 0)
            {
                string message = root.TryGetProperty("message", out JsonElement detail)
                    ? detail.GetString() ?? "No drivable route was found."
                    : "No drivable route was found.";
                SetRouteUnavailable(journey, destinationDays,
                    $"The itinerary was created, but OpenStreetMap could not route this itinerary: {message}");
                return;
            }

            JsonElement route = routes[0];
            if (!route.TryGetProperty("legs", out JsonElement legs) ||
                legs.ValueKind != JsonValueKind.Array ||
                legs.GetArrayLength() != destinationDays.Count ||
                !route.TryGetProperty("geometry", out JsonElement geometry) ||
                geometry.ValueKind != JsonValueKind.Object ||
                !geometry.TryGetProperty("coordinates", out JsonElement routeCoordinates) ||
                routeCoordinates.ValueKind != JsonValueKind.Array)
            {
                SetRouteUnavailable(journey, destinationDays,
                    "The itinerary was created, but the OpenStreetMap routing service returned incomplete route data. Driving distances are unavailable.");
                return;
            }

            for (int legIndex = 0; legIndex < destinationDays.Count; ++legIndex)
            {
                JsonElement leg = legs[legIndex];
                if (!leg.TryGetProperty("distance", out JsonElement distance) ||
                    !distance.TryGetDouble(out double distanceMeters) ||
                    !leg.TryGetProperty("duration", out JsonElement duration) ||
                    !duration.TryGetDouble(out double durationSeconds))
                {
                    SetRouteUnavailable(journey, destinationDays,
                        "The itinerary was created, but the OpenStreetMap routing service returned incomplete route data. Driving distances are unavailable.");
                    return;
                }

                JsonObject day = destinationDays[legIndex];
                day["routeDistanceMeters"] = distanceMeters;
                day["routeDuration"] = $"{durationSeconds.ToString(CultureInfo.InvariantCulture)}s";
                day["routeFrom"] = legIndex == 0
                    ? startingPoint?["name"]?.GetValue<string>() ?? "your starting point"
                    : "the previous mapped stop";
            }

            journey["routeCoordinates"] = JsonNode.Parse(routeCoordinates.GetRawText());
            journey["mapMessage"] =
                "OpenStreetMap map data © OpenStreetMap contributors. Driving routes and distances are provided by the public OSRM demo service; the route starts at your selected pickup point.";
        }
        catch (HttpRequestException)
        {
            SetRouteUnavailable(journey, destinationDays,
                "The itinerary was created, but the OpenStreetMap routing service could not be reached. Driving distances are unavailable.");
        }
        catch (OperationCanceledException) when (!cancellationToken.IsCancellationRequested)
        {
            SetRouteUnavailable(journey, destinationDays,
                "The itinerary was created, but the OpenStreetMap routing request timed out. Driving distances are unavailable.");
        }
        catch (JsonException)
        {
            SetRouteUnavailable(journey, destinationDays,
                "The itinerary was created, but the OpenStreetMap routing service returned an invalid response. Driving distances are unavailable.");
        }
    }

    private static void SetRouteUnavailable(JsonObject journey, IEnumerable<JsonObject> days, string message)
    {
        journey["mapMessage"] = message;
        foreach (JsonObject day in days)
        {
            day["routeNotice"] = "Driving distance is unavailable for this stop.";
        }
    }

    private static bool TryCoordinates(JsonObject? item, out double latitude, out double longitude)
    {
        latitude = 0;
        longitude = 0;
        if (item?["latitude"] is not JsonValue latitudeValue ||
            item["longitude"] is not JsonValue longitudeValue ||
            !latitudeValue.TryGetValue(out latitude) ||
            !longitudeValue.TryGetValue(out longitude))
        {
            return false;
        }

        return double.IsFinite(latitude) && latitude is >= -90 and <= 90 &&
               double.IsFinite(longitude) && longitude is >= -180 and <= 180;
    }

    private static string ReadText(MySqlDataReader reader, int ordinal) =>
        reader.IsDBNull(ordinal) ? string.Empty : Convert.ToString(reader.GetValue(ordinal), CultureInfo.InvariantCulture) ?? string.Empty;

    private static double? ReadNumber(MySqlDataReader reader, int ordinal) =>
        reader.IsDBNull(ordinal) ? null : Convert.ToDouble(reader.GetValue(ordinal), CultureInfo.InvariantCulture);

    private static double? ReadCoordinate(MySqlDataReader reader, int ordinal, double minimum, double maximum)
    {
        double? value = ReadNumber(reader, ordinal);
        return value.HasValue && double.IsFinite(value.Value) &&
               value.Value >= minimum && value.Value <= maximum
            ? value
            : null;
    }

    private sealed record PlaceRow(
        string Id, string Name, string Type, string Experience, string Climate, string Accessibility,
        string Difficulty, string Town, string District, double? Latitude, double? Longitude);

    private sealed record AccommodationRow(
        string Id, string Name, string Type, double Price, double Rating, string Town, string District,
        double? Latitude, double? Longitude);

    private sealed record RestaurantRow(
        string Id, string Name, double Rating, string Town, string District, double? Latitude, double? Longitude);

}
