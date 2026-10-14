using System.IO;
using System.Net.Http;
using System.Text.Json;
using System.Text.Json.Nodes;
using System.Windows;
using Microsoft.Web.WebView2.Core;
using MySqlConnector;

namespace WPF_Application;

public partial class MainWindow : Window
{
    private readonly JourneyPlanningService planningService = new();
    private string webContentRoot = string.Empty;
    private JsonObject? pendingJourney;

    public MainWindow()
    {
        InitializeComponent();
        Loaded += async (_, _) => await InitializeAsync();
    }

    private async Task InitializeAsync()
    {
        try
        {
            await webView.EnsureCoreWebView2Async();
            webView.CoreWebView2.WebMessageReceived += HandleWebMessage;
            webView.CoreWebView2.NavigationCompleted += HandleNavigationCompleted;

            webContentRoot = Path.GetFullPath(Path.Combine(AppContext.BaseDirectory, "WebUI"));
            string indexPath = Path.Combine(webContentRoot, "index.html");
            if (!File.Exists(indexPath))
            {
                throw new FileNotFoundException("The bundled LankaJourney WebUI could not be found.", indexPath);
            }

            webView.Source = new Uri(indexPath);
        }
        catch (Exception exception) when (exception is IOException or InvalidOperationException or System.Runtime.InteropServices.COMException)
        {
            MessageBox.Show(this, $"LankaJourney could not start: {exception.Message}", "Startup error",
                MessageBoxButton.OK, MessageBoxImage.Error);
            Close();
        }
    }

    private async void HandleWebMessage(object? sender, CoreWebView2WebMessageReceivedEventArgs eventArgs)
    {
        if (!IsPlanningPage(eventArgs.Source))
        {
            return;
        }

        try
        {
            string message = eventArgs.TryGetWebMessageAsString();
            using JsonDocument document = JsonDocument.Parse(message);
            if (!document.RootElement.TryGetProperty("type", out JsonElement type) ||
                type.GetString() != "generatePlan" ||
                !document.RootElement.TryGetProperty("plan", out JsonElement planElement))
            {
                throw new InvalidOperationException("The planning page sent an unsupported request.");
            }

            PlanningRequest requirements = planElement.Deserialize<PlanningRequest>(
                new JsonSerializerOptions(JsonSerializerDefaults.Web) { PropertyNameCaseInsensitive = true })
                ?? throw new InvalidOperationException("The journey requirements could not be read.");
            pendingJourney = await planningService.CreateJourneyAsync(requirements);
            webView.CoreWebView2.PostWebMessageAsJson(JsonSerializer.Serialize(new
            {
                type = "journeyReady"
            }));
        }
        catch (Exception exception) when (exception is JsonException or InvalidOperationException or
                                          MySqlException or IOException or HttpRequestException or
                                          System.ComponentModel.Win32Exception or OperationCanceledException)
        {
            webView.CoreWebView2.PostWebMessageAsJson(JsonSerializer.Serialize(new
            {
                type = "journeyError",
                message = exception.Message
            }));
        }
    }

    private void HandleNavigationCompleted(object? sender, CoreWebView2NavigationCompletedEventArgs eventArgs)
    {
        if (!eventArgs.IsSuccess || pendingJourney is null || !IsJourneyPage(webView.Source?.AbsoluteUri ?? string.Empty))
        {
            return;
        }

        webView.CoreWebView2.PostWebMessageAsJson(JsonSerializer.Serialize(new
        {
            type = "journeyResult",
            journey = pendingJourney
        }));
        pendingJourney = null;
    }

    private bool IsPlanningPage(string source)
    {
        return IsBundledPage(source, "planning.html");
    }

    private bool IsJourneyPage(string source)
    {
        return IsBundledPage(source, "journey.html");
    }

    private bool IsBundledPage(string source, string pageName)
    {
        if (!Uri.TryCreate(source, UriKind.Absolute, out Uri? sourceUri) || !sourceUri.IsFile) return false;
        string sourcePath = Path.GetFullPath(sourceUri.LocalPath);
        string expectedPath = Path.GetFullPath(Path.Combine(webContentRoot, pageName));
        return string.Equals(sourcePath, expectedPath, StringComparison.OrdinalIgnoreCase);
    }
}
