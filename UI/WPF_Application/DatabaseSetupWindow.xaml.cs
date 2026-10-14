using System.ComponentModel;
using System.IO;
using System.Windows;
using MySqlConnector;

namespace WPF_Application;

public partial class DatabaseSetupWindow : Window
{
    public string ConnectionString { get; private set; } = string.Empty;

    public DatabaseSetupWindow() : this(null)
    {
    }

    public DatabaseSetupWindow(string? existingConnectionString)
    {
        InitializeComponent();
        if (string.IsNullOrWhiteSpace(existingConnectionString)) return;

        var builder = new MySqlConnectionStringBuilder(existingConnectionString);
        serverBox.Text = builder.Server;
        portBox.Text = builder.Port.ToString();
        databaseBox.Text = builder.Database;
        usernameBox.Text = builder.UserID;
        passwordBox.Password = builder.Password;
    }

    private async void SaveButton_Click(object sender, RoutedEventArgs eventArgs)
    {
        saveButton.IsEnabled = false;
        statusText.Text = "Testing the MySQL connection...";
        try
        {
            if (!uint.TryParse(portBox.Text, out uint port) || port is 0 or > 65535)
            {
                throw new InvalidOperationException("Enter a valid MySQL port between 1 and 65535.");
            }

            var builder = new MySqlConnectionStringBuilder
            {
                Server = serverBox.Text.Trim(),
                Port = port,
                Database = databaseBox.Text.Trim(),
                UserID = usernameBox.Text.Trim(),
                Password = passwordBox.Password,
                ConnectionTimeout = 5
            };
            if (string.IsNullOrWhiteSpace(builder.Server) ||
                string.IsNullOrWhiteSpace(builder.Database) ||
                string.IsNullOrWhiteSpace(builder.UserID))
            {
                throw new InvalidOperationException("Server, database, and username are required.");
            }

            await using (var connection = new MySqlConnection(builder.ConnectionString))
            {
                await connection.OpenAsync();
            }

            DatabaseConnectionSettings.SaveConnectionString(builder.ConnectionString);
            ConnectionString = builder.ConnectionString;
            DialogResult = true;
        }
        catch (Exception exception) when (exception is MySqlException or InvalidOperationException or
                                          IOException or Win32Exception or
                                          System.Security.Cryptography.CryptographicException or
                                          ArgumentException)
        {
            statusText.Text = exception.Message;
            saveButton.IsEnabled = true;
        }
    }
}
