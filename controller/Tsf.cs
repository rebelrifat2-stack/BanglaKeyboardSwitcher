using System.Runtime.InteropServices;
using Microsoft.Win32;

namespace BanglaSwitcher;

public enum Mode { Avro, Bijoy, English }

/// <summary>Switches the active Windows Text Services Framework input profile.</summary>
static class Tsf
{
    // Must match tsf/dllmain.cpp
    public static readonly Guid Clsid = new("B7A3C5D1-5E2F-4A9B-9C31-7D4E8F20A6B1");
    public static readonly Guid Avro = new("2F6D1B84-93C7-4E55-A1D2-0C8B7E3F4A19");
    public static readonly Guid Bijoy = new("8E41C7A2-6B35-4D08-B9F3-1A5D2C7E9B60");

    const ushort LangBn = 0x0845, LangEn = 0x0409;
    const uint TF_PROFILETYPE_INPUTPROCESSOR = 1, TF_PROFILETYPE_KEYBOARDLAYOUT = 2;
    const uint TF_IPPMF_ENABLEPROFILE = 0x1, TF_IPPMF_FORSESSION = 0x20000000;

    [ComImport, Guid("71C6E74C-0F28-11D8-A82A-00065B84435C"), InterfaceType(ComInterfaceType.InterfaceIsIUnknown)]
    interface ITfInputProcessorProfileMgr
    {
        // ActivateProfile is the first method after IUnknown in this interface.
        [PreserveSig] int ActivateProfile(uint profileType, ushort langId, [In] ref Guid clsid,
            [In] ref Guid profile, IntPtr hkl, uint flags);
    }

    [ComImport, Guid("1F02B6C5-7842-4EE6-8A0B-9A24183A95CA"), InterfaceType(ComInterfaceType.InterfaceIsIUnknown)]
    interface ITfInputProcessorProfiles
    {
        [PreserveSig] int Register([In] ref Guid clsid);
        [PreserveSig] int Unregister([In] ref Guid clsid);
        [PreserveSig] int AddLanguageProfile([In] ref Guid clsid, ushort langId, [In] ref Guid profile,
            IntPtr description, uint descriptionLength, IntPtr iconFile, uint iconFileLength, uint iconIndex);
        [PreserveSig] int RemoveLanguageProfile([In] ref Guid clsid, ushort langId, [In] ref Guid profile);
        [PreserveSig] int EnumInputProcessorInfo(out IntPtr enumerator);
        [PreserveSig] int GetDefaultLanguageProfile(ushort langId, [In] ref Guid category, out Guid clsid, out Guid profile);
        [PreserveSig] int SetDefaultLanguageProfile(ushort langId, [In] ref Guid clsid, [In] ref Guid profile);
        [PreserveSig] int ActivateLanguageProfile([In] ref Guid clsid, ushort langId, [In] ref Guid profile);
        [PreserveSig] int GetActiveLanguageProfile([In] ref Guid clsid, out ushort langId, out Guid profile);
        [PreserveSig] int GetLanguageProfileDescription([In] ref Guid clsid, ushort langId, [In] ref Guid profile, out IntPtr description);
        [PreserveSig] int GetCurrentLanguage(out ushort langId);
        [PreserveSig] int ChangeCurrentLanguage(ushort langId);
    }

    [DllImport("input.dll", CharSet = CharSet.Unicode, SetLastError = true)]
    static extern bool InstallLayoutOrTip(string profile, uint flags);

    [DllImport("user32.dll", CharSet = CharSet.Unicode, SetLastError = true)]
    static extern IntPtr LoadKeyboardLayout(string klid, uint flags);

    /// <summary>True when the native input service DLL has been registered by the installer.</summary>
    public static bool ComponentInstalled()
    {
        using var key = Registry.ClassesRoot.OpenSubKey($@"CLSID\{Clsid:B}\InprocServer32");
        return key != null;
    }

    /// <summary>Adds both profiles to this user's input list, without admin rights.</summary>
    public static bool AddProfilesToUserList()
    {
        bool avroAdded = InstallLayoutOrTip($"0x0845:{Clsid:B}{Avro:B}".ToUpperInvariant(), 0);
        bool bijoyAdded = InstallLayoutOrTip($"0x0845:{Clsid:B}{Bijoy:B}".ToUpperInvariant(), 0);
        return avroAdded && bijoyAdded;
    }

    /// <summary>Returns null on success, otherwise a human-readable error.</summary>
    public static string? Activate(Mode mode)
    {
        object? comObject = null;
        try
        {
            if (mode != Mode.English && !ComponentInstalled())
                return "The input service is not installed. Please run the installer again (it needs administrator rights once).";

            var classId = new Guid("33C53A50-F456-4884-B049-85FD643ECFED"); // CLSID_TF_InputProcessorProfiles
            var type = Type.GetTypeFromCLSID(classId)!;
            comObject = Activator.CreateInstance(type)!;
            var manager = (ITfInputProcessorProfileMgr)comObject;
            var profiles = (ITfInputProcessorProfiles)comObject;
            Guid clsid, profile;
            int hr;

            if (mode == Mode.English)
            {
                IntPtr hkl = LoadKeyboardLayout("00000409", 0);
                if (hkl == IntPtr.Zero)
                    return "English (US) keyboard is not installed in Windows. Add it in Settings > Time & language > Language.";

                hr = profiles.ChangeCurrentLanguage(LangEn);
                if (hr != 0)
                    return $"Could not switch Windows to English (error 0x{hr:X8}).";

                clsid = Guid.Empty;
                profile = Guid.Empty;
                hr = manager.ActivateProfile(TF_PROFILETYPE_KEYBOARDLAYOUT, LangEn, ref clsid, ref profile, hkl, TF_IPPMF_FORSESSION);
            }
            else
            {
                if (!AddProfilesToUserList())
                    return "Windows could not add Avro/Bijoy to your input methods. Reinstall the app, then try again.";

                hr = profiles.ChangeCurrentLanguage(LangBn);
                if (hr != 0)
                    return $"Could not switch Windows to Bengali (error 0x{hr:X8}).";

                clsid = Clsid;
                profile = mode == Mode.Avro ? Avro : Bijoy;
                hr = manager.ActivateProfile(TF_PROFILETYPE_INPUTPROCESSOR, LangBn, ref clsid, ref profile,
                    IntPtr.Zero, TF_IPPMF_FORSESSION | TF_IPPMF_ENABLEPROFILE);
            }

            return hr == 0 ? null : $"Windows refused to activate this input method (error 0x{hr:X8}).";
        }
        catch (Exception ex)
        {
            return "Could not talk to Windows text services: " + ex.Message;
        }
        finally
        {
            if (comObject != null && Marshal.IsComObject(comObject))
                Marshal.ReleaseComObject(comObject);
        }
    }
}
