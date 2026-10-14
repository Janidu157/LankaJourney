using System.Windows;

namespace WPF_Application // Match your project's namespace
{
    public partial class MainWindow : Window
    {
        public MainWindow()
        {
            InitializeComponent();
            InitializeAsync();
        }

        private async void InitializeAsync()
        {
            // Initializes the underlying WebView2 browser environment
            await webView.EnsureCoreWebView2Async();
        }
    }
}