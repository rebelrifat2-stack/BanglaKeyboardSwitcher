using System.Drawing;

namespace BanglaSwitcher;

/// <summary>Three buttons + one status line. There is deliberately no text box.</summary>
class MainForm : Form
{
    readonly Button _avro, _bijoy, _english;
    readonly Label _status;
    Mode _active;

    static string StateFile =>
        Path.Combine(Environment.GetFolderPath(Environment.SpecialFolder.ApplicationData), "BanglaKeyboardSwitcher", "mode.txt");

    public MainForm()
    {
        Text = "Bangla Keyboard Switcher";
        FormBorderStyle = FormBorderStyle.FixedSingle;
        MaximizeBox = false;
        StartPosition = FormStartPosition.CenterScreen;
        ClientSize = new Size(320, 250);
        TopMost = true;                                   // stays reachable while you work
        Font = new Font("Nirmala UI", 12f);               // ships with Windows, has Bengali glyphs

        _avro    = MakeButton("অভ্র বাংলা", 14);
        _bijoy   = MakeButton("বিজয় / SutonnyMJ", 74);
        _english = MakeButton("Regular English", 134);
        _avro.Click    += (_, _) => Switch(Mode.Avro);
        _avro.Tag = Mode.Avro; _bijoy.Tag = Mode.Bijoy; _english.Tag = Mode.English;
        _bijoy.Click   += (_, _) => Switch(Mode.Bijoy);
        _english.Click += (_, _) => Switch(Mode.English);

        _status = new Label { Left = 14, Top = 202, Width = 292, Height = 36, TextAlign = ContentAlignment.MiddleLeft };
        Controls.AddRange(new Control[] { _avro, _bijoy, _english, _status });

        _active = LoadMode();
        Paint_();
        _status.Text = "Last chosen: " + ModeName(_active);
        Shown += (_, _) => { if (Tsf.ComponentInstalled()) try { Tsf.AddProfilesToUserList(); } catch { } };
    }

    Button MakeButton(string text, int top) => new()
    {
        Text = text, Left = 14, Top = top, Width = 292, Height = 52,
        FlatStyle = FlatStyle.Flat, BackColor = Color.WhiteSmoke
    };

    static string ModeName(Mode m) => m switch { Mode.Avro => "অভ্র বাংলা (Avro)", Mode.Bijoy => "বিজয় (Bijoy)", _ => "Regular English" };

    void Switch(Mode m)
    {
        string? err = Tsf.Activate(m);
        if (err != null)
        {
            _status.Text = "✗ " + err;
            MessageBox.Show(this, err, "Could not switch", MessageBoxButtons.OK, MessageBoxIcon.Warning);
            return;
        }
        _active = m; SaveMode(m); Paint_();
        _status.Text = "✓ Active: " + ModeName(m);
    }

    void Paint_()
    {
        foreach (var b in new[] { _avro, _bijoy, _english })
        {
            bool on = (Mode)b.Tag! == _active;
            b.BackColor = on ? Color.FromArgb(46, 160, 67) : Color.WhiteSmoke;
            b.ForeColor = on ? Color.White : Color.Black;
        }
    }

    static Mode LoadMode()
    {
        try { if (Enum.TryParse(File.ReadAllText(StateFile).Trim(), out Mode m)) return m; } catch { }
        return Mode.English;
    }
    static void SaveMode(Mode m)
    {
        try { Directory.CreateDirectory(Path.GetDirectoryName(StateFile)!); File.WriteAllText(StateFile, m.ToString()); } catch { }
    }
}
