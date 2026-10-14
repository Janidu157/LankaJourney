using System.IO;
using System.Net.Http;
using System.Runtime.InteropServices;
using System.Text.Json;
using System.Text.Json.Nodes;
using System.Windows;
using System.Windows.Interop;
using Microsoft.Web.WebView2.Core;
using MySqlConnector;

namespace WPF_Application;

public partial class MainWindow : Window
{
    private readonly JourneyPlanningService planningService = new();
    private string webContentRoot = string.Empty;
    private string connectionString = string.Empty;
    private JsonObject? pendingJourney;
    private DatabaseSetupWindow? activeDatabaseSetupWindow;
    private HwndSource? windowSource;
    private bool databaseSettingsShortcutRegistered;

    private const int DatabaseSettingsHotkeyId = 0x4C4A;
    private const int WmHotkey = 0x0312;
    private const uint ModifierControl = 0x0002;
    private const uint ModifierNoRepeat = 0x4000;
    private const uint VirtualKeyD = 0x44;

    public MainWindow()
    {
        InitializeComponent();
        SourceInitialized += RegisterDatabaseSettingsShortcut;
        Closed += UnregisterDatabaseSettingsShortcut;
        Loaded += async (_, _) => await InitializeAsync();
    }

    private void RegisterDatabaseSettingsShortcut(object? sender, EventArgs eventArgs)
    {
        IntPtr handle = new WindowInteropHelper(this).Handle;
        windowSource = HwndSource.FromHwnd(handle);
        windowSource?.AddHook(WindowMessageHook);
        databaseSettingsShortcutRegistered =
            RegisterHotKey(handle, DatabaseSettingsHotkeyId, ModifierControl | ModifierNoRepeat, VirtualKeyD);
        if (!databaseSettingsShortcutRegistered)
        {
            MessageBox.Show(this,
                "Ctrl+D could not be registered because another application is using it. Close that application and restart LankaJourney to enable the database settings shortcut.",
                "Shortcut unavailable", MessageBoxButton.OK, MessageBoxImage.Warning);
        }
    }

    private void UnregisterDatabaseSettingsShortcut(object? sender, EventArgs eventArgs)
    {
        windowSource?.RemoveHook(WindowMessageHook);
        if (databaseSettingsShortcutRegistered)
        {
            UnregisterHotKey(new WindowInteropHelper(this).Handle, DatabaseSettingsHotkeyId);
            databaseSettingsShortcutRegistered = false;
        }
    }

    private IntPtr WindowMessageHook(
        IntPtr windowHandle,
        int message,
        IntPtr wParam,
        IntPtr lParam,
        ref bool handled)
    {
        if (message == WmHotkey && wParam.ToInt32() == DatabaseSettingsHotkeyId)
        {
            handled = true;
            ToggleDatabaseSettings();
        }
        return IntPtr.Zero;
    }

    private async Task InitializeAsync()
    {
        try
        {
            string? savedConnectionString = DatabaseConnectionSettings.LoadConnectionString();
            if (!string.IsNullOrWhiteSpace(savedConnectionString))
            {
                connectionString = savedConnectionString;
                databaseToolbar.Visibility = Visibility.Collapsed;
            }
            else
            {
                connectionString = Environment.GetEnvironmentVariable("LANKAJOURNEY_MYSQL_CONNECTION")
                    ?? string.Empty;
            }

            if (string.IsNullOrWhiteSpace(connectionString))
            {
                if (!ShowDatabaseSetup())
                {
                    Close();
                    return;
                }
                databaseToolbar.Visibility = Visibility.Collapsed;
            }

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
        catch (Exception exception) when (exception is IOException or InvalidOperationException or
                                          System.Security.Cryptography.CryptographicException or
                                          System.ComponentModel.Win32Exception or
                                          System.Runtime.InteropServices.COMException)
        {
            MessageBox.Show(this, $"LankaJourney could not start: {exception.Message}", "Startup error",
                MessageBoxButton.OK, MessageBoxImage.Error);
            Close();
        }
    }

    private void DatabaseSettingsButton_Click(object sender, RoutedEventArgs eventArgs)
    {
        if (ShowDatabaseSetup())
        {
            databaseToolbar.Visibility = Visibility.Collapsed;
            MessageBox.Show(this, "The MySQL connection was tested and saved for your Windows account.",
                "Database settings", MessageBoxButton.OK, MessageBoxImage.Information);
        }
    }

    private bool ShowDatabaseSetup()
    {
        var setupWindow = new DatabaseSetupWindow(connectionString) { Owner = this };
        activeDatabaseSetupWindow = setupWindow;
        try
        {
            if (setupWindow.ShowDialog() != true) return false;
            connectionString = setupWindow.ConnectionString;
            return true;
        }
        finally
        {
            activeDatabaseSetupWindow = null;
        }
    }

    private void ToggleDatabaseSettings()
    {
        if (activeDatabaseSetupWindow is { IsVisible: true } setupWindow)
        {
            setupWindow.Close();
            return;
        }

        if (ShowDatabaseSetup())
        {
            databaseToolbar.Visibility = Visibility.Collapsed;
            MessageBox.Show(this, "The MySQL connection was tested and saved for your Windows account.",
                "Database settings", MessageBoxButton.OK, MessageBoxImage.Information);
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
            pendingJourney = await planningService.CreateJourneyAsync(requirements, connectionString);
            databaseToolbar.Visibility = Visibility.Collapsed;
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

    [DllImport("user32.dll", SetLastError = true)]
    [return: MarshalAs(UnmanagedType.Bool)]
    private static extern bool RegisterHotKey(IntPtr windowHandle, int id, uint modifiers, uint virtualKey);

    [DllImport("user32.dll", SetLastError = true)]
    [return: MarshalAs(UnmanagedType.Bool)]
    private static extern bool UnregisterHotKey(IntPtr windowHandle, int id);
}
