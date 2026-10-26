namespace BanglaSwitcher;

static class Program
{
    [STAThread]   // TSF/COM calls need a single-threaded apartment
    static void Main()
    {
        using var mutex = new Mutex(true, "BanglaKeyboardSwitcher.SingleInstance", out bool first);
        if (!first) return;
        ApplicationConfiguration.Initialize();
        Application.Run(new MainForm());
    }
}
