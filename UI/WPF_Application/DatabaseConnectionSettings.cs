using System.ComponentModel;
using System.IO;
using System.Runtime.InteropServices;
using System.Security.Cryptography;
using System.Text;

namespace WPF_Application;

internal static class DatabaseConnectionSettings
{
    private static readonly string SettingsPath = Path.Combine(
        Environment.GetFolderPath(Environment.SpecialFolder.LocalApplicationData),
        "LankaJourney",
        "mysql-connection.dat");

    public static string? LoadConnectionString()
    {
        if (!File.Exists(SettingsPath)) return null;

        byte[] encrypted = File.ReadAllBytes(SettingsPath);
        byte[] plaintext = Unprotect(encrypted);
        return Encoding.UTF8.GetString(plaintext);
    }

    public static void SaveConnectionString(string connectionString)
    {
        string? directory = Path.GetDirectoryName(SettingsPath);
        if (directory is null) throw new InvalidOperationException("The settings folder could not be determined.");
        Directory.CreateDirectory(directory);
        byte[] encrypted = Protect(Encoding.UTF8.GetBytes(connectionString));
        File.WriteAllBytes(SettingsPath, encrypted);
    }

    private static byte[] Protect(byte[] plaintext) => Transform(plaintext, protect: true);

    private static byte[] Unprotect(byte[] ciphertext) => Transform(ciphertext, protect: false);

    private static byte[] Transform(byte[] input, bool protect)
    {
        GCHandle inputHandle = GCHandle.Alloc(input, GCHandleType.Pinned);
        try
        {
            var inputBlob = new DataBlob(input.Length, inputHandle.AddrOfPinnedObject());
            bool succeeded = protect
                ? CryptProtectData(ref inputBlob, "LankaJourney MySQL connection", IntPtr.Zero,
                    IntPtr.Zero, IntPtr.Zero, CryptProtectUiForbidden, out DataBlob outputBlob)
                : CryptUnprotectData(ref inputBlob, IntPtr.Zero, IntPtr.Zero,
                    IntPtr.Zero, IntPtr.Zero, CryptProtectUiForbidden, out outputBlob);

            if (!succeeded) throw new Win32Exception(Marshal.GetLastWin32Error());
            try
            {
                byte[] output = new byte[outputBlob.Length];
                Marshal.Copy(outputBlob.Data, output, 0, output.Length);
                return output;
            }
            finally
            {
                LocalFree(outputBlob.Data);
            }
        }
        finally
        {
            inputHandle.Free();
        }
    }

    private const uint CryptProtectUiForbidden = 0x1;

    [StructLayout(LayoutKind.Sequential)]
    private struct DataBlob(int length, IntPtr data)
    {
        public int Length = length;
        public IntPtr Data = data;
    }

    [DllImport("crypt32.dll", SetLastError = true, CharSet = CharSet.Unicode)]
    [return: MarshalAs(UnmanagedType.Bool)]
    private static extern bool CryptProtectData(
        ref DataBlob input,
        string description,
        IntPtr optionalEntropy,
        IntPtr reserved,
        IntPtr prompt,
        uint flags,
        out DataBlob output);

    [DllImport("crypt32.dll", SetLastError = true)]
    [return: MarshalAs(UnmanagedType.Bool)]
    private static extern bool CryptUnprotectData(
        ref DataBlob input,
        IntPtr description,
        IntPtr optionalEntropy,
        IntPtr reserved,
        IntPtr prompt,
        uint flags,
        out DataBlob output);

    [DllImport("kernel32.dll")]
    private static extern IntPtr LocalFree(IntPtr memory);
}
