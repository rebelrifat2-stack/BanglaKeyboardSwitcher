using System.Runtime.InteropServices;
using Microsoft.Win32;

namespace BanglaSwitcher;

public enum Mode { Avro, Bijoy, English }

/// <summary>Talks to Windows Text Services Framework to activate an input profile.</summary>
static class Tsf
{
    // Must match tsf/dllmain.cpp
    public static readonly Guid Clsid = new("B7A3C5D1-5E2F-4A9B-9C31-7D4E8F20A6B1");
    public static readonly Guid Avro = new("2F6D1B84-93C7-4E55-A1D2-0C8B7E3F4A19");
    public static readonly Guid Bijoy = new("8E41C7A2-6B35-4D08-B9F3-1A5D2C7E9B60");

    const ushort LangBn = 0x0845, LangEn = 0x0409;
    const uint TF_PROFILETYPE_INPUTPROCESSOR = 1, TF_PROFILETYPE_KEYBOARDLAYOUT = 2;
    const uint TF_IPPMF_ENABLEPROFILE = 0x1;
    const uint TF_IPPMF_DONTCARECURRENTINPUTLANGUAGE = 0x4;
    const uint TF_IPPMF_FORSESSION = 0x20000000;

    [ComImport, Guid("71C6E74C-0F28-11D8-A82A-00065B84435C"), InterfaceType(ComInterfaceType.InterfaceIsIUnknown)]
    interface ITfInputProcessorProfileMgr
    {
        [PreserveSig]
        int ActivateProfile(uint profileType, ushort langId, [In] ref Guid clsid, [In] ref Guid profile, IntPtr hkl, uint flags);
    }

    [DllImport("input.dll", CharSet = CharSet.Unicode, SetLastError = true)]
    static extern bool InstallLayoutOrTip(string psz, uint flags);
    [DllImport("user32.dll", CharSet = CharSet.Unicode, SetLastError = true)]
    static extern IntPtr LoadKeyboardLayout(string klid, uint flags);

    public static bool ComponentInstalled()
    {
        using var k = Registry.ClassesRoot.OpenSubKey($@"CLSID\{Clsid:B}\InprocServer32");
        return k != null;
    }

    public static void AddProfilesToUserList()
    {
        foreach (var p in new[] { Avro, Bijoy })
            InstallLayoutOrTip($"0845:{Clsid:B}{p:B}".ToUpperInvariant(), 0);
    }

    public static string? Activate(Mode mode)
    {
        try
        {
            if (mode != Mode.English && !ComponentInstalled())
                return "The input service is not installed. Please run the installer again (it needs administrator rights once).";

            var t = Type.GetTypeFromCLSID(new Guid("33C53A50-F456-4884-B049-85FD643ECFED"))!;
            var mgr = (ITfInputProcessorProfileMgr)Activator.CreateInstance(t)!;
            Guid clsid, profile;
            int hr;

            if (mode == Mode.English)
            {
                IntPtr hkl = LoadKeyboardLayout("00000409", 0);
                if (hkl == IntPtr.Zero)
                    return "English (US) keyboard is not installed in Windows. Add it in Settings > Time & language > Language.";

                clsid = Guid.Empty;
                profile = Guid.Empty;
                hr = mgr.ActivateProfile(TF_PROFILETYPE_KEYBOARDLAYOUT, LangEn, ref clsid, ref profile, hkl, TF_IPPMF_FORSESSION);
            }
            else
            {
                clsid = Clsid;
                profile = mode == Mode.Avro ? Avro : Bijoy;
                uint flags = TF_IPPMF_FORSESSION | TF_IPPMF_ENABLEPROFILE | TF_IPPMF_DONTCARECURRENTINPUTLANGUAGE;
                hr = mgr.ActivateProfile(TF_PROFILETYPE_INPUTPROCESSOR, LangBn, ref clsid, ref profile, IntPtr.Zero, flags);
                if (hr != 0)
                {
                    AddProfilesToUserList();
                    hr = mgr.ActivateProfile(TF_PROFILETYPE_INPUTPROCESSOR, LangBn, ref clsid, ref profile, IntPtr.Zero, flags);
                }
            }

            return hr == 0 ? null : $"Windows refused to activate this input method (error 0x{hr:X8}).";
        }
        catch (Exception ex)
        {
            return "Could not talk to Windows text services: " + ex.Message;
        }
    }
}
